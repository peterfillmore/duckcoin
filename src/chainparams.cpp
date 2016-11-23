// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "random.h"
#include "util.h"
#include "utilstrencodings.h"
#include <assert.h>

#include <boost/assign/list_of.hpp>

using namespace std;
using namespace boost::assign;

//function to generate the genesis hash
//bool checkwork(uint256 hash, unsigned int nBits, uint256 proofofworklimit)
//{
//    bool fNegative;
//    bool fOverflow;
//    uint256 bnTarget;
//    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);
//
//    // Check range
//    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > proofofworklimit)
//        return error("CheckProofOfWork() : nBits below minimum work");
//
//    // Check proof of work matches claimed amount
//    if (hash > bnTarget)
//        return error("CheckProofOfWork() : hash doesn't match nBits");
//
//    return true;
//}

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

/**
 * Main network
 */

//! Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress> &vSeedsOut, const SeedSpec6 *data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7*24*60*60;
    for (unsigned int i = 0; i < count; i++)
    {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */
static Checkpoints::MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (10, uint256("0x2b49c3285b6a4fcbcc53552942cd6eb5a0a326d6cb8f7dcbc52b4c10d0a70085"))
        (20, uint256("0xae0e6fbe2a4f4c8ed8b96e8e5ac2cefde18304e26d0f50cf56c5b7cc7d5a525b"))
        (40, uint256("0xa8f22114cdfd5d11f338f2e85ad773688d9ac307f73e19611394a26ca0dfda23"))
	    (80, uint256("0x003d25bf102487fcd462d39139c67115337e98bbe5f7c830c220d50c76fdd7a5"))
        (160, uint256("0x47d161410589c990ad568ae7d32bd39487e9592d01b2324c9336a3fc5f7cc657"))
        (320, uint256("0x11254a91329bed86af0b4e29676ad16df5e9f7bc9642689e480a5550c912ebd1"))
        (640, uint256("0x033010a08213d500d89b684686a6e918eac14d54c9806237ce6fc9db8a1f60b1"))
        (1280, uint256("0x1052a2e991b42ada79af110b8ff4b2959f229d24ba50f16ab7a8b047cbd02c4b"))
        (2560, uint256("0x0b988529a0bb027cef28ab67618afbc7754b42fde8780eb98d1a61a5a0e5935f"))
        (5120, uint256("0x18ce85996d42f1e5ecda0cd6a7b41fab188e434fa1c2a656065647d8c91cfb44")) 
       ;
static const Checkpoints::CCheckpointData data = {
            &mapCheckpoints,
            1479008915, // * UNIX timestamp of last checkpoint block
            5120,       // * total number of transactions between genesis and last checkpoint
            100         //   (the tx=... number in the SetBestChain debug.log lines)
    };

static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        (0,uint256("0x"))
        ;
static const Checkpoints::CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        //1476932364,
        //0,
        //0 
    };

static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
        boost::assign::map_list_of
        (0,uint256("0x00233f3b65f7b654837cfd0210b9e34689023a14a0a158c808c4d26b4998ca08"))
        ;
static const Checkpoints::CCheckpointData dataRegtest = {
        &mapCheckpointsRegtest,
        0,
        0,
        0
    };

class CMainParams : public CChainParams {
public:
    CMainParams() {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";
        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0x5c; //\_0<
        pchMessageStart[1] = 0x5f;
        pchMessageStart[2] = 0x30;
        pchMessageStart[3] = 0x3c;
	vAlertPubKey=ParseHex("044e15b3a570f1531002be0f029da73cc6309100ebeb7c67aebb3a75e87cf712fb28ee243688ee8134cee119fdf6b860e9274028bfb62be8c6247e111d29dcf550");
        nDefaultPort = 20811;
        bnProofOfWorkLimit = ~uint256(0) >> 16;
        //bnProofOfWorkLimit = ~uint256(0) >> 20;
        nSubsidyHalvingInterval = 84000;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 0;
        nTargetTimespan = 60 * 60; // 1 hour
        nTargetSpacing = 60; // 1 minutes
        nMaxTipAge = 24 * 60 * 60;

        /**
         * Build the genesis block. Note that the output of the genesis coinbase cannot
         * be spent as it did not originally exist in the database.
         *
         * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
         *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
         *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
         *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
         *   vMerkleTree: 4a5e1e
         */
        const char* pszTimestamp = "Telegraph 17/10/2016 Adorable emotional support duck a hit on US flight";
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 50 * BREADCRUMB;
	txNew.vout[0].scriptPubKey=CScript()<<ParseHex("040fdf68e80058366728065a74e12edb03f82f5772a2a1fed8663262c3eac2a8c006e94bd76b0684a6d4210fba303081acc5b974f65f42e4e2ce45c81c50e3be6e")<<OP_CHECKSIG;
	genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nBits    = 0x1e5c303c;
 	genesis.nNonce = 525359; 

        hashGenesisBlock = genesis.GetHash();
		//hashGenesisBlock = uint256("0x01");
		//uint256 thash;
        	//uint256 hashTarget = uint256().SetCompact(genesis.nBits); 
		//uint256 powval = genesis.GetPoWHash();	
		//while(true)
        	//{
        	//    //thash = genesis.GetHash();
        	//    powval = genesis.GetPoWHash();
            	//    if(checkwork(powval, genesis.nBits,hashTarget))
		//		//if(checkwork(powval, genesis.nBits,bnProofOfWorkLimit))
        	//        		break;
        	//    if ((genesis.nNonce & 0xFFF) == 0)
        	//    {
        	//        printf("nonce %08X: hash = %s (target = %s)\n", genesis.nNonce, thash.ToString().c_str(), hashTarget.ToString().c_str());
        	//    }
        	//    ++genesis.nNonce;
        	//    if (genesis.nNonce == 0)
        	//    {
        	//        printf("NONCE WRAPPED, incrementing time\n");
        	//        ++genesis.nTime;
        	//    }
        	//}
        	//printf("genesis.GetPoWHash = %s \n", genesis.GetPoWHash());	
		//printf("genesis.nTime = %u \n", genesis.nTime);
        	//printf("genesis.nNonce = %u \n", genesis.nNonce);
        	//printf("genesis.GetHash = %s\n", genesis.GetHash().ToString().c_str());
		//printf("genesis.MerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
	
	    assert(hashGenesisBlock == uint256("0x1761527fd393fd6306ecb1552ac9a632ba1a33846f7b0b6609697e53ec73344c"));
        assert(genesis.hashMerkleRoot == uint256("0xc12c019a9471f59ea0bae289721b39a44cd7bd1c96739b592afe4f7d25012c2c"));

        vFixedSeeds.clear();
        vSeeds.clear(); 
        vSeeds.push_back(CDNSSeedData("www.xn--ss8hl1f.tk", "188.166.233.145"));

        base58Prefixes[PUBKEY_ADDRESS] = {12};
        base58Prefixes[SCRIPT_ADDRESS] = {5};
        base58Prefixes[SECRET_KEY] = {175};
        base58Prefixes[EXT_PUBLIC_KEY] = {0x44, 0x13, 0x82, 0x40};
        base58Prefixes[EXT_SECRET_KEY] = {0x44, 0x13, 0x17, 0x45};

        //convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        fRequireRPCPassword = true;
        fMiningRequiresPeers = false;
        fAllowMinDifficultyBlocks = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fSkipProofOfWorkCheck = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        // Duckcoin: Mainnet v2 enforced as of block 710k
        nEnforceV2AfterHeight = 710000;
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return data;
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";
        pchMessageStart[0] = 0x5d;
        pchMessageStart[1] = 0x60;
        pchMessageStart[2] = 0x31;
        pchMessageStart[3] = 0x3d;
	vAlertPubKey=ParseHex("04826bbca16397bc0720ede747faccc5a271c64814e3ff60dca294c23eb9581cfce66d6d1e3ac58659cb1ab4578c4da7a801e779e8fba2228e2674fbc636f8d4c5");
	nDefaultPort = 20911;
        nEnforceBlockUpgradeMajority = 51;
        nRejectBlockOutdatedMajority = 75;
        nToCheckBlockUpgradeMajority = 100;
        nMinerThreads = 0;
        nTargetTimespan = 60 * 60; // 1 hour
        nTargetSpacing = 60; // 1 minutes
        nMaxTipAge = 0x7fffffff;

        //! Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1476932364;
        genesis.nNonce = 61323;
        hashGenesisBlock = genesis.GetHash();
        //hashGenesisBlock = uint256("0x01");
	//if (true && genesis.GetHash() != hashGenesisBlock)
        //{
        //    printf("recalculating params for mainnet.\n");
        //    // deliberately empty for loop finds nonce value.
        //    for(genesis.nNonce == 0; genesis.GetHash() > bnProofOfWorkLimit; genesis.nNonce++){ } 
        //    printf("new mainnet genesis merkle root: %s\n", genesis.hashMerkleRoot.ToString().c_str());
        //    printf("genesis.nNonce = %u \n", genesis.nNonce);
        //    printf("new mainnet genesis hash: %s\n", genesis.GetHash().ToString().c_str());
        //}//checkwork(hashGenesisBlock, genesis.nBits);
		//uint256 thash;
        	//uint256 hashTarget = uint256().SetCompact(genesis.nBits); 
		//uint256 powval = genesis.GetPoWHash();	
		//while(true)
        	//{
        	//    //thash = scrypt_blockhash(BEGIN(block.nVersion));
        	//    thash = genesis.GetHash();
        	//    powval = genesis.GetPoWHash();
        	//    //if (thash <= hashTarget){
        	//	if(checkwork(powval, genesis.nBits, hashTarget))
        	//        	break;
		//    //}
        	//    if ((genesis.nNonce & 0xFFF) == 0)
        	//    {
        	//        printf("nonce %08X: hash = %s (target = %s)\n", genesis.nNonce, thash.ToString().c_str(), hashTarget.ToString().c_str());
        	//    }
        	//    ++genesis.nNonce;
        	//    if (genesis.nNonce == 0)
        	//    {
        	//        printf("NONCE WRAPPED, incrementing time\n");
        	//        ++genesis.nTime;
        	//    }
        	//}
        	//printf("genesis.nTime = %u \n", genesis.nTime);
        	//printf("genesis.nNonce = %u \n", genesis.nNonce);
        	//printf("genesis.GetHash = %s\n", thash.ToString().c_str());
		//printf("genesis.MerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
	assert(hashGenesisBlock == uint256("0x14f3eadd907ac50ce0afa8d89f96ee7a444e8390c5b555dfc89c6bc7131dc6c3"));

        vFixedSeeds.clear();
        vSeeds.clear();
        //vSeeds.push_back(CDNSSeedData("duckcointools.com", "testnet-seed.duckcointools.com"));
        //vSeeds.push_back(CDNSSeedData("xurious.com", "testnet-seed.duk.xurious.com"));
        //vSeeds.push_back(CDNSSeedData("wemine-testnet.com", "dnsseed.wemine-testnet.com"));

        base58Prefixes[PUBKEY_ADDRESS] = {21};
        base58Prefixes[SCRIPT_ADDRESS] = {39};
        base58Prefixes[SECRET_KEY] = {52};
        base58Prefixes[EXT_PUBLIC_KEY] = {0x33, 0x27, 0x74, 0x05};
        base58Prefixes[EXT_SECRET_KEY] = {0x33, 0x10, 0x94, 0x50};

        //convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        fRequireRPCPassword = true;
        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        // Duckcoin: Testnet v2 enforced as of block 400k
        nEnforceV2AfterHeight = 400000;
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataTestnet;
    }
};

    static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams {
public:
    CRegTestParams() {
        networkID = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";
        pchMessageStart[0] = 0x5b;
        pchMessageStart[1] = 0x5d;
        pchMessageStart[2] = 0x2a;
        pchMessageStart[3] = 0x3b;
        nSubsidyHalvingInterval = 150;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 1;
        nTargetTimespan = 3.5 * 24 * 60 * 60; // 3.5 days
        nTargetSpacing = 2.5 * 60; // 2.5 minutes
        bnProofOfWorkLimit = ~uint256(0) >> 1;
        nMaxTipAge = 24 * 60 * 60;
        genesis.nTime = 1476933599;
        genesis.nBits = 0x1f5f303d;
        genesis.nNonce = 150;
        hashGenesisBlock = genesis.GetHash();
        nDefaultPort = 21011;
        	//uint256 thash;
        	//uint256 hashTarget = uint256().SetCompact(genesis.nBits); 
		//uint256 powval = genesis.GetPoWHash();	
		//while(true)
        	//{
        	//    //thash = scrypt_blockhash(BEGIN(block.nVersion));
        	//    thash = genesis.GetHash();
        	//    powval = genesis.GetPoWHash();
        	//    //if (thash <= hashTarget){
        	//	if(checkwork(powval, genesis.nBits, hashTarget))
        	//        	break;
		//    //}
        	//    if ((genesis.nNonce & 0xFFF) == 0)
        	//    {
        	//        printf("nonce %08X: hash = %s (target = %s)\n", genesis.nNonce, thash.ToString().c_str(), hashTarget.ToString().c_str());
        	//    }
        	//    ++genesis.nNonce;
        	//    if (genesis.nNonce == 0)
        	//    {
        	//        printf("NONCE WRAPPED, incrementing time\n");
        	//        ++genesis.nTime;
        	//    }
        	//}
        	//printf("genesis.nTime = %u \n", genesis.nTime);
        	//printf("genesis.nNonce = %u \n", genesis.nNonce);
        	//printf("genesis.GetHash = %s\n", thash.ToString().c_str());
		//printf("genesis.MerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
	assert(hashGenesisBlock == uint256("0x0aaae37167fa79a6f56739003d9baa18d9989a4013b0dea43d9c29ce3207b887"));

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

        fRequireRPCPassword = false;
        fMiningRequiresPeers = false;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        // Duckcoin: v2 enforced using Bitcoin's supermajority rule
        nEnforceV2AfterHeight = -1;
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataRegtest;
    }
};
static CRegTestParams regTestParams;

/**
 * Unit test
 */
class CUnitTestParams : public CMainParams, public CModifiableParams {
public:
    CUnitTestParams() {
        networkID = CBaseChainParams::UNITTEST;
        strNetworkID = "unittest";
        nDefaultPort = 21111;
        vFixedSeeds.clear(); //! Unit test mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Unit test mode doesn't have any DNS seeds.

        fRequireRPCPassword = false;
        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fAllowMinDifficultyBlocks = false;
        fMineBlocksOnDemand = true;

        // Duckcoin: v2 enforced using Bitcoin's supermajority rule
        nEnforceV2AfterHeight = -1;
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        // UnitTest share the same checkpoints as MAIN
        return data;
    }

    //! Published setters to allow changing values in unit test cases
    virtual void setSubsidyHalvingInterval(int anSubsidyHalvingInterval)  { nSubsidyHalvingInterval=anSubsidyHalvingInterval; }
    virtual void setEnforceBlockUpgradeMajority(int anEnforceBlockUpgradeMajority)  { nEnforceBlockUpgradeMajority=anEnforceBlockUpgradeMajority; }
    virtual void setRejectBlockOutdatedMajority(int anRejectBlockOutdatedMajority)  { nRejectBlockOutdatedMajority=anRejectBlockOutdatedMajority; }
    virtual void setToCheckBlockUpgradeMajority(int anToCheckBlockUpgradeMajority)  { nToCheckBlockUpgradeMajority=anToCheckBlockUpgradeMajority; }
    virtual void setDefaultConsistencyChecks(bool afDefaultConsistencyChecks)  { fDefaultConsistencyChecks=afDefaultConsistencyChecks; }
    virtual void setAllowMinDifficultyBlocks(bool afAllowMinDifficultyBlocks) {  fAllowMinDifficultyBlocks=afAllowMinDifficultyBlocks; }
    virtual void setSkipProofOfWorkCheck(bool afSkipProofOfWorkCheck) { fSkipProofOfWorkCheck = afSkipProofOfWorkCheck; }
};
static CUnitTestParams unitTestParams;


static CChainParams *pCurrentParams = 0;

CModifiableParams *ModifiableParams()
{
   assert(pCurrentParams);
   assert(pCurrentParams==&unitTestParams);
   return (CModifiableParams*)&unitTestParams;
}

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams &Params(CBaseChainParams::Network network) {
    switch (network) {
        case CBaseChainParams::MAIN:
            return mainParams;
        case CBaseChainParams::TESTNET:
            return testNetParams;
        case CBaseChainParams::REGTEST:
            return regTestParams;
        case CBaseChainParams::UNITTEST:
            return unitTestParams;
        default:
            assert(false && "Unimplemented network");
            return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network) {
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}
