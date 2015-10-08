// Lightweight compile-time string hashes and sequential IDs generator.
// - rlyeh, zlib/libpng licensed. ~~ listening to Radiohead / Karma Police
//
// Contains code by Stefan 'Tivolo' Reinalter. Thanks dude! : )
// - [ref] http://www.altdevblogaday.com/2011/10/27/quasi-compile-time-string-hashing/

// Notes:
// $id() is very sensitive to compiler version and compilation settings!
// Please ensure your final disassembly IDs are fully unrolled (~using in-place immediates)

#pragma once
#include <cassert>
#include <cstdint>
#include <cstdio>

#include <iostream>
#include <map>
#include <string>
#include <typeinfo>

#define ID_VERSION "1.0.0" // (2015/10/01) Semantic versioning adherence

/* private API */
class id {
    static bool check_for_collisions( size_t hash, const char *str ) {
        static std::map< size_t, std::string > map;
        auto found = map.find( hash );
        if( found != map.end() ) {
            if( found->second != str ) {
                std::cerr << "<id/id.hpp> says: error! '"
                    << map[ hash ] << "' and '" << str << "' hashes collide!" << std::endl;
                return false;
            }
        } else {
            map.insert( std::pair< size_t, std::string >( hash, str ) );
        }
        return true;
    }

#ifdef _MSC_VER
#   define ID_CONSTEXPR __forceinline
#else
#   define ID_CONSTEXPR constexpr
#endif

    template <unsigned int N, unsigned int I>
    struct fnv1a {
    ID_CONSTEXPR static unsigned int hash( const char (&str)[N] ) {
        return ( id::fnv1a<N, I-1>::hash(str) ^ str[I-1] ) * 16777619u;
    }};

    template <unsigned int N>
    struct fnv1a<N, 1> {
    ID_CONSTEXPR static unsigned int hash( const char (&str)[N] ) {
        return ( 2166136261u ^ str[0] ) * 16777619u;
    }};

public:

    // compile-time hashed IDs have MSB flag enabled
    template <unsigned int N>
    ID_CONSTEXPR static unsigned int gen( const char (&str)[N] ) {
        return /*assert( check_for_collisions( id::fnv1a<N,N>::hash( str ) | 0x80000000, str ) ),*/ id::fnv1a<N,N>::hash( str ) | 0x80000000;
    }

    // sequential IDs have MSB flag disabled
    static unsigned int &gen() {
        static unsigned int seq = 0;
        return seq = (++seq) & 0x7FFFFFFFF, seq;
    }
};

/* public API */
#define $id(...) id::gen(#__VA_ARGS__)
