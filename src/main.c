#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, const char *argv[]) {
  if (argc < 2) {
    // TODO
    printf("Usage: %s \n", argv[0]);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
