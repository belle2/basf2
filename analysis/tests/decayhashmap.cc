/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <analysis/utility/DecayNode.h>
#include <analysis/utility/DecayTree.h>
#include <analysis/utility/DecayForest.h>
#include <framework/gearbox/Const.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;


namespace {

  TEST(DecayNodeTest, Constructor)
  {
    DecayNode photon_1(22);
    DecayNode photon_2(22);
    DecayNode pi0(111, {photon_1, photon_2});

    EXPECT_EQ(photon_1.pdg, Const::photon.getPDGCode());
    EXPECT_EQ(photon_1.daughters.size(), 0);

    EXPECT_EQ(pi0.pdg, Const::pi0.getPDGCode());
    EXPECT_EQ(pi0.daughters.size(), 2);

  }

  TEST(DecayNodeTest, FindDecay)
  {
    DecayNode photon_1(22);
    DecayNode photon_2(22);
    DecayNode photon_3(22);
    DecayNode pi0_1(111, {photon_1, photon_2});
    DecayNode pi0_2(111);

    DecayNode e_1(11);
    DecayNode e_2(-11);
    DecayNode pi0_3(111, {e_1, e_2});

    DecayNode pi(211);

    DecayNode D_1(321, {pi, pi0_1});
    DecayNode D_2(321, {pi, pi0_2});
    DecayNode D_3(321, {pi, pi0_3});

    EXPECT_TRUE(D_1.find_decay(D_1));
    EXPECT_TRUE(D_1.find_decay(D_2));
    EXPECT_FALSE(D_1.find_decay(D_3));
    EXPECT_TRUE(D_1.find_decay(pi0_1));
    EXPECT_TRUE(D_1.find_decay(pi0_2));
    EXPECT_FALSE(D_1.find_decay(pi0_3));

    EXPECT_TRUE(D_2.find_decay(D_1));
    EXPECT_TRUE(D_2.find_decay(D_2));
    EXPECT_TRUE(D_2.find_decay(D_3));
    EXPECT_TRUE(D_2.find_decay(pi0_1));
    EXPECT_TRUE(D_2.find_decay(pi0_2));
    EXPECT_TRUE(D_2.find_decay(pi0_3));

    EXPECT_FALSE(D_3.find_decay(D_1));
    EXPECT_TRUE(D_3.find_decay(D_2));
    EXPECT_TRUE(D_3.find_decay(D_3));
    EXPECT_FALSE(D_3.find_decay(pi0_1));
    EXPECT_TRUE(D_3.find_decay(pi0_2));
    EXPECT_TRUE(D_3.find_decay(pi0_3));

    EXPECT_TRUE(pi0_1.find_decay(photon_1));
    EXPECT_TRUE(pi0_1.find_decay(photon_2));
    EXPECT_TRUE(pi0_1.find_decay(photon_3));
    EXPECT_FALSE(pi0_1.find_decay(e_1));
  }

  TEST(DecayTreeTest, FindDecay)
  {
    DecayTree photon_1("22");
    DecayTree pi0_1("111 (--> 22 22)");
    DecayTree pi0_2("111 ");

    DecayTree e_1("11");
    DecayTree e_2("-11");
    DecayTree pi0_3("111 (--> 11 -11)");

    DecayTree D_1("321 (--> 11 111 (--> 22 22))");
    DecayTree D_2("321 (--> 11 111)");
    DecayTree D_3("321 (--> 11 111 (--> 11 -11))");

    EXPECT_TRUE(D_1.find_decay(D_1));
    EXPECT_TRUE(D_1.find_decay(D_2));
    EXPECT_FALSE(D_1.find_decay(D_3));
    EXPECT_TRUE(D_1.find_decay(pi0_1));
    EXPECT_TRUE(D_1.find_decay(pi0_2));
    EXPECT_FALSE(D_1.find_decay(pi0_3));

    EXPECT_TRUE(D_2.find_decay(D_1));
    EXPECT_TRUE(D_2.find_decay(D_2));
    EXPECT_TRUE(D_2.find_decay(D_3));
    EXPECT_TRUE(D_2.find_decay(pi0_1));
    EXPECT_TRUE(D_2.find_decay(pi0_2));
    EXPECT_TRUE(D_2.find_decay(pi0_3));

    EXPECT_FALSE(D_3.find_decay(D_1));
    EXPECT_TRUE(D_3.find_decay(D_2));
    EXPECT_TRUE(D_3.find_decay(D_3));
    EXPECT_FALSE(D_3.find_decay(pi0_1));
    EXPECT_TRUE(D_3.find_decay(pi0_2));
    EXPECT_TRUE(D_3.find_decay(pi0_3));

    EXPECT_TRUE(pi0_1.find_decay(photon_1));
    EXPECT_FALSE(pi0_1.find_decay(e_1));
    EXPECT_TRUE(pi0_3.find_decay(e_2));

  }

  TEST(DecayTreeTest, IsValid)
  {
    DecayTree photon_1("22");
    DecayTree pi0_1("111 (--> 22 22)");

    EXPECT_TRUE(photon_1.isValid());
    EXPECT_TRUE(pi0_1.isValid());

    DecayTree no_match(" No match ");
    EXPECT_FALSE(no_match.isValid());

  }

  TEST(DecayTreeTest, MatchSymbolPosition)
  {

    DecayTree D_0("321 (--> 11 111 (--> 11 -11))");
    DecayTree D_1("321 (--> 11 111 (--> ^11 -11))");
    DecayTree D_2("321 (--> ^11 111 (--> 11 -11))");
    DecayTree D_3("^321 (--> 11 111 (--> 11 -11))");

    EXPECT_EQ(D_0.getMatchSymbolPosition(), -1);
    EXPECT_EQ(D_0.getNumberOfDecayNodes(), 5);

    EXPECT_EQ(D_1.getMatchSymbolPosition(), 3);
    EXPECT_EQ(D_1.getNumberOfDecayNodes(), 5);
    EXPECT_EQ(D_1.getDecayNode(3).pdg, Const::electron.getPDGCode());
    EXPECT_EQ(D_1.getDecayNode(3).daughters.size(), 0);

    EXPECT_EQ(D_2.getMatchSymbolPosition(), 1);
    EXPECT_EQ(D_2.getNumberOfDecayNodes(), 5);
    EXPECT_EQ(D_2.getDecayNode(1).pdg, Const::electron.getPDGCode());
    EXPECT_EQ(D_2.getDecayNode(1).daughters.size(), 0);

    EXPECT_EQ(D_3.getMatchSymbolPosition(), 0);
    EXPECT_EQ(D_3.getNumberOfDecayNodes(), 5);
    EXPECT_EQ(D_3.getDecayNode(0).pdg, Const::kaon.getPDGCode());
    EXPECT_EQ(D_3.getDecayNode(0).daughters.size(), 2);

  }

  TEST(DecayTreeTest, Constructor)
  {

    DecayTree photon("22");
    EXPECT_EQ(photon.getNumberOfDecayNodes(), 1);
    EXPECT_EQ(photon.getDecayNode(0).pdg, Const::photon.getPDGCode());
    EXPECT_EQ(photon.getDecayNode(0).daughters.size(), 0);

    DecayTree pi0("111 (--> 22 22)");
    EXPECT_EQ(pi0.getNumberOfDecayNodes(), 3);
    EXPECT_EQ(pi0.getDecayNode(0).pdg, Const::pi0.getPDGCode());
    EXPECT_EQ(pi0.getDecayNode(0).daughters.size(), 2);
    EXPECT_EQ(pi0.getDecayNode(1).pdg, Const::photon.getPDGCode());
    EXPECT_EQ(pi0.getDecayNode(1).daughters.size(), 0);
    EXPECT_EQ(pi0.getDecayNode(2).pdg, Const::photon.getPDGCode());
    EXPECT_EQ(pi0.getDecayNode(2).daughters.size(), 0);

    DecayTree D("321 (--> 11 111 (--> 11 -11))");
    EXPECT_EQ(D.getNumberOfDecayNodes(), 5);
    EXPECT_EQ(D.getDecayNode(0).pdg, Const::kaon.getPDGCode());
    EXPECT_EQ(D.getDecayNode(0).daughters.size(), 2);
    EXPECT_EQ(D.getDecayNode(1).pdg, Const::electron.getPDGCode());
    EXPECT_EQ(D.getDecayNode(1).daughters.size(), 0);
    EXPECT_EQ(D.getDecayNode(2).pdg, Const::pi0.getPDGCode());
    EXPECT_EQ(D.getDecayNode(2).daughters.size(), 2);
    EXPECT_EQ(D.getDecayNode(3).pdg, Const::electron.getPDGCode());
    EXPECT_EQ(D.getDecayNode(3).daughters.size(), 0);
    EXPECT_EQ(D.getDecayNode(4).pdg, -Const::electron.getPDGCode());
    EXPECT_EQ(D.getDecayNode(4).daughters.size(), 0);

  }

  TEST(DecayForestTest, Constructor)
  {
    DecayForest D("321 (--> 11 111 (--> 11 -11)) | No match | 111 (--> 11 -11) | 512 (--> 321 (--> 11 111 (--> 11 -11)) ) ", false);

    EXPECT_EQ(D.getNumberOfTrees(), 4);
    EXPECT_EQ(D.getOriginalTreeNumber(), 2);

    auto& reconstructed_decay_tree = D.getReconstructedTree();
    EXPECT_EQ(reconstructed_decay_tree.getNumberOfDecayNodes(), 5);
    EXPECT_EQ(reconstructed_decay_tree.getDecayNode(0).pdg, Const::kaon.getPDGCode());

    auto& no_match_decay_tree = D.getTree(1);
    EXPECT_FALSE(no_match_decay_tree.isValid());

    auto& original_decay_tree = D.getOriginalTree();
    EXPECT_TRUE(original_decay_tree.isValid());
    EXPECT_EQ(original_decay_tree.getDecayNode(0).pdg, Const::pi0.getPDGCode());

  }

  TEST(DecayForestTest, SaveMemory)
  {
    DecayForest D("321 (--> 11 111 (--> 11 -11)) | No match | 111 (--> 11 -11) | 512 (--> 321 (--> 11 111 (--> 11 -11)) ) ", true);

    EXPECT_EQ(D.getNumberOfTrees(), 3);
    EXPECT_EQ(D.getOriginalTreeNumber(), 2);

    auto& reconstructed_decay_tree = D.getReconstructedTree();
    EXPECT_EQ(reconstructed_decay_tree.getNumberOfDecayNodes(), 5);
    EXPECT_EQ(reconstructed_decay_tree.getDecayNode(0).pdg, Const::kaon.getPDGCode());

    auto& no_match_decay_tree = D.getTree(1);
    EXPECT_FALSE(no_match_decay_tree.isValid());

    auto& original_decay_tree = D.getOriginalTree();
    EXPECT_TRUE(original_decay_tree.isValid());
    EXPECT_EQ(original_decay_tree.getDecayNode(0).pdg, Const::pi0.getPDGCode());

  }

  TEST(DecayForestTest, FindB2TauNuDecay)
  {
    DecayForest
    Y(" 300553 (--> -521 (--> 15 (--> -211)) 521 (--> -423 (--> -421 (--> 321 11) 22) 211 211 -211 111 (--> 22 22))) |  ^300553 (--> 521 (--> -423 (--> -421 (--> 321 11 -12) 22) 10223 (--> 213 (--> 211 111 (--> 22 22)) -211) 211) -521 (--> 15 (--> 16 -211 111 (--> 22 22)) -16)) |  300553 (--> 521 (--> -423 (--> -421 (--> 321 11 -12) 22) 10223 (--> 213 (--> 211 111 (--> 22 22)) -211) 211) ^-521 (--> 15 (--> 16 -211 111 (--> 22 22)) -16)) |  300553 (--> 521 (--> -423 (--> -421 (--> 321 11 -12) 22) 10223 (--> 213 (--> 211 111 (--> 22 22)) -211) 211) -521 (--> ^15 (--> 16 -211 111 (--> 22 22)) -16)) |  300553 (--> 521 (--> -423 (--> -421 (--> 321 11 -12) 22) 10223 (--> 213 (--> 211 111 (--> 22 22)) -211) 211) -521 (--> 15 (--> 16 ^-211 111 (--> 22 22)) -16)) |  300553 (--> ^521 (--> -423 (--> -421 (--> 321 11 -12) 22) 10223 (--> 213 (--> 211 111 (--> 22 22)) -211) 211) -521 (--> 15 (--> 16 -211 111 (--> 22 22)) -16)) |  300553 (--> 521 (--> ^-423 (--> -421 (--> 321 11 -12) 22) 10223 (--> 213 (--> 211 111 (--> 22 22)) -211) 211) -521 (--> 15 (--> 16 -211 111 (--> 22 22)) -16)) |  300553 (--> 521 (--> -423 (--> ^-421 (--> 321 11 -12) 22) 10223 (--> 213 (--> 211 111 (--> 22 22)) -211) 211) -521 (--> 15 (--> 16 -211 111 (--> 22 22)) -16)) |  300553 (--> 521 (--> -423 (--> -421 (--> ^321 11 -12) 22) 10223 (--> 213 (--> 211 111 (--> 22 22)) -211) 211) -521 (--> 15 (--> 16 -211 111 (--> 22 22)) -16)) |  300553 (--> 521 (--> -423 (--> -421 (--> 321 ^11 -12) 22) 10223 (--> 213 (--> 211 111 (--> 22 22)) -211) 211) -521 (--> 15 (--> 16 -211 111 (--> 22 22)) -16)) |  300553 (--> 521 (--> -423 (--> -421 (--> 321 11 -12) ^22) 10223 (--> 213 (--> 211 111 (--> 22 22)) -211) 211) -521 (--> 15 (--> 16 -211 111 (--> 22 22)) -16)) |  300553 (--> 521 (--> -423 (--> -421 (--> 321 11 -12) 22) 10223 (--> 213 (--> ^211 111 (--> 22 22)) -211) 211) -521 (--> 15 (--> 16 -211 111 (--> 22 22)) -16)) |  300553 (--> 521 (--> -423 (--> -421 (--> 321 11 -12) 22) 10223 (--> 213 (--> 211 111 (--> 22 22)) -211) ^211) -521 (--> 15 (--> 16 -211 111 (--> 22 22)) -16)) |  300553 (--> 521 (--> -423 (--> -421 (--> 321 11 -12) 22) 10223 (--> 213 (--> 211 111 (--> 22 22)) ^-211) 211) -521 (--> 15 (--> 16 -211 111 (--> 22 22)) -16)) |  300553 (--> 521 (--> -423 (--> -421 (--> 321 11 -12) 22) 10223 (--> 213 (--> 211 ^111 (--> 22 22)) -211) 211) -521 (--> 15 (--> 16 -211 111 (--> 22 22)) -16)) |  300553 (--> 521 (--> -423 (--> -421 (--> 321 11 -12) 22) 10223 (--> 213 (--> 211 111 (--> ^22 22)) -211) 211) -521 (--> 15 (--> 16 -211 111 (--> 22 22)) -16)) |  300553 (--> 521 (--> -423 (--> -421 (--> 321 11 -12) 22) 10223 (--> 213 (--> 211 111 (--> 22 ^22)) -211) 211) -521 (--> 15 (--> 16 -211 111 (--> 22 22)) -16))");

    DecayTree b2taunu_cc("521 (--> -15 16)");
    DecayTree b2taunu("-521 (--> 15 -16)");
    DecayTree b2taunu2e("-521 (--> 15 (11 -12 16) -16)");
    DecayTree b2taunu2rho("-521 (--> 15 (16 -211 111 (--> 22 22)) -16)");

    EXPECT_FALSE(Y.getOriginalTree().find_decay(b2taunu_cc));
    EXPECT_TRUE(Y.getOriginalTree().find_decay(b2taunu));
    EXPECT_FALSE(Y.getOriginalTree().find_decay(b2taunu2e));
    EXPECT_TRUE(Y.getOriginalTree().find_decay(b2taunu2rho));

  }

  TEST(DecayForestTest, RemoveRadiativePhotons)
  {
    DecayForest B("521 (--> -421 (--> 325 (--> 321 111 (--> 22 22)) -211) -13 14 22)", false, true);
    DecayForest B2("521 (--> -11 12 22)", false, true);
    DecayForest B3("521 (--> -11 12 22)", false, false);

    DecayTree case1(" 521 (--> -421 (-->  325 (--> 321 111 (--> 22 22)) -211) -13 14)", true);
    DecayTree case2(" 521 (--> -11 12 22)", true);
    DecayTree case3(" 521 (--> -11 12 22)", false);
    DecayTree case4(" 521 (--> -11 12)", false);


    DecayTree case5(" 521 (--> -11 12 22)", false);
    DecayTree case6(" 521 (--> -11 12 22)", true);
    DecayTree case7(" 521 (--> -11 12)", false);

    EXPECT_TRUE(B.getOriginalTree().find_decay(case1));

    EXPECT_TRUE(B2.getOriginalTree().find_decay(case2));
    EXPECT_FALSE(B2.getOriginalTree().find_decay(case3));
    EXPECT_TRUE(B2.getOriginalTree().find_decay(case4));

    EXPECT_TRUE(B3.getOriginalTree().find_decay(case5));
    EXPECT_FALSE(B3.getOriginalTree().find_decay(case6));
    EXPECT_FALSE(B3.getOriginalTree().find_decay(case7));
  }

}  // namespace
