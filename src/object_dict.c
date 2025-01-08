#include "object_dict.h"
#include "object_dict_iterator.h"
#include "value_methods.h"
#include "vm.h"
#include "value_int.h"

ObjDict *allocateDict() {
    ObjDict *dict = (ObjDict*)allocateObject(sizeof(ObjDict), VAL_DICT);
    QuadraticTableInit(&dict->table);
    return dict;
}

Value Dict_Contains(Value a, Value b) {
    Value res = QuadraticTableGet(&AS_DICT(a)->table, b);
    return BOOL_VAL(!IS_UNDEFINED(res));
}

Value Dict_Class(Value value) {
    return TYPE_CLASS(dict);
}

Value Dict_Iter(Value value) {
    return OBJ_VAL(allocateDictIterator(value));
}

Value Dict_GetAttr(Value obj, ObjString *name) {

}

Value Dict_GetItem(Value obj, Value key) {
    Value res = QuadraticTableGet(&AS_DICT(obj)->table, key);
    if (IS_UNDEFINED(res))
        reportRuntimeError("No key");
    return res;
}

Value Dict_SetItem(Value obj, Value key, Value value) {
    QuadraticTableSet(&AS_DICT(obj)->table, key, value);
}

int Dict_ToStr(Value value, char *buffer, size_t size) {
    size_t bytesLeft = size;

    int bytesWritten = writeToBuffer(buffer, bytesLeft, "{");
    bytesLeft -= movePointer(&buffer, bytesWritten);

    size_t capacity = AS_DICT(value)->table.capacity;
    size_t elementsCount = AS_DICT(value)->table.size;
    int c = 0;

    for (int i = 0; i < capacity; i++) {
        Entry *entry = &AS_DICT(value)->table.entries[i];
        if (IS_UNDEFINED(entry->key))
            continue;
        
        valueReprWrite(entry->key, buffer, bytesLeft);
        writeToBuffer(buffer, bytesLeft, ": ");
        valueReprWrite(entry->value, buffer, bytesLeft);
        if (++c < elementsCount)
            writeToBuffer(buffer, bytesLeft, ", ");
    }

    writeToBuffer(buffer, bytesLeft, "}");
}