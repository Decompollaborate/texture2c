#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "png_texture.h"

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        // TODO
        printf("Usage: %s pngfile \n", argv[0]);
        return EXIT_FAILURE;
    }

    PngTexture pngTex;
    PngTexture_Init(&pngTex);

    PngTexture_ReadPng(&pngTex, argv[1], TextureType_rgba16);


    PngTexture_Destroy(&pngTex);

    return EXIT_SUCCESS;
}
