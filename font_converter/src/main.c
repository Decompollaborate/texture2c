/**
 * Plan:
 * raw data -> bit arrays (4)
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
// #include <inttypes.h>

#include "bmp.h"

/* Defines, may replace some with args later */

/* total rows of characters */
#define ROW_MAX 32
/* total characters in one row */
#define CHARACTERS_PER_ROW 2
/* number of palettes */
#define PALETTE_MAX 4
/* height in pixels of one character */
#define CHARACTER_HEIGHT 8
/* width in pixels of one character */
#define CHARACTER_WIDTH 8
/* pixels per array element */
#define PIXEL_DENSITY 2
/* width of character in array elements */
#define CHARACTER_ROW_SIZE CHARACTER_WIDTH / PIXEL_DENSITY

void ConstructPalettes(int* palettes) {
    int i;
    // paletteArray = malloc(PALETTE_MAX * sizeof(int));
    // Either free this or malloc it elsewhere, or put it on the stack in main if it's going to always be size 4

    for (i = 0; i < PALETTE_MAX; i++) {
        palettes[i] = 0x11 << i;
    }
}

/* A lot of this assumes two pixels / element */
uint8_t* SetBitArray(uint8_t* array, uint8_t* fileBuffer, size_t count, int paletteMask) {
    size_t i;

    if (count & 1) {
        assert(!"SetBitArray: error: total size must be even");
    }

    for (i = 0; i < count; i++) {
        size_t subArrayIndex;
        for (subArrayIndex = 0; subArrayIndex < PIXEL_DENSITY; subArrayIndex++) {
            array[PIXEL_DENSITY * i + subArrayIndex] =
                ((fileBuffer[i] & paletteMask & (0xF0 >> (4 * subArrayIndex))) != 0);
        }
    }

    return array;
}

int main(int argc, char** argv) {
    int palettes[PALETTE_MAX];
    FILE* inputFile;
    size_t fileLength;
    uint8_t* inFileBuffer;
    size_t inFileBufferCount;
    FILE* outputFile;
    size_t i;
    uint8_t* bitArrays[PALETTE_MAX];

    /* Process command line */

    if (argc < 2) {
        fprintf(stderr, "No input file specified\n");
        return EXIT_FAILURE;
    }

    inputFile = fopen(argv[1], "rb");

    if (inputFile == NULL) {
        return EXIT_FAILURE;
    }

    outputFile = stdout;

    if (outputFile == NULL) {
        return EXIT_FAILURE;
    }

#if 0
    /* Construct required variables */

    ConstructPalettes(palettes);

    fseek(inputFile, 0, SEEK_END);
    fileLength = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);
    // printf("File length: 0x%zX\n", fileLength);

    inFileBuffer = malloc(fileLength * sizeof(uint8_t));
    inFileBufferCount = fileLength;

    /* Read inputFile. Now finished with input, so close it. */
    fread(inFileBuffer, sizeof(uint8_t), fileLength / sizeof(uint8_t), inputFile);
    fclose(inputFile);

    /* Construct the bit arrays */
    for (i = 0; i < PALETTE_MAX; i++) {
        bitArrays[i] = malloc(PIXEL_DENSITY * inFileBufferCount * sizeof(uint8_t));
        SetBitArray(bitArrays[i], inFileBuffer, inFileBufferCount, palettes[i]);
    }

    /* Finished with inFileBuffer now */
    free(inFileBuffer);

    // for (i = 0; i < PALETTE_MAX; i++) {
    //     size_t pixel;
    //     for (pixel = 0; pixel < PIXEL_DENSITY * inFileBufferCount; pixel++) {
    //         if (!(pixel & 0xF)) {
    //             printf("\n");
    //         }
    //         fprintf(outputFile, "%s", (bitArrays[i][pixel] ? "X" : " "));
    //     }
    //     printf("\n");
    // }

#undef ROW_MAX
#define ROW_MAX 32
    WriteBMPFile(outputFile, bitArrays[0], CHARACTERS_PER_ROW * CHARACTER_WIDTH, ROW_MAX * CHARACTER_HEIGHT);
    // printf("\n");
#endif

    {
        BMPFile file;
        uint8_t* pixelsArray;

        ReadBMPHeaders(&file, inputFile);
        if (CheckBMP(&file)) {
            printf("Malloc array size: %d\n", file.imageHeader.biWidth * ABS(file.imageHeader.biHeight));
            pixelsArray = malloc(file.imageHeader.biWidth * ABS(file.imageHeader.biHeight) * sizeof(uint8_t));
            ExtractBMPPixels(pixelsArray, &file, inputFile);
            free(pixelsArray);
        }
    }

#if 0

    for (i = 0; i < fileLength / sizeof(uint8_t); i++) {
        fprintf(outputFile, "%s", (inFileBuffer[i] & 0x10 ? "\x1b[47m \x1b[0m" : " "));
        fprintf(outputFile, "%s", (inFileBuffer[i] & 0x1 ? "\x1b[47m \x1b[0m" : " "));
        if (!((i + 1) & 0x7)) {
            printf("\n");
        }
    }

    {
        size_t row;
        for (row = 0; row < CHARACTER_HEIGHT * ROW_MAX; row++) {
            size_t charColumn;
            for (charColumn = 0; charColumn < CHARACTERS_PER_ROW; charColumn++) {
                size_t palette;
                for (palette = 0; palette < PALETTE_MAX; palette++) {
                    size_t column;
                    for (column = 0; column < CHARACTER_ROW_SIZE; column++) {
                        size_t currentElement =
                            CHARACTERS_PER_ROW * CHARACTER_ROW_SIZE * row + CHARACTER_ROW_SIZE * charColumn + column;
                        int pixel1 = inFileBuffer[currentElement] >> (PALETTE_MAX + palette) & 1;
                        int pixel2 = inFileBuffer[currentElement] >> palette & 1;
                        fprintf(outputFile, "%d,", pixel1);
                        fprintf(outputFile, "%d,", pixel2);
                    }
                }
            }
            printf("\n");
        }
    }

    free(inFileBuffer);
    fclose(inputFile);
#endif

    if (outputFile != stdout) {
        fclose(outputFile);
    }

    free(palettes);
    for (i = 0; i < PALETTE_MAX; i++) {
        free(bitArrays[i]);
    }

    return EXIT_SUCCESS;
}
