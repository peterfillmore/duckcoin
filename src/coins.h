// Copyright (c) 2009-2010 Silvio Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITBREADCRUMB_BREADCRUMBS_H
#define BITBREADCRUMB_BREADCRUMBS_H

#include "compressor.h"
#include "serialize.h"
#include "uint256.h"
#include "undo.h"

#include <assert.h>
#include <stdint.h>

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>

/** 
 * Pruned version of CTransaction: only retains metadata and unspent transaction outputs
 *
 * Serialized format:
 * - VARINT(nVersion)
 * - VARINT(nCode)
 * - unspentness bitvector, for vout[2] and further; least significant byte first
 * - the non-spent CTxOuts (via CTxOutCompressor)
 * - VARINT(nHeight)
 *
 * The nCode value consists of:
 * - bit 1: IsBreadcrumbBase()
 * - bit 2: vout[0] is not spent
 * - bit 4: vout[1] is not spent
 * - The higher bits encode N, the number of non-zero bytes in the following bitvector.
 *   - In case both bit 2 and bit 4 are unset, they encode N-1, as there must be at
 *     least one non-spent output).
 *
 * Example: 0104835800816115944e077fe7c803cfa57f29b36bf87c1d358bb85e
 *          <><><--------------------------------------------><---->
 *          |  \                  |                             /
 *    version   code             vout[1]                  height
 *
 *    - version = 1
 *    - code = 4 (vout[1] is not spent, and 0 non-zero bytes of bitvector follow)
 *    - unspentness bitvector: as 0 non-zero bytes follow, it has length 0
 *    - vout[1]: 835800816115944e077fe7c803cfa57f29b36bf87c1d35
 *               * 8358: compact amount representation for 60000000000 (600 BTC)
 *               * 00: special txout type pay-to-pubkey-hash
 *               * 816115944e077fe7c803cfa57f29b36bf87c1d35: address uint160
 *    - height = 203998
 *
 *
 * Example: 0109044086ef97d5790061b01caab50f1b8e9c50a5057eb43c2d9563a4eebbd123008c988f1a4a4de2161e0f50aac7f17e7f9555caa486af3b
 *          <><><--><--------------------------------------------------><----------------------------------------------><---->
 *         /  \   \                     |                                                           |                     /
 *  version  code  unspentness       vout[4]                                                     vout[16]           height
 *
 *  - version = 1
 *  - code = 9 (coinbase, neither vout[0] or vout[1] are unspent,
 *                2 (1, +1 because both bit 2 and bit 4 are unset) non-zero bitvector bytes follow)
 *  - unspentness bitvector: bits 2 (0x04) and 14 (0x4000) are set, so vout[2+2] and vout[14+2] are unspent
 *  - vout[4]: 86ef97d5790061b01caab50f1b8e9c50a5057eb43c2d9563a4ee
 *             * 86ef97d579: compact amount representation for 234925952 (2.35 BTC)
 *             * 00: special txout type pay-to-pubkey-hash
 *             * 61b01caab50f1b8e9c50a5057eb43c2d9563a4ee: address uint160
 *  - vout[16]: bbd123008c988f1a4a4de2161e0f50aac7f17e7f9555caa4
 *              * bbd123: compact amount representation for 110397 (0.001 BTC)
 *              * 00: special txout type pay-to-pubkey-hash
 *              * 8c988f1a4a4de2161e0f50aac7f17e7f9555caa4: address uint160
 *  - height = 120891
 */
class CBreadcrumbs
{
public:
    //! whether transaction is a coinbase
    bool fBreadcrumbBase;

    //! unspent transaction outputs; spent outputs are .IsNull(); spent outputs at the end of the array are dropped
    std::vector<CTxOut> vout;

    //! at which height this transaction was included in the active block chain
    int nHeight;

    //! version of the CTransaction; accesses to this value should probably check for nHeight as well,
    //! as new tx version will probably only be introduced at certain heights
    int nVersion;

    void FromTx(const CTransaction &tx, int nHeightIn) {
        fBreadcrumbBase = tx.IsBreadcrumbBase();
        vout = tx.vout;
        nHeight = nHeightIn;
        nVersion = tx.nVersion;
        ClearUnspendable();
    }

    //! construct a CBreadcrumbs from a CTransaction, at a given height
    CBreadcrumbs(const CTransaction &tx, int nHeightIn) {
        FromTx(tx, nHeightIn);
    }

    void Clear() {
        fBreadcrumbBase = false;
        std::vector<CTxOut>().swap(vout);
        nHeight = 0;
        nVersion = 0;
    }

    //! empty constructor
    CBreadcrumbs() : fBreadcrumbBase(false), vout(0), nHeight(0), nVersion(0) { }

    //!remove spent outputs at the end of vout
    void Cleanup() {
        while (vout.size() > 0 && vout.back().IsNull())
            vout.pop_back();
        if (vout.empty())
            std::vector<CTxOut>().swap(vout);
    }

    void ClearUnspendable() {
        BOOST_FOREACH(CTxOut &txout, vout) {
            if (txout.scriptPubKey.IsUnspendable())
                txout.SetNull();
        }
        Cleanup();
    }

    void swap(CBreadcrumbs &to) {
        std::swap(to.fBreadcrumbBase, fBreadcrumbBase);
        to.vout.swap(vout);
        std::swap(to.nHeight, nHeight);
        std::swap(to.nVersion, nVersion);
    }

    //! equality test
    friend bool operator==(const CBreadcrumbs &a, const CBreadcrumbs &b) {
         // Empty CBreadcrumbs objects are always equal.
         if (a.IsPruned() && b.IsPruned())
             return true;
         return a.fBreadcrumbBase == b.fBreadcrumbBase &&
                a.nHeight == b.nHeight &&
                a.nVersion == b.nVersion &&
                a.vout == b.vout;
    }
    friend bool operator!=(const CBreadcrumbs &a, const CBreadcrumbs &b) {
        return !(a == b);
    }

    void CalcMaskSize(unsigned int &nBytes, unsigned int &nNonzeroBytes) const;

    bool IsBreadcrumbBase() const {
        return fBreadcrumbBase;
    }

    unsigned int GetSerializeSize(int nType, int nVersion) const {
        unsigned int nSize = 0;
        unsigned int nMaskSize = 0, nMaskCode = 0;
        CalcMaskSize(nMaskSize, nMaskCode);
        bool fFirst = vout.size() > 0 && !vout[0].IsNull();
        bool fSecond = vout.size() > 1 && !vout[1].IsNull();
        assert(fFirst || fSecond || nMaskCode);
        unsigned int nCode = 8*(nMaskCode - (fFirst || fSecond ? 0 : 1)) + (fBreadcrumbBase ? 1 : 0) + (fFirst ? 2 : 0) + (fSecond ? 4 : 0);
        // version
        nSize += ::GetSerializeSize(VARINT(this->nVersion), nType, nVersion);
        // size of header code
        nSize += ::GetSerializeSize(VARINT(nCode), nType, nVersion);
        // spentness bitmask
        nSize += nMaskSize;
        // txouts themself
        for (unsigned int i = 0; i < vout.size(); i++)
            if (!vout[i].IsNull())
                nSize += ::GetSerializeSize(CTxOutCompressor(REF(vout[i])), nType, nVersion);
        // height
        nSize += ::GetSerializeSize(VARINT(nHeight), nType, nVersion);
        return nSize;
    }

    template<typename Stream>
    void Serialize(Stream &s, int nType, int nVersion) const {
        unsigned int nMaskSize = 0, nMaskCode = 0;
        CalcMaskSize(nMaskSize, nMaskCode);
        bool fFirst = vout.size() > 0 && !vout[0].IsNull();
        bool fSecond = vout.size() > 1 && !vout[1].IsNull();
        assert(fFirst || fSecond || nMaskCode);
        unsigned int nCode = 8*(nMaskCode - (fFirst || fSecond ? 0 : 1)) + (fBreadcrumbBase ? 1 : 0) + (fFirst ? 2 : 0) + (fSecond ? 4 : 0);
        // version
        ::Serialize(s, VARINT(this->nVersion), nType, nVersion);
        // header code
        ::Serialize(s, VARINT(nCode), nType, nVersion);
        // spentness bitmask
        for (unsigned int b = 0; b<nMaskSize; b++) {
            unsigned char chAvail = 0;
            for (unsigned int i = 0; i < 8 && 2+b*8+i < vout.size(); i++)
                if (!vout[2+b*8+i].IsNull())
                    chAvail |= (1 << i);
            ::Serialize(s, chAvail, nType, nVersion);
        }
        // txouts themself
        for (unsigned int i = 0; i < vout.size(); i++) {
            if (!vout[i].IsNull())
                ::Serialize(s, CTxOutCompressor(REF(vout[i])), nType, nVersion);
        }
        // coinbase height
        ::Serialize(s, VARINT(nHeight), nType, nVersion);
    }

    template<typename Stream>
    void Unserialize(Stream &s, int nType, int nVersion) {
        unsigned int nCode = 0;
        // version
        ::Unserialize(s, VARINT(this->nVersion), nType, nVersion);
        // header code
        ::Unserialize(s, VARINT(nCode), nType, nVersion);
        fBreadcrumbBase = nCode & 1;
        std::vector<bool> vAvail(2, false);
        vAvail[0] = (nCode & 2) != 0;
        vAvail[1] = (nCode & 4) != 0;
        unsigned int nMaskCode = (nCode / 8) + ((nCode & 6) != 0 ? 0 : 1);
        // spentness bitmask
        while (nMaskCode > 0) {
            unsigned char chAvail = 0;
            ::Unserialize(s, chAvail, nType, nVersion);
            for (unsigned int p = 0; p < 8; p++) {
                bool f = (chAvail & (1 << p)) != 0;
                vAvail.push_back(f);
            }
            if (chAvail != 0)
                nMaskCode--;
        }
        // txouts themself
        vout.assign(vAvail.size(), CTxOut());
        for (unsigned int i = 0; i < vAvail.size(); i++) {
            if (vAvail[i])
                ::Unserialize(s, REF(CTxOutCompressor(vout[i])), nType, nVersion);
        }
        // coinbase height
        ::Unserialize(s, VARINT(nHeight), nType, nVersion);
        Cleanup();
    }

    //! mark an outpoint spent, and construct undo information
    bool Spend(const COutPoint &out, CTxInUndo &undo);

    //! mark a vout spent
    bool Spend(int nPos);

    //! check whether a particular output is still available
    bool IsAvailable(unsigned int nPos) const {
        return (nPos < vout.size() && !vout[nPos].IsNull());
    }

    //! check whether the entire CBreadcrumbs is spent
    //! note that only !IsPruned() CBreadcrumbs can be serialized
    bool IsPruned() const {
        BOOST_FOREACH(const CTxOut &out, vout)
            if (!out.IsNull())
                return false;
        return true;
    }
};

class CBreadcrumbsKeyHasher
{
private:
    uint256 salt;

public:
    CBreadcrumbsKeyHasher();

    /**
     * This *must* return size_t. With Boost 1.46 on 32-bit systems the
     * unordered_map will behave unpredictably if the custom hasher returns a
     * uint64_t, resulting in failures when syncing the chain (#4634).
     */
    size_t operator()(const uint256& key) const {
        return key.GetHash(salt);
    }
};

struct CBreadcrumbsCacheEntry
{
    CBreadcrumbs coins; // The actual cached data.
    unsigned char flags;

    enum Flags {
        DIRTY = (1 << 0), // This cache entry is potentially different from the version in the parent view.
        FRESH = (1 << 1), // The parent view does not have this entry (or it is pruned).
    };

    CBreadcrumbsCacheEntry() : coins(), flags(0) {}
};

typedef boost::unordered_map<uint256, CBreadcrumbsCacheEntry, CBreadcrumbsKeyHasher> CBreadcrumbsMap;

struct CBreadcrumbsStats
{
    int nHeight;
    uint256 hashBlock;
    uint64_t nTransactions;
    uint64_t nTransactionOutputs;
    uint64_t nSerializedSize;
    uint256 hashSerialized;
    CAmount nTotalAmount;

    CBreadcrumbsStats() : nHeight(0), hashBlock(0), nTransactions(0), nTransactionOutputs(0), nSerializedSize(0), hashSerialized(0), nTotalAmount(0) {}
};


/** Abstract view on the open txout dataset. */
class CBreadcrumbsView
{
public:
    //! Retrieve the CBreadcrumbs (unspent transaction outputs) for a given txid
    virtual bool GetBreadcrumbs(const uint256 &txid, CBreadcrumbs &coins) const;

    //! Just check whether we have data for a given txid.
    //! This may (but cannot always) return true for fully spent transactions
    virtual bool HaveBreadcrumbs(const uint256 &txid) const;

    //! Retrieve the block hash whose state this CBreadcrumbsView currently represents
    virtual uint256 GetBestBlock() const;

    //! Do a bulk modification (multiple CBreadcrumbs changes + BestBlock change).
    //! The passed mapBreadcrumbs can be modified.
    virtual bool BatchWrite(CBreadcrumbsMap &mapBreadcrumbs, const uint256 &hashBlock);

    //! Calculate statistics about the unspent transaction output set
    virtual bool GetStats(CBreadcrumbsStats &stats) const;

    //! As we use CBreadcrumbsViews polymorphically, have a virtual destructor
    virtual ~CBreadcrumbsView() {}
};


/** CBreadcrumbsView backed by another CBreadcrumbsView */
class CBreadcrumbsViewBacked : public CBreadcrumbsView
{
protected:
    CBreadcrumbsView *base;

public:
    CBreadcrumbsViewBacked(CBreadcrumbsView *viewIn);
    bool GetBreadcrumbs(const uint256 &txid, CBreadcrumbs &coins) const;
    bool HaveBreadcrumbs(const uint256 &txid) const;
    uint256 GetBestBlock() const;
    void SetBackend(CBreadcrumbsView &viewIn);
    bool BatchWrite(CBreadcrumbsMap &mapBreadcrumbs, const uint256 &hashBlock);
    bool GetStats(CBreadcrumbsStats &stats) const;
};


class CBreadcrumbsViewCache;

/** 
 * A reference to a mutable cache entry. Encapsulating it allows us to run
 *  cleanup code after the modification is finished, and keeping track of
 *  concurrent modifications. 
 */
class CBreadcrumbsModifier
{
private:
    CBreadcrumbsViewCache& cache;
    CBreadcrumbsMap::iterator it;
    CBreadcrumbsModifier(CBreadcrumbsViewCache& cache_, CBreadcrumbsMap::iterator it_);

public:
    CBreadcrumbs* operator->() { return &it->second.coins; }
    CBreadcrumbs& operator*() { return it->second.coins; }
    ~CBreadcrumbsModifier();
    friend class CBreadcrumbsViewCache;
};

/** CBreadcrumbsView that adds a memory cache for transactions to another CBreadcrumbsView */
class CBreadcrumbsViewCache : public CBreadcrumbsViewBacked
{
protected:
    /* Whether this cache has an active modifier. */
    bool hasModifier;

    /**
     * Make mutable so that we can "fill the cache" even from Get-methods
     * declared as "const".  
     */
    mutable uint256 hashBlock;
    mutable CBreadcrumbsMap cacheBreadcrumbs;

public:
    CBreadcrumbsViewCache(CBreadcrumbsView *baseIn);
    ~CBreadcrumbsViewCache();

    // Standard CBreadcrumbsView methods
    bool GetBreadcrumbs(const uint256 &txid, CBreadcrumbs &coins) const;
    bool HaveBreadcrumbs(const uint256 &txid) const;
    uint256 GetBestBlock() const;
    void SetBestBlock(const uint256 &hashBlock);
    bool BatchWrite(CBreadcrumbsMap &mapBreadcrumbs, const uint256 &hashBlock);

    /**
     * Return a pointer to CBreadcrumbs in the cache, or NULL if not found. This is
     * more efficient than GetBreadcrumbs. Modifications to other cache entries are
     * allowed while accessing the returned pointer.
     */
    const CBreadcrumbs* AccessBreadcrumbs(const uint256 &txid) const;

    /**
     * Return a modifiable reference to a CBreadcrumbs. If no entry with the given
     * txid exists, a new one is created. Simultaneous modifications are not
     * allowed.
     */
    CBreadcrumbsModifier ModifyBreadcrumbs(const uint256 &txid);

    /**
     * Push the modifications applied to this cache to its base.
     * Failure to call this method before destruction will cause the changes to be forgotten.
     * If false is returned, the state of this cache (and its backing view) will be undefined.
     */
    bool Flush();

    //! Calculate the size of the cache (in number of transactions)
    unsigned int GetCacheSize() const;

    /** 
     * Amount of bitcoins coming in to a transaction
     * Note that lightweight clients may not know anything besides the hash of previous transactions,
     * so may not be able to calculate this.
     *
     * @param[in] tx	transaction for which we are checking input total
     * @return	Sum of value of all inputs (scriptSigs)
     */
    CAmount GetValueIn(const CTransaction& tx) const;

    //! Check whether all prevouts of the transaction are present in the UTXO set represented by this view
    bool HaveInputs(const CTransaction& tx) const;

    //! Return priority of tx at height nHeight
    double GetPriority(const CTransaction &tx, int nHeight) const;

    const CTxOut &GetOutputFor(const CTxIn& input) const;

    friend class CBreadcrumbsModifier;

private:
    CBreadcrumbsMap::iterator FetchBreadcrumbs(const uint256 &txid);
    CBreadcrumbsMap::const_iterator FetchBreadcrumbs(const uint256 &txid) const;
};

#endif // BITBREADCRUMB_BREADCRUMBS_H
