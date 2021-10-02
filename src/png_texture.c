#include "png_texture.h"

#include <assert.h>
#include <stdio.h>

#include "bit_convert.h"

void PngTexture_Init(PngTexture* texture) {
    texture->format = TextureType_Max;
    ImageBackend_Init(&texture->textureData);
    texture->textureBuffer = NULL;
    texture->hasData = false;
}

void PngTexture_Destroy(PngTexture* texture) {
    ImageBackend_Destroy(&texture->textureData);
    if (texture->textureBuffer != NULL) {
        free(texture->textureBuffer);
    }
}

void PngTexture_ReadRgba16(PngTexture* texture) {
    size_t width = texture->textureData.width;
    size_t height = texture->textureData.height;

    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            size_t pos = ((y * width) + x) * 2;
            RGBAPixel pixel = ImageBackend_GetPixel(&texture->textureData, y, x);

            uint8_t r = pixel.r / 8;
            uint8_t g = pixel.g / 8;
            uint8_t b = pixel.b / 8;

            uint8_t alphaBit = pixel.a != 0;

            uint16_t data = (r << 11) + (g << 6) + (b << 1) + alphaBit;

            texture->textureBuffer[pos + 0] = (data & 0xFF00) >> 8;
            texture->textureBuffer[pos + 1] = (data & 0x00FF);
        }
    }
}

void PngTexture_ReadRgba32(PngTexture* texture) {
    size_t width = texture->textureData.width;
    size_t height = texture->textureData.height;

    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            size_t pos = ((y * width) + x) * 4;
            RGBAPixel pixel = ImageBackend_GetPixel(&texture->textureData, y, x);

            texture->textureBuffer[pos + 0] = pixel.r;
            texture->textureBuffer[pos + 1] = pixel.g;
            texture->textureBuffer[pos + 2] = pixel.b;
            texture->textureBuffer[pos + 3] = pixel.a;
        }
    }
}

void PngTexture_ReadI4(PngTexture* texture) {
    size_t width = texture->textureData.width;
    size_t height = texture->textureData.height;

    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x += 2) {
            size_t pos = ((y * width) + x) / 2;
            uint8_t r1 = ImageBackend_GetPixel(&texture->textureData, y, x).r;
            uint8_t r2 = ImageBackend_GetPixel(&texture->textureData, y, x + 1).r;

            texture->textureBuffer[pos] = (uint8_t)(((r1 / 16) << 4) + (r2 / 16));
        }
    }
}

void PngTexture_ReadI8(PngTexture* texture) {
    size_t width = texture->textureData.width;
    size_t height = texture->textureData.height;

    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            size_t pos = (y * width) + x;
            RGBAPixel pixel = ImageBackend_GetPixel(&texture->textureData, y, x);
            texture->textureBuffer[pos] = pixel.r;
        }
    }
}

void PngTexture_ReadIA4(PngTexture* texture) {
    size_t width = texture->textureData.width;
    size_t height = texture->textureData.height;

    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x += 2) {
            size_t pos = ((y * width) + x) / 2;
            uint8_t data = 0;

            for (uint16_t i = 0; i < 2; i++) {
                RGBAPixel pixel = ImageBackend_GetPixel(&texture->textureData, y, x + i);
                uint8_t cR = pixel.r;
                uint8_t alphaBit = pixel.a != 0;

                if (i == 0)
                    data |= (((cR / 32) << 1) + alphaBit) << 4;
                else
                    data |= ((cR / 32) << 1) + alphaBit;
            }

            texture->textureBuffer[pos] = data;
        }
    }
}

void PngTexture_ReadIA8(PngTexture* texture) {
    size_t width = texture->textureData.width;
    size_t height = texture->textureData.height;

    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            size_t pos = ((y * width) + x) * 1;
            RGBAPixel pixel = ImageBackend_GetPixel(&texture->textureData, y, x);

            uint8_t r = pixel.r;
            uint8_t a = pixel.a;

            texture->textureBuffer[pos] = ((r / 16) << 4) + (a / 16);
        }
    }
}

void PngTexture_ReadIA16(PngTexture* texture) {
    size_t width = texture->textureData.width;
    size_t height = texture->textureData.height;

    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            size_t pos = ((y * width) + x) * 2;
            RGBAPixel pixel = ImageBackend_GetPixel(&texture->textureData, y, x);

            uint8_t cR = pixel.r;
            uint8_t aR = pixel.a;

            texture->textureBuffer[pos + 0] = cR;
            texture->textureBuffer[pos + 1] = aR;
        }
    }
}

void PngTexture_ReadCI4(PngTexture* texture) {
    size_t width = texture->textureData.width;
    size_t height = texture->textureData.height;

    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x += 2) {
            size_t pos = ((y * width) + x) / 2;

            uint8_t cR1 = ImageBackend_GetIndexedPixel(&texture->textureData, y, x);
            uint8_t cR2 = ImageBackend_GetIndexedPixel(&texture->textureData, y, x + 1);

            texture->textureBuffer[pos] = (cR1 << 4) | (cR2);
        }
    }
}

void PngTexture_ReadCI8(PngTexture* texture) {
    size_t width = texture->textureData.width;
    size_t height = texture->textureData.height;

    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            size_t pos = ((y * width) + x);
            uint8_t cR = ImageBackend_GetIndexedPixel(&texture->textureData, y, x);

            texture->textureBuffer[pos] = cR;
        }
    }
}

typedef void (*ReadPngCallbacks)(PngTexture* texture);

ReadPngCallbacks readPngArray[TextureType_Max] = {
    [TextureType_rgba16] = PngTexture_ReadRgba16, [TextureType_rgba32] = PngTexture_ReadRgba32,
    [TextureType_i4] = PngTexture_ReadI4,         [TextureType_i8] = PngTexture_ReadI8,
    [TextureType_ia4] = PngTexture_ReadIA4,       [TextureType_ia8] = PngTexture_ReadIA8,
    [TextureType_ia16] = PngTexture_ReadIA16,     [TextureType_ci4] = PngTexture_ReadCI4,
    [TextureType_ci8] = PngTexture_ReadCI8,
};

void PngTexture_ReadPng(PngTexture* texture, const char* pngPath, TextureType texType) {
    assert(!texture->hasData);
    texture->format = texType;

    assert(texType >= 0 && texType < TextureType_Max);

    ImageBackend_ReadPng(&texture->textureData, pngPath);

    size_t bufferSize = texture->textureData.width * texture->textureData.height;
    bufferSize *= ImageBackend_GetBytesPerPixel(&texture->textureData);
    texture->textureBuffer = malloc(bufferSize);

    readPngArray[texType](texture);

    texture->hasData = true;
}

void PngTexture_Write64(PngTexture* texture, FILE* outFile) {
    size_t width = texture->textureData.width;
    size_t height = texture->textureData.height;

    size_t bufferSize = width * height * ImageBackend_GetBytesPerPixel(&texture->textureData);

    for (size_t i = 0; i < bufferSize; i += 8) {
        fprintf(outFile, "0x%016lX, ", ToUInt64BE(texture->textureBuffer, i));
        if (i % 32 == 24) {
            fprintf(outFile, "\n");
        }
    }
}

void PngTexture_WriteRaw(PngTexture* texture, const char* outPath) {
    assert(texture->hasData);

    FILE* outFile = fopen(outPath, "w");

    PngTexture_Write64(texture, outFile);

    fclose(outFile);
}
