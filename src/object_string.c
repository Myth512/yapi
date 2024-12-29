#include <string.h>
#include <stdlib.h>

#include "object_string.h"
#include "string_methods.h"
#include "value_int.h"
#include "object_class.h"

ObjString *allocateString(size_t length) {
    size_t size = sizeof(ObjString) + length + 1;
    ObjString *string = (ObjString*)allocateObject(size);
    string->isInterned = false;
    string->isHashed = false;
    string->length = length;
    return string;
}

ObjString *copyString(const char *chars, size_t length) {
    ObjString *string = allocateString(length);
    memcpy(string->chars, chars, length);
    string->chars[string->length] = '\0';
    return string;
}

static char convertToEscapeChar(char c) {
    switch (c) {
        case 'a':
            return '\a';
        case 'b':
            return '\b';
        case 'f':
            return '\f';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        case 'v':
            return '\v';
        case '0':
            return '\0';
        case 'e':
            return '\e';
        default:
            return c;
    }
}

static int resolveEscapeSequence(const char *source, int sourceLength, char *destination) {
    const char *sourceChar = source;
    const char *endChar = source + sourceLength;
    char *destinationChar = destination;
    int destinationLength = 0;
    while (sourceChar < endChar) {
        if (*sourceChar != '\\') {
            *destinationChar = *sourceChar;
        } else {
            sourceChar++;
            *destinationChar = convertToEscapeChar(*sourceChar);
        }
        sourceChar++;
        destinationChar++;
        destinationLength++;
    }
    *destinationChar = '\0';
    return destinationLength;
}

ObjString *copyEscapedString(const char *chars, size_t length) {
    ObjString *string = allocateString(length);
    string->length = resolveEscapeSequence(chars, length, string->chars);
    return string;
}

static bool compareStrings(ObjString *a, ObjString *b) {
    if (a->length != b->length)
        return false;
    if (a->isInterned && b->isInterned)
        return a == b;
    return strcmp(a->chars, b->chars) == 0;
}

Value stringEqual(Value a, Value b) {
    if (IS_STRING(b))
        return BOOL_VAL(compareStrings(AS_STRING(a), AS_STRING(b)));
    return NOT_IMPLEMENTED_VAL;
}

Value stringNotEqual(Value a, Value b) {
    if (IS_STRING(b))
        return BOOL_VAL(!compareStrings(AS_STRING(a), AS_STRING(b)));
    return NOT_IMPLEMENTED_VAL;
}

Value stringGreater(Value a, Value b) {
    if (IS_STRING(b))
        return BOOL_VAL(strcmp(AS_CHARS(a), AS_CHARS(b)) > 0);
    return NOT_IMPLEMENTED_VAL;
}

Value stringGreaterEqual(Value a, Value b) {
    if (IS_STRING(b))
        return BOOL_VAL(strcmp(AS_CHARS(a), AS_CHARS(b)) >= 0);
    return NOT_IMPLEMENTED_VAL;
}

Value stringLess(Value a, Value b) {
    if (IS_STRING(b))
        return BOOL_VAL(strcmp(AS_CHARS(a), AS_CHARS(b)) < 0);
    return NOT_IMPLEMENTED_VAL;
}

Value stringLessEqual(Value a, Value b) {
    if (IS_STRING(b))
        return BOOL_VAL(strcmp(AS_CHARS(a), AS_CHARS(b)) <= 0);
    return NOT_IMPLEMENTED_VAL;
}

Value stringAdd(Value a, Value b) {
    if (!IS_STRING(b))
        return NOT_IMPLEMENTED_VAL;
    
    ObjString *s1 = AS_STRING(a);
    ObjString *s2 = AS_STRING(b);

    int length = s1->length + s2->length;

    ObjString *result = allocateString(length);

    memcpy(result->chars, s1->chars, s1->length);
    memcpy(result->chars + s1->length, s2->chars, s2->length);
    result->chars[length] = '\0';

    return STRING_VAL(result);
}

Value stringMultiply(Value a, Value b) {
    if (!IS_INT(b))
        return NOT_IMPLEMENTED_VAL;
    
    ObjString *string = AS_STRING(a);
    long long scalar = AS_INT(b);

    size_t oldLength = string->length;
    size_t newLength = oldLength * scalar;

    ObjString *result = allocateString(newLength);

    for (int i = 0; i < scalar; i++)
        memcpy(result->chars + i * oldLength, string->chars, oldLength);

    return STRING_VAL(result);
}

OptValue stringGetField(Value string, ObjString *name) {
    const struct StringMethod *result = in_string_set(name->chars, name->length);
    if (result) {
        ObjNativeMethod *method = createNativeMethod(string, result->method, result->name);
        return (OptValue){.hasValue=true, .value=OBJ_VAL(method)};
    } 
    return (OptValue){.hasValue=false};
}

Value stringGetAt(ObjString *string, Value index) {
    // if (!isInt(index))
    //     reportRuntimeError("Index must be integer number");
    
    // int i = asInt(index);
    
    // int length = string->length; 
    // if (i >= length || i < -length)
    //     reportRuntimeError("Index is out of range");
    // if (i < 0)
    //     i += length;
    
    // const char chr = string->chars[i];
    // ObjString *result = copyString(&chr, 1);

    // return OBJ_VAL(result);
}

static uint64_t hashString(const char *value) {
    uint64_t hash = 0;
    while (*value) {
        hash = (hash * 31) + (unsigned char)(*value); 
        value++;  
    }
    return hash;
}

uint64_t stringHash(Value value) {
    ObjString *string = AS_STRING(value);
    if (!string->isHashed) {
        string->hash = hashString(string->chars);
        string->isHashed = true;
    }
    return string->hash;
}

int stringLen(Value string) {
    return AS_STRING(string)->length;
}

bool stringToBool(Value string) {
    return stringLen(string);
}

int stringToInt(Value string) {
    return atoi(AS_CHARS(string));
}

double stringToFloat(Value string) {
    return atof(AS_CHARS(string));
}

int stringToStr(Value value, char *buffer, size_t size) {
    writeToBuffer(buffer, size, "%s", AS_CHARS(value));
}