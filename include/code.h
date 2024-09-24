#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include <stdlib.h>

#include "memory.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_ADD,
    OP_SUBTRUCT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MOD,
    OP_NOT,
    OP_NEGATE,
    OP_RETURN,
} OpCode;

typedef struct {
    int size;
    int capacity;
    uint8_t *code;
    int *lines;
    ValueVec constants;
} CodeVec;

void initCodeVec(CodeVec *code);

void freeCodeVec(CodeVec *codeVec);

void pushInstruction(CodeVec *codeVec, uint8_t byte, int line);

int pushConstant(CodeVec *vec, Value value);

#endif