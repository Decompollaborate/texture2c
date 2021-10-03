/**
 * Docblock: Hello reader
 */

/* Includes */
#include "main.h"

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "generic_buffer.h"
#include "help.h"
#include "image_backend.h"
#include "macros.h"
#include "png_texture.h"
#include "jpeg_texture.h"

/* Defines */
#define OPTSRT "c:e:i:p:o:u:hlry"

void ReadPng(GenericBuffer* buf, const char* inPath, TextureType texType) {
    ImageBackend textureData;
    ImageBackend_Init(&textureData);

    FILE* inFile = fopen(inPath, "rb");
    ImageBackend_ReadPng(&textureData, inFile);
    fclose(inFile);

    PngTexture_CopyPng(buf, &textureData, texType);

    ImageBackend_Destroy(&textureData);
}

void ReadJpeg(GenericBuffer* buf, const char* inPath) {
    FILE* inFile = fopen(inPath, "rb");

    JpegTexture_ReadJpeg(buf, inFile, true);
    JpegTexture_CheckValidJpeg(buf);

    fclose(inFile);
}

//#define COMPRESS_TEST


/* Options */

// clang-format off
static OptInfo optInfo[] = {
    { { "c-type", required_argument, NULL, 'c' }, "TYPE", "Use TYPE as the type of the C array generated. Default is u8/u16/u32/u64, same as -u" },
    { { "extra-prefix", required_argument, NULL, 'e' }, "PREFIX", "Add PREFIX before the C declaration, e.g. for attributes" },
    { { "image-format", required_argument, NULL, 'i' }, "IMG", "Read image as of format IMG. One of 'jpg', 'png'" },
    { { "pixel-format", required_argument, NULL, 'p' }, "FMT", "Output pixel data in format FMT. One of rgba32, rgba16, ia16, ia8, ia4, i8, i4. Default: rgba16" },
    { { "output-path", required_argument, NULL, 'o' }, "FILE", "Write output to FILE, or stdout if not specified" },
    { { "bit-group-size", required_argument, NULL, 'u' }, "SIZE", "Number of bits in each array element of output. One of 8,16,32,64. Default is inferred from -p, 32 for rgba32, 16 for rgba16/ia16, 8 for the rest" },
    { { "var-name", required_argument, NULL, 'v' }, "NAME", "Use NAME as variable name of C array. Default: inputFileTex" },

    { { "help", no_argument, NULL, 'h' }, NULL, "Display this message and exit" },
    { { "blob", no_argument, NULL, 'b' }, NULL, "Treat file as a binary blob rather than a texture" },
    { { "palette", no_argument, NULL, 'l' }, NULL, "Extract the palette a PNG uses instead of the image" },
    { { "raw", no_argument, NULL, 'r' }, NULL, "Output a raw array, i.e. only the contents of the {}. Ignores -c, -e, -v" },
    { { "yaz0", no_argument, NULL, 'y' }, NULL, "Compress the output using yaz0" },
    { { NULL, 0, NULL, 0 }, NULL, NULL },
};
// clang-format on

static size_t optCount = ARRAY_COUNT(optInfo);
static struct option longOptions[ARRAY_COUNT(optInfo)];

void ConstructLongOpts() {
    size_t i;

    for (i = 0; i < optCount; i++) {
        longOptions[i] = optInfo[i].longOpt;
    }
}

int main(int argc, char** argv) {
    int opt;

    ConstructLongOpts();

    while (true) {
        int optionIndex = 0;

        if ((opt = getopt_long(argc, argv, OPTSRT, longOptions, &optionIndex)) == -1) {
            break;
        }

        switch (opt) {
            /* Options */
            case 'c':
                printf("Using type: %s\n", optarg);
                break;

            case 'e':
                printf("Adding extra prefix: %s\n", optarg);
                break;

            case 'i':
                printf("Input format: %s\n", optarg);
                break;

            case 'p':
                printf("Output pixel format: %s\n", optarg);
                break;

            case 'o':
                printf("Output path: %s\n", optarg);
                break;

            case 'u':
                printf("Bit grouping size: %s\n", optarg);
                break;

            case 'v':
                printf("Output variable name: %s\n", optarg);
                break;

            /* Flags */
            case 'h':
                PrintHelp(optCount, optInfo);
                return EXIT_FAILURE;

            case 'l':
                printf("Extracting palette from PNG...\n");
                break;

            case 'r':
                printf("Raw mode selected.\n");
                break;

            case 'y':
                printf("Compressing output...\n");
                break;

            default:
                printf("?? getopt returned character code 0%o ??\n", opt);
                break;
        }
    }

    if (argv[optind] == NULL) {
        printf("Mandatory argument 'input-file' missing\n");
        return EXIT_FAILURE;
    }

    if (argc < 2) {
        // TODO
        printf("Usage: %s [] inputFile \n"
               "Try %s --help for more information.\n",
               argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
    GenericBuffer genericBuf;
    GenericBuffer_Init(&genericBuf);

    // ReadPng(&genericBuf, argv[1], TextureType_rgba16);
    ReadJpeg(&genericBuf, argv[1]);

#ifdef COMPRESS_TEST
    GenericBuffer_Yaz0Compress(&genericBuf);
#endif
    FILE* outFile = fopen(argv[2], "w");
    GenericBuffer_WriteRaw(&genericBuf, TypeBitWidth_64, outFile);
    fclose(outFile);

    GenericBuffer_Destroy(&genericBuf);

    return EXIT_SUCCESS;
}
