CC = gcc

SRC = \
    ratsnake.c \
    IR_compiler.c \
    vm/vm.c \
    vm/stackframe.c \
    hashmap/hashmap.c \
    CorePrimitives/core_primitives.c \

TARGET = ratsnake

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -o $@ $(SRC) -lm

clean:
	rm -f $(TARGET)
