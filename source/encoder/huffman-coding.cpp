#include <queue>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <clocale>
#include <cstring>
#include <map>
#include <iostream>
#include <syslog.h>

#define NO_OP           ((void)0)
#define __I64_MAX      ((uint64_t)-1)   // Senitel that identifies a non-leaf node of the Huffman tree
#define __BLOCK_SIZE    sizeof(uint64_t)
#define __MAGIC_NUMBER  4 + 3 + 1
#define __MAGIC         "1.7.2.9"       // Magic number identifies the huffman encoded file (Courtesy: S.R. Ramalujan)

#include "encoder/huffman-coding.hpp"

/*
 * 64-bit reverse routine:
 * Reverses the bit sequence in a qword
 */

#define REV64( value ) \
    ({ \
        auto qword = value;\
        uint64_t m1  = 0x5555555555555555U, \
                 m2  = 0x3333333333333333U, \
                 m4  = 0x0F0F0F0F0F0F0F0FU, \
                 m8  = 0x00FF00FF00FF00FFU, \
                 m16 = 0x0000FFFF0000FFFFU, \
                 m32 = 0x00000000FFFFFFFFU; \
        \
        (qword) = ((qword) &  m1) << 1U  | (((qword) >>  1U) &  m1); \
        (qword) = ((qword) &  m2) << 2U  | (((qword) >>  2U) &  m2); \
        (qword) = ((qword) &  m4) << 4U  | (((qword) >>  4U) &  m4); \
        (qword) = ((qword) &  m8) << 8U  | (((qword) >>  8U) &  m8); \
        (qword) = ((qword) & m16) << 16U | (((qword) >> 16U) & m16); \
        (qword) = ((qword) & m32) << 32U | (((qword) >> 32U) & m32); \
        qword;\
    })

/*
 * Converts an integer into a utf-8 byte sequence
 */

static std::string encode( uint32_t val )
{

    std::string sRep;
    if (val < 0x80) {
        sRep += (char)(val);
    } else if (val < 0x800) {
        sRep += (char)(0xC0U | (val >> 6U));
        sRep += (char)(0x80U | (val & 0x3FU));
    } else if (val < 0x10000)
    {
        sRep += (char)(0xE0U |  (val >> 12U));
        sRep += (char)(0x80U | ((val >>  6U) & 0x3FU));
        sRep += (char)(0x80U |  (val         & 0x3FU));
    } else if (val < 0x200000)
    {
        sRep += (char)(0xF0U |  (val >> 18U));
        sRep += (char)(0x80U | ((val >> 12U) & 0x3FU));
        sRep += (char)(0x80U | ((val >>  6U) & 0x3FU));
        sRep += (char)(0x80U |  (val         & 0x3FU));
    }else if( val < 0x4000000 )
    {
        sRep += (char)( 0xF8U | (  val >> 24U           ));
        sRep += (char)( 0x80U | (( val >> 18U ) & 0x3FU ));
        sRep += (char)( 0x80U | (( val >> 12U ) & 0x3FU ));
        sRep += (char)( 0x80U | (( val >> 6U )  & 0x3FU ));
        sRep += (char)( 0x80U | (  val          & 0x3FU ));
    }else if( val < 0x80000000 )
    {
        sRep += (char)( 0xFCU |  ( val >> 30U           ));
        sRep += (char)( 0xF8U | (( val >> 24U ) & 0x3FU ));
        sRep += (char)( 0x80U | (( val >> 18U ) & 0x3FU ));
        sRep += (char)( 0x80U | (( val >> 12U ) & 0x3FU ));
        sRep += (char)( 0x80U | (( val >> 6U )  & 0x3FU ));
        sRep += (char)( 0x80U |  ( val          & 0x3FU ));
    }
    return sRep;
}

/*
 * Hueristic function that guesses the number of bytes in
 * the utf-8 sequence without actually confurming that
 * the next byte is a continuation byte.
 */

static size_t byteGuess( uint8_t c )
{
    for( uint8_t n = 2u; n <= 6u; ++n )
    {
        bool val = ( ( uint8_t)(c >> n) ^ (0xFFU >> n));
        if( !val )
            return 8 - n;
    }
    return 1;
}

/*
 * The patch function converts count bytes from idx in str into
 * its integer equivalent.
 */

static uint32_t patch(uint8_t *str, size_t idx, uint8_t count )
{
    if( count == 1 )
        return str[idx];

    uint32_t copy = count;
    while( copy > 1 )
        str[idx + --copy] &= 0x3FU;

    str[ idx ] &= 0xFFU >> (count + 1u);
    count -= 1;
    size_t i = ( count << 2u ) + ( count << 1u);

    uint32_t value = 0;
    while( (int8_t)count >= 0 )
    {
        value += str[ idx++ ] << i;
        --count;
        i -= 6;
    }

    return value;
}

HuffmanCoding::HuffmanCoding( GeneratorMode mode, const char *filename, const char *lang)
{
    if( lang == nullptr)
    {
        const char *locale = setlocale( LC_NAME, "");
        const char *end = strchr( locale, '_');
        memcpy( _header.locale, locale, end - locale);
    }
    else
        memcpy( _header.locale, lang, strlen(lang));

    uint32_t rlocale = REV64((( uint64_t)*( uint32_t *)_header.locale) << 32u);
    memcpy(_header.locale, &rlocale, __LANG);

    if( mode == GeneratorMode::ENCODE)
        buildInternal( filename);
}

void HuffmanCoding::buildInternal( const char *filename)
{
    int fd = open( filename, O_RDONLY);

    if( fd == -1)
    {
        syslog( LOG_ERR, "Internal structure build failed");
        return;
    }

    struct stat prop{};

    if( fstat( fd, &prop) == -1)
    {
        syslog( LOG_ERR, "Unable to stat() file");
        return;
    }

    // Map the file into memory so that it can be accessed again with lower cost
    base_ = ( char *)mmap(nullptr, _header.info_length = prop.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if( base_ == MAP_FAILED)
    {
        syslog( LOG_ERR, "Memory allocation failed(mmap)");
        return;
    }

    root_ = buildTree();
    extractEncoding( root_);

    memcpy(_header.MAGIC, ( const char *)__MAGIC, sizeof( __MAGIC));
}

HuffmanCoding::~HuffmanCoding()
{
    if( base_)          munmap( base_, _header.info_length);
    if( _header.coding) free( _header.coding);
    if( root_)          destroy( root_);
}

std::multiset<HuffmanCoding::HuffmanNode *, HuffmanCoding::HuffmanNodeComparator> HuffmanCoding::generateFrequencyTable()
{
    std::map<uint64_t, uint64_t> distribution;

    size_t read = 0;
    while( read < _header.info_length)
    {
        distribution[ *(uint64_t *)( base_ + read)] += 1;
        read += __BLOCK_SIZE;
    }

    std::multiset<HuffmanNode *, HuffmanNodeComparator> queue;

    _header.clength = distribution.size();
    _header.coding = ( ShortCode *)calloc( _header.clength, sizeof( ShortCode));

    for( size_t i = 0; i < _header.clength; ++i)
    {
        ShortCode& code = _header.coding[ i];
        auto dIter = std::next( distribution.cbegin(), i);
        optimizer_[ dIter->first] = &code;

        queue.insert( new HuffmanNode( { .quantity = dIter->first, .frequency = dIter->second}));
    }

    return queue;
}

HuffmanCoding::HuffmanNode *HuffmanCoding::buildTree()
{
    auto queue = generateFrequencyTable();

    while( queue.size() > 1)
    {
        auto lnode = *queue.cbegin(); queue.erase( queue.begin());
        auto rnode = *queue.cbegin(); queue.erase( queue.begin());

        auto *combo = new HuffmanNode( {  .quantity = __I64_MAX,
                                           .frequency = lnode->information.frequency + rnode->information.frequency},
                                       lnode, rnode);
        queue.insert( combo);
    }

    return *queue.begin();
}

HuffmanCoding::HuffmanNode *HuffmanCoding::rebuildTree()
{
    HuffmanNode *root = nullptr;

    for( size_t i = 0; i < _header.clength; ++i)
    {
        ShortCode& recovery = _header.coding[ i];
        if( recovery.shift == 0)
            continue;

        insert( root, recovery);

        optimizer_[ recovery.quantity] = &recovery;
    }

    return root;
}

void HuffmanCoding::insert( HuffmanNode *&node, ShortCode recovery)
{
    if( recovery.shift == 0)
    {
        if( node == nullptr)
            node = new HuffmanNode( { .quantity = recovery.quantity});
        return;
    }

    if (node == nullptr)
        node = new HuffmanNode( { .quantity = __I64_MAX});

    HuffmanNode *&next = ( recovery.weight & ( 1u << --recovery.shift)) ? node->right : node->left;
    recovery.weight &= ~( 1u << recovery.shift);

    insert( next, recovery);
}

HuffmanCoding::HuffmanNode *HuffmanCoding::find( HuffmanNode *node, ShortCode recovery)
{
    if( recovery.shift == 0 || node == nullptr || ( node->left == nullptr && node->right))
        return node;

    HuffmanNode *&next = ( recovery.weight & ( 1u << --recovery.shift)) ? node->right : node->left;
    recovery.weight &= ~( 1u << recovery.shift);

    return find( next, recovery);
}

void HuffmanCoding::extractEncoding( HuffmanNode *node, uint16_t code, uint8_t shift)
{
    if( node == nullptr)
        return;

    extractEncoding( node->left, code << 1u, shift + 1);
    extractEncoding( node->right,  ( ( uint16_t)( code << 1u) | 1u), shift + 1);

    if( node->information.quantity != __I64_MAX)
        *( optimizer_[ node->information.quantity]) = { .weight = code, .shift = shift, .quantity = node->information.quantity};
}

void HuffmanCoding::save( const char *filename)
{
    int fd = open( filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if( fd == -1)
    {
        syslog( LOG_ERR, "Unable to encode file");
        return;
    }

    ssize_t dummy = 0;

    dummy += write( fd, _header.MAGIC, sizeof( __MAGIC));
    dummy += write( fd, &_header.version, sizeof( _header.version));
    dummy += write( fd, _header.locale, __LANG);
    dummy += write( fd,  &_header.clength, sizeof( _header.clength) * 2);

    for( size_t i = 0; i < _header.clength; ++i)
    {
        ShortCode code = _header.coding[ i];
        code.quantity = REV64( code.quantity);
        dummy += write( fd, &code, sizeof( ShortCode));
    }

    size_t written = 0;
    while( written < _header.info_length)
    {
        ShortCode code = *( optimizer_[ *(uint64_t *)( base_ + written)]);
        auto u8weight = encode( code.weight),
            u8shift = encode( code.shift);
        dummy += write( fd, u8weight.data(), u8weight.size());
        dummy += write( fd, u8shift.data(),  u8shift.size());

        written += __BLOCK_SIZE;
    }
}

std::string HuffmanCoding::retrieve(const char *start, size_t bytes)
{
    int desps[ 2];
    ( void)pipe( desps);   // Write into file descriptor desp[ 1] then read from it at desps[ 0]

    ( void)write( desps[ 1], start, bytes);
    close( desps[ 1]);

    return retrieveImpl( desps[ 0], bytes);
}

std::string HuffmanCoding::retrieveImpl( int sfd, size_t nbytes)
{
    ssize_t actual = 0, out = 0;
    actual += read( sfd, &_header.MAGIC, sizeof( __MAGIC));
    if( strcmp( _header.MAGIC, __MAGIC) != 0)
    {
        syslog( LOG_ERR, "Invalid file format");
        return {};
    }
    actual += read( sfd, &_header.version, sizeof( _header.version));
    actual += read( sfd, _header.locale, __LANG);
    actual += read( sfd, &_header.clength, sizeof( _header.clength) * 2);
    _header.coding = ( ShortCode *)calloc( _header.clength, sizeof( ShortCode));
    if( ( out = read( sfd, _header.coding, sizeof( ShortCode) * _header.clength)) < 0)
    {
        syslog( LOG_ERR, "Invalid file format");
        return {};
    }

    actual += out;

    const ssize_t total = sizeof( __MAGIC) + sizeof( _header.version) + \
                          sizeof( _header.clength) * 2 + sizeof( ShortCode) * _header.clength + __LANG;

    if( actual != total)
    {
        syslog( LOG_ERR, "Invalid file format");
        return {};
    }

    uint32_t locale = REV64( ((uint64_t)*(uint32_t *)_header.locale)) >> 32u;

    memcpy( _header.locale, &locale, __LANG);

    for( size_t i = 0; i < _header.clength; ++i)
        _header.coding[ i].quantity = REV64( _header.coding[ i].quantity);

    HuffmanNode *head = rebuildTree();

    ssize_t length, idx = 0;
    char *buf = ( char *)calloc(  length = nbytes - actual, sizeof( char));
    if( buf == nullptr)
    {
        syslog( LOG_ERR, "Memory allocation failed");
        return {};
    }

    if( length != read( sfd, buf, length))
    {
        syslog( LOG_ERR, "Invalid file format");
        return {};
    }

    std::string destination( _header.info_length, '\0');

    ssize_t written = 0;
    while( idx < length)
    {
        auto bweight = byteGuess(*(buf + idx));
        editByteCount(bweight, *((uint8_t *)(buf + idx) + 1));
        auto weight = patch( (uint8_t *)buf, idx, bweight);
        idx += bweight;
        auto bshift = byteGuess(*(buf + idx));
        editByteCount(bshift, *((uint8_t *) (buf + idx) + 1));
        auto shift  = patch( ( uint8_t *)buf, idx, bshift);
        idx += bshift;

        // Get block with having the defined weight and shift in O(log n)
        HuffmanNode *mayblock = find( head, { .weight = weight, .shift = shift});

        if( mayblock != nullptr)
        {
            ssize_t left = _header.info_length - written;
            /*
             * Write the decoded sequence into the destination making sure that
             * the decoded bytes equals the read bytes
             */
            memcpy( ( &destination[ 0] + written),  &mayblock->information.quantity,
                    left < (ssize_t)__BLOCK_SIZE ? left : (ssize_t)__BLOCK_SIZE);
            written += __BLOCK_SIZE;
        }
    }

    free( buf);
    free( _header.coding); _header.coding = nullptr;
    close( sfd);

    return destination;
}

std::string HuffmanCoding::retrieve(const char *filename)
{
    int sfd = open( filename, O_RDONLY);
    if( sfd == -1)
    {
        syslog( LOG_ERR, "Unable to decode file");
        return {};
    }

    struct stat prop{};
    if( fstat(sfd, &prop) == -1)
    {
        syslog( LOG_ERR, "Unable to stat() file");
        return {};
    }

    return retrieveImpl( sfd, prop.st_size);
}


/*
 *  Correct byteGuess by confirming if the next byte seen is a continuation byte or
 *  a leading byte.
 */

void HuffmanCoding::editByteCount( size_t& value, uint8_t byte)
{
    if( value > 1)
    {
        if( !( byte & 0x80u))
            value = 1;
    }
}

void HuffmanCoding::destroy(HuffmanCoding::HuffmanNode *&node)
{
    if( node == nullptr)
        return;

    destroy( node->left);
    destroy( node->right);

    delete node; node = nullptr;
}

const char *HuffmanCoding::currentLocale()
{
    return _header.locale;
}