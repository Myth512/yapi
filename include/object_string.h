#ifndef OBJECT_STRING_H
#define OBJECT_STRING_H

#include "object.h"

#define STRING_VAL(string)      ((Value){.type=VAL_STRING, .as.object=(Obj*)string})

#define IS_STRING(value)        ((value).type == VAL_STRING) 

#define AS_STRING(value)        ((ObjString*)((value).as.object))
#define AS_CHARS(value)         (((ObjString*)((value).as.object))->chars)

#define STRING_METHODS (ValueMethods){ \
    .eq = String_Equal,                \
    .ne = String_NotEqual,             \
    .gt = String_Greater,              \
    .ge = String_GreaterEqual,         \
    .lt = String_Less,                 \
    .le = String_LessEqual,            \
    .add = String_Add,                 \
    .radd = String_Add,                \
    .mul = String_Multiply,            \
    .rmul = String_RightMultiply,      \
    .contains = String_Contains,       \
    .class = String_Class,             \
    .init = String_Init,               \
    .iter = String_Iter,               \
    .getattr = String_GetAttribute,    \
    .getitem = String_GetItem,         \
    .hash = String_Hash,               \
    .len = String_Len,                 \
    .toBool = String_ToBool,           \
    .toInt = String_ToInt,             \
    .toFloat = String_ToFloat,         \
    .str = String_ToStr,               \
    .repr = String_ToRepr,             \
}

struct ObjString {
    Obj obj;
    int length;
    uint64_t hash;
    bool isHashed;
    bool isInterned;
    char chars[];
};

ObjString *allocateString(size_t length);

ObjString *copyString(const char *chars, size_t length);

ObjString *copyEscapedString(const char *chars, size_t length);

Value String_Equal(Value a, Value b);

Value String_NotEqual(Value a, Value b);

Value String_Greater(Value, Value b);

Value String_GreaterEqual(Value a, Value b);

Value String_Less(Value a, Value b);

Value String_LessEqual(Value a, Value b);

Value String_Add(Value a, Value b);

Value String_Multiply(Value a, Value b);

Value String_RightMultiply(Value a, Value b);

Value String_Contains(Value a, Value b);

Value String_Class(Value value);

Value String_Init(Value callee, int argc, Value *argv);

Value String_Iter(Value value);

Value String_GetAttribute(Value value, ObjString *name);

Value String_GetItem(Value value, Value key);

uint64_t String_Hash(Value value);

long long String_Len(Value value);

bool String_ToBool(Value value);

long long String_ToInt(Value value);

double String_ToFloat(Value value);

int String_ToStr(Value value, char *buffer, size_t size);

int String_ToRepr(Value value, char *buffer, size_t size);

#endif