#include "jpeg_texture.h"

#include <assert.h>
#include <string.h>

#include "macros.h"


#define JPEG_MARKER 0xFFD8FFE0
#define MARKER_DQT 0xFFDB


void JpegTexture_ReadJpeg(GenericBuffer *dst, FILE* inFile, bool fillBuffer) {
    assert(dst != NULL);
    assert(inFile != NULL);
    // TODO?
    assert(!dst->hasData);

    dst->bufferSize = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t);
    dst->buffer = calloc(dst->bufferSize, sizeof(uint8_t));
    dst->bufferLength = fread(dst->buffer, 1, dst->bufferSize, inFile);

    assert(dst->bufferLength < dst->bufferSize);

    if (fillBuffer) {
        dst->bufferLength = dst->bufferSize;
    }

    dst->hasData = true;
}

void JpegTexture_CheckValidJpeg(GenericBuffer *dst) {
    // TODO
    (void)dst;
}
