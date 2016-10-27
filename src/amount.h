// Copyright (c) 2009-2010 Silvio Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITBREADCRUMB_AMOUNT_H
#define BITBREADCRUMB_AMOUNT_H

#include "serialize.h"

#include <stdlib.h>
#include <string>

typedef int64_t CAmount;

static const CAmount BREADCRUMB = 100000000;
static const CAmount CENT = 1000000;

/** No amount larger than this (in silvio) is valid */
/** a coin for every duck in america! */
static const CAmount MAX_MONEY = 45600000 * BREADCRUMB;
inline bool MoneyRange(const CAmount& nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

/** Type-safe wrapper class to for fee rates
 * (how much to pay based on transaction size)
 */
class CFeeRate
{
private:
    CAmount nSilviosPerK; // unit is silvios-per-1,000-bytes
public:
    CFeeRate() : nSilviosPerK(0) { }
    explicit CFeeRate(const CAmount& _nSilviosPerK): nSilviosPerK(_nSilviosPerK) { }
    CFeeRate(const CAmount& nFeePaid, size_t nSize);
    CFeeRate(const CFeeRate& other) { nSilviosPerK = other.nSilviosPerK; }

    CAmount GetFee(size_t size) const; // unit returned is silvios
    CAmount GetFeePerK() const { return GetFee(1000); } // silvios-per-1000-bytes

    friend bool operator<(const CFeeRate& a, const CFeeRate& b) { return a.nSilviosPerK < b.nSilviosPerK; }
    friend bool operator>(const CFeeRate& a, const CFeeRate& b) { return a.nSilviosPerK > b.nSilviosPerK; }
    friend bool operator==(const CFeeRate& a, const CFeeRate& b) { return a.nSilviosPerK == b.nSilviosPerK; }
    friend bool operator<=(const CFeeRate& a, const CFeeRate& b) { return a.nSilviosPerK <= b.nSilviosPerK; }
    friend bool operator>=(const CFeeRate& a, const CFeeRate& b) { return a.nSilviosPerK >= b.nSilviosPerK; }
    std::string ToString() const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(nSilviosPerK);
    }
};

#endif //  BITBREADCRUMB_AMOUNT_H
