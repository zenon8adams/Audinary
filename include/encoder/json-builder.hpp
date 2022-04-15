#ifndef JSON_BUILDER_HPP
#define JSON_BUILDER_HPP

#include <iomanip>

class JsonBuilder
{
public:
    JsonBuilder& add( const std::string& key, const std::string& value, bool quote = true);

    JsonBuilder& add( const std::string& key, bool value);

    JsonBuilder& add( const std::string& key, int value);

    inline void extend( char stamp = '{');

    JsonBuilder& startList( const std::string& key);

    JsonBuilder& add( const std::string& value);

    JsonBuilder& endList();

    std::string final( char stamp = '}');

private:
    std::string session = "{";
};

#endif //JSON_BUILDER_HPP
