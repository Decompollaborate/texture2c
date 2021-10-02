#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "image_backend.h"

typedef enum TextureType {
    TextureType_rgba16,
    TextureType_rgba32,
    TextureType_i4,
    TextureType_i8,
    TextureType_ia4,
    TextureType_ia8,
    TextureType_ia16,
    TextureType_ci4,
    TextureType_ci8,
    TextureType_Max,
} TextureType;

typedef struct PngTexture {
    TextureType format;
    ImageBackend textureData;
    uint8_t* textureBuffer;
    size_t bufferLength;
    bool hasData;
    bool isCompressed;
} PngTexture;

void PngTexture_Init(PngTexture* texture);
void PngTexture_Destroy(PngTexture* texture);

void PngTexture_ReadPng(PngTexture* texture, const char* pngPath, TextureType texType);
void PngTexture_WriteRaw(PngTexture* texture, const char* outPath);

void PngTexture_Yaz0Compress(PngTexture* texture);
