// Copyright (c) 2009-2010 Silvio Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITBREADCRUMB_TXDB_H
#define BITBREADCRUMB_TXDB_H

#include "leveldbwrapper.h"
#include "main.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

class CBreadcrumbs;
class uint256;

//! -dbcache default (MiB)
static const int64_t nDefaultDbCache = 100;
//! max. -dbcache in (MiB)
static const int64_t nMaxDbCache = sizeof(void*) > 4 ? 4096 : 1024;
//! min. -dbcache in (MiB)
static const int64_t nMinDbCache = 4;

/** CBreadcrumbsView backed by the LevelDB coin database (chainstate/) */
class CBreadcrumbsViewDB : public CBreadcrumbsView
{
protected:
    CLevelDBWrapper db;
public:
    CBreadcrumbsViewDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);

    bool GetBreadcrumbs(const uint256 &txid, CBreadcrumbs &coins) const;
    bool HaveBreadcrumbs(const uint256 &txid) const;
    uint256 GetBestBlock() const;
    bool BatchWrite(CBreadcrumbsMap &mapBreadcrumbs, const uint256 &hashBlock);
    bool GetStats(CBreadcrumbsStats &stats) const;
};

/** Access to the block database (blocks/index/) */
class CBlockTreeDB : public CLevelDBWrapper
{
public:
    CBlockTreeDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);
private:
    CBlockTreeDB(const CBlockTreeDB&);
    void operator=(const CBlockTreeDB&);
public:
    bool WriteBlockIndex(const CDiskBlockIndex& blockindex);
    bool ReadBlockFileInfo(int nFile, CBlockFileInfo &fileinfo);
    bool WriteBlockFileInfo(int nFile, const CBlockFileInfo &fileinfo);
    bool ReadLastBlockFile(int &nFile);
    bool WriteLastBlockFile(int nFile);
    bool WriteReindexing(bool fReindex);
    bool ReadReindexing(bool &fReindex);
    bool ReadTxIndex(const uint256 &txid, CDiskTxPos &pos);
    bool WriteTxIndex(const std::vector<std::pair<uint256, CDiskTxPos> > &list);
    bool WriteFlag(const std::string &name, bool fValue);
    bool ReadFlag(const std::string &name, bool &fValue);
    bool LoadBlockIndexGuts();
};

#endif // BITBREADCRUMB_TXDB_H
