/*
    Author:      Adesina Meekness
    E-mail:      zenon8adams@gmail.com
    Created:     10-12-2019
    Last Edited: 14-01-2021
*/

#include <deque>
#include <ctime>
#include <sstream>
#include "model/numbers-to-words.hpp"
#include "model/textual-time.hpp"
#include "model/beeper.hpp"

using namespace std::string_literals;

TextualTime& TextualTime::instance()
{
    return _single;
}

std::string TextualTime::get()
{
    (void)_default;

    stringlist splitted;

    auto formatted = currentTime_();

    size_t split_pos = formatted.find( ':');

    std::string hour_raw = formatted.substr( 0, split_pos),
        minutes_raw = formatted.substr( split_pos + 1);

    int hour_as_int = std::stoi( hour_raw),
        minute_as_int = std::stoi( minutes_raw);
    bool close_to_midnight = hour_as_int == 23;

    std::string zone = " A.M.";
    if( hour_as_int > 12)
    {
        zone = " P.M.";
        hour_as_int -= 12;
    }

    bool hour_reset = hour_as_int == 0,
        minute_reset = minute_as_int == 0;

    if( minute_reset) {
        std::string hour = WordGenerator::get( std::to_string( hour_as_int)).front();
        Beeper::instance()->play();
        if( hour_reset)
            return prefix_() + sep_() + hour + midnight_();

        return prefix_() + sep_() + hour + sep_() + suffix_();
    }

    std::string minute_text = minute_as_int == 1 ? " minute" : " minutes";

    switch( minute_as_int) {
    case 1 ... 14:
    case 16 ... 29:
        return prefix_() + sep_() + minute_( minute_as_int) + minute_text + " past " + hour_( hour_as_int, zone);
    case 15:
        return prefix_() + " quater past " + hour_( hour_as_int, zone);
    case 30:
        return prefix_() + sep_() + "half past " + hour_( hour_as_int, zone);
    case 31 ... 44:
    case 46 ... 59:
        return prefix_() + sep_() + nextMinute_( minute_as_int) +  minute_text + nextHour_( hour_as_int, close_to_midnight, zone);
    case 45:
        return prefix_() + " quater" + nextHour_( hour_as_int, close_to_midnight, zone);
    default:
        return {};
    }
}

TextualTime& TextualTime::setRefrenceLocalization( const std::string& raw)
{
    _default.clear();
    std::stringstream dstream( raw);

    for( std::string line; std::getline( dstream, line); )
        _default.push_back( line);

    return *this;
}

int TextualTime::index()
{
    if( _default.empty())
        return -1;


    auto time = std::find( _default.cbegin(), _default.cend(), TextualTime::get());

    return time - _default.cbegin();
}

std::string TextualTime::nextHour_( int hour, bool close_to_midnight, const std::string& zone)
{
    const size_t time = ( hour + 1) % 12;
    return close_to_midnight ? " till " + midnight_()
                             : " to " + WordGenerator::get( std::to_string( time == 0 ? 12 : time)).front() + zone;
}

std::string TextualTime::hour_( int hour, const std::string& zone)
{
    return hour ?  WordGenerator::get( std::to_string( hour)).front() + zone : midnight_();
}

std::string TextualTime::minute_( int minute)
{
    return WordGenerator::get( std::to_string( minute)).front();
}

std::string TextualTime::nextMinute_( int minute)
{
    return WordGenerator::get( std::to_string( 60 - minute)).front();
}

std::string TextualTime::currentTime_()
{
    auto current = std::time(nullptr);
    auto time_store = std::localtime( &current);

    std::string formatted_time = std::to_string( time_store->tm_hour) + ":" + std::to_string( time_store->tm_min);

    return formatted_time;
}

std::string TextualTime::prefix_()
{
    return "It's";
}

std::string TextualTime::suffix_()
{
    return "O'clock";
}

std::string TextualTime::midnight_()
{
    return "midnight";
}

std::string& TextualTime::sep_()
{
    static std::string sep( " ");

    return sep;
}

TextualTime TextualTime::_single;