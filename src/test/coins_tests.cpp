// Copyright (c) 2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "coins.h"
#include "random.h"
#include "uint256.h"

#include <vector>
#include <map>

#include <boost/test/unit_test.hpp>

namespace
{
class CBreadcrumbsViewTest : public CBreadcrumbsView
{
    uint256 hashBestBlock_;
    std::map<uint256, CBreadcrumbs> map_;

public:
    bool GetBreadcrumbs(const uint256& txid, CBreadcrumbs& coins) const
    {
        std::map<uint256, CBreadcrumbs>::const_iterator it = map_.find(txid);
        if (it == map_.end()) {
            return false;
        }
        coins = it->second;
        if (coins.IsPruned() && insecure_rand() % 2 == 0) {
            // Randomly return false in case of an empty entry.
            return false;
        }
        return true;
    }

    bool HaveBreadcrumbs(const uint256& txid) const
    {
        CBreadcrumbs coins;
        return GetBreadcrumbs(txid, coins);
    }

    uint256 GetBestBlock() const { return hashBestBlock_; }

    bool BatchWrite(CBreadcrumbsMap& mapBreadcrumbs, const uint256& hashBlock)
    {
        for (CBreadcrumbsMap::iterator it = mapBreadcrumbs.begin(); it != mapBreadcrumbs.end(); ) {
            map_[it->first] = it->second.coins;
            if (it->second.coins.IsPruned() && insecure_rand() % 3 == 0) {
                // Randomly delete empty entries on write.
                map_.erase(it->first);
            }
            mapBreadcrumbs.erase(it++);
        }
        mapBreadcrumbs.clear();
        hashBestBlock_ = hashBlock;
        return true;
    }

    bool GetStats(CBreadcrumbsStats& stats) const { return false; }
};
}

BOOST_AUTO_TEST_SUITE(coins_tests)

static const unsigned int NUM_SIMULATION_ITERATIONS = 40000;

// This is a large randomized insert/remove simulation test on a variable-size
// stack of caches on top of CBreadcrumbsViewTest.
//
// It will randomly create/update/delete CBreadcrumbs entries to a tip of caches, with
// txids picked from a limited list of random 256-bit hashes. Occasionally, a
// new tip is added to the stack of caches, or the tip is flushed and removed.
//
// During the process, booleans are kept to make sure that the randomized
// operation hits all branches.
BOOST_AUTO_TEST_CASE(coins_cache_simulation_test)
{
    // Various coverage trackers.
    bool removed_all_caches = false;
    bool reached_4_caches = false;
    bool added_an_entry = false;
    bool removed_an_entry = false;
    bool updated_an_entry = false;
    bool found_an_entry = false;
    bool missed_an_entry = false;

    // A simple map to track what we expect the cache stack to represent.
    std::map<uint256, CBreadcrumbs> result;

    // The cache stack.
    CBreadcrumbsViewTest base; // A CBreadcrumbsViewTest at the bottom.
    std::vector<CBreadcrumbsViewCache*> stack; // A stack of CBreadcrumbsViewCaches on top.
    stack.push_back(new CBreadcrumbsViewCache(&base)); // Start with one cache.

    // Use a limited set of random transaction ids, so we do test overwriting entries.
    std::vector<uint256> txids;
    txids.resize(NUM_SIMULATION_ITERATIONS / 8);
    for (unsigned int i = 0; i < txids.size(); i++) {
        txids[i] = GetRandHash();
    }

    for (unsigned int i = 0; i < NUM_SIMULATION_ITERATIONS; i++) {
        // Do a random modification.
        {
            uint256 txid = txids[insecure_rand() % txids.size()]; // txid we're going to modify in this iteration.
            CBreadcrumbs& coins = result[txid];
            CBreadcrumbsModifier entry = stack.back()->ModifyBreadcrumbs(txid);
            BOOST_CHECK(coins == *entry);
            if (insecure_rand() % 5 == 0 || coins.IsPruned()) {
                if (coins.IsPruned()) {
                    added_an_entry = true;
                } else {
                    updated_an_entry = true;
                }
                coins.nVersion = insecure_rand();
                coins.vout.resize(1);
                coins.vout[0].nValue = insecure_rand();
                *entry = coins;
            } else {
                coins.Clear();
                entry->Clear();
                removed_an_entry = true;
            }
        }

        // Once every 1000 iterations and at the end, verify the full cache.
        if (insecure_rand() % 1000 == 1 || i == NUM_SIMULATION_ITERATIONS - 1) {
            for (std::map<uint256, CBreadcrumbs>::iterator it = result.begin(); it != result.end(); it++) {
                const CBreadcrumbs* coins = stack.back()->AccessBreadcrumbs(it->first);
                if (coins) {
                    BOOST_CHECK(*coins == it->second);
                    found_an_entry = true;
                } else {
                    BOOST_CHECK(it->second.IsPruned());
                    missed_an_entry = true;
                }
            }
        }

        if (insecure_rand() % 100 == 0) {
            // Every 100 iterations, change the cache stack.
            if (stack.size() > 0 && insecure_rand() % 2 == 0) {
                stack.back()->Flush();
                delete stack.back();
                stack.pop_back();
            }
            if (stack.size() == 0 || (stack.size() < 4 && insecure_rand() % 2)) {
                CBreadcrumbsView* tip = &base;
                if (stack.size() > 0) {
                    tip = stack.back();
                } else {
                    removed_all_caches = true;
                }
                stack.push_back(new CBreadcrumbsViewCache(tip));
                if (stack.size() == 4) {
                    reached_4_caches = true;
                }
            }
        }
    }

    // Clean up the stack.
    while (stack.size() > 0) {
        delete stack.back();
        stack.pop_back();
    }

    // Verify coverage.
    BOOST_CHECK(removed_all_caches);
    BOOST_CHECK(reached_4_caches);
    BOOST_CHECK(added_an_entry);
    BOOST_CHECK(removed_an_entry);
    BOOST_CHECK(updated_an_entry);
    BOOST_CHECK(found_an_entry);
    BOOST_CHECK(missed_an_entry);
}

BOOST_AUTO_TEST_SUITE_END()
