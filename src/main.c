/**
 * Docblock: Hello reader
 */

/* Includes */
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "png_texture.h"
#include "generic_buffer.h"
#include "image_backend.h"

/* Defines */
#define OPTSRT ""

/**
 * Options:
 *   -c, --c-type           C type to use as prefix for output array, defaults to value of -u
 *   -e, --extra-prefix     Add an extra prefix, e.g. an alignment macro
 *   -i, --image-format     input type (jpeg or png) (optional, should try to guess from file extension and ...)
 *   -p, --pixel-format     texture output format, one of rgba32,rgba16,ia16,ia8,ia4,i8,i4,ci8,ci4 (yuv or whatever not
 *                          supported) (default is rgba16)
 *   -o, --output-path      output file path (output to stdout if not specified)
 *   -u,                    bytes per array element, one of 8,16, (without, default is chosen per type, 32/16/8 for
 *                          32/16/(8 or 4))
 *
 * Flags:
 *   -h, --help
 *   -l, --palette          Rip the palette from a palettised PNG (should err if is not palettised) as rgba16; ignores
 *                          -f, print a warning
 *   -r, --raw              output only the raw bytes in specified -u
 *
 * Positional argument:
 *   input-file             input file path
 */


void ReadPng(GenericBuffer* buf, const char* inPath, TextureType texType) {
    ImageBackend textureData;
    ImageBackend_Init(&textureData);

    FILE *inFile = fopen(inPath, "rb");
    ImageBackend_ReadPng(&textureData, inFile);
    fclose(inFile);

    PngTexture_CopyPng(buf, &textureData, texType);

    ImageBackend_Destroy(&textureData);
}

#define COMPRESS_TEST

int main(int argc, const char* argv[]) {
    if (argc < 3) {
        // TODO
        printf("Usage: %s pngfile out \n", argv[0]);
        return EXIT_FAILURE;
    }

    GenericBuffer genericBuf;
    GenericBuffer_Init(&genericBuf);

    ReadPng(&genericBuf, argv[1], TextureType_rgba16);

#ifdef COMPRESS_TEST
    GenericBuffer_Yaz0Compress(&genericBuf);
#endif
    FILE *outFile = fopen(argv[2], "w");
    GenericBuffer_WriteRaw(&genericBuf, TypeBitWidth_64, outFile);
    fclose(outFile);

    GenericBuffer_Destroy(&genericBuf);

    return EXIT_SUCCESS;
}
