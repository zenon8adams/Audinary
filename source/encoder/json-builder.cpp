#include "encoder/json-builder.hpp"

JsonBuilder& JsonBuilder::add( const std::string& key, const std::string& value, bool quote)
{
    extend();
    session += "\"" + key + "\"" + ": " + ( quote ? "\"" + value + "\"" : value);
    return *this;
}

JsonBuilder& JsonBuilder::add( const std::string& key, bool value)
{
    extend();
    session += "\"" + key + "\"" + ": " + (value ? "true" : "false");
    return *this;
}

JsonBuilder& JsonBuilder::add( const std::string& key, int value)
{
    extend();
    session += "\"" + key + "\"" + ": " + std::to_string( value);
    return *this;
}

inline void JsonBuilder::extend( char stamp)
{
    if( session[ session.size() - 1] != stamp)
        session += ',';
}

JsonBuilder& JsonBuilder::startList( const std::string& key)
{
    return add( key, "[", false);
}

JsonBuilder& JsonBuilder::add( const std::string& value)
{
    extend( '[');
    session += "\"" + value + "\"";

    return *this;
}

JsonBuilder& JsonBuilder::endList()
{
    final( ']');

    return *this;
}

std::string JsonBuilder::final( char stamp)
{
    if( session[ session.size() - 1] != stamp)
        session += stamp;

    return session;
}