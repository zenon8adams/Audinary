#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <fontconfig/fontconfig.h>

#include "encoder/huffman-coding.hpp"
#include "model/beeper.hpp"
#include "encoder/json-builder.hpp"
#include "model/textual-time.hpp"

#define NO_OP                     ( void)0
#define IDX( value)               Router::jetkins_one_at_a_time_hash( value)
#define LIST_COMPRESSION_OPERATOR "$"

#define TRUE                      "true"
#define NONE                      "none"

#define TIME                      "time"
#define NL                        '\n'

#define FONTS                     "fonts"
#define EMIT_ID                   "emitID"
#define NOTIFICATION              "notification"
#define TRACK                     "track"
#define INTERNATIONALIZATION      "i18n"


#define IFONTS                    IDX( FONTS)
#define IEMIT_ID                  IDX( EMIT_ID)
#define INOTIFICATION             IDX( NOTIFICATION)
#define ITRACK                    IDX( TRACK)
#define IINTERNATIONALIZATION     IDX( INTERNATIONALIZATION)

class Router
{
public:
    static std::string generateResponse( const std::unordered_map<std::string, std::string>& params);
private:

    static inline std::string unquote( const std::string& value);

    static void try_process( const std::pair<std::string, std::string>& value, JsonBuilder& builder);

    static inline uint32_t jetkins_one_at_a_time_hash( const std::string& value);

    static void requestFontList( JsonBuilder& json, const std::string& locale);

    static inline const std::unordered_map<uint32_t, uint8_t>& eventPriority();
};

#endif //ROUTER_HPP