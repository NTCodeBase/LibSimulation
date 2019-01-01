//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//    .--------------------------------------------------.
//    |  This file is part of NTCodeBase                 |
//    |  Created 2018 by NT (https://ttnghia.github.io)  |
//    '--------------------------------------------------'
//                            \o/
//                             |
//                            / |
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#pragma once
#include <LibCommon/CommonSetup.h>
#include <unordered_map>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class StringHash {
    static constexpr const int8_t STRING_END_BYTE = 0;
    static constexpr const UInt   STRING_BIT_SHIT = 5u;
    static constexpr const UInt   STRING_HASH_KEY = 5381u;
public:
#if 0
    static constexpr size_t hash(const char* cstr) {
        size_t d = 5381lu;
        size_t i = 0lu;
        while(cstr[i] != '\0') {
            d = d * 33lu + cstr[i++];
        }
        return d;
    }

#endif

    static constexpr UInt hash(const char* pTail, UInt hash = STRING_HASH_KEY) {
#if __cplusplus >= 201402L
        while(*pTail != STRING_END_BYTE) {
            hash = (hash << STRING_BIT_SHIT) + hash + (int32_t)*pTail;
            pTail++;
        }
        return hash;
#elif __cplusplus >= 201103L
        return (pTail[0] == STRING_END_BYTE) ? hash :
               hash_function(pTail + 1, ((hash << STRING_BIT_SHIT) + hash) + (int32_t)*pTail);
#endif
    }

    static bool isValidHash(const char* cstr) {
        const auto str     = String(cstr);
        const auto hashVal = hash(cstr);
        if(s_HashedStrings.find(hashVal) != s_HashedStrings.end() && s_HashedStrings[hashVal] != str) {
            return false;
        }
        s_HashedStrings[hashVal] = str;
        return true;
    }

private:
    ////////////////////////////////////////////////////////////////////////////////
    // helper variable to verify the validity of perfect string hashing
    static inline std::unordered_map<UInt, String> s_HashedStrings;
};
