/*
    Author:      Adesina Meekness
    E-mail:      zenon8adams@gmail.com
    Created:     10-12-2019
    Last Edited: 14-01-2021
*/


#include <vector>
using namespace std::string_literals;

class TextualTime
{
    TextualTime() = default;
    static TextualTime _single;
public:
    static TextualTime& instance();

    std::string get();

    TextualTime& setRefrenceLocalization( const std::string& raw);

    int index();

private:

    static std::string nextHour_( int hour, bool close_to_midnight, const std::string& zone);

    static std::string hour_( int hour, const std::string& zone);

    static std::string minute_( int minute);

    static std::string nextMinute_( int minute);

    static std::string currentTime_();

    static std::string prefix_();

    static std::string suffix_();

    static std::string midnight_();

    static std::string& sep_();

    std::vector<std::string> _default;
};
