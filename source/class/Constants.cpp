/**************
 * GEMWIRE    *
 *    PURPURI *
 **************/

#include "../../headers/Class.hpp"
#include <string>

bool Class::ParseConstants(const char *&Code) {
    Constants = new ConstantPoolEntry* [ConstantCount + 1];

    puts("Reading constants..");

    if(Constants == NULL) return false;

    for(int i = 1; i < ConstantCount; i++) {
        Constants[i] = (ConstantPoolEntry*) Code;

        int Size = GetConstantsCount(Code);
        Code += Size;

        // Long and Double types increase the constant offset by two
        if(Constants[i]->Tag == TypeLong || Constants[i]->Tag == TypeDouble) {
            Constants[i + 1] = NULL;
            i++;
        }
    }

    for(int i = 1; i < ConstantCount; i++) {
        
        if(Constants == NULL) return false;
        if(Constants[i] == NULL) continue;

        //printf("Constant %d has type %d\n", i, Constants[i]->Tag);
        char* Temp;
        switch(Constants[i]->Tag) {
            case TypeUtf8: 
                GetStringConstant(i, Temp);
                //printf("\tValue %s\n", Temp);
                break;
            
            
            case TypeInteger: {
                Temp = (char*)Constants[i];
                uint32_t val = ReadIntFromStream(&Temp[1]);
            
                printf("%d:\tValue %d\n", i, val);
                break;
            }

            case TypeLong: {
                Temp = (char*)Constants[i];
                size_t val = ReadLongFromStream(&Temp[1]);
                printf("%d:\tValue %zd\n", i, val);
                break;
            }

            case TypeFloat: {
                Temp = (char*)Constants[i];
                Variable val = *(Variable*) &Temp[1];
            
                printf("%d:\tValue %.6f\n", i, val.floatVal);
                break;
            }

            case TypeDouble: {
                Temp = (char*)Constants[i];
                Variable val = *(Variable*) &Temp[1];
                printf("%d:\tValue %.6f\n", i, val.doubleVal);
                break;
            }

            case TypeClass: {
                Temp = (char*)Constants[i];
                uint16_t val = ReadShortFromStream(&Temp[1]);
                GetStringConstant(val, Temp);
                printf("%d:\tName %s\n", i, Temp);
                break;
            }

            case TypeInterfaceMethod:
            case TypeMethod: {
                Temp = (char*)Constants[i];
                uint16_t classInd = ReadShortFromStream(&Temp[1]);
                uint16_t nameAndDescInd = ReadShortFromStream(&Temp[3]);
                Temp = (char*)Constants[classInd];
                uint16_t val = ReadShortFromStream(&Temp[1]);
                char* ClassName;
                if(!GetStringConstant(val, ClassName)) exit(3);

                Temp = (char*)Constants[nameAndDescInd];
                uint16_t nameInd = ReadShortFromStream(&Temp[1]);
                uint16_t descInd = ReadShortFromStream(&Temp[3]);
                char* MethodName = NULL, *MethodDesc = NULL;
                if(!GetStringConstant(nameInd, MethodName)) exit(3);
                if(!GetStringConstant(descInd, MethodDesc)) exit(3);

                if(MethodName == NULL || MethodDesc == NULL)
                    __builtin_unreachable();
                printf("%d:\tMethod %s%s belongs to class %s\n", i, MethodName, MethodDesc, ClassName);
                break;
            }

            case TypeNamed: {
                Temp = (char*)Constants[i];
                uint16_t nameInd = ReadShortFromStream(&Temp[1]);
                uint16_t descInd = ReadShortFromStream(&Temp[3]);
                char* MethodName, *MethodDesc;
                if(!GetStringConstant(nameInd, MethodName)) exit(3);
                if(!GetStringConstant(descInd, MethodDesc)) exit(3);
                printf("%d:\tType %s%s\n", i, MethodName, MethodDesc);
                break;
            }
            
            case TypeField: {
                Temp = (char*)Constants[i];
                uint16_t classInd = ReadShortFromStream(&Temp[1]);
                uint16_t nameAndDescInd = ReadShortFromStream(&Temp[3]);
                Temp = (char*)Constants[classInd];
                uint16_t val = ReadShortFromStream(&Temp[1]);
                char* ClassName;
                if(!GetStringConstant(val, ClassName)) exit(3);

                Temp = (char*)Constants[nameAndDescInd];
                uint16_t nameInd = ReadShortFromStream(&Temp[1]);
                uint16_t descInd = ReadShortFromStream(&Temp[3]);
                char* FieldName = NULL, *FieldDesc = NULL;
                if(!GetStringConstant(nameInd, FieldName)) exit(3);
                if(!GetStringConstant(descInd, FieldDesc)) exit(3);

                if(FieldName == NULL || FieldDesc == NULL)
                    __builtin_unreachable();
                printf("%d:\tField %s(%s) belongs to class %s\n", i, FieldName, FieldDesc, ClassName);
                break;
            }

            case TypeString: 
                printf("%d:\tValue unknown. Potential forward reference\n", i);
                break;
            default:
                printf("%d:\tValue unknown. Unrecognized type.\n", i);
        }
    }
    
    return true;
}

bool Class::GetStringConstant(uint32_t index, char *&String) {
    if(index < 1 || index >= ConstantCount)
        return false;

    if(Constants[index]->Tag != TypeUtf8)
        return false;
    
    char* Entry = (char*)Constants[index];

    uint16_t Length = ReadShortFromStream(&Entry[1]);

    char* Buffer = new char[Length + 1];
    Buffer[Length] = 0;

    memcpy(Buffer, &(Entry[3]), Length);

    //printf("Retrieving constant %s from ID %d\n", Buffer, index);
    
    String = Buffer;
    return true;
}

uint32_t Class::GetConstantsCount(const char* Code) {
    ConstantPoolEntry* Pool = (ConstantPoolEntry*) Code;

    switch(Pool->Tag) {
        case TypeUtf8: return 3 + ReadShortFromStream(Code + 1);
        case TypeInteger: return 5;
        case TypeFloat: return 5;
        case TypeLong: return 9;
        case TypeDouble: return 9;
        case TypeClass: return 3;
        case TypeString: return 3;
        case TypeField: return 5;
        case TypeMethod: return 5;
        case TypeInterfaceMethod: return 5;
        case TypeNamed: return 5;
        default: break;
    }

    return 0;
}

bool Engine::MethodClassMatches(uint16_t MethodInd, Class* pClass, const char* TestName) {
    char* Data = (char*) pClass->Constants[MethodInd];
    uint16_t classInd = ReadShortFromStream(&Data[1]);

    Data = (char*) pClass->Constants[classInd];
    uint16_t val = ReadShortFromStream(&Data[1]);

    char* ClassName;
    if(!pClass->GetStringConstant(val, ClassName)) exit(3);

    return !strcmp(ClassName, TestName);
}