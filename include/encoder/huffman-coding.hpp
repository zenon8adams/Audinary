#ifndef HUFFMAN_CODING_HPP
#define HUFFMAN_CODING_HPP

#include <set>
#include <unordered_map>

#define __I64_MAX      ((uint64_t)-1)
#define __BLOCK_SIZE    sizeof(uint64_t)
#define __MAGIC_NUMBER  4 + 3 + 1
#define __MAGIC         "1.7.2.9"
#define __LANG          3 + 1

class HuffmanCoding
{
public:
    enum class GeneratorMode
    {
        ENCODE,
        DECODE
    };

    explicit HuffmanCoding( GeneratorMode mode, const char *filename = nullptr, const char *lang = nullptr);
    ~HuffmanCoding();
    std::string retrieve( const char *filename);
    void save( const char *filename);
    const char *currentLocale();

    std::string retrieve(const char *start, size_t bytes);

private:

    struct Info
    {
        uint64_t quantity = __I64_MAX;
        uint64_t frequency = 0;
    };

    struct HuffmanNode
    {
        Info information;
        HuffmanNode *left = nullptr,
            *right = nullptr;
        explicit HuffmanNode( Info in): information( in)
        {
        }

        HuffmanNode( Info in, HuffmanNode *l, HuffmanNode *r)
            : information( in), left( l), right( r)
        {
        }
    };

    struct HuffmanNodeComparator
    {
        bool operator()( HuffmanNode * one, HuffmanNode * other) const
        {
            return one->information.frequency <= other->information.frequency;
        }
    };

    struct ShortCode
    {
        uint32_t weight{};
        uint32_t shift{};
        uint64_t quantity{};
    };

    struct Header
    {
        char      MAGIC[ __MAGIC_NUMBER]{},
                  locale[ __LANG]{};
        uint64_t  clength{}, info_length{};
        ShortCode *coding = nullptr;
        float     version = 1.0;
    } _header;

    std::multiset<HuffmanNode *, HuffmanNodeComparator> generateFrequencyTable();

    void buildInternal( const char *filename);

    HuffmanNode *buildTree();

    HuffmanNode *rebuildTree();

    static void insert( HuffmanNode *&node, ShortCode recovery);

    static HuffmanNode *find( HuffmanNode *node, ShortCode recovery);

    void extractEncoding( HuffmanNode *node, uint16_t code = 0u, uint8_t shift = 0);

    static void destroy( HuffmanNode *&node);
    static void editByteCount(size_t& value, uint8_t byte);

    std::unordered_map<uint64_t, ShortCode *> optimizer_;
    char *base_ = nullptr;

    HuffmanNode *root_ = nullptr;

    std::string retrieveImpl(int sfd, size_t nbytes);

};

#endif //HUFFMAN_CODING_HPP
