// Copyright (c) 2012-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "coins.h"

#include "random.h"

#include <assert.h>

/**
 * calculate number of bytes for the bitmask, and its number of non-zero bytes
 * each bit in the bitmask represents the availability of one output, but the
 * availabilities of the first two outputs are encoded separately
 */
void CBreadcrumbs::CalcMaskSize(unsigned int &nBytes, unsigned int &nNonzeroBytes) const {
    unsigned int nLastUsedByte = 0;
    for (unsigned int b = 0; 2+b*8 < vout.size(); b++) {
        bool fZero = true;
        for (unsigned int i = 0; i < 8 && 2+b*8+i < vout.size(); i++) {
            if (!vout[2+b*8+i].IsNull()) {
                fZero = false;
                continue;
            }
        }
        if (!fZero) {
            nLastUsedByte = b + 1;
            nNonzeroBytes++;
        }
    }
    nBytes += nLastUsedByte;
}

bool CBreadcrumbs::Spend(const COutPoint &out, CTxInUndo &undo) {
    if (out.n >= vout.size())
        return false;
    if (vout[out.n].IsNull())
        return false;
    undo = CTxInUndo(vout[out.n]);
    vout[out.n].SetNull();
    Cleanup();
    if (vout.size() == 0) {
        undo.nHeight = nHeight;
        undo.fBreadcrumbBase = fBreadcrumbBase;
        undo.nVersion = this->nVersion;
    }
    return true;
}

bool CBreadcrumbs::Spend(int nPos) {
    CTxInUndo undo;
    COutPoint out(0, nPos);
    return Spend(out, undo);
}


bool CBreadcrumbsView::GetBreadcrumbs(const uint256 &txid, CBreadcrumbs &coins) const { return false; }
bool CBreadcrumbsView::HaveBreadcrumbs(const uint256 &txid) const { return false; }
uint256 CBreadcrumbsView::GetBestBlock() const { return uint256(0); }
bool CBreadcrumbsView::BatchWrite(CBreadcrumbsMap &mapBreadcrumbs, const uint256 &hashBlock) { return false; }
bool CBreadcrumbsView::GetStats(CBreadcrumbsStats &stats) const { return false; }


CBreadcrumbsViewBacked::CBreadcrumbsViewBacked(CBreadcrumbsView *viewIn) : base(viewIn) { }
bool CBreadcrumbsViewBacked::GetBreadcrumbs(const uint256 &txid, CBreadcrumbs &coins) const { return base->GetBreadcrumbs(txid, coins); }
bool CBreadcrumbsViewBacked::HaveBreadcrumbs(const uint256 &txid) const { return base->HaveBreadcrumbs(txid); }
uint256 CBreadcrumbsViewBacked::GetBestBlock() const { return base->GetBestBlock(); }
void CBreadcrumbsViewBacked::SetBackend(CBreadcrumbsView &viewIn) { base = &viewIn; }
bool CBreadcrumbsViewBacked::BatchWrite(CBreadcrumbsMap &mapBreadcrumbs, const uint256 &hashBlock) { return base->BatchWrite(mapBreadcrumbs, hashBlock); }
bool CBreadcrumbsViewBacked::GetStats(CBreadcrumbsStats &stats) const { return base->GetStats(stats); }

CBreadcrumbsKeyHasher::CBreadcrumbsKeyHasher() : salt(GetRandHash()) {}

CBreadcrumbsViewCache::CBreadcrumbsViewCache(CBreadcrumbsView *baseIn) : CBreadcrumbsViewBacked(baseIn), hasModifier(false), hashBlock(0) { }

CBreadcrumbsViewCache::~CBreadcrumbsViewCache()
{
    assert(!hasModifier);
}

CBreadcrumbsMap::const_iterator CBreadcrumbsViewCache::FetchBreadcrumbs(const uint256 &txid) const {
    CBreadcrumbsMap::iterator it = cacheBreadcrumbs.find(txid);
    if (it != cacheBreadcrumbs.end())
        return it;
    CBreadcrumbs tmp;
    if (!base->GetBreadcrumbs(txid, tmp))
        return cacheBreadcrumbs.end();
    CBreadcrumbsMap::iterator ret = cacheBreadcrumbs.insert(std::make_pair(txid, CBreadcrumbsCacheEntry())).first;
    tmp.swap(ret->second.coins);
    if (ret->second.coins.IsPruned()) {
        // The parent only has an empty entry for this txid; we can consider our
        // version as fresh.
        ret->second.flags = CBreadcrumbsCacheEntry::FRESH;
    }
    return ret;
}

bool CBreadcrumbsViewCache::GetBreadcrumbs(const uint256 &txid, CBreadcrumbs &coins) const {
    CBreadcrumbsMap::const_iterator it = FetchBreadcrumbs(txid);
    if (it != cacheBreadcrumbs.end()) {
        coins = it->second.coins;
        return true;
    }
    return false;
}

CBreadcrumbsModifier CBreadcrumbsViewCache::ModifyBreadcrumbs(const uint256 &txid) {
    assert(!hasModifier);
    std::pair<CBreadcrumbsMap::iterator, bool> ret = cacheBreadcrumbs.insert(std::make_pair(txid, CBreadcrumbsCacheEntry()));
    if (ret.second) {
        if (!base->GetBreadcrumbs(txid, ret.first->second.coins)) {
            // The parent view does not have this entry; mark it as fresh.
            ret.first->second.coins.Clear();
            ret.first->second.flags = CBreadcrumbsCacheEntry::FRESH;
        } else if (ret.first->second.coins.IsPruned()) {
            // The parent view only has a pruned entry for this; mark it as fresh.
            ret.first->second.flags = CBreadcrumbsCacheEntry::FRESH;
        }
    }
    // Assume that whenever ModifyBreadcrumbs is called, the entry will be modified.
    ret.first->second.flags |= CBreadcrumbsCacheEntry::DIRTY;
    return CBreadcrumbsModifier(*this, ret.first);
}

const CBreadcrumbs* CBreadcrumbsViewCache::AccessBreadcrumbs(const uint256 &txid) const {
    CBreadcrumbsMap::const_iterator it = FetchBreadcrumbs(txid);
    if (it == cacheBreadcrumbs.end()) {
        return NULL;
    } else {
        return &it->second.coins;
    }
}

bool CBreadcrumbsViewCache::HaveBreadcrumbs(const uint256 &txid) const {
    CBreadcrumbsMap::const_iterator it = FetchBreadcrumbs(txid);
    // We're using vtx.empty() instead of IsPruned here for performance reasons,
    // as we only care about the case where a transaction was replaced entirely
    // in a reorganization (which wipes vout entirely, as opposed to spending
    // which just cleans individual outputs).
    return (it != cacheBreadcrumbs.end() && !it->second.coins.vout.empty());
}

uint256 CBreadcrumbsViewCache::GetBestBlock() const {
    if (hashBlock == uint256(0))
        hashBlock = base->GetBestBlock();
    return hashBlock;
}

void CBreadcrumbsViewCache::SetBestBlock(const uint256 &hashBlockIn) {
    hashBlock = hashBlockIn;
}

bool CBreadcrumbsViewCache::BatchWrite(CBreadcrumbsMap &mapBreadcrumbs, const uint256 &hashBlockIn) {
    assert(!hasModifier);
    for (CBreadcrumbsMap::iterator it = mapBreadcrumbs.begin(); it != mapBreadcrumbs.end();) {
        if (it->second.flags & CBreadcrumbsCacheEntry::DIRTY) { // Ignore non-dirty entries (optimization).
            CBreadcrumbsMap::iterator itUs = cacheBreadcrumbs.find(it->first);
            if (itUs == cacheBreadcrumbs.end()) {
                if (!it->second.coins.IsPruned()) {
                    // The parent cache does not have an entry, while the child
                    // cache does have (a non-pruned) one. Move the data up, and
                    // mark it as fresh (if the grandparent did have it, we
                    // would have pulled it in at first GetBreadcrumbs).
                    assert(it->second.flags & CBreadcrumbsCacheEntry::FRESH);
                    CBreadcrumbsCacheEntry& entry = cacheBreadcrumbs[it->first];
                    entry.coins.swap(it->second.coins);
                    entry.flags = CBreadcrumbsCacheEntry::DIRTY | CBreadcrumbsCacheEntry::FRESH;
                }
            } else {
                if ((itUs->second.flags & CBreadcrumbsCacheEntry::FRESH) && it->second.coins.IsPruned()) {
                    // The grandparent does not have an entry, and the child is
                    // modified and being pruned. This means we can just delete
                    // it from the parent.
                    cacheBreadcrumbs.erase(itUs);
                } else {
                    // A normal modification.
                    itUs->second.coins.swap(it->second.coins);
                    itUs->second.flags |= CBreadcrumbsCacheEntry::DIRTY;
                }
            }
        }
        CBreadcrumbsMap::iterator itOld = it++;
        mapBreadcrumbs.erase(itOld);
    }
    hashBlock = hashBlockIn;
    return true;
}

bool CBreadcrumbsViewCache::Flush() {
    bool fOk = base->BatchWrite(cacheBreadcrumbs, hashBlock);
    cacheBreadcrumbs.clear();
    return fOk;
}

unsigned int CBreadcrumbsViewCache::GetCacheSize() const {
    return cacheBreadcrumbs.size();
}

const CTxOut &CBreadcrumbsViewCache::GetOutputFor(const CTxIn& input) const
{
    const CBreadcrumbs* coins = AccessBreadcrumbs(input.prevout.hash);
    assert(coins && coins->IsAvailable(input.prevout.n));
    return coins->vout[input.prevout.n];
}

CAmount CBreadcrumbsViewCache::GetValueIn(const CTransaction& tx) const
{
    if (tx.IsBreadcrumbBase())
        return 0;

    CAmount nResult = 0;
    for (unsigned int i = 0; i < tx.vin.size(); i++)
        nResult += GetOutputFor(tx.vin[i]).nValue;

    return nResult;
}

bool CBreadcrumbsViewCache::HaveInputs(const CTransaction& tx) const
{
    if (!tx.IsBreadcrumbBase()) {
        for (unsigned int i = 0; i < tx.vin.size(); i++) {
            const COutPoint &prevout = tx.vin[i].prevout;
            const CBreadcrumbs* coins = AccessBreadcrumbs(prevout.hash);
            if (!coins || !coins->IsAvailable(prevout.n)) {
                return false;
            }
        }
    }
    return true;
}

double CBreadcrumbsViewCache::GetPriority(const CTransaction &tx, int nHeight) const
{
    if (tx.IsBreadcrumbBase())
        return 0.0;
    double dResult = 0.0;
    BOOST_FOREACH(const CTxIn& txin, tx.vin)
    {
        const CBreadcrumbs* coins = AccessBreadcrumbs(txin.prevout.hash);
        assert(coins);
        if (!coins->IsAvailable(txin.prevout.n)) continue;
        if (coins->nHeight < nHeight) {
            dResult += coins->vout[txin.prevout.n].nValue * (nHeight-coins->nHeight);
        }
    }
    return tx.ComputePriority(dResult);
}

CBreadcrumbsModifier::CBreadcrumbsModifier(CBreadcrumbsViewCache& cache_, CBreadcrumbsMap::iterator it_) : cache(cache_), it(it_) {
    assert(!cache.hasModifier);
    cache.hasModifier = true;
}

CBreadcrumbsModifier::~CBreadcrumbsModifier()
{
    assert(cache.hasModifier);
    cache.hasModifier = false;
    it->second.coins.Cleanup();
    if ((it->second.flags & CBreadcrumbsCacheEntry::FRESH) && it->second.coins.IsPruned()) {
        cache.cacheBreadcrumbs.erase(it);
    }
}
