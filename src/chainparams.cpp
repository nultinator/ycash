// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2015-2020 The Zcash Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include "key_io.h"
#include "main.h"
#include "crypto/equihash.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>
#include <optional>
#include <variant>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, const uint256& nNonce, const std::vector<unsigned char>& nSolution, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    // To create a genesis block for a new chain which is Overwintered:
    //   txNew.nVersion = OVERWINTER_TX_VERSION
    //   txNew.fOverwintered = true
    //   txNew.nVersionGroupId = OVERWINTER_VERSION_GROUP_ID
    //   txNew.nExpiryHeight = <default value>
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 520617983 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nSolution = nSolution;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = genesis.BuildMerkleTree();
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database (and is in any case of zero value).
 *
 * >>> from pyblake2 import blake2s
 * >>> 'Zcash' + blake2s(b'The Economist 2016-10-29 Known unknown: Another crypto-currency is born. BTC#436254 0000000000000000044f321997f336d2908cf8c8d6893e88dbf067e2d949487d ETH#2521903 483039a6b6bd8bd05f0584f9a078d075e454925eb71c1f13eaff59b405a721bb DJIA close on 27 Oct 2016: 18,169.68').hexdigest()
 *
 * CBlock(hash=00040fe8, ver=4, hashPrevBlock=00000000000000, hashMerkleRoot=c4eaa5, nTime=1477641360, nBits=1f07ffff, nNonce=4695, vtx=1)
 *   CTransaction(hash=c4eaa5, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff071f0104455a6361736830623963346565663862376363343137656535303031653335303039383462366665613335363833613763616331343161303433633432303634383335643334)
 *     CTxOut(nValue=0.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: c4eaa5
 */
static CBlock CreateGenesisBlock(uint32_t nTime, const uint256& nNonce, const std::vector<unsigned char>& nSolution, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "Zcash0b9c4eef8b7cc417ee5001e3500984b6fea35683a7cac141a043c42064835d34";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nSolution, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

const arith_uint256 maxUint = UintToArith256(uint256S("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        strCurrencyUnits = "YEC";
        bip44CoinType = 347; // As registered in https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        consensus.fCoinbaseMustBeShielded = true;
        consensus.nSubsidySlowStartInterval = 20000;
        consensus.nPreBlossomSubsidyHalvingInterval = Consensus::PRE_BLOSSOM_HALVING_INTERVAL;
        consensus.nPostBlossomSubsidyHalvingInterval = POST_BLOSSOM_HALVING_INTERVAL(Consensus::PRE_BLOSSOM_HALVING_INTERVAL);
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 4000;
        const size_t N = 200, K = 9;
        static_assert(equihash_parameters_acceptable(N, K));
        consensus.nEquihashN = N;
        consensus.nEquihashK = K;
        consensus.powLimit = uint256S("0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 32; // 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // 16% adjustment up
        consensus.nPreBlossomPowTargetSpacing = Consensus::PRE_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPostBlossomPowTargetSpacing = Consensus::POST_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = std::nullopt;
        consensus.fPowNoRetargeting = false;
        consensus.minDifficultyAtYcashFork = false;
        consensus.scaledDifficultyAtYcashFork = true;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170005;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight = 347500;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].hashActivationBlock =
            uint256S("0000000003761c0d0c3974b54bdb425613bbb1eaadd6e70b764de82f195ea243");
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170007;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight = 419200;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].hashActivationBlock =
            uint256S("00000000025a57200d898ac7f21e26bf29028bbe96ec46e05b2c17cc9db9e4f3");
        consensus.vUpgrades[Consensus::UPGRADE_YCASH].nProtocolVersion = 270007;
        consensus.vUpgrades[Consensus::UPGRADE_YCASH].nActivationHeight = 570000;
        consensus.vUpgrades[Consensus::UPGRADE_YCASH].hashActivationBlock =
            uint256S("0000014fbc5917ba8bcacf3336faf588d86b32443aa3a490a587af5750c77ec5");
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 270009;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight = 1100000;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nProtocolVersion = 270011;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nActivationHeight = 1100003;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nProtocolVersion = 270013;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nActivationHeight = 1100006;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nProtocolVersion = 270015;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nProtocolVersion = 0x7FFFFFFF;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        consensus.nFundingPeriodLength = consensus.nPostBlossomSubsidyHalvingInterval / 48;

        // guarantees the first 2 characters, when base58 encoded, are "s1"
        keyConstants.base58Prefixes[PUBKEY_ADDRESS]     = {0x1C,0x28};
        // guarantees the first 2 characters, when base58 encoded, are "s3"
        keyConstants.base58Prefixes[SCRIPT_ADDRESS]     = {0x1C,0x2C};
        // the first character, when base58 encoded, is "5" or "K" or "L" (as in Bitcoin)
        keyConstants.base58Prefixes[SECRET_KEY]         = {0x80};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        keyConstants.base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x88,0xB2,0x1E};
        keyConstants.base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x88,0xAD,0xE4};
        // guarantees the first 2 characters, when base58 encoded, are "yc"
        keyConstants.base58Prefixes[ZCPAYMENT_ADDRESS]  = {0x16,0x36};
        // guarantees the first 4 characters, when base58 encoded, are "ZiVK"
        keyConstants.base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAB,0xD3};
        // guarantees the first 2 characters, when base58 encoded, are "SK"
        keyConstants.base58Prefixes[ZCSPENDING_KEY]     = {0xAB,0x36};

        keyConstants.bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "ys";
        keyConstants.bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "zviews";
        keyConstants.bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "zivks";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-main";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_FVK]         = "zxviews";

        // Legacy (Zcash) prefixes, required for pre-fork founders addresses validation
        // guarantees the first 2 characters, when base58 encoded, are "t1"
        keyConstants.base58Prefixes[LEGACY_PUBKEY_ADDRESS]     = {0x1C,0xB8};
        // guarantees the first 2 characters, when base58 encoded, are "t3"
        keyConstants.base58Prefixes[LEGACY_SCRIPT_ADDRESS]     = {0x1C,0xBD};
        // the first character, when base58 encoded, is "5" or "K" or "L" (as in Bitcoin)
        keyConstants.base58Prefixes[LEGACY_SECRET_KEY]         = {0x80};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        keyConstants.base58Prefixes[LEGACY_EXT_PUBLIC_KEY]     = {0x04,0x88,0xB2,0x1E};
        keyConstants.base58Prefixes[LEGACY_EXT_SECRET_KEY]     = {0x04,0x88,0xAD,0xE4};
        // guarantees the first 2 characters, when base58 encoded, are "zc"
        keyConstants.base58Prefixes[LEGACY_ZCPAYMENT_ADDRESS]  = {0x16,0x9A};
        // guarantees the first 4 characters, when base58 encoded, are "ZiVK"
        keyConstants.base58Prefixes[LEGACY_ZCVIEWING_KEY]      = {0xA8,0xAB,0xD3};
        // guarantees the first 2 characters, when base58 encoded, are "SK"
        keyConstants.base58Prefixes[LEGACY_ZCSPENDING_KEY]     = {0xAB,0x36};

        keyConstants.bech32HRPs[LEGACY_SAPLING_PAYMENT_ADDRESS]      = "zs";
        keyConstants.bech32HRPs[LEGACY_SAPLING_FULL_VIEWING_KEY]     = "zviews";
        keyConstants.bech32HRPs[LEGACY_SAPLING_INCOMING_VIEWING_KEY] = "zivks";
        keyConstants.bech32HRPs[LEGACY_SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-main";
        keyConstants.bech32HRPs[LEGACY_SAPLING_EXTENDED_FVK]         = "zxviews";

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0000000000000000000000000000000000000000000000000152d608a8c7cab7");

        /**
         * The message start string should be awesome! ⓩ❤
         */
        pchMessageStart[0] = 0x24;
        pchMessageStart[1] = 0xe9;
        pchMessageStart[2] = 0x27;
        pchMessageStart[3] = 0x64;
        vAlertPubKey = ParseHex("04ba73cc5c962a359005140276ae60106afbed978d3824d0ad8d77195357e0493dad248688e3f469f8183de9582d984183f94f9a2e59198ffe4c5376e4d720daec");
        nDefaultPort = 8833;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(
            1477641360,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000001257"),
            ParseHex("000a889f00854b8665cd555f4656f68179d31ccadc1b1f7fb0952726313b16941da348284d67add4686121d4e3d930160c1348d8191c25f12b267a6a9c131b5031cbf8af1f79c9d513076a216ec87ed045fa966e01214ed83ca02dc1797270a454720d3206ac7d931a0a680c5c5e099057592570ca9bdf6058343958b31901fce1a15a4f38fd347750912e14004c73dfe588b903b6c03166582eeaf30529b14072a7b3079e3a684601b9b3024054201f7440b0ee9eb1a7120ff43f713735494aa27b1f8bab60d7f398bca14f6abb2adbf29b04099121438a7974b078a11635b594e9170f1086140b4173822dd697894483e1c6b4e8b8dcd5cb12ca4903bc61e108871d4d915a9093c18ac9b02b6716ce1013ca2c1174e319c1a570215bc9ab5f7564765f7be20524dc3fdf8aa356fd94d445e05ab165ad8bb4a0db096c097618c81098f91443c719416d39837af6de85015dca0de89462b1d8386758b2cf8a99e00953b308032ae44c35e05eb71842922eb69797f68813b59caf266cb6c213569ae3280505421a7e3a0a37fdf8e2ea354fc5422816655394a9454bac542a9298f176e211020d63dee6852c40de02267e2fc9d5e1ff2ad9309506f02a1a71a0501b16d0d36f70cdfd8de78116c0c506ee0b8ddfdeb561acadf31746b5a9dd32c21930884397fb1682164cb565cc14e089d66635a32618f7eb05fe05082b8a3fae620571660a6b89886eac53dec109d7cbb6930ca698a168f301a950be152da1be2b9e07516995e20baceebecb5579d7cdbc16d09f3a50cb3c7dffe33f26686d4ff3f8946ee6475e98cf7b3cf9062b6966e838f865ff3de5fb064a37a21da7bb8dfd2501a29e184f207caaba364f36f2329a77515dcb710e29ffbf73e2bbd773fab1f9a6b005567affff605c132e4e4dd69f36bd201005458cfbd2c658701eb2a700251cefd886b1e674ae816d3f719bac64be649c172ba27a4fd55947d95d53ba4cbc73de97b8af5ed4840b659370c556e7376457f51e5ebb66018849923db82c1c9a819f173cccdb8f3324b239609a300018d0fb094adf5bd7cbb3834c69e6d0b3798065c525b20f040e965e1a161af78ff7561cd874f5f1b75aa0bc77f720589e1b810f831eac5073e6dd46d00a2793f70f7427f0f798f2f53a67e615e65d356e66fe40609a958a05edb4c175bcc383ea0530e67ddbe479a898943c6e3074c6fcc252d6014de3a3d292b03f0d88d312fe221be7be7e3c59d07fa0f2f4029e364f1f355c5d01fa53770d0cd76d82bf7e60f6903bc1beb772e6fde4a70be51d9c7e03c8d6d8dfb361a234ba47c470fe630820bbd920715621b9fbedb49fcee165ead0875e6c2b1af16f50b5d6140cc981122fcbcf7c5a4e3772b3661b628e08380abc545957e59f634705b1bbde2f0b4e055a5ec5676d859be77e20962b645e051a880fddb0180b4555789e1f9344a436a84dc5579e2553f1e5fb0a599c137be36cabbed0319831fea3fddf94ddc7971e4bcf02cdc93294a9aab3e3b13e3b058235b4f4ec06ba4ceaa49d675b4ba80716f3bc6976b1fbf9c8bf1f3e3a4dc1cd83ef9cf816667fb94f1e923ff63fef072e6a19321e4812f96cb0ffa864da50ad74deb76917a336f31dce03ed5f0303aad5e6a83634f9fcc371096f8288b8f02ddded5ff1bb9d49331e4a84dbe1543164438fde9ad71dab024779dcdde0b6602b5ae0a6265c14b94edd83b37403f4b78fcd2ed555b596402c28ee81d87a909c4e8722b30c71ecdd861b05f61f8b1231795c76adba2fdefa451b283a5d527955b9f3de1b9828e7b2e74123dd47062ddcc09b05e7fa13cb2212a6fdbc65d7e852cec463ec6fd929f5b8483cf3052113b13dac91b69f49d1b7d1aec01c4a68e41ce157"),
            0x1f07ffff, 4, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00040fe8ec8471911baa1db1266ea15dd06b4a8a5c453883c000b031973dce08"));
        assert(genesis.hashMerkleRoot == uint256S("0xc4eaa58879081de3c24a7b117ed2b28300e7ec4c4c1dff1d3f1268b7857a4ddb"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("ycash.xyz", "seed.ycash.xyz")); // Ycash Seed

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, consensus.hashGenesisBlock)
            (2500, uint256S("0x00000006dc968f600be11a86cbfbf7feb61c7577f45caced2e82b6d261d19744"))
            (15000, uint256S("0x00000000b6bc56656812a5b8dcad69d6ad4446dec23b5ec456c18641fb5381ba"))
            (67500, uint256S("0x000000006b366d2c1649a6ebb4787ac2b39c422f451880bc922e3a6fbd723616"))
            (100000, uint256S("0x000000001c5c82cd6baccfc0879e3830fd50d5ede17fa2c37a9a253c610eb285"))
            (133337, uint256S("0x0000000002776ccfaf06cc19857accf3e20c01965282f916b8a886e3e4a05be9"))
            (180000, uint256S("0x000000001205b742eac4a1b3959635bdf8aeada078d6a996df89740f7b54351d"))
            (222222, uint256S("0x000000000cafb9e56445a6cabc8057b57ee6fcc709e7adbfa195e5c7fac61343"))
            (270000, uint256S("0x00000000025c1cfa0258e33ab050aaa9338a3d4aaa3eb41defefc887779a9729"))
            (304600, uint256S("0x00000000028324e022a45014c4a4dc51e95d41e6bceb6ad554c5b65d5cea3ea5"))
            (410100, uint256S("0x0000000002c565958f783a24a4ac17cde898ff525e75ed9baf66861b0b9fcada"))
            (497000, uint256S("0x0000000000abd333f0acca6ffdf78a167699686d6a7d25c33fca5f295061ffff"))
            (525000, uint256S("0x0000000001a36c500378be8862d9bf1bea8f1616da6e155971b608139cc7e39b"))
            (572760, uint256S("0x00000008db657f58222e38e354c18ccbb6c74cf525ef4f3a95f0f8a324a3166d"))
            (980000, uint256S("0x00000510ccc6bae2ddb38b9313ffb9686397f6cd0dee243462baeaf8911d0791"))
            (1035353, uint256S("0x00000543378a75f173914390c672838c9fbd9dc86ae647ffae18ed6b626edb30")),
            1633786161,     // * UNIX timestamp of last checkpoint block
            6114050,        // * total number of transactions between genesis and last checkpoint
            3400            // * estimated number of transactions per day after checkpoint
                            //   total number of tx / (checkpoint block height / (24 * 24))
        };

        // Hardcoded fallback value for the Sprout shielded value pool balance
        // for nodes that have not reindexed since the introduction of monitoring
        // in #2795.
        nSproutValuePoolCheckpointHeight = 520633;
        nSproutValuePoolCheckpointBalance = 22145062442933;
        fZIP209Enabled = true;
        hashSproutValuePoolCheckpointBlock = uint256S("0000000000c7b46b6bc04b4cbf87d8bb08722aebd51232619b214f7273f8460e");

        // Pre-fork founders reward script expects a vector of 2-of-3 multisig addresses
        vFoundersRewardAddress = {
            "t3Vz22vK5z2LcKEdg16Yv4FFneEL1zg9ojd", /* main-index: 0*/
            "t3cL9AucCajm3HXDhb5jBnJK2vapVoXsop3", /* main-index: 1*/
            "t3fqvkzrrNaMcamkQMwAyHRjfDdM2xQvDTR", /* main-index: 2*/
            "t3TgZ9ZT2CTSK44AnUPi6qeNaHa2eC7pUyF", /* main-index: 3*/
            "t3SpkcPQPfuRYHsP5vz3Pv86PgKo5m9KVmx", /* main-index: 4*/
            "t3Xt4oQMRPagwbpQqkgAViQgtST4VoSWR6S", /* main-index: 5*/
            "t3ayBkZ4w6kKXynwoHZFUSSgXRKtogTXNgb", /* main-index: 6*/
            "t3adJBQuaa21u7NxbR8YMzp3km3TbSZ4MGB", /* main-index: 7*/
            "t3K4aLYagSSBySdrfAGGeUd5H9z5Qvz88t2", /* main-index: 8*/
            "t3RYnsc5nhEvKiva3ZPhfRSk7eyh1CrA6Rk", /* main-index: 9*/
            "t3Ut4KUq2ZSMTPNE67pBU5LqYCi2q36KpXQ", /* main-index: 10*/
            "t3ZnCNAvgu6CSyHm1vWtrx3aiN98dSAGpnD", /* main-index: 11*/
            "t3fB9cB3eSYim64BS9xfwAHQUKLgQQroBDG", /* main-index: 12*/
            "t3cwZfKNNj2vXMAHBQeewm6pXhKFdhk18kD", /* main-index: 13*/
            "t3YcoujXfspWy7rbNUsGKxFEWZqNstGpeG4", /* main-index: 14*/
            "t3bLvCLigc6rbNrUTS5NwkgyVrZcZumTRa4", /* main-index: 15*/
            "t3VvHWa7r3oy67YtU4LZKGCWa2J6eGHvShi", /* main-index: 16*/
            "t3eF9X6X2dSo7MCvTjfZEzwWrVzquxRLNeY", /* main-index: 17*/
            "t3esCNwwmcyc8i9qQfyTbYhTqmYXZ9AwK3X", /* main-index: 18*/
            "t3M4jN7hYE2e27yLsuQPPjuVek81WV3VbBj", /* main-index: 19*/
            "t3gGWxdC67CYNoBbPjNvrrWLAWxPqZLxrVY", /* main-index: 20*/
            "t3LTWeoxeWPbmdkUD3NWBquk4WkazhFBmvU", /* main-index: 21*/
            "t3P5KKX97gXYFSaSjJPiruQEX84yF5z3Tjq", /* main-index: 22*/
            "t3f3T3nCWsEpzmD35VK62JgQfFig74dV8C9", /* main-index: 23*/
            "t3Rqonuzz7afkF7156ZA4vi4iimRSEn41hj", /* main-index: 24*/
            "t3fJZ5jYsyxDtvNrWBeoMbvJaQCj4JJgbgX", /* main-index: 25*/
            "t3Pnbg7XjP7FGPBUuz75H65aczphHgkpoJW", /* main-index: 26*/
            "t3WeKQDxCijL5X7rwFem1MTL9ZwVJkUFhpF", /* main-index: 27*/
            "t3Y9FNi26J7UtAUC4moaETLbMo8KS1Be6ME", /* main-index: 28*/
            "t3aNRLLsL2y8xcjPheZZwFy3Pcv7CsTwBec", /* main-index: 29*/
            "t3gQDEavk5VzAAHK8TrQu2BWDLxEiF1unBm", /* main-index: 30*/
            "t3Rbykhx1TUFrgXrmBYrAJe2STxRKFL7G9r", /* main-index: 31*/
            "t3aaW4aTdP7a8d1VTE1Bod2yhbeggHgMajR", /* main-index: 32*/
            "t3YEiAa6uEjXwFL2v5ztU1fn3yKgzMQqNyo", /* main-index: 33*/
            "t3g1yUUwt2PbmDvMDevTCPWUcbDatL2iQGP", /* main-index: 34*/
            "t3dPWnep6YqGPuY1CecgbeZrY9iUwH8Yd4z", /* main-index: 35*/
            "t3QRZXHDPh2hwU46iQs2776kRuuWfwFp4dV", /* main-index: 36*/
            "t3enhACRxi1ZD7e8ePomVGKn7wp7N9fFJ3r", /* main-index: 37*/
            "t3PkLgT71TnF112nSwBToXsD77yNbx2gJJY", /* main-index: 38*/
            "t3LQtHUDoe7ZhhvddRv4vnaoNAhCr2f4oFN", /* main-index: 39*/
            "t3fNcdBUbycvbCtsD2n9q3LuxG7jVPvFB8L", /* main-index: 40*/
            "t3dKojUU2EMjs28nHV84TvkVEUDu1M1FaEx", /* main-index: 41*/
            "t3aKH6NiWN1ofGd8c19rZiqgYpkJ3n679ME", /* main-index: 42*/
            "t3MEXDF9Wsi63KwpPuQdD6by32Mw2bNTbEa", /* main-index: 43*/
            "t3WDhPfik343yNmPTqtkZAoQZeqA83K7Y3f", /* main-index: 44*/
            "t3PSn5TbMMAEw7Eu36DYctFezRzpX1hzf3M", /* main-index: 45*/
            "t3R3Y5vnBLrEn8L6wFjPjBLnxSUQsKnmFpv", /* main-index: 46*/
            "t3Pcm737EsVkGTbhsu2NekKtJeG92mvYyoN", /* main-index: 47*/
        };

        assert(vFoundersRewardAddress.size() <= consensus.GetLastFoundersRewardBlockHeight(0));

        vYcashFoundersRewardAddress = {
            "s1hfWJ4ej1H3s8XCUb7YnrU68K64AsGVUHE", "s1iZaRoYtafWspcieQxg6hhaU4DfZyAdGQf", "s1RSr6xec6Cc98emM4cdq45rkVekHMjRWbw", "s1RsqYeweoKVepivLPLsiajE8c6khu5UKhS", 
            "s1MNmqMWyV4nMWE4oDb1nqJs7haJrv9QTKp", "s1RP95ESdcu33gMtU7deLW9TP6yDZncjRQ7", "s1h8W7xQbiU8Zxu21Zcg82NByjkWMcEbNtX", "s1PVcdfcrJrDCmXxgSTGuKGSNhwSYZ51XKJ", 
            "s1PkV5nFkgQN4EGuTtEcmm4CxeBVx2L5HHv", "s1jiVSTfMaFUrWnf17BGc416oomHbut58Ue", "s1Zr2KdHtnK2zNSMQDrAVv3KU51mgDbqgwe", "s1QkY6tmBHPZacXPMPmsjP37Kxgs5mcgcAn", 
            "s1Xu76ZmGDENdLFAiuj5iMdp1RA4hWSNieq", "s1bdiEnfBYaEgrt2TmnY3ZHmdhg5AEw9tjN", "s1asM9Ui4U13GjmLoAhvfK6J5QihemQR9Pk", "s1QhTSXYu4K1cTNomN27wiep9WC9HBZjrxJ", 
            "s1j3Ef2qCNjwRAM18BgwsPAZFzZ475BWM5S", "s1QZibiN7iqVCfVBES9Gn7e3o5psxRKtpwE", "s1fdiDZHkzp8K8UajpVwYUdyFeb6jNVyoKv", "s1iMShbVRH1eCGxK2ZoLMDn5o9NcwXkNPVF", 
            "s1YtUXAMt8m31gGeP5m3Y53B1wrMk3FFigJ", "s1gy9aqWUihGRjZa3vqc7136vqTGNAWyefF", "s1NNozrex18HZqcHCGpGoSRkj8hqHLEPaVC", "s1NYNDdqthMf7D7sZbnLGuecDtXb48Ne2bf", 
            "s1P7UJ9Wp7jstJPUbvMSRVFjN8tfQueQSK3", "s1RDeyH7xg8y9veb9XfAmtKzrMTjFS14c4T", "s1NmH6MNXU19xoHjUfpQpb4dEMSy1Wbs9tC", "s1UnFL2yrZapMKmB5EqaBKpogZmnXLUgELB", 
            "s1XT3W1sLFdgmGoecQbPdJbjUDMurW8CFA2", "s1gyVwgangQxLCAcm8VS4SWqXDqeohNg7hd", "s1k3eWbqnbVM1xtZEDc81UbFdwXgXNnbtdH", "s1Wr6eAh3gZWwBVRZcND9YCzdNfR9cARkD4", 
            "s1dtjp2KHWZ6qF2LvgNiEwjJV2dA2c6y75V", "s1cjQf9kmjQdTmnn6mbBaesHMNLt2JqjyEV", "s1URQeusSoi7fkgyAwCshFzobUzmLGH4U3b", "s1Z9YqM2h48HUf8kcSHS89q4Z6Bg9xua3kA", 
            "s1TLmZzMDsDhYfh4vpY7NpRB4kao2UEEqKu", "s1QEWvfC1uifDfi78NY7cArw9xLEja7QAZR", "s1b4kfW9WMUtd2H7X4C64KLzqPWdMPXRMtS", "s1cHTXzCXhKYAX7sY7D8YGcmopjN8Yngoju", 
            "s1QKjMQDeF9FLVo2sL8m11VC4ZA18s61s2K", "s1gV8D561ZpmaZVxG176cQM1bMFMnHLvujE", "s1caQmLCYVDZegcMoBckHD2RXjBh7ikpj2j", "s1Y63AsWsJTk5t5nSZfaFcFWmtfnFUUAu2V", 
            "s1Y2U4GsfZdP9LAbC97GAmSdihBX5FU9gQn", "s1bPYWZMXzyN2ML2vswDiCckmas775QFs2Q", "s1erG25RcWYCiBPbT7khTU4ULhzm8jJZ7pv", "s1kYEiPdFZ3oV389q2MmSYY932qPF1ygVtx", 
        };
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        strCurrencyUnits = "TAY";
        bip44CoinType = 1;
        consensus.fCoinbaseMustBeShielded = true;
        consensus.nSubsidySlowStartInterval = 20000;
        consensus.nPreBlossomSubsidyHalvingInterval = Consensus::PRE_BLOSSOM_HALVING_INTERVAL;
        consensus.nPostBlossomSubsidyHalvingInterval = POST_BLOSSOM_HALVING_INTERVAL(Consensus::PRE_BLOSSOM_HALVING_INTERVAL);
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 400;
        const size_t N = 200, K = 9;
        static_assert(equihash_parameters_acceptable(N, K));
        consensus.nEquihashN = N;
        consensus.nEquihashK = K;
        consensus.powLimit = uint256S("07ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        //consensus.powLimit = uint256S("0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f");
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 32; // 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // 16% adjustment up
        consensus.nPreBlossomPowTargetSpacing = Consensus::PRE_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPostBlossomPowTargetSpacing = Consensus::POST_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = 299187;
        consensus.fPowNoRetargeting = false;
        consensus.minDifficultyAtYcashFork = true;
        consensus.scaledDifficultyAtYcashFork = false;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170003;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight = 207500;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].hashActivationBlock =
            uint256S("0000257c4331b098045023fcfbfa2474681f4564ab483f84e4e1ad078e4acf44");
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170007;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight = 280000;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].hashActivationBlock =
            uint256S("000420e7fcc3a49d729479fb0b560dd7b8617b178a08e9e389620a9d1dd6361a");
        consensus.vUpgrades[Consensus::UPGRADE_YCASH].nProtocolVersion = 270007;
        consensus.vUpgrades[Consensus::UPGRADE_YCASH].nActivationHeight = 510248;   // Around June 7, 2019
        consensus.vUpgrades[Consensus::UPGRADE_YCASH].hashActivationBlock =
            uint256S("0305d164e8f4dc75b9e9a6a15b7b381dbc1c9cb55f1534267be7c125923255c8");
        
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 270008;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight = 661610; // Around October 30, 2021, 2PM GMT

        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nProtocolVersion = 270010;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nActivationHeight = 661622;

        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nProtocolVersion = 270012;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nActivationHeight = 661634;

        consensus.vUpgrades[Consensus::UPGRADE_NU5].nProtocolVersion = 270014;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;


        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nProtocolVersion = 0x7FFFFFFF;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        consensus.nFundingPeriodLength = consensus.nPostBlossomSubsidyHalvingInterval / 48;

        // guarantees the first 2 characters, when base58 encoded, are "sm"
        keyConstants.base58Prefixes[PUBKEY_ADDRESS]     = {0x1C,0x95};
        // guarantees the first 2 characters, when base58 encoded, are "t2"
        keyConstants.base58Prefixes[SCRIPT_ADDRESS]     = {0x1C,0x2A};
        // the first character, when base58 encoded, is "9" or "c" (as in Bitcoin)
        keyConstants.base58Prefixes[SECRET_KEY]         = {0xEF};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        keyConstants.base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};
        keyConstants.base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};
        // guarantees the first 2 characters, when base58 encoded, are "yt"
        keyConstants.base58Prefixes[ZCPAYMENT_ADDRESS]  = {0x16,0x52};
        // guarantees the first 4 characters, when base58 encoded, are "ZiVt"
        keyConstants.base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAC,0x0C};
        // guarantees the first 2 characters, when base58 encoded, are "ST"
        keyConstants.base58Prefixes[ZCSPENDING_KEY]     = {0xAC,0x08};

        keyConstants.bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "ytestsapling";
        keyConstants.bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "zviewtestsapling";
        keyConstants.bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "zivktestsapling";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-test";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_FVK]         = "zxviewtestsapling";

        // Legacy (Zcash) prefixes, required for pre-fork founders addresses validation
        // guarantees the first 2 characters, when base58 encoded, are "tm"
        keyConstants.base58Prefixes[LEGACY_PUBKEY_ADDRESS]     = {0x1D,0x25};
        // guarantees the first 2 characters, when base58 encoded, are "t2"
        keyConstants.base58Prefixes[LEGACY_SCRIPT_ADDRESS]     = {0x1C,0xBA};
        // the first character, when base58 encoded, is "9" or "c" (as in Bitcoin)
        keyConstants.base58Prefixes[LEGACY_SECRET_KEY]         = {0xEF};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        keyConstants.base58Prefixes[LEGACY_EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};
        keyConstants.base58Prefixes[LEGACY_EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};
        // guarantees the first 2 characters, when base58 encoded, are "zt"
        keyConstants.base58Prefixes[LEGACY_ZCPAYMENT_ADDRESS]  = {0x16,0xB6};
        // guarantees the first 4 characters, when base58 encoded, are "ZiVt"
        keyConstants.base58Prefixes[LEGACY_ZCVIEWING_KEY]      = {0xA8,0xAC,0x0C};
        // guarantees the first 2 characters, when base58 encoded, are "ST"
        keyConstants.base58Prefixes[LEGACY_ZCSPENDING_KEY]     = {0xAC,0x08};

        keyConstants.bech32HRPs[LEGACY_SAPLING_PAYMENT_ADDRESS]      = "ztestsapling";
        keyConstants.bech32HRPs[LEGACY_SAPLING_FULL_VIEWING_KEY]     = "zviewtestsapling";
        keyConstants.bech32HRPs[LEGACY_SAPLING_INCOMING_VIEWING_KEY] = "zivktestsapling";
        keyConstants.bech32HRPs[LEGACY_SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-test";
        keyConstants.bech32HRPs[LEGACY_SAPLING_EXTENDED_FVK]         = "zxviewtestsapling";

        // On testnet we activate this rule at the same height as Blossom activation.
        static_assert(6 * Consensus::POST_BLOSSOM_POW_TARGET_SPACING * 7 < MAX_FUTURE_BLOCK_TIME_MTP - 60,
                      "MAX_FUTURE_BLOCK_TIME_MTP is too low given block target spacing");
        consensus.nFutureTimestampSoftForkHeight = consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000001959b78e6f");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0x1a;
        pchMessageStart[2] = 0xf9;
        pchMessageStart[3] = 0xbf;
        vAlertPubKey = ParseHex("041c64ece576904e60264571717fc027692455afaacdb91d3c7f68724ad17161d6db24632dbac26849bd6d66e534ddf800eb4fe3a4ae3a0b690f737c85625869a2");
        nDefaultPort = 18833;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(
            1477648033,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000006"),
            ParseHex("00a6a51259c3f6732481e2d035197218b7a69504461d04335503cd69759b2d02bd2b53a9653f42cb33c608511c953673fa9da76170958115fe92157ad3bb5720d927f18e09459bf5c6072973e143e20f9bdf0584058c96b7c2234c7565f100d5eea083ba5d3dbaff9f0681799a113e7beff4a611d2b49590563109962baa149b628aae869af791f2f70bb041bd7ebfa658570917f6654a142b05e7ec0289a4f46470be7be5f693b90173eaaa6e84907170f32602204f1f4e1c04b1830116ffd0c54f0b1caa9a5698357bd8aa1f5ac8fc93b405265d824ba0e49f69dab5446653927298e6b7bdc61ee86ff31c07bde86331b4e500d42e4e50417e285502684b7966184505b885b42819a88469d1e9cf55072d7f3510f85580db689302eab377e4e11b14a91fdd0df7627efc048934f0aff8e7eb77eb17b3a95de13678004f2512293891d8baf8dde0ef69be520a58bbd6038ce899c9594cf3e30b8c3d9c7ecc832d4c19a6212747b50724e6f70f6451f78fd27b58ce43ca33b1641304a916186cfbe7dbca224f55d08530ba851e4df22baf7ab7078e9cbea46c0798b35a750f54103b0cdd08c81a6505c4932f6bfbd492a9fced31d54e98b6370d4c96600552fcf5b37780ed18c8787d03200963600db297a8f05dfa551321d17b9917edadcda51e274830749d133ad226f8bb6b94f13b4f77e67b35b71f52112ce9ba5da706ad9573584a2570a4ff25d29ab9761a06bdcf2c33638bf9baf2054825037881c14adf3816ba0cbd0fca689aad3ce16f2fe362c98f48134a9221765d939f0b49677d1c2447e56b46859f1810e2cf23e82a53e0d44f34dae932581b3b7f49eaec59af872cf9de757a964f7b33d143a36c270189508fcafe19398e4d2966948164d40556b05b7ff532f66f5d1edc41334ef742f78221dfe0c7ae2275bb3f24c89ae35f00afeea4e6ed187b866b209dc6e83b660593fce7c40e143beb07ac86c56f39e895385924667efe3a3f031938753c7764a2dbeb0a643fd359c46e614873fd0424e435fa7fac083b9a41a9d6bf7e284eee537ea7c50dd239f359941a43dc982745184bf3ee31a8dc850316aa9c6b66d6985acee814373be3458550659e1a06287c3b3b76a185c5cb93e38c1eebcf34ff072894b6430aed8d34122dafd925c46a515cca79b0269c92b301890ca6b0dc8b679cdac0f23318c105de73d7a46d16d2dad988d49c22e9963c117960bdc70ef0db6b091cf09445a516176b7f6d58ec29539166cc8a38bbff387acefffab2ea5faad0e8bb70625716ef0edf61940733c25993ea3de9f0be23d36e7cb8da10505f9dc426cd0e6e5b173ab4fff8c37e1f1fb56d1ea372013d075e0934c6919393cfc21395eea20718fad03542a4162a9ded66c814ad8320b2d7c2da3ecaf206da34c502db2096d1c46699a91dd1c432f019ad434e2c1ce507f91104f66f491fed37b225b8e0b2888c37276cfa0468fc13b8d593fd9a2675f0f5b20b8a15f8fa7558176a530d6865738ddb25d3426dab905221681cf9da0e0200eea5b2eba3ad3a5237d2a391f9074bf1779a2005cee43eec2b058511532635e0fea61664f531ac2b356f40db5c5d275a4cf5c82d468976455af4e3362cc8f71aa95e71d394aff3ead6f7101279f95bcd8a0fedce1d21cb3c9f6dd3b182fce0db5d6712981b651f29178a24119968b14783cafa713bc5f2a65205a42e4ce9dc7ba462bdb1f3e4553afc15f5f39998fdb53e7e231e3e520a46943734a007c2daa1eda9f495791657eefcac5c32833936e568d06187857ed04d7b97167ae207c5c5ae54e528c36016a984235e9c5b2f0718d7b3aa93c7822ccc772580b6599671b3c02ece8a21399abd33cfd3028790133167d0a97e7de53dc8ff"),
            0x2007ffff, 4, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x05a60a92d99d85997cce3b87616c089f6124d7342af37106edc76126334a2c38"));
        assert(genesis.hashMerkleRoot == uint256S("0xc4eaa58879081de3c24a7b117ed2b28300e7ec4c4c1dff1d3f1268b7857a4ddb"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("ycash.xyz", "testseed.ycash.xyz")); // Ycash

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;


        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, consensus.hashGenesisBlock)
            (38000, uint256S("0x001e9a2d2e2892b88e9998cf7b079b41d59dd085423a921fe8386cecc42287b8"))
            (650000, uint256S("0x005dd2092f75382581468e870b51233a8950bf9c396689c513e5a80f00c14609")),
            1633853210,  // * UNIX timestamp of last checkpoint block
            862651,       // * total number of transactions between genesis and last checkpoint
            765          //   total number of tx / (checkpoint block height / (24 * 24))
        };

        // Hardcoded fallback value for the Sprout shielded value pool balance
        // for nodes that have not reindexed since the introduction of monitoring
        // in #2795.
        nSproutValuePoolCheckpointHeight = 440329;
        nSproutValuePoolCheckpointBalance = 40000029096803;
        fZIP209Enabled = true;
        hashSproutValuePoolCheckpointBlock = uint256S("000a95d08ba5dcbabe881fc6471d11807bcca7df5f1795c99f3ec4580db4279b");

        // Founders reward script expects a vector of 2-of-3 multisig addresses
        vFoundersRewardAddress = {
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", "t2N9PH9Wk9xjqYg9iin1Ua3aekJqfAtE543", "t2NGQjYMQhFndDHguvUw4wZdNdsssA6K7x2", "t2ENg7hHVqqs9JwU5cgjvSbxnT2a9USNfhy",
            "t2BkYdVCHzvTJJUTx4yZB8qeegD8QsPx8bo", "t2J8q1xH1EuigJ52MfExyyjYtN3VgvshKDf", "t2Crq9mydTm37kZokC68HzT6yez3t2FBnFj", "t2EaMPUiQ1kthqcP5UEkF42CAFKJqXCkXC9",
            "t2F9dtQc63JDDyrhnfpzvVYTJcr57MkqA12", "t2LPirmnfYSZc481GgZBa6xUGcoovfytBnC", "t26xfxoSw2UV9Pe5o3C8V4YybQD4SESfxtp", "t2D3k4fNdErd66YxtvXEdft9xuLoKD7CcVo",
            "t2DWYBkxKNivdmsMiivNJzutaQGqmoRjRnL", "t2C3kFF9iQRxfc4B9zgbWo4dQLLqzqjpuGQ", "t2MnT5tzu9HSKcppRyUNwoTp8MUueuSGNaB", "t2AREsWdoW1F8EQYsScsjkgqobmgrkKeUkK",
            "t2Vf4wKcJ3ZFtLj4jezUUKkwYR92BLHn5UT", "t2K3fdViH6R5tRuXLphKyoYXyZhyWGghDNY", "t2VEn3KiKyHSGyzd3nDw6ESWtaCQHwuv9WC", "t2F8XouqdNMq6zzEvxQXHV1TjwZRHwRg8gC",
            "t2BS7Mrbaef3fA4xrmkvDisFVXVrRBnZ6Qj", "t2FuSwoLCdBVPwdZuYoHrEzxAb9qy4qjbnL", "t2SX3U8NtrT6gz5Db1AtQCSGjrpptr8JC6h", "t2V51gZNSoJ5kRL74bf9YTtbZuv8Fcqx2FH",
            "t2FyTsLjjdm4jeVwir4xzj7FAkUidbr1b4R", "t2EYbGLekmpqHyn8UBF6kqpahrYm7D6N1Le", "t2NQTrStZHtJECNFT3dUBLYA9AErxPCmkka", "t2GSWZZJzoesYxfPTWXkFn5UaxjiYxGBU2a",
            "t2RpffkzyLRevGM3w9aWdqMX6bd8uuAK3vn", "t2JzjoQqnuXtTGSN7k7yk5keURBGvYofh1d", "t2AEefc72ieTnsXKmgK2bZNckiwvZe3oPNL", "t2NNs3ZGZFsNj2wvmVd8BSwSfvETgiLrD8J",
            "t2ECCQPVcxUCSSQopdNquguEPE14HsVfcUn", "t2JabDUkG8TaqVKYfqDJ3rqkVdHKp6hwXvG", "t2FGzW5Zdc8Cy98ZKmRygsVGi6oKcmYir9n", "t2DUD8a21FtEFn42oVLp5NGbogY13uyjy9t",
            "t2UjVSd3zheHPgAkuX8WQW2CiC9xHQ8EvWp", "t2TBUAhELyHUn8i6SXYsXz5Lmy7kDzA1uT5", "t2Tz3uCyhP6eizUWDc3bGH7XUC9GQsEyQNc", "t2NysJSZtLwMLWEJ6MH3BsxRh6h27mNcsSy",
            "t2KXJVVyyrjVxxSeazbY9ksGyft4qsXUNm9", "t2J9YYtH31cveiLZzjaE4AcuwVho6qjTNzp", "t2QgvW4sP9zaGpPMH1GRzy7cpydmuRfB4AZ", "t2NDTJP9MosKpyFPHJmfjc5pGCvAU58XGa4",
            "t29pHDBWq7qN4EjwSEHg8wEqYe9pkmVrtRP", "t2Ez9KM8VJLuArcxuEkNRAkhNvidKkzXcjJ", "t2D5y7J5fpXajLbGrMBQkFg2mFN8fo3n8cX", "t2UV2wr1PTaUiybpkV3FdSdGxUJeZdZztyt",
            };
        assert(vFoundersRewardAddress.size() <= consensus.GetLastFoundersRewardBlockHeight(0));

        vYcashFoundersRewardAddress = {
            "smDw2LWkeuJ1NGBDDZvdNbzY8A9D1mkkDZm", "smDxM6WPpz3HcK6m9cCnhQkBXMLnUf3cryA", "smEKdQPcZHYTmcTbVkqfWryRbEZWMrapjMo", "smEVfJmuGErW6ZM3XSNwbJR6cPU3iPABAqY", 
            "smEkWMsbV1CBZosu9wtq69f2vNXBT1owNKe", "smFd3Dh5MjEttRHd9S8kx153Vzesefzjc2d", "smGBXB9SrjnEDf7ASQxvnujBRc1qBb58o5q", "smGLTYjSriA3n8EMf4JTiHLGUCzUYjau3WV", 
            "smGVF2kDywxhjfzBqoFDE1AyXZEafTLSjbH", "smGVrxUHUzd2gaURPw2ASoE3L5WMFcxJcp1", "smHTCd59Q9pFzrzA73f6het1ozDzeQaA8E3", "smHy6JaGM9gkaGBJ4DF4p5FbFvoUbpAusWd", 
            "smJ1fpQdKNuchkxuVUMBkcCWoBcWFmyDAyZ", "smJ2j3Gea5XH7ERpyYzvo6YQKaoYfzkMUS3", "smJ8gtE5EX5oFSp4c5cCDpxoajXTQu73VSC", "smJyGxvwpCxPaFJM6TwZ6cwT1qz5PMPPBeL", 
            "smKEt1iVgCDY9V4915HnGpFK3zyTPyQixMa", "smKXxXsNLUVE8Qro6R9EyaEZmvgxqjbsFX9", "smLTH7FEiXUVpWjhoL91ToMoSZPU8xAvEh9", "smLo65rNyiEYiVHxWZHNNP9QU1HsQu3QX7b", 
            "smMDUN36MqFE4thY54CQnWBpU4ePuayF9TV", "smMWmoipQ7YVuRAJaQHKqhHbbTg28mS6ET5", "smMtCCZsR3s7ZiEYskFietNPfSb1eVHNoZi", "smMuh9QVkpQg93Gb54ucaVykbk9FgjZBN3D", 
            "smNM44GrsbMWHQRhhayqieCadzURNDLkxkW", "smNhJLQs3LmHWtVscrnJmrLhhjPcDD3unJZ", "smPHxC1438rANivn1omntbRF5Nf2wSZfFhs", "smSAidYKoFY2fmi2efcoJPSeBpdVC2vyHvj", 
            "smShCYXefsx8RcnW2b7duPKBcD5TXWY5K2A", "smTRxVMtveLrdzVb1B9rLKfDZ3Qp8kAna7r", "smTnar3ernxTG5voae2bUC185EicBdSKVux", "smUmPeP8VwuPsNTcJcB7YZY1b1HJJsJfkYp", 
            "smVSGTzPP4dbj3HXRR84WoBFWw4EVuuyVi2", "smW8AA9LAKGMj4EXxNtfJQFLFaiuzTyGwYa", "smWAgHkGiQ1fZHF9ZBjYzKS7ZX8JtjvWbaU", "smX1HT3n9mtPGeK7qMBETCEGL5UG8eC8nmy", 
            "smX3udkLyb3qZ2z2muEinbvNuSi5M2JiiE5", "smXEcB2QZaZgevkB7CS1Tz2BZNQ9cnNwXBj", "smXddMXWZvpRDq4FjuTGxLWYTtwbNwc36g5", "smXfPS6G7aiVECG8qFvZc9oFe1bzEvCKECG", 
            "smXxQi63m9x2WfhdBYogWdKzavVpbnnGyJq", "smZ53EtRafyjGZyjiNDb1FiGbwXbtE3aqTB", "smZJPM9KsKAdfotD6Woh2nb8wLedvhf4Nw5", "smZyciv3CGZLAFU8d2yKff33FU8f2nek5yx", 
            "smZzRpCLENnxN5JgMHaKtMruTCi7jsa7Tak", "smaWKSqvUJvajRquGbaHBdNbL78fDf6hdwE", "smazMQ9G7NLJXAzX4ZMKc8x6DigyeoEucgk", "smbTaZmsKNEVstoQVyZcJAMJExiytfnwaMU" 
        };
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        strCurrencyUnits = "REG";
        bip44CoinType = 1;
        consensus.fCoinbaseMustBeShielded = false;
        consensus.nSubsidySlowStartInterval = 0;
        consensus.nPreBlossomSubsidyHalvingInterval = Consensus::PRE_BLOSSOM_REGTEST_HALVING_INTERVAL;
        consensus.nPostBlossomSubsidyHalvingInterval = POST_BLOSSOM_HALVING_INTERVAL(Consensus::PRE_BLOSSOM_REGTEST_HALVING_INTERVAL);
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        const size_t N = 48, K = 5;
        static_assert(equihash_parameters_acceptable(N, K));
        consensus.nEquihashN = N;
        consensus.nEquihashK = K;
        consensus.powLimit = uint256S("0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f");
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 0; // Turn off adjustment down
        consensus.nPowMaxAdjustUp = 0; // Turn off adjustment up
        consensus.nPreBlossomPowTargetSpacing = Consensus::PRE_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPostBlossomPowTargetSpacing = Consensus::POST_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = 0;
        consensus.fPowNoRetargeting = true;
        consensus.minDifficultyAtYcashFork = false;
        consensus.scaledDifficultyAtYcashFork = false;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170003;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170006;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_YCASH].nProtocolVersion = 270007;
        consensus.vUpgrades[Consensus::UPGRADE_YCASH].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 270008;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight = 
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nProtocolVersion = 270010;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nProtocolVersion = 270012;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nProtocolVersion = 270014;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nProtocolVersion = 0x7FFFFFFF;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        consensus.nFundingPeriodLength = consensus.nPostBlossomSubsidyHalvingInterval / 48;
        // Defined funding streams can be enabled with node config flags.

        // These prefixes are the same as the testnet prefixes
        keyConstants.base58Prefixes[PUBKEY_ADDRESS]     = {0x1C,0x95};
        keyConstants.base58Prefixes[SCRIPT_ADDRESS]     = {0x1C,0x2A};
        keyConstants.base58Prefixes[SECRET_KEY]         = {0xEF};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        keyConstants.base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};
        keyConstants.base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};
        keyConstants.base58Prefixes[ZCPAYMENT_ADDRESS]  = {0x16,0x52};
        keyConstants.base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAC,0x0C};
        keyConstants.base58Prefixes[ZCSPENDING_KEY]     = {0xAC,0x08};

        keyConstants.bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "yregtestsapling";
        keyConstants.bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "zviewregtestsapling";
        keyConstants.bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "zivkregtestsapling";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-regtest";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_FVK]         = "zxviewregtestsapling";

        // Legacy (Zcash) prefixes, required for pre-fork founders addresses validation
        // These prefixes are the same as the testnet prefixes
        keyConstants.base58Prefixes[LEGACY_PUBKEY_ADDRESS]     = {0x1D,0x25};
        keyConstants.base58Prefixes[LEGACY_SCRIPT_ADDRESS]     = {0x1C,0xBA};
        keyConstants.base58Prefixes[LEGACY_SECRET_KEY]         = {0xEF};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        keyConstants.base58Prefixes[LEGACY_EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};
        keyConstants.base58Prefixes[LEGACY_EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};
        keyConstants.base58Prefixes[LEGACY_ZCPAYMENT_ADDRESS]  = {0x16,0xB6};
        keyConstants.base58Prefixes[LEGACY_ZCVIEWING_KEY]      = {0xA8,0xAC,0x0C};
        keyConstants.base58Prefixes[LEGACY_ZCSPENDING_KEY]     = {0xAC,0x08};

        keyConstants.bech32HRPs[LEGACY_SAPLING_PAYMENT_ADDRESS]      = "zregtestsapling";
        keyConstants.bech32HRPs[LEGACY_SAPLING_FULL_VIEWING_KEY]     = "zviewregtestsapling";
        keyConstants.bech32HRPs[LEGACY_SAPLING_INCOMING_VIEWING_KEY] = "zivkregtestsapling";
        keyConstants.bech32HRPs[LEGACY_SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-regtest";
        keyConstants.bech32HRPs[LEGACY_SAPLING_EXTENDED_FVK]         = "zxviewregtestsapling";

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        pchMessageStart[0] = 0xaa;
        pchMessageStart[1] = 0xe8;
        pchMessageStart[2] = 0x3f;
        pchMessageStart[3] = 0x5f;
        nDefaultPort = 18344;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(
            1296688602,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000009"),
            ParseHex("01936b7db1eb4ac39f151b8704642d0a8bda13ec547d54cd5e43ba142fc6d8877cab07b3"),
            0x200f0f0f, 4, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x029f11d80ef9765602235e1bc9727e3eb6ba20839319f761fee920d63401e327"));
        assert(genesis.hashMerkleRoot == uint256S("0xc4eaa58879081de3c24a7b117ed2b28300e7ec4c4c1dff1d3f1268b7857a4ddb"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData){
            boost::assign::map_list_of
            ( 0, uint256S("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206")),
            0,
            0,
            0
        };

        // Founders reward script expects a vector of 2-of-3 multisig addresses
        vFoundersRewardAddress = { "t2FwcEhFdNXuFMv1tcYwaBJtYVtMj8b1uTg" };
        assert(vFoundersRewardAddress.size() <= consensus.GetLastFoundersRewardBlockHeight(0));

        vYcashFoundersRewardAddress = {
            "smDw2LWkeuJ1NGBDDZvdNbzY8A9D1mkkDZm", "smDxM6WPpz3HcK6m9cCnhQkBXMLnUf3cryA", "smEKdQPcZHYTmcTbVkqfWryRbEZWMrapjMo", "smEVfJmuGErW6ZM3XSNwbJR6cPU3iPABAqY", 
            "smEkWMsbV1CBZosu9wtq69f2vNXBT1owNKe", "smFd3Dh5MjEttRHd9S8kx153Vzesefzjc2d", "smGBXB9SrjnEDf7ASQxvnujBRc1qBb58o5q", "smGLTYjSriA3n8EMf4JTiHLGUCzUYjau3WV", 
            "smGVF2kDywxhjfzBqoFDE1AyXZEafTLSjbH", "smGVrxUHUzd2gaURPw2ASoE3L5WMFcxJcp1", "smHTCd59Q9pFzrzA73f6het1ozDzeQaA8E3", "smHy6JaGM9gkaGBJ4DF4p5FbFvoUbpAusWd", 
            "smJ1fpQdKNuchkxuVUMBkcCWoBcWFmyDAyZ", "smJ2j3Gea5XH7ERpyYzvo6YQKaoYfzkMUS3", "smJ8gtE5EX5oFSp4c5cCDpxoajXTQu73VSC", "smJyGxvwpCxPaFJM6TwZ6cwT1qz5PMPPBeL", 
            "smKEt1iVgCDY9V4915HnGpFK3zyTPyQixMa", "smKXxXsNLUVE8Qro6R9EyaEZmvgxqjbsFX9", "smLTH7FEiXUVpWjhoL91ToMoSZPU8xAvEh9", "smLo65rNyiEYiVHxWZHNNP9QU1HsQu3QX7b", 
            "smMDUN36MqFE4thY54CQnWBpU4ePuayF9TV", "smMWmoipQ7YVuRAJaQHKqhHbbTg28mS6ET5", "smMtCCZsR3s7ZiEYskFietNPfSb1eVHNoZi", "smMuh9QVkpQg93Gb54ucaVykbk9FgjZBN3D", 
            "smNM44GrsbMWHQRhhayqieCadzURNDLkxkW", "smNhJLQs3LmHWtVscrnJmrLhhjPcDD3unJZ", "smPHxC1438rANivn1omntbRF5Nf2wSZfFhs", "smSAidYKoFY2fmi2efcoJPSeBpdVC2vyHvj", 
            "smShCYXefsx8RcnW2b7duPKBcD5TXWY5K2A", "smTRxVMtveLrdzVb1B9rLKfDZ3Qp8kAna7r", "smTnar3ernxTG5voae2bUC185EicBdSKVux", "smUmPeP8VwuPsNTcJcB7YZY1b1HJJsJfkYp", 
            "smVSGTzPP4dbj3HXRR84WoBFWw4EVuuyVi2", "smW8AA9LAKGMj4EXxNtfJQFLFaiuzTyGwYa", "smWAgHkGiQ1fZHF9ZBjYzKS7ZX8JtjvWbaU", "smX1HT3n9mtPGeK7qMBETCEGL5UG8eC8nmy", 
            "smX3udkLyb3qZ2z2muEinbvNuSi5M2JiiE5", "smXEcB2QZaZgevkB7CS1Tz2BZNQ9cnNwXBj", "smXddMXWZvpRDq4FjuTGxLWYTtwbNwc36g5", "smXfPS6G7aiVECG8qFvZc9oFe1bzEvCKECG", 
            "smXxQi63m9x2WfhdBYogWdKzavVpbnnGyJq", "smZ53EtRafyjGZyjiNDb1FiGbwXbtE3aqTB", "smZJPM9KsKAdfotD6Woh2nb8wLedvhf4Nw5", "smZyciv3CGZLAFU8d2yKff33FU8f2nek5yx", 
            "smZzRpCLENnxN5JgMHaKtMruTCi7jsa7Tak", "smaWKSqvUJvajRquGbaHBdNbL78fDf6hdwE", "smazMQ9G7NLJXAzX4ZMKc8x6DigyeoEucgk", "smbTaZmsKNEVstoQVyZcJAMJExiytfnwaMU" 
        };


    }

    void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
    {
        assert(idx > Consensus::BASE_SPROUT && idx < Consensus::MAX_NETWORK_UPGRADES);
        consensus.vUpgrades[idx].nActivationHeight = nActivationHeight;
    }

    void UpdateFundingStreamParameters(Consensus::FundingStreamIndex idx, Consensus::FundingStream fs)
    {
        assert(idx >= Consensus::FIRST_FUNDING_STREAM && idx < Consensus::MAX_FUNDING_STREAMS);
        consensus.vFundingStreams[idx] = fs;
    }

    void UpdateRegtestPow(
        int64_t nPowMaxAdjustDown,
        int64_t nPowMaxAdjustUp,
        uint256 powLimit,
        bool noRetargeting)
    {
        consensus.nPowMaxAdjustDown = nPowMaxAdjustDown;
        consensus.nPowMaxAdjustUp = nPowMaxAdjustUp;
        consensus.powLimit = powLimit;
        consensus.fPowNoRetargeting = noRetargeting;
    }

    void SetRegTestZIP209Enabled() {
        fZIP209Enabled = true;
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);

    // Some python qa rpc tests need to enforce the coinbase consensus rule
    if (network == CBaseChainParams::REGTEST && mapArgs.count("-regtestshieldcoinbase")) {
        regTestParams.SetRegTestCoinbaseMustBeShielded();
    }

    // When a developer is debugging turnstile violations in regtest mode, enable ZIP209
    if (network == CBaseChainParams::REGTEST && mapArgs.count("-developersetpoolsizezero")) {
        regTestParams.SetRegTestZIP209Enabled();
    }
}


// Block height must be >0 and <=last founders reward block height
// Index variable i ranges from 0 - (vFoundersRewardAddress.size()-1)
std::string CChainParams::GetFoundersRewardAddressAtHeight(int nHeight) const {
    // Pre YCash
    if (!this->GetConsensus().NetworkUpgradeActive(nHeight, Consensus::UPGRADE_YCASH)) {
        KeyIO keyIO(*this);
        int preBlossomMaxHeight = consensus.GetLastFoundersRewardBlockHeight(0);
        // zip208
        // FounderAddressAdjustedHeight(height) :=
        // height, if not IsBlossomActivated(height)
        // BlossomActivationHeight + floor((height - BlossomActivationHeight) / BlossomPoWTargetSpacingRatio), otherwise
        bool blossomActive = consensus.NetworkUpgradeActive(nHeight, Consensus::UPGRADE_BLOSSOM);
        if (blossomActive) {
            int blossomActivationHeight = consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight;
            nHeight = blossomActivationHeight + ((nHeight - blossomActivationHeight) / Consensus::BLOSSOM_POW_TARGET_SPACING_RATIO);
        }
        assert(nHeight > 0 && nHeight <= preBlossomMaxHeight);
        size_t addressChangeInterval = (preBlossomMaxHeight + vFoundersRewardAddress.size()) / vFoundersRewardAddress.size();
        size_t i = nHeight / addressChangeInterval;
        return keyIO.ZecToYec(vFoundersRewardAddress[i]);
    } else {
        // Ycash 
        // Make sure we have at least 48 addresses  (approx 4 years worth)
        //assert(vYcashFoundersRewardAddress.size() == 48);

        size_t addressChangeInterval = 17917; // Every one month, approx
        size_t i = (nHeight - consensus.vUpgrades[Consensus::UPGRADE_YCASH].nActivationHeight) / addressChangeInterval;

        return vYcashFoundersRewardAddress[i % vYcashFoundersRewardAddress.size()];
    }
}

// Block height must be >0 and <=last founders reward block height for pre YCash. The founders reward address
// is expected to be a multisig (P2SH) address.
// Post YCash, return a regular address (For now)
CScript CChainParams::GetFoundersRewardScriptAtHeight(int nHeight) const {    
    KeyIO keyIO(*this);
    if (!this->GetConsensus().NetworkUpgradeActive(nHeight, Consensus::UPGRADE_YCASH)) {
        assert(nHeight > 0 && nHeight <= consensus.GetLastFoundersRewardBlockHeight(nHeight));

        CTxDestination address = keyIO.DecodeDestination(GetFoundersRewardAddressAtHeight(nHeight).c_str());
        assert(IsValidDestination(address));
        assert(IsScriptDestination(address));
        CScriptID scriptID = std::get<CScriptID>(address); // address is a variant
        CScript script = CScript() << OP_HASH160 << ToByteVector(scriptID) << OP_EQUAL;
        return script;
    } else {
        CTxDestination address = keyIO.DecodeDestination(GetFoundersRewardAddressAtHeight(nHeight).c_str());
        assert(IsValidDestination(address));
        assert(IsKeyDestination(address) || IsScriptDestination(address));

        if (IsKeyDestination(address)) { // Address is a regular address
            CScriptID keyID = std::get<CKeyID>(address); 
            CScript script =  CScript() << OP_DUP << OP_HASH160 << ToByteVector(keyID) << OP_EQUALVERIFY << OP_CHECKSIG;
            return script;
        } else { // Address is multisig address
            CScriptID scriptID = std::get<CScriptID>(address); 
            CScript script = CScript() << OP_HASH160 << ToByteVector(scriptID) << OP_EQUAL;
            return script;
        }
    }
}

std::string CChainParams::GetZcashFoundersRewardAddressAtIndex(int i) const {
    assert(i >= 0 && i < vFoundersRewardAddress.size());
    KeyIO keyIO(*this);
    return keyIO.ZecToYec(vFoundersRewardAddress[i]);
}

std::string CChainParams::GetYcashFoundersRewardAddressAtIndex(int i) const {
    assert(i >= 0 && i < vYcashFoundersRewardAddress.size());
    return vYcashFoundersRewardAddress[i];
}

void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
{
    regTestParams.UpdateNetworkUpgradeParameters(idx, nActivationHeight);
}

// To help debugging, let developers change the equihash parameters for a network upgrade.
// TODO: Restrict this to regtest mode in the future.
void UpdateEquihashUpgradeParameters(Consensus::UpgradeIndex idx, unsigned int n, unsigned int k)
{
    assert(idx > Consensus::BASE_SPROUT && idx < Consensus::MAX_NETWORK_UPGRADES);
    EquihashUpgradeInfo[idx].N = n;
    EquihashUpgradeInfo[idx].K = k;
}

// Return Equihash parameter N at a given block height.
unsigned int CChainParams::EquihashN(int nHeight) const {
    if (this->strNetworkID == "regtest")
        return GetConsensus().nEquihashN;

    unsigned int n = EquihashUpgradeInfo[CurrentEpoch(nHeight, GetConsensus())].N;
    if (n == EquihashInfo::DEFAULT_PARAMS) {
        n = GetConsensus().nEquihashN;
    }
    return n;
}

// Return Equihash parameter K at a given block height.
unsigned int CChainParams::EquihashK(int nHeight) const {
    if (this->strNetworkID == "regtest")
        return GetConsensus().nEquihashK;

    unsigned int k = EquihashUpgradeInfo[CurrentEpoch(nHeight, GetConsensus())].K;
    if (k == EquihashInfo::DEFAULT_PARAMS) {
        k = GetConsensus().nEquihashK;
    }
    return k;
}

void UpdateFundingStreamParameters(Consensus::FundingStreamIndex idx, Consensus::FundingStream fs)
{
    regTestParams.UpdateFundingStreamParameters(idx, fs);
}

void UpdateRegtestPow(
    int64_t nPowMaxAdjustDown,
    int64_t nPowMaxAdjustUp,
    uint256 powLimit,
    bool noRetargeting)
{
    regTestParams.UpdateRegtestPow(nPowMaxAdjustDown, nPowMaxAdjustUp, powLimit, noRetargeting);
}
