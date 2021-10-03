/**
 * Docblock: Hello reader
 */

/* Includes */
#include <assert.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "generic_buffer.h"
#include "image_backend.h"
#include "macros.h"
#include "png_texture.h"
#include "jpeg_texture.h"

/* Defines */
#define OPTSRT "c:e:i:p:o:u:hlry"

/**
 * Options:
 *   -c, --c-type           C type to use as prefix for output array, defaults to value of -u
 *   -e, --extra-prefix     Add an extra prefix, e.g. an alignment macro
 *   -i, --image-format     input type (jpeg or png) (optional, should try to guess from file extension and ...)
 *   -p, --pixel-format     texture output format, one of rgba32,rgba16,ia16,ia8,ia4,i8,i4,ci8,ci4 (yuv or whatever not
 *                          supported) (default is rgba16)
 *   -o, --output-path      output file path (output to stdout if not specified)
 *   -u, --bit-group-size   bytes per array element, one of 8,16, (without, default is chosen per type, 32/16/8 for
 *                          32/16/(8 or 4))
 *
 * Flags:
 *   -h, --help
 *   -l, --palette          Rip the palette from a palettised PNG (should err if is not palettised) as rgba16; ignores
 *                          -f, print a warning
 *   -r, --raw              output only the raw bytes in specified -u
 *   -y, --yaz0             compress output
 *
 * Positional argument:
 *   input-file             input file path
 */

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

// clang-format off
static struct {
    struct option longOpt;
    char* helpArg;
    char* helpMsg;
} optInfo[] = {
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

#define CLAMP_MIN(x, min) ((x) < (min) ? (min) : (x))

/**
 * Index for start, copy characters up to max 'textWidth'
 * When encounter an \n, break immediately, and start the next line if there is more
 * look backwards to find a space. copy up until the space
 *
 * print
 * Increment index by the number actually copied
 *
 * Add another \n at the end, maybe
 */

/**
 * Reflow text to a specified width, with word wrap.
 */
// char* ReflowText(char* dest, const char* src, size_t width) {
//     size_t index = 0;
//     size_t outdex = 0;
//     size_t column = 0;
//     size_t lookAhead = 0;

//     assert(src != NULL);
//     assert(dest != NULL);
//     assert(width > 0);

//     while (index < strlen(src)) {
//         char c = src[index];
        
//         if (column == width) {
//             dest[outdex] = '\n';
//             column = 0;
//         }

//         switch (c) {
//             case ' ':
//                 /* lookahead */
//                 for (lookAhead = 1; lookAhead < width - column; lookAhead++) {
//                     if (src[index + lookAhead] == ' ') {
//                         dest[outdex] = src[index];
//                         break;
//                     }
//                 }
                

//                 break;

//             case '\n':
//                 column = 0;

//             default:
//                 dest[outdex] = src[index];
//                 break;
//         }

//         index++;
//         outdex++;
//         column++;
//     }

// }

void PrintHanging(const char* string, size_t lineWidth, size_t indent, size_t firstLineIndent) {
    size_t index = CLAMP_MIN(lineWidth - firstLineIndent, 0);
    // char* lineString = malloc((lineWidth + 1) * sizeof(char));
    ssize_t textWidth = lineWidth - indent;

    assert(lineWidth > 0);
    assert(textWidth > 0);

    // TODO: modify string to deal with \n and break at spaces

    // /* Print first line */
    printf("%*s\n", (int)index, string);

    // /* Print rest of string */
    while (index < strlen(string)) {
        printf("%*s%*s", (int)indent, "", (int)textWidth, &string[index]);
        index += textWidth;
    }

    // free(lineString);
}

void PrintHelp() {
    size_t i;
    static char prolog[] = "Help prolog";
    static char epilog[] = "Pithy closing remark";

    printf("%s", prolog);
    printf("\n");

    for (i = 0; i < optCount; i++) {
        if (optInfo[i].longOpt.val == 0) {
            break;
        }

        if (optInfo[i].helpArg == NULL) {
            printf("  -%c, --%-18s  %s\n", optInfo[i].longOpt.val, optInfo[i].longOpt.name, optInfo[i].helpMsg);
        } else {
            int padding = 18 - strlen(optInfo[i].longOpt.name) - strlen(optInfo[i].helpArg) - 1;
            printf("  -%c, --%s=%s%*s  %s\n", optInfo[i].longOpt.val, optInfo[i].longOpt.name, optInfo[i].helpArg,
                   CLAMP_MIN(padding, 0), "", optInfo[i].helpMsg);
        }
    }

    printf("\n");
    printf("%s", epilog);
    printf("\n");
}

int main(int argc, char* argv[]) {
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
                PrintHelp();
                // printf("Help message goes here.\n\n");
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
