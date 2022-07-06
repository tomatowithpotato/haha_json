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

enum class JsonType { Object, Array, String, Number, Boolean, Null };

typedef decltype(nullptr) NullType;

// template<typename T> JsonType __toJsonType();
// // 下边的要写在源文件里头
// template<> JsonType __toJsonType<std::string>() { return JsonType::String; }
// template<> JsonType __toJsonType<int>() { return JsonType::Number; }
// template<> JsonType __toJsonType<double>() { return JsonType::Number; }
// template<> JsonType __toJsonType<bool>() { return JsonType::Boolean; }
// template<> JsonType __toJsonType<NullType>() { return JsonType::Null; }


class JsonValue{
public:
    typedef std::shared_ptr<JsonValue> ptr;
    explicit JsonValue(JsonType type):type_(type){}
    JsonType getType() const { return type_; }
    virtual std::string toString() { return ""; }

private:
    JsonType type_;
    // std::string valueString_;
    // int valueInt_;
    // double valueDouble_;
};


class JsonString : public JsonValue{
public:
    typedef std::shared_ptr<JsonString> ptr;
    explicit JsonString(const std::string &str):JsonValue(JsonType::String),val_(str){}
    std::string toString() override { return val_; }
    bool operator <(const JsonString &rhs) const{
        return val_ < rhs.val_;
    }
private:
    std::string val_;
};

class JsonBoolean : public JsonValue{
public:
    typedef std::shared_ptr<JsonBoolean> ptr;
    explicit JsonBoolean(bool val):JsonValue(JsonType::Boolean),val_(val){}
private:
    bool val_;
};


class JsonNumber : public JsonValue{
public:
    typedef std::shared_ptr<JsonNumber> ptr;
    explicit JsonNumber(int val):JsonValue(JsonType::Number),integer_(val),double_(static_cast<double>(val)){}
    explicit JsonNumber(double val):JsonValue(JsonType::Number),integer_(static_cast<int>(val)),double_(val){}
private:
    int integer_;
    double double_;
};


class JsonNull : public JsonValue{
public:
    typedef std::shared_ptr<JsonNull> ptr;
    JsonNull():JsonValue(JsonType::Null){}
};


class JsonArray : public JsonValue{
public:
    typedef std::shared_ptr<JsonArray> ptr;
    JsonArray() : JsonValue(JsonType::Array){}
    size_t size() const { return arr_.size(); }
    bool empty() const { return arr_.empty(); }
    void add(JsonValue::ptr val) { arr_.emplace_back(val); }
    void add(const std::string &str) { arr_.emplace_back(std::make_shared<JsonString>(str)); }
    void add(bool val) { arr_.emplace_back(std::make_shared<JsonBoolean>(val)); }
    void add(int val) { arr_.emplace_back(std::make_shared<JsonNumber>(val)); }
    void add(double val) { arr_.emplace_back(std::make_shared<JsonNumber>(val)); }
    void add() { arr_.emplace_back(std::make_shared<JsonNull>()); }
private:
    std::vector<JsonValue::ptr> arr_;
};


class JsonObject : public JsonValue{
public:
    typedef std::shared_ptr<JsonObject> ptr;
    typedef std::pair<JsonString, JsonValue::ptr> kv_pair;
    JsonObject() : JsonValue(JsonType::Object){}
    size_t size() const { return map_.size(); }
    bool empty() const { return map_.empty(); }
    void add(const JsonString &&key, JsonValue::ptr val){
        map_.insert({key, val});
    }
    void add(const JsonString &key, JsonValue::ptr val){
        map_.insert({key, val});
    }
    void add(const std::string &key, JsonValue::ptr val){
        map_.insert({JsonString(key), val});
    }
    void add(const std::string &key, const std::string &val) { 
        map_.insert({JsonString(key), std::make_shared<JsonString>(val)}); 
    }
    void add(const std::string &key, bool val) { 
        map_.insert({JsonString(key), std::make_shared<JsonBoolean>(val)}); 
    }
    void add(const std::string &key, int val) { 
        map_.insert({JsonString(key), std::make_shared<JsonNumber>(val)}); 
    }
    void add(const std::string &key, double val) { 
        map_.insert({JsonString(key), std::make_shared<JsonNumber>(val)}); 
    }
    void add(const std::string &key) { 
        map_.insert({JsonString(key), std::make_shared<JsonNull>()}); 
    }
    void del(const std::string &key) {
        map_.erase(JsonString(key));
    }
    template<typename T>
    T& get(const std::string &key){
        return *std::static_pointer_cast<T>(map_.at(JsonString(key)));
    }

private:
    std::map<JsonString, JsonValue::ptr> map_;
    // std::vector<kv_pair> kv_arr_;
};


} // namespace json

} // namespace haha


#endif