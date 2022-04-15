#ifndef NUMBERS_TO_WORDS
#define NUMBERS_TO_WORDS

using stringlist = std::deque<std::string>;

class WordGenerator
{
public:

    static auto get( std::string number) -> stringlist;

private:
    WordGenerator() = default;

    static stringlist& order_();

    static const char *unit_(int n);

    static const char *ten_(int n);

    static const char *others_(int n);

    static stringlist parse_( std::string num);
};

#endif
