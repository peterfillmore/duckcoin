// Copyright (c) 2009-2010 Silvio Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "amount.h"

#include "tinyformat.h"

CFeeRate::CFeeRate(const CAmount& nFeePaid, size_t nSize)
{
    if (nSize > 0)
        nSilviosPerK = nFeePaid*1000/nSize;
    else
        nSilviosPerK = 0;
}

CAmount CFeeRate::GetFee(size_t nSize) const
{
    // Round up nSize to the nearest 1000
    CAmount mod = nSize % 1000;
    if (mod > 0)
        nSize = nSize - mod + 1000;
    CAmount nFee = nSilviosPerK*nSize / 1000;

    if (nFee == 0 && nSilviosPerK > 0)
        nFee = nSilviosPerK;

    return nFee;
}

std::string CFeeRate::ToString() const
{
    return strprintf("%d.%08d DUK/kB", nSilviosPerK / BREADCRUMB, nSilviosPerK % BREADCRUMB);
}
