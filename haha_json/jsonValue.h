#ifndef __HAHA_JSON_JSONVALUE_H__
#define __HAHA_JSON_JSONVALUE_H__

#include <memory>
#include <vector>
#include <variant>
#include <map>
#include "jsonUtil.h"


namespace haha
{

namespace json
{

enum class JsonType { UNKOWN, Object, Array, String, Number, Boolean, Null };

typedef decltype(nullptr) NullType;

struct Number{
    int integer_;
    double double_;
};

// template<typename T> JsonType __toJsonType();
// // 下边的要写在源文件里头
// template<> JsonType __toJsonType<std::string>() { return JsonType::String; }
// template<> JsonType __toJsonType<int>() { return JsonType::Number; }
// template<> JsonType __toJsonType<double>() { return JsonType::Number; }
// template<> JsonType __toJsonType<bool>() { return JsonType::Boolean; }
// template<> JsonType __toJsonType<NullType>() { return JsonType::Null; }

std::string getJsonTypeName(JsonType json_type);

enum class JsonFormatType { RAW, SPACE, NEWLINE };

class PrintFormatter{
public:
    PrintFormatter(const JsonFormatType &fmt = JsonFormatType::RAW, 
                    int indent = 0,
                    bool ensure_ascii = true)
        :format_(fmt),
        indent_(indent),
        ensure_ascii_(ensure_ascii)
    {
    }
    JsonFormatType formatType() const { return format_; }
    bool ensureAscii() const { return ensure_ascii_; }
private:
    JsonFormatType format_ = JsonFormatType::RAW;
    int indent_ = 0;
    bool ensure_ascii_ = true;
};


class JsonValueBase{
public:
    typedef std::shared_ptr<JsonValueBase> ptr;
    explicit JsonValueBase(JsonType type = JsonType::UNKOWN):type_(type){}
    virtual ~JsonValueBase() {}

    JsonValueBase(const JsonValueBase& jsvb);

    JsonType getType() const { return type_; }
    // 是否为标量：字符串、数字、bool、null
    bool isScalar() const { return !isIterable(); }
    // 可迭代类型：对象、数组
    bool isIterable() const { return type_ == JsonType::Object || type_ == JsonType::Array; }
    virtual std::string toString (const PrintFormatter &format = PrintFormatter(), int depth = 0) const { return ""; }

protected:
    JsonValueBase::ptr copyFrom(JsonValueBase::ptr src);

protected:
    JsonType type_;

protected:
    std::variant<bool,decltype(nullptr),
                Number,
                std::string,
                std::vector<JsonValueBase::ptr>,
                std::map<JsonValueBase::ptr, JsonValueBase::ptr>> val_ = nullptr;
};

template<typename T>
class JsonValue : public JsonValueBase{
public:
    typedef T ValueType;
    JsonValue(JsonType type, const T& val)
        :JsonValueBase(type){ val_ = val; }
    JsonValue():JsonValueBase(){}
    const T& getValue() const { return std::get<ValueType>(val_); }
protected:
    T& getValue() { return std::get<ValueType>(val_); }
};


class JsonString : public JsonValue<std::string>{
public:
    typedef std::shared_ptr<JsonString> ptr;
    explicit JsonString(const std::string &str):JsonValue(JsonType::String, str){}
    // JsonString(const JsonString& another);
    std::string toString(const PrintFormatter &format = PrintFormatter(), int depth = 0) const override;
    bool operator <(const JsonString &rhs) const{
        return getValue() < rhs.getValue();
    }
};

class JsonBoolean : public JsonValue<bool>{
public:
    typedef std::shared_ptr<JsonBoolean> ptr;
    explicit JsonBoolean(bool val):JsonValue(JsonType::Boolean, val){}
    std::string toString(const PrintFormatter &format = PrintFormatter(), int depth = 0) const override { 
        return getValue() ? "true" : "false";
    }
};


class JsonNumber : public JsonValue<Number>{
public:
    typedef std::shared_ptr<JsonNumber> ptr;
    explicit JsonNumber(int val)
        :JsonValue(JsonType::Number, {val, static_cast<double>(val)})
    {
    }
    explicit JsonNumber(double val)
        :JsonValue(JsonType::Number, {static_cast<int>(val), val})
    {
    }
    std::string toString(const PrintFormatter &format = PrintFormatter(), int depth = 0) const override { 
        return std::to_string(getValue().double_); 
    }
    int toInt() { return getValue().integer_; }
    double toDouble() { return getValue().double_; }
};


class JsonNull : public JsonValue<decltype(nullptr)>{
public:
    typedef std::shared_ptr<JsonNull> ptr;
    JsonNull():JsonValue(JsonType::Null, nullptr){}
    std::string toString(const PrintFormatter &format = PrintFormatter(), int depth = 0) const override { 
        return "null"; 
    }
};


class JsonArray : public JsonValue<std::vector<JsonValueBase::ptr>>{
public:
    typedef typename std::vector<JsonValueBase::ptr> Array;
    typedef typename Array::iterator Iterator;
    typedef typename Array::const_iterator ConstIterator;

    typedef std::shared_ptr<JsonArray> ptr;

    JsonArray() : JsonValue(JsonType::Array, Array()){}
    JsonArray(const JsonArray &another);

    ConstIterator begin() const { return getValue().begin(); }
    ConstIterator end() const { return getValue().end(); }
    Iterator begin() { return getValue().begin(); }
    Iterator end() { return getValue().end(); }

    size_t size() const { return getValue().size(); }
    bool empty() const { return getValue().empty(); }

    void add(JsonValueBase::ptr val) { getValue().emplace_back(val); }
    void add(const std::string &str) { getValue().emplace_back(std::make_shared<JsonString>(str)); }
    void add(bool val) { getValue().emplace_back(std::make_shared<JsonBoolean>(val)); }
    void add(int val) { getValue().emplace_back(std::make_shared<JsonNumber>(val)); }
    void add(double val) { getValue().emplace_back(std::make_shared<JsonNumber>(val)); }
    void add() { getValue().emplace_back(std::make_shared<JsonNull>()); }

    std::string toString(const PrintFormatter &format = PrintFormatter(), int depth = 0) const override {
        std::string res = "[";
        auto fmt = format.formatType();

        for(size_t i = 0; i < getValue().size(); ++i){
            if(fmt == JsonFormatType::NEWLINE){
                res += '\n';
                res += std::string(depth+1, '\t');
            }
            res += getValue()[i]->toString(format, depth+1);
            res += i+1 < getValue().size() ? "," : "";
            if(fmt == JsonFormatType::SPACE && i+1 < getValue().size()){
                res += ' ';
            }
        }

        res += fmt == JsonFormatType::NEWLINE && getValue().size() ? '\n' + std::string(depth, '\t') : "";
        res += "]";
        return res;
    }
};


class JsonObject : public JsonValue<std::map<JsonValueBase::ptr, JsonValueBase::ptr>>{
public:
    typedef typename std::map<JsonValueBase::ptr, JsonValueBase::ptr> Map;
    typedef typename Map::iterator Iterator;
    typedef typename Map::const_iterator ConstIterator;

    typedef std::shared_ptr<JsonObject> ptr;
    typedef std::pair<std::string, JsonValueBase::ptr> kv_pair;

    JsonObject() : JsonValue(JsonType::Object, Map()){}

    JsonObject(const JsonObject& another);

    size_t size() const { return getValue().size(); }

    bool empty() const { return getValue().empty(); }

    ConstIterator begin() const { return getValue().begin(); }
    ConstIterator end() const { return getValue().end(); }
    Iterator begin() { return getValue().begin(); }
    Iterator end() { return getValue().end(); }

    void add(const JsonString::ptr key, JsonValueBase::ptr val){
        getValue().insert({std::static_pointer_cast<JsonValueBase>(key), val});
    }
    void add(const std::string &key, JsonValueBase::ptr val){
        getValue().insert({str2base(key), val});
    }
    void add(const std::string &key, const std::string &val) { 
        getValue().insert({str2base(key), std::make_shared<JsonString>(val)}); 
    }
    void add(const std::string &key, bool val) { 
        getValue().insert({str2base(key), std::make_shared<JsonBoolean>(val)}); 
    }
    void add(const std::string &key, int val) { 
        getValue().insert({str2base(key), std::make_shared<JsonNumber>(val)}); 
    }
    void add(const std::string &key, double val) { 
        getValue().insert({str2base(key), std::make_shared<JsonNumber>(val)}); 
    }
    void add(const std::string &key) { 
        getValue().insert({str2base(key), std::make_shared<JsonNull>()}); 
    }
    void del(const std::string &key) {
        getValue().erase(str2base(key));
    }

    template<typename T = JsonValueBase>
    T& get(const std::string &key){
        return *std::static_pointer_cast<T>(getValue().at(str2base(key)));
    }

    std::string toString(const PrintFormatter &format = PrintFormatter(), int depth = 0) const override {
        std::string res = "{";
        auto fmt = format.formatType();
        int cnt = 0;
        for(const auto &[k, v] : getValue()){
            if(fmt == JsonFormatType::NEWLINE){
                res += '\n';
                res += std::string(depth+1, '\t');
            }
            res += k->toString(format, depth+1);
            res += ':';
            res += fmt == JsonFormatType::RAW ? "" : " ";
            res += v->toString(format, depth+1);
            res += (cnt + 1 < (int)getValue().size()) ? "," : "";
            res += (cnt + 1 < (int)getValue().size() && fmt == JsonFormatType::SPACE) ? " " : "";
            ++cnt;
        }
        res += fmt == JsonFormatType::NEWLINE && getValue().size() ? '\n' + std::string(depth, '\t') : "";
        res += '}';
        return res;
    }

private:
    JsonValueBase::ptr str2base(std::string str){
        return std::static_pointer_cast<JsonValueBase>(std::make_shared<JsonString>(str));
    }
};


/* 
convert to type T,  
but not safe !!!
一个不做任何检查的指针类型转换函数
不安全
*/
template<typename T>
T::ptr pointer_cast(JsonValueBase::ptr source){
    static_assert(
        std::is_same<T, JsonString>::value ||
        std::is_same<T, JsonNumber>::value ||
        std::is_same<T, JsonBoolean>::value ||
        std::is_same<T, JsonNull>::value ||
        std::is_same<T, JsonArray>::value ||
        std::is_same<T, JsonObject>::value,
        "error input type"
    );
    return std::static_pointer_cast<T>(source);
}

} // namespace json

} // namespace haha


#endif