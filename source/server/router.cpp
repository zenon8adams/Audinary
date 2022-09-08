#include <string>
#include <cstring>
#include <cstdint>
#include "server/router.hpp"

std::string Router::generateResponse( const std::unordered_map<std::string, std::string>& params)
{
    JsonBuilder response;
    std::vector<std::pair<std::string, std::string>> vparams( params.cbegin(), params.cend());

    std::sort( vparams.begin(), vparams.end(), []( const auto& one, const auto& other)
    {
        auto iter_one   = eventPriority().find( IDX( one.first)),
            iter_other = eventPriority().find( IDX( other.first)),
            sentinel    = eventPriority().cend();
        return ( iter_one == sentinel ? 1 : iter_one->second) > ( iter_other == sentinel ? 1 : iter_other->second);
    });

    for( const auto& each : vparams)
        try_process( each, response);

    return response.final();
}

inline std::string Router::unquote( const std::string& value)
{
    return value.substr( 1, value.size() - 2);
}

void Router::try_process( const std::pair<std::string, std::string>& value, JsonBuilder& builder)
{
    static std::string locale;
    static std::unordered_map<uint32_t, std::function<void( const std::pair<std::string, std::string>& value, JsonBuilder&)>> _mapping = {
        { IFONTS, []( const auto& value, JsonBuilder& response)
        {
            value.second == TRUE ? requestFontList( response, locale) : NO_OP;
        }},
        { IEMIT_ID, []( const auto& value, JsonBuilder& response)
        {
            value.second == TRUE ? response.add( TIME, TextualTime::instance().index())
                                 : response.add( TIME, TextualTime::instance().get());
        }},
        { INOTIFICATION, []( const auto& value, JsonBuilder& response){ Beeper::instance()->setEnabled( unquote( value.second) != NONE);}},
        { ITRACK, []( const auto& value, JsonBuilder& response)
        {
            std::string filename = unquote( value.second);
            // Remove the file:// Uri (file.substr(7)
            !filename.empty() ? Beeper::instance()->installBeeper( filename.substr( 7)) : NO_OP;
        }},
        { IINTERNATIONALIZATION, []( const auto& value, JsonBuilder& response)
        {
            static HuffmanCoding decoder = HuffmanCoding(HuffmanCoding::GeneratorMode::DECODE);

            std::string filename = unquote( value.second);
            if( filename.empty())
                return;

            std::string decoded  = decoder.retrieve( filename.substr( 7).c_str()),
                escaped;

            for ( auto&& each: decoded)
            {
                if( each == NL)   escaped += LIST_COMPRESSION_OPERATOR;
                else              escaped += each;
            }

            response.add( INTERNATIONALIZATION, escaped);
            locale = decoder.currentLocale();
        }}

    };

    auto fn = _mapping[ IDX( value.first)];
    fn ? fn( value, builder) : NO_OP;
}

inline uint32_t Router::jetkins_one_at_a_time_hash( const std::string& value)
{
    uint32_t hash = 0;
    const char *p = value.data();
    while( *p)
    {
        hash += *( p++);
        hash += hash << 10u;
        hash ^= hash >> 6u;
    }

    hash += hash << 3u;
    hash ^= hash >> 11u;
    hash += hash << 15u;

    return hash;
}

void Router::requestFontList( JsonBuilder& json, const std::string& locale)
{
    if( !FcInit())
        return;

    FcConfig *config = FcConfigGetCurrent();
    FcConfigSetRescanInterval( config, 0);
    FcPattern *pattern = locale.empty() ? FcPatternCreate() : FcNameParse( ( const FcChar8 *)( ":lang=" + locale).c_str());
    FcObjectSet *fontObjectSet = FcObjectSetBuild( FC_FAMILY, nullptr);
    FcFontSet *fontSet = FcFontList( config, pattern, fontObjectSet);

    if( fontSet && fontSet->nfont > 0)
    {
        int i = 0;
        ( void)json.startList( "fonts");
        do
        {
            const char *font = ( const char *)FcNameUnparse( fontSet->fonts[ i]),
                *breakp = strchr( font, ',');
            std::string fontName;
            std::copy_if( font, font + ( breakp ? breakp - font : strlen( font)),
                          std::back_inserter( fontName), []( const auto& piece){ return piece != '\\'; });

            json.add( fontName);
            free( ( FcChar8 *)font);
        }
        while( ++i < fontSet->nfont);

        ( void)json.endList();
    }

    if( fontSet)
        FcFontSetDestroy( fontSet);

}

inline const std::unordered_map<uint32_t, uint8_t>& Router::eventPriority()
{
    static const std::unordered_map<uint32_t, uint8_t > priority = {
        { IFONTS,                1},
        { IEMIT_ID,              1},
        { INOTIFICATION,         1},
        { ITRACK,                1},
        { IINTERNATIONALIZATION, 2}
    };

    return priority;
}
