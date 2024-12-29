#include <math.h>

#include "value.h"
#include "value_int.h"
#include "value_float.h"
#include "vm.h"

Value Float_Equal(Value a, Value b) {
    if (b.type == VAL_INT)
        return BOOL_VAL(AS_FLOAT(a) == AS_INT(b));
    if (b.type == VAL_FLOAT)
        return BOOL_VAL(AS_FLOAT(a) == AS_FLOAT(b));
    return NOT_IMPLEMENTED_VAL;
}

Value Float_NotEqual(Value a, Value b) {
    if (b.type == VAL_INT)
        return BOOL_VAL(AS_FLOAT(a) != AS_INT(b));
    if (b.type == VAL_FLOAT)
        return BOOL_VAL(AS_FLOAT(a) != AS_FLOAT(b));
    return NOT_IMPLEMENTED_VAL;
}

Value Float_Greater(Value a, Value b) {
    if (b.type == VAL_INT)
        return BOOL_VAL(AS_FLOAT(a) > AS_INT(b));
    if (b.type == VAL_FLOAT)
        return BOOL_VAL(AS_FLOAT(a) > AS_FLOAT(b));
    return NOT_IMPLEMENTED_VAL;
}

Value Float_GreaterEqual(Value a, Value b) {
    if (b.type == VAL_INT)
        return BOOL_VAL(AS_FLOAT(a) >= AS_INT(b));
    if (b.type == VAL_FLOAT)
        return BOOL_VAL(AS_FLOAT(a) >= AS_FLOAT(b));
    return NOT_IMPLEMENTED_VAL;
}

Value Float_Less(Value a, Value b) {
    if (b.type == VAL_INT)
        return BOOL_VAL(AS_FLOAT(a) < AS_INT(b));
    if (b.type == VAL_FLOAT)
        return BOOL_VAL(AS_FLOAT(a) < AS_FLOAT(b));
    return NOT_IMPLEMENTED_VAL;
}

Value Float_LessEqual(Value a, Value b) {
    if (b.type == VAL_INT)
        return BOOL_VAL(AS_FLOAT(a) <= AS_INT(b));
    if (b.type == VAL_FLOAT)
        return BOOL_VAL(AS_FLOAT(a) <= AS_FLOAT(b));
    return NOT_IMPLEMENTED_VAL;
}

Value Float_Add(Value a, Value b) {
    if (b.type == VAL_INT)
        return FLOAT_VAL(AS_FLOAT(a) + AS_INT(b));
    if (b.type == VAL_FLOAT)
        return FLOAT_VAL(AS_FLOAT(a) + AS_FLOAT(b));
    return NOT_IMPLEMENTED_VAL;
}

Value Float_Subtract(Value a, Value b) {
    if (b.type == VAL_INT)
        return FLOAT_VAL(AS_FLOAT(a) - AS_INT(b));
    if (b.type == VAL_FLOAT)
        return FLOAT_VAL(AS_FLOAT(a) - AS_FLOAT(b));
    return NOT_IMPLEMENTED_VAL;
}

Value Float_Multiply(Value a, Value b) {
    if (b.type == VAL_INT)
        return FLOAT_VAL(AS_FLOAT(a) * AS_INT(b));
    if (b.type == VAL_FLOAT)
        return FLOAT_VAL(AS_FLOAT(a) * AS_FLOAT(b));
    return NOT_IMPLEMENTED_VAL;
}

Value Float_TrueDivide(Value a, Value b) {
    if (b.type == VAL_INT) {
        if (AS_INT(b) == 0)
            reportRuntimeError("Division by zero");
        return FLOAT_VAL(AS_FLOAT(a) / AS_INT(b));
    }
    if (b.type == VAL_FLOAT) {
        if (AS_FLOAT(b) == 0)
            reportRuntimeError("Division by zero");
        return FLOAT_VAL(AS_FLOAT(a) / AS_FLOAT(b));
    }
    return NOT_IMPLEMENTED_VAL;
}

Value Float_FloorDivide(Value a, Value b) {
    if (b.type == VAL_INT) {
        if (AS_INT(b) == 0)
            reportRuntimeError("Division by zero");
        return FLOAT_VAL((long long)(AS_FLOAT(a) / AS_INT(b)));
    }
    if (b.type == VAL_FLOAT) {
        if (AS_FLOAT(b) == 0)
            reportRuntimeError("Division by zero");
        return FLOAT_VAL((long long)(AS_FLOAT(a) / AS_FLOAT(b)));
    }
    return NOT_IMPLEMENTED_VAL;
}

Value Float_Modulo(Value a, Value b) {
    if (b.type == VAL_INT) {
        if (AS_INT(b) == 0)
            reportRuntimeError("Division by zero");
        return FLOAT_VAL(fmod(AS_FLOAT(a), AS_INT(b)));
    }
    if (b.type == VAL_FLOAT) {
        if (AS_FLOAT(b) == 0)
            reportRuntimeError("Division by zero");
        return FLOAT_VAL(fmod(AS_FLOAT(a), AS_FLOAT(b)));
    }
    return NOT_IMPLEMENTED_VAL;
}

Value Float_Power(Value a, Value b) {
    if (b.type == VAL_INT)
        return FLOAT_VAL(pow(AS_FLOAT(a), AS_INT(b)));
    if (b.type == VAL_FLOAT)
        return FLOAT_VAL(pow(AS_FLOAT(a), AS_FLOAT(b)));
    return NOT_IMPLEMENTED_VAL;
}

Value Float_Positive(Value a) {
    return a;
}

Value Float_Negative(Value a) {
    return FLOAT_VAL(-AS_FLOAT(a));
}

uint64_t Float_Hash(Value value) {
    union {
        double d;
        uint64_t i;
    } u;
    u.d = AS_FLOAT(value);

    uint64_t hash = u.i;
    hash ^= (hash >> 33);
    hash *= 0xff51afd7ed558ccd;
    hash ^= (hash >> 33);
    hash *= 0xc4ceb9fe1a85ec53;
    hash ^= (hash >> 33);

    return hash;
}

bool Float_ToBool(Value value) {
    return AS_FLOAT(value);
}

long long Float_ToInt(Value value) {
    return AS_FLOAT(value);
}

double Float_ToFloat(Value value) {
    return AS_FLOAT(value);
}

int Float_ToStr(Value value, char *buffer, size_t size) {
    writeToBuffer(buffer, size, "%g", AS_FLOAT(value));
}