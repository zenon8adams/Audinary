#include <string>
#include <deque>
#include "model/numbers-to-words.hpp"

using stringlist = std::deque<std::string>;

auto WordGenerator::get( std::string number) -> stringlist
{
    return parse_( std::move(number));
}

stringlist& WordGenerator::order_()
{
    static stringlist _order = {
        {
            "thousand", "million", "billion", "trillion", "quadrillion",
            "quintillion", "sextillion", "septillion", "octillion",
            "nonillion", "decillion", "undecillion", "deudecillion",
            "tredecillion", "quattourdecillion", "quindecillion",
            "sexdecillion", "septondecillion", "octodecillion",
            "novemdecillion", "vigindecillion"
        }
    };

    return _order;
}

const char *WordGenerator::unit_(int n)
{
    if( n < '1' || n > '9' )
        return "";
    static const char *transl[ ] = { "one", "two", "three", "four", "five",
                                     "six", "seven", "eight", "nine" };
    return transl[ n - '1'];
}

const char *WordGenerator::ten_(int n)
{
    if( n < '1' || n > '9' )
        return "";
    static const char *transl[ ] = { "ten", "twenty", "thirty", "fourty", "fifty",
                                     "sixty", "seventy", "eighty", "ninety" };

    return transl[ n - '1'];
}

const char *WordGenerator::others_(int n)
{
    if( n < 11 || n > 19 )
        return "";
    static const char *transl[ ] = { "eleven", "twelve", "thirteen", "fourteen",
                                     "fifteen", "sixteen", "seventeen", "eighteen",
                                     "nineteen" };
    return transl[ n - 11];
}

stringlist WordGenerator::parse_( std::string num)
{
    stringlist toks;
    if(num.size() == 1){
        toks.push_front(unit_(num[0]));
        return toks;
    }
    for(size_t nlen = num.size()-1, idx = nlen-1, count = 1, pos = (size_t)-1; idx != (size_t)-1; --idx){
        if(num[idx] != ',' && num[idx] != ' ')
            ++count;
        std::string tmp{};
        if(count == 3){
            if(num[idx+1] != '0' && num[idx+2] != '0'){
                if(num[idx+1] == '1')
                    tmp = others_(10 * (num[idx+1]-'0') + (num[idx+2]-'0'));
                else
                    tmp = ten_(num[idx+1]) + std::string(1, '-') + unit_(num[idx+2]);
            }else if(num[idx+1] != '0' && num[idx+2] == '0'){

                if(num[idx+1] == '1' && num[idx+2] != '0'){

                    tmp = others_(10 * (num[idx+1]-'0') + (num[idx+2]-'0'));
                }
                else{

                    tmp = ten_(num[idx+1]);
                    if(num[idx+1] != '0') std::string(1, '-') + unit_(num[idx+1]);
                }
            }else
                tmp = unit_(num[idx+2]);
            if(num[idx] != '0'){
                if(!tmp.empty())
                {
                    tmp.insert( 0, unit_(num[idx]) + std::string(" hundred and "));
                }
                else{
                    tmp = unit_(num[idx]) + std::string(" hundred");
                    if(toks.empty() && idx > 0)
                        tmp.insert( 0, "and");
                }
            }else{
                if(toks.empty() && idx > 0)
                    tmp.insert( 0, "and");
            }
            if(pos != (size_t)-1 && !tmp.empty()) tmp += " " + order_()[pos];
            ++pos;
            count = 0;
            if(!tmp.empty())
                toks.push_front(tmp);
            tmp.clear();
        }else if(idx == 0){
            if(count == 2){
                if(num[idx] != '0' && num[idx+1] != '0'){
                    if(num[idx] == '1')
                        tmp = others_(10 * (num[idx]-'0') + (num[idx+1]-'0'));
                    else
                        tmp = ten_(num[idx]) + std::string(1, '-') + unit_(num[idx+1]);
                }else if(num[idx] != '0' && num[idx+1] == '0'){
                    if(num[idx] == '1' && num[idx+1] != '0')
                        tmp = others_(10 * (num[idx]-'0') + (num[idx+1]-'0'));
                    else{
                        tmp = ten_(num[idx]);
                        if( num[idx+1] != '0') std::string(1, '-') + unit_(num[idx+1]);
                    }
                }else
                    tmp = unit_(num[idx+1]);
            }else if(count == 1){
                tmp = unit_(num[idx]);
            }
            if(pos != (size_t)-1 && !tmp.empty()) tmp += " " + order_()[pos];
            if(!tmp.empty())
                toks.push_front(tmp);

            tmp.clear();
        }
    }
    return toks;
}