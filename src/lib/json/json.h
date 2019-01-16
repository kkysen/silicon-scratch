//
// Created by Khyber on 1/16/2019.
//

#ifndef SiliconScratch_json_H
#define SiliconScratch_json_H

#include "nlohmann/json.hpp"

namespace json {
    
    template <template <typename, typename, typename...> class ObjectType,
            template <typename, typename...> class ArrayType,
            class StringType,
            class BooleanType,
            class NumberIntegerType,
            class NumberUnsignedType,
            class NumberFloatType,
            template <typename> class AllocatorType,
            template <typename, typename = void> class JSONSerializer>
    using GenericJson = nlohmann::basic_json<
            ObjectType,
            ArrayType,
            StringType,
            BooleanType,
            NumberIntegerType,
            NumberUnsignedType,
            NumberFloatType,
            AllocatorType,
            JSONSerializer
    >;
    
    using Json = nlohmann::json;
    
    template <typename GenericJson>
    using JsonPtr = nlohmann::json_pointer<GenericJson>;
    
    template <typename GenericJson>
    using JsonSax = nlohmann::json_sax<GenericJson>;
    
    template <typename GenericJson, typename Value>
    using AdlSerializer = nlohmann::adl_serializer<GenericJson, Value>;
    
    namespace detail = nlohmann::detail;
    
}

#endif // SiliconScratch_json_H
