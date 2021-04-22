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

        if(Constants[i] == NULL) continue;

        if(Constants[i]->Tag == TypeString) {
            // Convert to std::string
            std::string NewString(&Code[3]);
            // Save into the vector for easy searching.
            StringConstants.insert(StringConstants.begin() + i, NewString);
        }
            
        int Size = GetConstantsCount(Code);
        Code += Size;

        // Long and Double types increase the constant offset by two
        if(Constants[i]->Tag == TypeLong || Constants[i]->Tag == TypeDouble) {
            Constants[i + 1] = NULL;
            i++;
        }
    }

    // NameAndDesc has to be parsed after Strings but before Methods, so it has its own little loop.
    for(int i = 1; i < ConstantCount; i++) {
        if(Constants[i] == NULL) continue;

        if(Constants[i]->Tag == TypeNamed) {
            char* Temp = (char*)Constants[i];
            uint16_t nameInd = ReadShortFromStream(&Temp[1]);
            uint16_t descInd = ReadShortFromStream(&Temp[3]);
            std::string Name = GetStringConstant(nameInd);
            std::string Desc = GetStringConstant(descInd);

            std::string NameAndDesc = Name;
            NameAndDesc.append(Desc);

            StringConstants.insert(StringConstants.begin() + i, NameAndDesc);
            printf("%d:\tType %s\n", i, NameAndDesc.c_str());
            break;
        }
    }

    for(int i = 1; i < ConstantCount; i++) {
        
        if(Constants[i] == NULL) continue;

        //printf("Constant %d has type %d\n", i, Constants[i]->Tag);
        char* Temp;
        switch(Constants[i]->Tag) {
            case TypeUtf8: 
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
                std::string name = GetStringConstant(val);

                StringConstants.insert(StringConstants.begin() + i, name);

                printf("%d:\tName %s\n", i, name.c_str());
                break;
            }

            case TypeInterfaceMethod:
            case TypeMethod: {
                Temp = (char*)Constants[i];
                uint16_t classInd = ReadShortFromStream(&Temp[1]);
                uint16_t nameAndDescInd = ReadShortFromStream(&Temp[3]);

                std::string ClassName = GetStringConstant(classInd);
                std::string NameAndDesc = GetStringConstant(nameAndDescInd);

                StringConstants.insert(StringConstants.begin() + i, NameAndDesc);

                printf("%d:\tMethod %s belongs to class %s\n", i, NameAndDesc.c_str(), ClassName.c_str());
                break;
            }

            
            case TypeField: {
                Temp = (char*)Constants[i];
                uint16_t classInd = ReadShortFromStream(&Temp[1]);
                uint16_t nameAndDescInd = ReadShortFromStream(&Temp[3]);

                std::string ClassName = GetStringConstant(classInd);
                std::string FieldDesc = GetStringConstant(nameAndDescInd);

                StringConstants.insert(StringConstants.begin() + i, FieldDesc);

                printf("%d:\tField %s belongs to class %s\n", i, FieldDesc.c_str(), ClassName.c_str());
                break;
            }

            default:
                printf("%d:\tValue unknown. Unrecognized type.\n", i);
        }
    }
    
    return true;
}

std::string Class::GetStringConstant(uint32_t index) {
    if(index < 1 || index >= ConstantCount)
        return Unknown;
    
    return StringConstants.at(index);
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

    std::string ClassName = pClass->GetStringConstant(classInd);

    return ClassName.compare(TestName) == COMPARE_MATCH;
}