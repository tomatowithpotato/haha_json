#include "jsonValue.h"

using namespace haha::json;

namespace haha
{

namespace json
{

/* ---------------------------------------------about copy--------------------------------------------- */

JsonValueBase::ptr JsonValueBase::copyFrom(JsonValueBase::ptr src){
    auto type = src->getType();
    switch (type)
    {
    case JsonType::String:
        {
            auto p = pointer_cast<JsonString>(src);
            JsonString::ptr res = std::make_shared<JsonString>(*p);
            return std::static_pointer_cast<JsonValueBase>(res);
        }
        break;
    case JsonType::Number:
        {
            auto p = pointer_cast<JsonNumber>(src);
            JsonNumber::ptr res = std::make_shared<JsonNumber>(*p);
            return std::static_pointer_cast<JsonValueBase>(res);
        }
        break;
    case JsonType::Boolean:
        {
            auto p = pointer_cast<JsonBoolean>(src);
            JsonBoolean::ptr res = std::make_shared<JsonBoolean>(*p);
            return std::static_pointer_cast<JsonValueBase>(res);
        }
        break;
    case JsonType::Null:
        {
            auto p = pointer_cast<JsonNull>(src);
            JsonNull::ptr res = std::make_shared<JsonNull>(*p);
            return std::static_pointer_cast<JsonValueBase>(res);
        }
        break;
    case JsonType::Array:
        {
            auto p = pointer_cast<JsonArray>(src);
            JsonArray::ptr res = std::make_shared<JsonArray>(*p);
            return std::static_pointer_cast<JsonValueBase>(res);
        }
        break;
    case JsonType::Object:
        {
            auto p = pointer_cast<JsonObject>(src);
            JsonObject::ptr res = std::make_shared<JsonObject>(*p);
            return std::static_pointer_cast<JsonValueBase>(res);
        }
        break;
    default:
        break;
    }
    return nullptr;
}

JsonValueBase::JsonValueBase(const JsonValueBase& jsvb){
    if(jsvb.isScalar()){
        val_ = jsvb.val_;
    }
    else{
        if(jsvb.type_ == JsonType::Array){
            val_ = std::vector<JsonValueBase::ptr>();
            auto &arr = std::get<std::vector<JsonValueBase::ptr>>(val_);
            auto &arr1 = std::get<std::vector<JsonValueBase::ptr>>(jsvb.val_);
            arr.reserve(arr1.size());
            for(auto p : arr1){
                arr.emplace_back(copyFrom(p));
            }
        }
        else if(jsvb.type_ == JsonType::Object){
            val_ = std::map<JsonString, JsonValueBase::ptr>();
            auto &obj = std::get<std::map<JsonString, JsonValueBase::ptr>>(val_);
            auto &obj1 = std::get<std::map<JsonString, JsonValueBase::ptr>>(jsvb.val_);
            for(auto [k, v] : obj1){
                auto vp = copyFrom(v);
                obj.insert({k, vp});
            }
        }
    }
    type_ = jsvb.type_;
}


JsonArray::JsonArray(const JsonArray &another){
    val_ = std::vector<JsonValueBase::ptr>();
    auto &arr = std::get<std::vector<JsonValueBase::ptr>>(val_);
    auto &arr1 = std::get<std::vector<JsonValueBase::ptr>>(another.val_);
    arr.reserve(arr1.size());
    for(auto p : arr1){
        arr.emplace_back(copyFrom(p));
    }
    type_ = another.type_;
}


JsonArray& JsonArray::operator=(const JsonArray& another){
    val_ = std::vector<JsonValueBase::ptr>(); // 这个过程会自动把原来的值析构掉
    auto &arr = std::get<std::vector<JsonValueBase::ptr>>(val_);
    auto &arr1 = std::get<std::vector<JsonValueBase::ptr>>(another.val_);
    arr.reserve(arr1.size());
    for(auto p : arr1){
        arr.emplace_back(copyFrom(p));
    }
    type_ = another.type_;
    return *this;
}


JsonObject::JsonObject(const JsonObject& another){
    val_ = std::map<JsonString, JsonValueBase::ptr>();
    auto &obj = std::get<std::map<JsonString, JsonValueBase::ptr>>(val_);
    auto &obj1 = std::get<std::map<JsonString, JsonValueBase::ptr>>(another.val_);
    for(auto [k, v] : obj1){
        auto vp = copyFrom(v);
        obj.insert({k, vp});
    }
    type_ = another.type_;
}

JsonObject& JsonObject::operator=(const JsonObject& another){
    val_ = std::map<JsonString, JsonValueBase::ptr>(); // 这个过程会自动把原来的值析构掉
    auto &obj = std::get<std::map<JsonString, JsonValueBase::ptr>>(val_);
    auto &obj1 = std::get<std::map<JsonString, JsonValueBase::ptr>>(another.val_);
    for(auto [k, v] : obj1){
        auto vp = copyFrom(v);
        obj.insert({k, vp});
    }
    type_ = another.type_;
    return *this;
}

/* ---------------------------------------------type conversion--------------------------------------------- */

std::string getJsonTypeName(JsonType json_type){
    switch (json_type)
    {
    case JsonType::Object:
        return "Object";
        break;
    case JsonType::Array:
        return "Array";
        break;
    case JsonType::String:
        return "String";
        break;
    case JsonType::Number:
        return "Number";
        break;
    case JsonType::Boolean:
        return "Boolean";
        break;
    case JsonType::Null:
        return "Null";
        break;
    default:
        break;
    }
    return "---error---";
}


std::string JsonString::toString(const PrintFormatter &format, int depth) const{
    std::string res;
    res.reserve(getValue().size());
    res += '"';

    std::string_view view(getValue());
    char utf_str[6] = {0};
    while(!view.empty()){
        // 普通字符
        if(view[0] > 31 && view[0] != '\"' && view[0] != '\\'){
            res += view[0];
        }
        else{
            res += '\\';
            switch (view[0])
            {
            case '\\':
                res += '\\';
                break;
            case '\"':
                res += '"';
                break;
            case '\b':
                res += 'b';
                break;
            case '\f':
                res += 'f';
                break;
            case '\n':
                res += 'n';
                break;
            case '\r':
                res += 'r';
                break;
            case '\t':
                res += 't';
                break;
            default:
                // uxxxx
                if(format.ensureAscii()){
                    sprintf(utf_str, "u%04x", *(unsigned char *)view.data());
                    res += utf_str;
                }
                else{
                    res.pop_back();
                    res += view[0];
                }
                break;
            }
        }
        view.remove_prefix(1);
    }
    res += '"';
    return res;
}


} // namespace json


} // namespace haha