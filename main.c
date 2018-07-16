#ifdef __MINGW32__
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct {
    size_t size;
    char *content;
} raw_file;

typedef struct {
    uint16_t numEntries;
} disk_header;

typedef struct {
    char filename[12];
    uint8_t _unknown;
    uint16_t size;
    uint32_t offset;
} __attribute__((gcc_struct, packed)) file_record;

raw_file readRawFile(const char *filename)
{
    raw_file result = { 0 };
    FILE *fp = fopen(filename, "rb");
    if(!fp) { fprintf(stderr, "Could not open file %s...\n", filename); exit(EXIT_FAILURE); }
    fseek(fp, 0, SEEK_END);
    result.size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    result.content = malloc(result.size);
    fread(result.content, result.size, 1, fp);
    fclose(fp);
    return result;
}

#define consume(file, type) (type*)consume_(file, sizeof(type))
void* consume_(raw_file *file, uint32_t size)
{
    void *result = file->content;
    file->content = file->content + size;
    return result;
}

void writeFile(const char *filename, const char *at, uint32_t size)
{
    FILE *fp = fopen(filename, "wb");
    if(!fp) { fprintf(stderr, "Could not open file %s...\n", filename); exit(EXIT_FAILURE); }
    printf("Writing file %s...\n", filename);
    fwrite(at, size, 1, fp);
    fclose(fp);
}

void extractDisk(raw_file file, const char *out_dir)
{
    char *start = file.content;
    disk_header *header = consume(&file, disk_header);
    for(int i = 0; i < header->numEntries; ++i) {
        file_record *record = consume(&file, file_record);
        char fnbuffer[1024];
        sprintf(fnbuffer, "%s/%s", out_dir, record->filename);
        writeFile(fnbuffer, start + record->offset, record->size);
    }
}

int main(int argc, char **argv)
{
    if(argc != 3) {
        fprintf(stderr, "usage: beyond disk_file out_dir"
                "\n  out_dir: needs to be manually created");
        return EXIT_FAILURE;
    }
    raw_file file = readRawFile(argv[1]);
    extractDisk(file, argv[2]);

    return EXIT_SUCCESS;
}
