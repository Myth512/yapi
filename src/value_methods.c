#include "value_methods.h"
#include "value_none.h"
#include "value_int.h"
#include "value_float.h"
#include "value_type.h"
#include "object_string.h"
#include "object_list.h"
#include "object_tuple.h"
#include "object_dict.h"
#include "object_class.h"
#include "object_instance.h"
#include "vm.h"

#define GET_METHOD(value, name) MethodTable[(value).type].name

static int Undefined_ToStr(Value value, char *buffer, size_t size) {
    writeToBuffer(buffer, size, "undefined");
}

ValueMethods MethodTable[] = {
    [VAL_UNDEFINED] = (ValueMethods){.str=Undefined_ToStr, .repr=Undefined_ToStr},
    [VAL_NONE]   = NONE_METHODS, 
    [VAL_BOOL]   = BOOL_METHODS,
    [VAL_INT]    = INT_METHODS, 
    [VAL_FLOAT]  = FLOAT_METHODS,
    [VAL_TYPE] = TYPE_METHODS,
    [VAL_STRING] = STRING_METHODS,
    [VAL_LIST] = LIST_METHODS,
    [VAL_TUPLE] = TUPLE_METHODS,
    [VAL_DICT] = DICT_METHODS,
    [VAL_NATIVE] = NATIVE_METHODS,
    [VAL_FUNCTION] = FUNCTION_METHODS,
    [VAL_CLOSURE] = CLOSURE_METHODS,
    [VAL_CLASS] = CLASS_METHODS,
    [VAL_NATIVE_CLASS] = NATIVE_CLASS_METHODS,
    [VAL_METHOD] = MEHTOD_METHODS,
    [VAL_NATIVE_METHOD] = NATIVE_METHOD_METHODS,
    [VAL_INSTANCE] = INSTANCE_METHODS
};

char *getValueType(Value value) {
    Value class = valueClass(value);
    if (IS_CLASS(class))
        return AS_CLASS(class)->name->chars;
    if (IS_NATIVE_CLASS(class))
        return AS_NATIVE_CLASS(class)->name->chars;
    return "unknown type";
}

Value unaryMethod(Value a, UnaryMethod method, char *name) {
    if (method != 0) {
        Value res = method(a);
        if (!IS_NOT_IMPLEMENTED(res))
            return res;
    }
    operatorNotImplementedUnary(name, a);
}

Value binaryMethod(Value a, Value b, BinaryMethod left, BinaryMethod right, char *name) {
    if (left != NULL) {
        Value res = left(a, b);
        if (!IS_NOT_IMPLEMENTED(res))
            return res;
    }
    if (right != NULL) {
        Value res = right(b, a);
        if (!IS_NOT_IMPLEMENTED(res))
            return res;
    }
    operatorNotImplemented(name, a, b);
}

Value valueEqual(Value a, Value b) {
    BinaryMethod left = GET_METHOD(a, eq);
    if (left != NULL) {
        Value res = left(a, b);
        if (!IS_NOT_IMPLEMENTED(res))
            return res;
    }
    BinaryMethod right = GET_METHOD(b, eq);
    if (right != NULL) {
        Value res = right(b, a);
        if (!IS_NOT_IMPLEMENTED(res))
            return res;
    }
    return BOOL_VAL(valueId(a) == valueId(b));
}

Value valueNotEqual(Value a, Value b) {
    BinaryMethod left = GET_METHOD(a, ne);
    if (left != NULL) {
        Value res = left(a, b);
        if (!IS_NOT_IMPLEMENTED(res))
            return res;
    }
    BinaryMethod right = GET_METHOD(b, ne);
    if (right != NULL) {
        Value res = right(b, a);
        if (!IS_NOT_IMPLEMENTED(res))
            return res;
    }
    return BOOL_VAL(valueId(a) != valueId(b));
}

Value valueGreater(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, gt), GET_METHOD(b, lt), ">");
}

Value valueGreaterEqual(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, ge), GET_METHOD(b, le), ">=");
}

Value valueLess(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, lt), GET_METHOD(b, gt), "<");
}

Value valueLessEqual(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, le), GET_METHOD(b, ge), "<=");
}

Value valueAdd(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, add), GET_METHOD(b, radd), "+");
}

Value valueSubtract(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, sub), GET_METHOD(b, rsub), "-");
}

Value valueMultiply(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, mul), GET_METHOD(b, rmul), "*");
}

Value valueTrueDivide(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, truediv), GET_METHOD(b, rtruediv), "/");
}

Value valueFloorDivide(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, floordiv), GET_METHOD(b, rfloordiv), "//");
}

Value valueModulo(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, mod), GET_METHOD(b, rmod), "%%");
}

Value valuePower(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, pow), GET_METHOD(b, rpow), "**");
}

Value valuePositive(Value a) {
    return unaryMethod(a, GET_METHOD(a, pos), "+");
}

Value valueNegative(Value a) {
    return unaryMethod(a, GET_METHOD(a, neg), "-");
}

Value valueAnd(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, and), GET_METHOD(b, rand), "&");
}

Value valueXor(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, xor), GET_METHOD(b, rxor), "^");
}

Value valueOr(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, or), GET_METHOD(b, ror), "|");
}

Value valueInvert(Value a) {
    return unaryMethod(a, GET_METHOD(a, invert), "~");
}

Value valueLeftShift(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, lshift), GET_METHOD(b, rlshift), "<<");
}

Value valueRightShift(Value a, Value b) {
    return binaryMethod(a, b, GET_METHOD(a, rshift), GET_METHOD(b, rrshift), ">>");
}

Value valueGetAttr(Value obj, ObjString *name) {
    Value (*method)(Value, ObjString*) = GET_METHOD(obj, getattr);
    return method(obj, name);
}

void valueSetAttr(Value obj, ObjString *name, Value value) {
    Value (*method)(Value, ObjString*, Value) = GET_METHOD(obj, setattr);
    if (method == NULL)
        reportRuntimeError("read only");
    method(obj, name, value);
}

void valueDelAttr(Value obj, ObjString *name) {
    Value (*method)(Value, ObjString*) = GET_METHOD(obj, delattr);
    if (method == NULL)
        reportRuntimeError("read only");
    method(obj, name);
}

Value valueGetItem(Value obj, Value key) {
    Value (*method)(Value, Value) = GET_METHOD(obj, getitem);
    if (method == NULL)
        reportRuntimeError("not subscriptable");
    return method(obj, key);
}

void valueSetItem(Value obj, Value key, Value value) {
    Value (*method)(Value, Value, Value) = GET_METHOD(obj, setitem);
    if (method == NULL)
        reportRuntimeError("not subscriptable");
    method(obj, key, value);
}

void valueDelItem(Value obj, Value key) {
    Value (*method)(Value, Value) = GET_METHOD(obj, delitem);
    if (method == NULL)
        reportRuntimeError("not item deletion");
    method(obj, key);
}

Value valueInit(Value callee, int argc, Value *argv) {
    Value (*method)(Value, int, Value*) = GET_METHOD(callee, init);
    return method(callee, argc, argv);
}

Value valueCall(Value callee, int argc, int kwargc, Value *argv) {
    Value (*method)(Value, int, int, Value*) = GET_METHOD(callee, call);
    if (method == NULL)
        reportRuntimeError("no call :(");
    method(callee, argc, kwargc, argv);
}

Value valueClass(Value value) {
    Value (*method)(Value) = GET_METHOD(value, class);
    if (method == NULL)
        return UNDEFINED_VAL;
    return method(value);
}

uint64_t valueHash(Value value) {
    uint64_t (*method)(Value) = GET_METHOD(value, hash);
    if (method == NULL)
        reportRuntimeError("unhashable type: '%s'", getValueType(value));
    return method(value);
}

uint64_t valueId(Value value) {
    switch (value.type) {
        case VAL_NONE:
        case VAL_BOOL:
        case VAL_INT:
        case VAL_FLOAT:
        case VAL_UNDEFINED:
        case VAL_NOT_IMPLEMENTED:
            return (uint64_t)&value;
        default:
            return (uint64_t)value.as.object;
    }
}

long long valueLen(Value value) {
    long long (*method)(Value) = GET_METHOD(value, len);
    if (method == NULL)
        reportRuntimeError("object of type '%s' has not len()", getValueType(value));
    return method(value);
}

bool valueToBool(Value value) {
    bool (*method)(Value) = GET_METHOD(value, toBool);
    if (method == NULL)
        reportRuntimeError("object of type '%s' can not be converted to bool", getValueType(value));
    return method(value);
}

long long valueToInt(Value value) {
    long long (*method)(Value) = GET_METHOD(value, toInt);
    if (method == NULL)
        reportRuntimeError("object of type '%s' can not be converted to int", getValueType(value));
    return method(value);
}

double valueToFloat(Value value) {
    double (*method)(Value) = GET_METHOD(value, toFloat);
    if (method == NULL)
        reportRuntimeError("object of type '%s' can not be converted to float", getValueType(value));
    return method(value);
}

int valueWrite(Value value, char *buffer, size_t size) {
    int (*str)(Value, char*, size_t) = GET_METHOD(value, str);
    return str(value, buffer, size);
}

int valuePrint(Value value) {
    int (*str)(Value, char*, size_t) = GET_METHOD(value, str);
    if (str == NULL)
        return printf("TODO");
    return str(value, NULL, 0);
}

int valueReprWrite(Value value, char *buffer, size_t size) {
    int (*repr)(Value, char*, size_t) = GET_METHOD(value, repr);
    if (repr == NULL)
        return printf("TODO");
    return repr(value, buffer, size);
}

int valueRepr(Value value) {
    int (*repr)(Value, char*, size_t) = GET_METHOD(value, repr);
    if (repr == NULL)
        return printf("TODO");
    return repr(value, NULL, 0);
}

ObjString *valueToStr(Value value) {
    const size_t size = 256;
    char buffer[size];

    int (*str)(Value, char*, size_t) = GET_METHOD(value, str);
    int length = str(value, buffer, size);

    ObjString *string = copyString(buffer, length);
    return string;
}

ObjString *valueToRepr(Value value) {
    const size_t size = 256;
    char buffer[size];

    int (*repr)(Value, char*, size_t) = GET_METHOD(value, repr);
    int length = repr(value, buffer, size);

    ObjString *string = copyString(buffer, length);
    return string;
}