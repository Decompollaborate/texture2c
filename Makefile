DEBUG       ?= 0
LLD         ?= 1
ASAN        ?= 0

ELF         := texture2c.elf

CC          := clang
IINC        := -I src
WARNINGS    := -Wall -Wextra -Wshadow -Werror=implicit-function-declaration
CFLAGS      := 
LDFLAGS     := -lpng

ifeq ($(DEBUG),0)
  OPTFLAGS  := -O2
  CFLAGS    += -Werror
else
  OPTFLAGS  := -O0 -g3
endif

ifneq ($(ASAN),0)
  CFLAGS    += -fsanitize=address
endif

ifneq ($(LLD),0)
  LDFLAGS   += -fuse-ld=lld
endif

SRC_DIRS    := $(shell find src -type d)
C_FILES     := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
H_FILES     := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))
O_FILES     := $(foreach f,$(C_FILES:.c=.o),build/$f)

all: $(ELF)

clean:
	$(RM) -r $(O_FILES) $(ELF)

.PHONY: all clean

# create build directories
$(shell mkdir -p $(foreach dir,$(SRC_DIRS),build/$(dir)))

$(ELF): $(O_FILES)
	$(CC) $(IINC) $(WARNINGS) $(CFLAGS) $(OPTFLAGS) $(LDFLAGS) -o $@ $^

build/src/%.o: src/%.c $(H_FILES)
	$(CC) -c $(IINC) $(WARNINGS) $(CFLAGS) $(OPTFLAGS) -o $@ $<
