#include "generic_buffer.h"

#include <assert.h>
#include <string.h>
#include <inttypes.h>

#include "bit_convert.h"
#include "yaz0/yaz0.h"
#include "macros.h"


void GenericBuffer_Init(GenericBuffer* buffer) {
    buffer->buffer = NULL;
    buffer->bufferSize = 0;
    buffer->bufferLength = 0;
    buffer->hasData = false;
    buffer->isCompressed = false;
}

void GenericBuffer_Destroy(GenericBuffer* buffer) {
    if (buffer->buffer != NULL) {
        free(buffer->buffer);
    }
}


void GenericBuffer_WriteRaw(GenericBuffer* buffer, TypeBitWidth bitWidth, FILE* outFile) {
    assert(buffer->hasData);
    assert(bitWidth >= 0 && bitWidth < TypeBitWidth_Max);

    size_t step = 8;
    switch (bitWidth) {
        case TypeBitWidth_64:
            step = 8;
            break;
        case TypeBitWidth_32:
            step = 4;
            break;

        case TypeBitWidth_16:
            step = 2;
            break;

        case TypeBitWidth_8:
            step = 1;
            break;

        default:
            break;
    }

    for (size_t i = 0; i < buffer->bufferLength; i += step) {
        switch (bitWidth) {
            case TypeBitWidth_64:
                fprintf(outFile, "0x%016" PRIX64 ", ", ToUInt64BE(buffer->buffer, i));
                break;

            case TypeBitWidth_32:
                fprintf(outFile, "0x%08" PRIX32 ", ", ToUInt32BE(buffer->buffer, i));
                break;

            case TypeBitWidth_16:
                fprintf(outFile, "0x%04" PRIX16 ", ", ToUInt16BE(buffer->buffer, i));
                break;

            case TypeBitWidth_8:
                fprintf(outFile, "0x%02" PRIX8 ", ", ToUInt8BE(buffer->buffer, i));
                break;

            default:
                break;
        }

        if ((i / step + 1) % 4 == 0) {
            fprintf(outFile, "\n");
        }
    }
}

void GenericBuffer_Yaz0Compress(GenericBuffer* buffer) {
    assert(buffer->hasData);
    assert(!buffer->isCompressed);

    size_t uncompressedSize = buffer->bufferLength;

    uint8_t* tempBuffer = malloc(uncompressedSize * sizeof(uint8_t) * 2);

    // compress data
    size_t compSize = yaz0_encode(buffer->buffer, tempBuffer, uncompressedSize);

    // make Yaz0 header
    uint8_t header[16] = { 0 };
    header[0] = 'Y';
    header[1] = 'a';
    header[2] = 'z';
    header[3] = '0';
    FromUInt32ToBE(header, 4, uncompressedSize);

    memcpy(buffer->buffer, header, ARRAY_COUNT(header));
    memcpy(buffer->buffer + ARRAY_COUNT(header), tempBuffer, compSize);

    buffer->bufferLength = ARRAY_COUNT(header) + compSize;
    buffer->isCompressed = true;

    free(tempBuffer);
}
