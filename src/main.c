/**
 * Docblock: Hello reader
 */

/* Includes */
#include "main.h"

#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "generic_buffer.h"
#include "help.h"
#include "image_backend.h"
#include "macros.h"
#include "png_texture.h"
#include "jpeg_texture.h"

/* Defines */
#define OPTSRT "c:e:i:p:o:u:hlry"

typedef enum {
    FORMAT_PNG,
    FORMAT_JPEG,
} ImageFileFormat;

typedef struct {
    FILE* inputFile;
    FILE* outputFile;
    ImageFileFormat inputFileFormat;
    TextureType pixelFormat;
    TypeBitWidth bitGroupSize; // Is this the right type to use here?
    char* extraPrefix;
    char* CType;
    char* varName;

    bool blobMode;
    bool extractPalette;
    bool rawOut;
    bool compress;
} State;

State gState = {
    NULL, NULL, -1, TextureType_rgba16, -1, NULL, NULL, NULL, false, false, false, false,
};

void GuessInputFileFormat() {
}

typedef struct {
    const char* string;
    int eNum;
} PoorMansDict;

PoorMansDict textureTypeDict[] = {
    { "rgba16", TextureType_rgba16 }, { "rgba32", TextureType_rgba32 }, { "i4", TextureType_i4 },
    { "i8", TextureType_i8 },         { "ia4", TextureType_ia4 },       { "ia8", TextureType_ia8 },
    { "ia16", TextureType_ia16 },     { "ci4", TextureType_ci4 },       { "ci8", TextureType_ci8 }, { NULL, -1 }
};

PoorMansDict bitGroupSizeDict[] = {
    { "8", TypeBitWidth_8 },
    { "16", TypeBitWidth_16 },
    { "32", TypeBitWidth_32 },
    { "64", TypeBitWidth_64 },
    { NULL, -1 },
};

int BadDictLookup(const char* string, const PoorMansDict* dict) {
    size_t i;

    for (i = 0; dict[i].string != NULL; i++) {
        if (strcmp(dict[i].string, string) == 0) {
            return dict[i].eNum;
        }
    }
    fprintf(stderr, "String '%s' not found in dictionary", string);
    return -1;
}

char* BadDictReverseLookup(char* dest, int eNum, const PoorMansDict* dict) {
    size_t i;

    for (i = 0; dict[i].eNum != -1; i++) {
        if (dict[i].eNum == eNum) {
            // if (ARRAY_COUNT(dest) > strlen(dict[i].string)) {
                return strcpy(dest, dict[i].string);
            // } else {
            //     printf("error: value found, but destination string is too short to copy into");
            //     return NULL;
            // }
        }
    }
    fprintf(stderr, "error: numeric value '%d' not found in dictionary", eNum);
    return NULL;
}

void ReadPng(GenericBuffer* buf, FILE* inFile, TextureType texType) {
    ImageBackend textureData;
    ImageBackend_Init(&textureData);

    ImageBackend_ReadPng(&textureData, inFile);

    PngTexture_CopyPng(buf, &textureData, texType);

    ImageBackend_Destroy(&textureData);
}

void ReadJpeg(GenericBuffer* buf, FILE* inFile) {
    JpegTexture_ReadJpeg(buf, inFile, true);
    JpegTexture_CheckValidJpeg(buf);
}

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

    if (argc < 2) {
        // TODO
        printf("Usage: %s [options] inputFile \n"
               "Try %s --help for more information.\n",
               argv[0], argv[0]);
        return EXIT_FAILURE;
    }

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
                gState.CType = optarg;
                break;

            case 'e':
                printf("Adding extra prefix: %s\n", optarg);
                gState.extraPrefix = optarg;
                break;

            case 'i':
                printf("Input format: %s\n", optarg);
                if ((strcmp(optarg, "png") == 0) || (strcmp(optarg, "PNG") == 0)) {
                    gState.inputFileFormat = FORMAT_PNG;
                } else if ((strcmp(optarg, "jpg") == 0) || (strcmp(optarg, "JPG") == 0) || (strcmp(optarg, "jpeg") == 0) ||
                           (strcmp(optarg, "JPEG") == 0)) {
                    gState.inputFileFormat = FORMAT_JPEG;
                }
                break;

            case 'p':
                printf("Output pixel format: %s\n", optarg);
                gState.pixelFormat = (TextureType)BadDictLookup(optarg, textureTypeDict);
                break;

            case 'o':
                printf("Output path: %s\n", optarg);
                gState.outputFile = fopen(optarg, "w");
                break;

            case 'u':
                printf("Bit grouping size: %s\n", optarg);
                gState.bitGroupSize = (TypeBitWidth)BadDictLookup(optarg, bitGroupSizeDict);
                break;

            case 'v':
                printf("Output variable name: %s\n", optarg);
                gState.varName = optarg;
                break;

            /* Flags */
            case 'b':
                gState.blobMode = true;
                break;

            case 'h':
                PrintHelp(optCount, optInfo);
                return EXIT_FAILURE;

            case 'l':
                printf("Extracting palette from PNG...\n");
                gState.extractPalette = true;
                break;

            case 'r':
                printf("Raw mode selected.\n");
                gState.rawOut = true;
                break;

            case 'y':
                printf("Compressing output...\n");
                gState.compress = true;
                break;

            default:
                printf("?? getopt returned character code 0%o ??\n", opt);
                break;
        }
    }

    /* Check and set input file */
    if (argv[optind] == NULL) {
        printf("Mandatory argument 'input-file' missing\n");
        return EXIT_FAILURE;
    } else {
        printf("Using input file: %s\n", argv[optind]);
        gState.inputFile = fopen(argv[optind], "rb"); // What if it doesn't exist?
    }

    /**
     * Set default output file.
     * Have to do this since stdout is not constant.
     */
    if (gState.outputFile == NULL) {
        gState.outputFile = stdout;
    }

    /* Option interaction verification */
    /**
     * Check for:
     *  C options passed in raw mode
     *  bitGroupSize disagreeing with C type
     */

    if (gState.rawOut) {
        if (gState.varName != NULL) {
            printf("note: raw mode will not use var-name\n");
        }
        if (gState.CType != NULL) {
            printf("note: raw mode will not use c-type\n");
        }
        if (gState.extraPrefix != NULL) {
            printf("note: raw mode will not use extra-prefix\n");
        }
    }

    /* Natural types by default */
    if (gState.bitGroupSize == (TypeBitWidth)-1) {
        switch (gState.pixelFormat) {
            case TextureType_rgba32:
                gState.bitGroupSize = TypeBitWidth_32;
                break;

            case TextureType_rgba16:
            case TextureType_ia16:
                gState.bitGroupSize = TypeBitWidth_16;
                break;

            case TextureType_i8:
            case TextureType_ia8:
            case TextureType_ci8:
            case TextureType_i4:
            case TextureType_ia4:
            case TextureType_ci4:
                gState.bitGroupSize = TypeBitWidth_8;
                break;

            default:
                printf("error: unknown texture type specified\n");
                return EXIT_FAILURE;
        }
    }

    if (gState.CType != NULL) {
        int size = 0;
        if ((strcmp(gState.CType, "u64") == 0) && (gState.bitGroupSize != TypeBitWidth_64)) {
            size = 64;
        } else if ((strcmp(gState.CType, "u32") == 0) && (gState.bitGroupSize != TypeBitWidth_32)) {
            size = 32;
        } else if ((strcmp(gState.CType, "u16") == 0) && (gState.bitGroupSize != TypeBitWidth_16)) {
            size = 16;
        } else if ((strcmp(gState.CType, "u8") == 0) && (gState.bitGroupSize != TypeBitWidth_8)) {
            size = 8;
        }

        if (size != 0) {
            printf("warning: c-type '%s' does not match bit-group-size %d\n", gState.CType, 1 << ( gState.bitGroupSize + 3 ));
        }
    } else {
        /* Set default C type */
        switch (gState.bitGroupSize) {
            case TypeBitWidth_64:
                gState.CType = "u64";
                break;

            case TypeBitWidth_32:
                gState.CType = "u32";
                break;

            case TypeBitWidth_16:
                gState.CType = "u16";
                break;

            case TypeBitWidth_8:
                gState.CType = "u8";
                break;

            default:
                printf("error: unknown bit-group-size specified\n");
                return EXIT_FAILURE;
        }
    }

    assert(gState.inputFile != NULL);

    GenericBuffer genericBuf;
    GenericBuffer_Init(&genericBuf);

    switch (gState.inputFileFormat) {
        case FORMAT_PNG:
            ReadPng(&genericBuf, gState.inputFile, gState.pixelFormat);
            break;

        case FORMAT_JPEG:
            ReadJpeg(&genericBuf, gState.inputFile);
            break;

        default:
            assert(!"Input format not implemented?");
            break;
    }

    if (gState.compress) {
        GenericBuffer_Yaz0Compress(&genericBuf);
    }

    assert(gState.outputFile != NULL);
    //if (gState.rawOut) {
        GenericBuffer_WriteRaw(&genericBuf, gState.bitGroupSize, gState.outputFile);
    //} else {
        // TODO
    //}

    GenericBuffer_Destroy(&genericBuf);

    if (gState.inputFile != stdin) {
        fclose(gState.inputFile);
    }
    if (gState.outputFile != stdout) {
        fclose(gState.outputFile);
    }

    return EXIT_SUCCESS;
}
