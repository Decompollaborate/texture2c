#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

// #include <stddef.h>
// #include <stdint.h>
// #include <stdio.h>
// #include <png.h>

#include "lib/image_backend.h"

#define PIXELS_PER_BYTE 2

void WriteI4PNG(FILE* outputFile, uint8_t* pixelArray, uint32_t width, uint32_t height) {
    ImageBackend image;

    ImageBackend_Init(&image);

    image.width = width;
    image.height = height;
    image.pixelMatrix = (uint8_t**)malloc(sizeof(uint8_t*) * image.height);
    {
        size_t y;
        for (y = 0; y < image.height; y++) {
            image.pixelMatrix[y] = (uint8_t*)calloc(image.width / PIXELS_PER_BYTE, sizeof(uint8_t));
        }
    }
    image.paletteLen = 0;
    image.colorType = PNG_COLOR_TYPE_GRAY;
    image.bitDepth = 4;
    image.hasImageData = true;
    image.isColorIndexed = false;
    puts("Set up image struct");
    {
        size_t row;
        for (row = 0; row < height; row++) {
            size_t column;
            // printf("start row %zd\n", row);
            for (column = 0; column < width; column++) {
                // printf("%02X ",
                //        pixelArray[width * row + column] << ((8 / PIXELS_PER_BYTE) - (column & (1))));
                image.pixelMatrix[row][column / PIXELS_PER_BYTE] |=
                    pixelArray[width * row + column] << ((8 / PIXELS_PER_BYTE) - 4 * (column & (1)));
            }
            puts("");
            // printf("row %zd done\n", row);
        }
        for (row = 0; row < height; row++) {
            size_t column;
            // printf("start row %zd\n", row);
            for (column = 0; column < width / 2; column++) {
                printf("%02X", image.pixelMatrix[row][column]);
            }
            puts("");
            // printf("row %zd done\n", row);
        }
    }
    // puts("Set up pixels");

    ImageBackend_WritePng(&image, outputFile);
    ImageBackend_Destroy(&image);
}

/* ByteBuffer */

typedef struct {
    size_t length;
    uint8_t* bytes;
} ByteBuffer;

ByteBuffer* Buffer_Create(ByteBuffer* buffer, size_t length) {
    buffer->bytes = malloc(length);
    buffer->length = length;
    return buffer;
}

ByteBuffer* Buffer_Resize(ByteBuffer* buffer, size_t newLength) {
    buffer->bytes = realloc(buffer->bytes, newLength);
    buffer->length = newLength;
    return buffer;
}

ByteBuffer* Buffer_Append(ByteBuffer* buffer, uint8_t* appendage, size_t length) {
    size_t currentEnd = buffer->length;

    Buffer_Resize(buffer, buffer->length + length);
    memcpy(&buffer->bytes[currentEnd], appendage, length);
    return buffer;
}

void Buffer_Free(ByteBuffer* buffer) {
    free(buffer->bytes);
}

/* Conversion */

void I4_UnpackByte(uint8_t* dest, uint8_t byte) {
    int i;
    for (i = 0; i < 2; i++) {
        dest[i] = (byte >> (4 - 4 * i)) & 0xF;
    }
}

int main(int argc, char** argv) {
    FILE* inFile;
    // FILE* outFile = fopen("crash_screen_font.png", "wb");
    size_t inFileSize;
    ByteBuffer inData;
    ByteBuffer bitArray;
    size_t pixelsPerByte = 2;

    // inFile = fopen("crash_screen_font.bin", "rb");
    inFile = fopen("kanji.bin", "rb");

    fseek(inFile, 0, SEEK_END);
    inFileSize = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);

    Buffer_Create(&inData, inFileSize);
    fread(inData.bytes, inFileSize, 1, inFile);

    fclose(inFile);

    Buffer_Create(&bitArray, inFileSize * pixelsPerByte);

    {
        size_t i;
        for (i = 0; i < inData.length; i++) {
            I4_UnpackByte(&bitArray.bytes[i * pixelsPerByte], inData.bytes[i]);
        }
    }
    Buffer_Free(&inData);

    {
        size_t charWidth = 16;
        size_t charHeight = 16;
        size_t imageWidth = 16;
        size_t imageHeight = inFileSize / imageWidth * pixelsPerByte;

        size_t charsPerRow = imageWidth / charWidth;
        size_t charRows = imageHeight / charHeight;

        // size_t charNum = 35;

        FILE* outFile;

        /* override for testing */
        imageHeight = 256;

        outFile = fopen("test_i4.png", "wb");

        WriteI4PNG(outFile, bitArray.bytes, imageWidth, imageHeight);

        fclose(outFile);

        // {
        //     size_t i;
        //     for (i = 0; i < imageHeight * imageWidth; i++) {
        //         printf("%X", bitArray.bytes[i]);
        //         if ((i + 1) % imageWidth == 0) {
        //             printf("\n");
        //         }
        //     }
            
        // }
        // for (charNum = 0; charNum < charRows * charsPerRow; charNum++) {

        //     size_t charPos[2] = { charNum % charsPerRow, charNum / charsPerRow };
        //     size_t charStart = charPos[0] * charWidth + charPos[1] * imageWidth * charHeight;

        //     printf("%X, %X, %X\n", charPos[0], charPos[1], charStart);

        //     {
        //         size_t i;
        //         size_t index = charStart;
        //         printf("+------------+\n");
        //         printf("|");
        //         for (i = 0; i < charWidth * charHeight; i++, index++) {

        //             printf("%s", bitArray.bytes[index] ? "\x1b[47m  \x1b[0m" : "  ");
        //             if (((i + 1) % charWidth) == 0) {
        //                 printf("|\n");
        //                 if (((i + 1) % (charWidth * charHeight)) != 0) {
        //                     printf("|");
        //                 }
        //                 index += imageWidth - charWidth;
        //             }
        //         }
        //         printf("+------------+\n");
        //     }
        // }
    }

    // {
    //     size_t i;
    //     printf("  ");
    //     for (i = 0; i < 32; i++) {
    //         printf("%2d", i);
    //     }
    //     for (i = 0; i < bitArray.length; i++) {
    //         if ((i & 0x1F) == 0) {
    //             putchar('\n');
    //             printf("%2d ", i / 0x20);
    //         }
    //         printf("%s", bitArray.bytes[i] ? "\x1b[47m  \x1b[0m" : "  ");
    //     }
    //     putchar('\n');
    // }
    Buffer_Free(&bitArray);

    return 0;
}
