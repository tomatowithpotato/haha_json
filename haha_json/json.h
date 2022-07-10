#ifndef __HAHA_JSON_JSON_H__
#define __HAHA_JSON_JSON_H__

#include "jsonValue.h"
#include "jsonTypeCast.h"
#include "jsonUtil.h"
#include "jsonReader.h"
#include <string>
#include <vector>
#include <list>

namespace haha
{

namespace json
{

JsonValueBase::ptr parse(const char *str);
JsonValueBase::ptr parse(const std::string &str);

JsonValueBase::ptr parse(std::string_view &str);
JsonValueBase::ptr parse_value(std::string_view &str);
JsonValueBase::ptr parse_string(std::string_view &str);
JsonValueBase::ptr parse_number(std::string_view &str);
JsonValueBase::ptr parse_array(std::string_view &str);
JsonValueBase::ptr parse_object(std::string_view &str);

class Json{
public:
    typedef std::shared_ptr<Json> ptr;

    bool fromString(const char *string);
    bool fromString(const std::string &str);
    bool fromString(std::string_view str);

    std::string toString() const { return obj_ ? obj_->toString() : "---failed parse---"; }
    std::string toString(bool ensure_ascii) const { 
        return obj_ ? obj_->toString(ensure_ascii) : "---failed parse---"; 
    }
    std::string toString(int indent, bool ensure_ascii) const { 
        return obj_ ? obj_->toString(indent, ensure_ascii) : "---failed parse---"; 
    }
    std::string toString(const JsonFormatType &fmt, int indent, bool ensure_ascii) const {
         return obj_ ? obj_->toString(fmt, indent, ensure_ascii) : "---failed parse---"; 
    }

    bool fromFile(const char* filePath);
    bool fromFile(const std::string &filePath);

    bool toFile(const char* filePath);
    bool toFile(const std::string &filePath);

    JsonType getType() const { return obj_ ? obj_->getType() : JsonType::UNKOWN; }

    template<typename T = JsonValueBase>
    std::shared_ptr<T> getValuePtr() const { return std::static_pointer_cast<T>(obj_); }

private:
    std::string_view view_;
    JsonValueBase::ptr obj_;
    JsonReader reader_;
};

} // namespace json


} // namespace haha


#endif