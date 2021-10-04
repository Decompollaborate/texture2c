#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum TypeBitWidth {
    TypeBitWidth_8,
    TypeBitWidth_16,
    TypeBitWidth_32,
    TypeBitWidth_64,
    TypeBitWidth_Max,
} TypeBitWidth;

typedef struct GenericBuffer {
    uint8_t* buffer;
    size_t bufferSize;
    size_t bufferLength;
    bool hasData;
    bool isCompressed;
} GenericBuffer;

void GenericBuffer_Init(GenericBuffer* buffer);
void GenericBuffer_Destroy(GenericBuffer* buffer);

void GenericBuffer_WriteRaw(GenericBuffer* buffer, TypeBitWidth bitWidth, FILE* outFile);

void GenericBuffer_Yaz0Compress(GenericBuffer* buffer);
void GenericBuffer_Yaz0Decompress(GenericBuffer* buffer);
