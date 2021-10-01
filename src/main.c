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

/* Defines */
#define OPTSRT ""


/**
 * Options:
 *   -c, --c-type           C type to use as prefix for output array, defaults to value of -u
 *   -e, --extra-prefix     Add an extra prefix, e.g. an alignment macro
 *   -i, --image-format     input type (jpeg or png) (optional, should try to guess from file extension and )
 *   -p, --pixel-format     texture output format, one of rgba32,rgba16,ia16,ia8,ia4,i8,i4,ci8,ci4 (yuv or whatever not supported) (default is rgba16)
 *   -o, --output-path      output file path (output to stdout if not specified)
 *   -u,                    bytes per array element, one of 8,16, (without, default is chosen per type, 32/16/8 for 32/16/(8 or 4))
 * 
 * Flags:
 *   -h, --help          
 *   -l, --palette          Rip the palette from a palettised PNG (should err if is not palettised) as rgba16; ignores -f, print a warning
 *   -r, --raw              output only the raw bytes in specified -u
 * 
 * Positional argument:
 *   input-file             input file path
 */


int main(int argc, const char *argv[]) {
    if (argc < 3) {
        // TODO
        printf("Usage: %s pngfile out \n", argv[0]);
        return EXIT_FAILURE;
    }

    PngTexture pngTex;
    PngTexture_Init(&pngTex);

    PngTexture_ReadPng(&pngTex, argv[1], TextureType_rgba16);
    PngTexture_WriteRaw(&pngTex,argv[2]);


    PngTexture_Destroy(&pngTex);

    return EXIT_SUCCESS;
}
