#include "jsonValue.h"

using namespace haha::json;

namespace haha{

namespace json
{

/* 类型映射转换 */
template<typename S> struct TypeMap;
template<> struct TypeMap<std::string> { using T = JsonString; };
template<> struct TypeMap<int> { using T = JsonInteger; };
template<> struct TypeMap<double> { using T = JsonDouble; };
template<> struct TypeMap<bool> { using T = JsonBoolean; };
template<> struct TypeMap<decltype(nullptr)> { using T = JsonNull; };
template<> struct TypeMap<JsonArray::ValueType> { using T = JsonArray; };
template<> struct TypeMap<JsonObject::ValueType> { using T = JsonObject; };


/* 判断类型是否匹配 */
template<typename T, typename V> 
struct isMatchType{
    bool operator() (){
        return std::is_same<T, typename T::ValueType>::value;
    }
};

} // namespace json


}


