#ifndef __HAHA_JSON_JSONVALUE_H__
#define __HAHA_JSON_JSONVALUE_H__

#include <memory>
#include <vector>
#include <map>
#include "jsonUtil.h"


namespace haha
{

namespace json
{

enum class JsonType { UNKOWN, Object, Array, String, Number, Boolean, Null };

typedef decltype(nullptr) NullType;

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
    explicit JsonValueBase(JsonType type):type_(type){}
    JsonType getType() const { return type_; }
    virtual std::string toString (const PrintFormatter &format = PrintFormatter(), int depth = 0) const { return ""; }

private:
    JsonType type_;
    // std::string valueString_;
    // int valueInt_;
    // double valueDouble_;
};

template<typename T>
class JsonValue : public JsonValueBase{
public:
    JsonValue(JsonType type, const T& val)
        :JsonValueBase(type), val_(val){}
    virtual const T& getValue() const { return val_; }
protected:
    T val_;
};


class JsonString : public JsonValue<std::string>{
public:
    typedef std::shared_ptr<JsonString> ptr;
    explicit JsonString(const std::string &str):JsonValue(JsonType::String, str){}
    std::string toString(const PrintFormatter &format = PrintFormatter(), int depth = 0) const override;
    bool operator <(const JsonString &rhs) const{
        return val_ < rhs.val_;
    }
};

class JsonBoolean : public JsonValue<bool>{
public:
    typedef std::shared_ptr<JsonBoolean> ptr;
    explicit JsonBoolean(bool val):JsonValue(JsonType::Boolean, val){}
    std::string toString(const PrintFormatter &format = PrintFormatter(), int depth = 0) const override { 
        return val_ ? "true" : "false";
    }
};


struct Number{
    int integer_;
    double double_;
};


class JsonNumber : public JsonValue<Number>{
public:
    typedef std::shared_ptr<JsonNumber> ptr;
    explicit JsonNumber(int val)
        :JsonValue(JsonType::Number, {val, static_cast<double>(val)})
    {
    }
    explicit JsonNumber(double val)
        :JsonValue(JsonType::Number,{static_cast<int>(val), val})
    {
    }
    std::string toString(const PrintFormatter &format = PrintFormatter(), int depth = 0) const override { 
        return std::to_string(val_.double_); 
    }
    int toInt() { return val_.integer_; }
    double toDouble() { return val_.double_; }
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

    ConstIterator begin() const { return val_.begin(); }
    ConstIterator end() const { return val_.end(); }
    Iterator begin() { return val_.begin(); }
    Iterator end() { return val_.end(); }

    size_t size() const { return val_.size(); }
    bool empty() const { return val_.empty(); }

    void add(JsonValueBase::ptr val) { val_.emplace_back(val); }
    void add(const std::string &str) { val_.emplace_back(std::make_shared<JsonString>(str)); }
    void add(bool val) { val_.emplace_back(std::make_shared<JsonBoolean>(val)); }
    void add(int val) { val_.emplace_back(std::make_shared<JsonNumber>(val)); }
    void add(double val) { val_.emplace_back(std::make_shared<JsonNumber>(val)); }
    void add() { val_.emplace_back(std::make_shared<JsonNull>()); }

    std::string toString(const PrintFormatter &format = PrintFormatter(), int depth = 0) const override {
        std::string res = "[";
        auto fmt = format.formatType();

        for(size_t i = 0; i < val_.size(); ++i){
            if(fmt == JsonFormatType::NEWLINE){
                res += '\n';
                res += std::string(depth+1, '\t');
            }
            res += val_[i]->toString(format, depth+1);
            res += i+1 < val_.size() ? "," : "";
            if(fmt == JsonFormatType::SPACE && i+1 < val_.size()){
                res += ' ';
            }
        }

        res += fmt == JsonFormatType::NEWLINE && val_.size() ? '\n' + std::string(depth, '\t') : "";
        res += "]";
        return res;
    }
};


class JsonObject : public JsonValue<std::map<JsonString, JsonValueBase::ptr>>{
public:
    typedef typename std::map<JsonString, JsonValueBase::ptr> Map;
    typedef typename Map::iterator Iterator;
    typedef typename Map::const_iterator ConstIterator;

    typedef std::shared_ptr<JsonObject> ptr;
    typedef std::pair<JsonString, JsonValueBase::ptr> kv_pair;

    JsonObject() : JsonValue(JsonType::Object, Map()){}

    size_t size() const { return val_.size(); }

    bool empty() const { return val_.empty(); }

    ConstIterator begin() const { return val_.begin(); }
    ConstIterator end() const { return val_.end(); }
    Iterator begin() { return val_.begin(); }
    Iterator end() { return val_.end(); }

    void add(const JsonString &&key, JsonValueBase::ptr val){
        val_.insert({key, val});
    }
    void add(const JsonString &key, JsonValueBase::ptr val){
        val_.insert({key, val});
    }
    void add(const std::string &key, JsonValueBase::ptr val){
        val_.insert({JsonString(key), val});
    }
    void add(const std::string &key, const std::string &val) { 
        val_.insert({JsonString(key), std::make_shared<JsonString>(val)}); 
    }
    void add(const std::string &key, bool val) { 
        val_.insert({JsonString(key), std::make_shared<JsonBoolean>(val)}); 
    }
    void add(const std::string &key, int val) { 
        val_.insert({JsonString(key), std::make_shared<JsonNumber>(val)}); 
    }
    void add(const std::string &key, double val) { 
        val_.insert({JsonString(key), std::make_shared<JsonNumber>(val)}); 
    }
    void add(const std::string &key) { 
        val_.insert({JsonString(key), std::make_shared<JsonNull>()}); 
    }
    void del(const std::string &key) {
        val_.erase(JsonString(key));
    }
    template<typename T>
    T& get(const std::string &key){
        return *std::static_pointer_cast<T>(val_.at(JsonString(key)));
    }

    std::string toString(const PrintFormatter &format = PrintFormatter(), int depth = 0) const override {
        std::string res = "{";
        auto fmt = format.formatType();
        int cnt = 0;
        for(const auto &[k, v] : val_){
            if(fmt == JsonFormatType::NEWLINE){
                res += '\n';
                res += std::string(depth+1, '\t');
            }
            res += k.toString(format, depth+1);
            res += ':';
            res += fmt == JsonFormatType::RAW ? "" : " ";
            res += v->toString(format, depth+1);
            res += (cnt + 1 < (int)val_.size()) ? "," : "";
            res += (cnt + 1 < (int)val_.size() && fmt == JsonFormatType::SPACE) ? " " : "";
            ++cnt;
        }
        res += fmt == JsonFormatType::NEWLINE && val_.size() ? '\n' + std::string(depth, '\t') : "";
        res += '}';
        return res;
    }
};


/* 
convert to type T,  
but not safe !!!
一个不做任何检查的类型转换函数
你应该在使用前自行检查类型转换是否合法
*/
template<typename T>
T type_cast(JsonValueBase::ptr source);

} // namespace json

} // namespace haha


#endif