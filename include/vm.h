#ifndef VM_H
#define VM_H

#include "common.h"
#include "code.h"
#include "table.h"
#include "object.h"
#include "object_function.h"
#include "object_class.h"
#include "object_module.h"

#define FRAMES_SIZE 64
#define STACK_SIZE (FRAMES_SIZE * UINT8_MAX)

#define TYPE_CLASS(name)    (OBJ_VAL(vm.types.name))

#define PARSE_ARGS(...) \
    parseArgs(argc, kwargc, sizeof(keywords) / sizeof(char*), keywords, ##__VA_ARGS__)

typedef struct {
    ObjClosure *closure;
    uint8_t *ip;
    Value *slots;
    uint8_t *exceptAddr[256];
    int exceptPointer;
    bool isMethod;
} CallFrame;

typedef struct {
    ObjString *init;
    ObjString *add;
    ObjString *ladd;
    ObjString *radd;
    ObjString *inc;
    ObjString *dec;
    ObjString *sub;
    ObjString *lsub;
    ObjString *rsub;
    ObjString *neg;
    ObjString *mul;
    ObjString *lmul;
    ObjString *rmul;
    ObjString *div;
    ObjString *ldiv;
    ObjString *rdiv;
    ObjString *mod;
    ObjString *lmod;
    ObjString *rmod;
    ObjString *pow;
    ObjString *lpow;
    ObjString *rpow;
    ObjString *eq;
    ObjString *ne;
    ObjString *lt;
    ObjString *le;
    ObjString *gt;
    ObjString *ge;
    ObjString *call;
    ObjString *getat;
    ObjString *setat;
    ObjString *len;
    ObjString *bool_;
    ObjString *str;
    ObjString *int_;
    ObjString *float_;
} MagicStrings;

typedef struct {
    ObjNativeClass *object;
    ObjNativeClass *none;
    ObjNativeClass *bool_;
    ObjNativeClass *int_;
    ObjNativeClass *float_;
    ObjNativeClass *type;
    ObjNativeClass *str;
    ObjNativeClass *strIterator;
    ObjNativeClass *list;
    ObjNativeClass *listIterator;
    ObjNativeClass *tuple;
    ObjNativeClass *tupleIterator;
    ObjNativeClass *dict;
    ObjNativeClass *dictIterator;
    ObjNativeClass *exception;
    ObjNativeClass *zeroDivisionError;
    ObjNativeClass *stopIteration;
    ObjNativeClass *nameError;
    ObjNativeClass *typeError;
    ObjNativeClass *valueError;
    ObjNativeClass *indexError;
    ObjNativeClass *keyError;
    ObjNativeClass *attributeError;
    ObjNativeClass *runtimeError;
    ObjNativeClass *assertionError;
    ObjNativeClass *notImplementedError;
    ObjNativeClass *super;
    ObjNativeClass *range;
    ObjNativeClass *rangeIterator;
    ObjNativeClass *notImplementedType;
    ObjNativeClass *slice;
} BaseTypes;

typedef struct {
    CallFrame frames[FRAMES_SIZE];
    int frameSize;
    CodeVec *code;
    uint8_t *ip;
    Value stack[STACK_SIZE];
    Value *top;
    Table builtin;
    MagicStrings magicStrings;
    BaseTypes types;
    ObjUpvalue *openUpvalues;
    Obj *objects;
    size_t bytesAllocated;
    size_t nextGC;
    bool allowStackPrinting;
    const char *path;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

extern VM vm;
extern CallFrame *frame;

void push(Value value);

Value pop();

void insert(int distance, Value value);

void parseArgs(int argc, int kwargc, int arity, char *keywords[], ...);

void raise();

void raiseIfException();

void reportRuntimeError(const char *format, ...);

void reportArityError(int min, int max, int got);

void operatorNotImplemented(char *operator, Value a, Value b);

void operatorNotImplementedUnary(char *operator, Value a);

void functionNotImplemented(char *function, Value a);

void call(ObjClosure *closure, int argc, int kwargc, bool isMethod);

Value callNovaValue(Value callee, int argc);

OptValue callNovaMethod(Value obj, ObjString *methodName);

OptValue callNovaMethod1arg(Value obj, ObjString *methodName, Value arg);

OptValue callNovaMethod2args(Value obj, ObjString *methodName, Value arg1, Value arg2);

void initVM(const char *scriptPath);

void freeVM();

InterpretResult interpret(const char *source, const char *path);

#endif