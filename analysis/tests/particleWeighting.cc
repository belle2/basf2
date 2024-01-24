/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/dbobjects/ParticleWeightingLookUpTable.h>
#include <analysis/dbobjects/ParticleWeightingBinLimits.h>
#include <analysis/dbobjects/ParticleWeightingAxis.h>

#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {

  TEST(ParticleWeighting, BinLimits)
  {
    EXPECT_B2FATAL(ParticleWeightingBinLimits(42, 42));
#ifndef __clang_analyzer__
    auto* a = new ParticleWeightingBinLimits(0, 1);
    EXPECT_TRUE(a->first() == 0);
    EXPECT_TRUE(a->second() == 1);
    auto* b = new ParticleWeightingBinLimits(1, 0);
    EXPECT_TRUE(b->first() == 0);
    EXPECT_TRUE(b->second() == 1);
#endif
  }

  TEST(ParticleWeighting, Axis)
  {
    auto* a = new ParticleWeightingAxis();
    EXPECT_TRUE(a->getName() == "");
    a->setName("Test axis name");
    EXPECT_TRUE(a->getName() == "Test axis name");

    auto* bin1 = new ParticleWeightingBinLimits(0, 1);
    auto* bin1b = new ParticleWeightingBinLimits(1, 2);
    auto* bin2 = new ParticleWeightingBinLimits(2, 3);

    int added_bin_id_1 = a->addBin(bin1);
    EXPECT_EQ(added_bin_id_1, 1);
    int added_bin_id_2 = a->addBin(bin2);
    EXPECT_EQ(added_bin_id_2, 2);
#ifndef __clang_analyzer__
    auto* bin3 = new ParticleWeightingBinLimits(0, 5);
    auto* bin4 = new ParticleWeightingBinLimits(0.5, 1.5);
    auto* bin5 = new ParticleWeightingBinLimits(2.5, 3);
    EXPECT_B2FATAL(a->addBin(bin3));
    EXPECT_B2FATAL(a->addBin(bin4));
    EXPECT_B2FATAL(a->addBin(bin5));
#endif
    EXPECT_TRUE(a->findBin(0.5) == 1);
    EXPECT_TRUE(a->findBin(1.5) == -1);
    EXPECT_TRUE(a->findBin(bin1) == 1);
    EXPECT_TRUE(a->addBin(bin1b) == 3);
  }

  TEST(ParticleWeighting, KeyMap)
  {
    auto* bl1 = new ParticleWeightingBinLimits(0, 1);
    auto* bl2 = new ParticleWeightingBinLimits(1, 2);
    auto* bl3 = new ParticleWeightingBinLimits(1, 3);
    string a1 = "Axis1";
    string a2 = "Axis2";
    string a3 = "Axis3";
    NDBin bin1;
    bin1.insert(make_pair(a1, bl1));
    bin1.insert(make_pair(a2, bl1));
    NDBin bin2;
    bin2.insert(make_pair(a1, bl2));
    bin2.insert(make_pair(a2, bl2));
    NDBin bin3;
    bin3.insert(make_pair(a1, bl3));
    bin3.insert(make_pair(a2, bl3));
    NDBin bin1a;
    bin1a.insert(make_pair(a1, bl2));
    bin1a.insert(make_pair(a2, bl2));
    bin1a.insert(make_pair(a3, bl3));
    NDBin bin1b;
    bin1b.insert(make_pair(a1, bl1));
    bin1b.insert(make_pair(a1, bl2));
    NDBin bin1c;
    bin1c.insert(make_pair(a3, bl3));
    bin1c.insert(make_pair(a2, bl2));
    map<string, double> entry1;
    entry1.insert(make_pair(a1, 0.1));
    entry1.insert(make_pair(a2, 0.2));
    map<string, double> entry2;
    entry2.insert(make_pair(a1, 1.1));
    entry2.insert(make_pair(a2, 1.2));
    map<string, double> entry3;
    entry3.insert(make_pair(a1, 10));
    entry3.insert(make_pair(a2, 10));

    auto* kmp = new ParticleWeightingKeyMap();
    int added_bin_id_1 = kmp->addKey(bin1);
    EXPECT_EQ(added_bin_id_1, 0);
    int added_bin_id_2 = kmp->addKey(bin2);
    EXPECT_EQ(added_bin_id_2, 1);
    EXPECT_B2FATAL(kmp->addKey(bin1, 42));
    EXPECT_B2FATAL(kmp->addKey(bin1a));
    EXPECT_B2FATAL(kmp->addKey(bin1b));
    EXPECT_B2FATAL(kmp->addKey(bin1c));
    EXPECT_EQ(kmp->getKey(entry1), 0);
    EXPECT_EQ(kmp->getKey(entry2), 1);
    EXPECT_EQ(kmp->getKey(entry3), -1);
  }

  TEST(ParticleWeighting, LookUpTable)
  {
    auto* bl1 = new ParticleWeightingBinLimits(0, 1);
    auto* bl2 = new ParticleWeightingBinLimits(1, 2);
    auto* bl3 = new ParticleWeightingBinLimits(1, 3);
    string a1 = "Axis1";
    string a2 = "Axis2";
    string v1 = "Var1";
    string v2 = "Var2";
    NDBin bin1;
    bin1.insert(make_pair(a1, bl1));
    bin1.insert(make_pair(a2, bl1));
    NDBin bin2;
    bin2.insert(make_pair(a1, bl2));
    bin2.insert(make_pair(a2, bl2));
    NDBin bin3;
    bin3.insert(make_pair(a1, bl3));
    bin3.insert(make_pair(a2, bl3));
    WeightInfo info1;
    info1.insert(make_pair(v1, 41));
    info1.insert(make_pair(v2, 42));
    WeightInfo info2;
    info2.insert(make_pair(v1, 31));
    info2.insert(make_pair(v2, 32));
    WeightInfo info3;
    info3.insert(make_pair(v1, 21));
    info3.insert(make_pair(v2, 22));
    map<string, double> entry1;
    entry1.insert(make_pair(a1, 0.1));
    entry1.insert(make_pair(a2, 0.2));
    map<string, double> entry1a;
    entry1a.insert(make_pair(a1, 100));
    entry1a.insert(make_pair(a2, 200));

    auto* ltb = new ParticleWeightingLookUpTable();
    ltb->addEntry(info1, bin1);
    ltb->addEntry(info2, bin2);
    vector<string> axes = ltb->getAxesNames();
    vector<string> known_axes;
    known_axes.push_back(a1);
    known_axes.push_back(a2);
    EXPECT_TRUE(equal(axes.begin(), axes.end(), known_axes.begin()));
    WeightInfo obtained_info = ltb->getInfo(entry1);
    double info_at_1 = obtained_info.at(v1);
    EXPECT_EQ(info_at_1, 41);
    double info_at_2 = obtained_info.at(v2);
    EXPECT_EQ(info_at_2, 42);

    EXPECT_B2FATAL(ltb->getInfo(entry1a));

    ltb->defineOutOfRangeWeight(info3);
    obtained_info = ltb->getInfo(entry1a);
    info_at_2 = obtained_info.at(v2);
    EXPECT_EQ(info_at_2, 22);

  }

}  // namespace
