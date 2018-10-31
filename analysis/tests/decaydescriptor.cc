/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace Belle2;

namespace {
  TEST(DecayDescriptorTest, TrivialUse)
  {
    // trivial decay descriptor == particle name
    DecayDescriptor dd;
    bool initok = dd.init("K+");
    EXPECT_EQ(initok, true);
    ASSERT_NE(dd.getMother(), nullptr);
    EXPECT_EQ(dd.getMother()->getName(), "K+");
    EXPECT_EQ(dd.getMother()->getLabel(), "");
    EXPECT_EQ(dd.getMother()->getFullName(), "K+");
    EXPECT_EQ(dd.getMother()->getPDGCode(), 321);
    EXPECT_EQ(dd.getNDaughters(), 0);
  }

  TEST(DecayDescriptorTest, NormalBehaviour)
  {
    DecayDescriptor dd;
    bool initok = dd.init("B0:cand -> K+:loose pi-:loose");

    EXPECT_EQ(initok, true);
    ASSERT_NE(dd.getMother(), nullptr);
    EXPECT_EQ(dd.getMother()->getName(), "B0");
    EXPECT_EQ(dd.getMother()->getLabel(), "cand");
    EXPECT_EQ(dd.getMother()->getPDGCode(), 511);
    EXPECT_EQ(dd.getMother()->getFullName(), "B0:cand");
    EXPECT_EQ(dd.getNDaughters(), 2);

    EXPECT_EQ(dd.getDaughter(0)->getNDaughters(), 0);
    EXPECT_EQ(dd.getDaughter(0)->getMother()->getName(), "K+");
    EXPECT_EQ(dd.getDaughter(0)->getMother()->getLabel(), "loose");
    EXPECT_EQ(dd.getDaughter(0)->getMother()->getPDGCode(), 321);
    EXPECT_EQ(dd.getDaughter(0)->getMother()->getFullName(), "K+:loose");

    EXPECT_EQ(dd.getDaughter(1)->getNDaughters(), 0);
    EXPECT_EQ(dd.getDaughter(1)->getMother()->getName(), "pi-");
    EXPECT_EQ(dd.getDaughter(1)->getMother()->getLabel(), "loose");
    EXPECT_EQ(dd.getDaughter(1)->getMother()->getPDGCode(), -211);
    EXPECT_EQ(dd.getDaughter(1)->getMother()->getFullName(), "pi-:loose");

    ASSERT_EQ(dd.getDaughter(2), nullptr);
  }

  TEST(DecayDescriptorTest, Granddaughters)
  {
    DecayDescriptor dd;
    bool initok = dd.init(
                    "B0:cand -> [D0:dau1 -> K+:grandau pi-:grandau] [pi0:dau2 -> gamma:grandau [gamma:converted -> e+:gtgrandau e-:gtgrandau]]"
                  );
    EXPECT_EQ(initok, true);
    ASSERT_NE(dd.getMother(), nullptr);
    EXPECT_EQ(dd.getNDaughters(), 2);

    // D0 -> K pi
    EXPECT_EQ(dd.getDaughter(0)->getMother()->getName(), "D0");
    EXPECT_EQ(dd.getDaughter(0)->getMother()->getPDGCode(), 421);
    EXPECT_EQ(dd.getDaughter(0)->getNDaughters(), 2);
    EXPECT_EQ(dd.getDaughter(0)->getDaughter(0)->getMother()->getName(), "K+");
    EXPECT_EQ(dd.getDaughter(0)->getDaughter(0)->getNDaughters(), 0);
    EXPECT_EQ(dd.getDaughter(0)->getDaughter(1)->getMother()->getName(), "pi-");
    EXPECT_EQ(dd.getDaughter(0)->getDaughter(1)->getNDaughters(), 0);
    ASSERT_EQ(dd.getDaughter(0)->getDaughter(2), nullptr);

    // pi0 -> gamma gamma; gamma -> ee
    EXPECT_EQ(dd.getDaughter(1)->getMother()->getName(), "pi0");
    EXPECT_EQ(dd.getDaughter(1)->getMother()->getPDGCode(), 111);
    EXPECT_EQ(dd.getDaughter(1)->getNDaughters(), 2);
    EXPECT_EQ(dd.getDaughter(1)->getDaughter(0)->getMother()->getName(), "gamma");
    EXPECT_EQ(dd.getDaughter(1)->getDaughter(0)->getMother()->getLabel(), "grandau");
    EXPECT_EQ(dd.getDaughter(1)->getDaughter(0)->getMother()->getPDGCode(), 22);
    EXPECT_EQ(dd.getDaughter(1)->getDaughter(0)->getNDaughters(), 0);
    EXPECT_EQ(dd.getDaughter(1)->getDaughter(1)->getMother()->getName(), "gamma");
    EXPECT_EQ(dd.getDaughter(1)->getDaughter(1)->getMother()->getLabel(), "converted");
    EXPECT_EQ(dd.getDaughter(1)->getDaughter(1)->getMother()->getPDGCode(), 22);
    EXPECT_EQ(dd.getDaughter(1)->getDaughter(1)->getNDaughters(), 2);
    EXPECT_EQ(dd.getDaughter(1)->getDaughter(1)->getDaughter(0)->getMother()->getName(), "e+");
    EXPECT_EQ(dd.getDaughter(1)->getDaughter(1)->getDaughter(0)->getMother()->getPDGCode(), -11);
    EXPECT_EQ(dd.getDaughter(1)->getDaughter(1)->getDaughter(1)->getMother()->getName(), "e-");
    EXPECT_EQ(dd.getDaughter(1)->getDaughter(1)->getDaughter(1)->getMother()->getPDGCode(), 11);
    ASSERT_EQ(dd.getDaughter(1)->getDaughter(1)->getDaughter(2), nullptr);
    ASSERT_EQ(dd.getDaughter(1)->getDaughter(2), nullptr);

    ASSERT_EQ(dd.getDaughter(2), nullptr);
  }

  TEST(DecayDescriptorTest, SelectionParticles)
  {
    DecayDescriptor dd1;
    bool initok = dd1.init("B0:B2Dzpipi -> [D0 -> ^K+:loose pi-:loose] pi+:loose pi-:loose");
    EXPECT_EQ(initok, true);
    std::vector<std::string> names = dd1.getSelectionNames();
    ASSERT_EQ(names.size(), 1);
    EXPECT_EQ(names[0], "B0_D0_K");

    // add another selection particle to an already existing decay descriptor
    // not sure exactly who is using this feature, but might as well test it.
    initok = false;
    initok = dd1.init("B0:B2Dzpipi -> [D0 -> K+:loose ^pi-:loose] pi+:loose pi-:loose");
    EXPECT_EQ(initok, true);
    names = dd1.getSelectionNames();
    ASSERT_EQ(names.size(), 2);
    EXPECT_EQ(names[0], "B0_D0_K");
    EXPECT_EQ(names[1], "B0_D0_pi");

    // more complex decay string with multiple particles of the same type
    initok = false;
    DecayDescriptor dd2;
    initok = dd2.init("vpho:complex -> [D0 -> ^K+:loose pi-:loose] ^e+:loose ^e-:loose ^gamma:loose");
    EXPECT_EQ(initok, true);
    names = dd2.getSelectionNames();
    ASSERT_EQ(names.size(), 4);
    EXPECT_EQ(names[0], "vpho_D0_K");
    EXPECT_EQ(names[1], "vpho_e0");
    EXPECT_EQ(names[2], "vpho_e1");
    EXPECT_EQ(names[3], "vpho_gamma");
  }


  TEST(DecayDescriptorTest, BadLabelTest)
  {
    // use of illegal characters in labels
    DecayDescriptor dd1;
    bool initok = dd1.init("B0:lab[el -> K+:loose pi-:loose");
    EXPECT_EQ(initok, false);
    EXPECT_EQ(dd1.getMother()->getName(), "");
    EXPECT_EQ(dd1.getMother()->getLabel(), "");

    initok = true;
    DecayDescriptor dd2;
    initok = dd2.init("B0:lab^el -> K+:loose pi-:loose");
    EXPECT_EQ(initok, false);
    EXPECT_EQ(dd2.getMother()->getName(), "");
    EXPECT_EQ(dd2.getMother()->getLabel(), "");

    initok = true;
    DecayDescriptor dd3;
    initok = dd3.init("B0:lab]el -> K+:loose pi-:loose");
    EXPECT_EQ(initok, false);
    EXPECT_EQ(dd3.getMother()->getName(), "");
    EXPECT_EQ(dd3.getMother()->getLabel(), "");

    initok = true;
    DecayDescriptor dd4;
    initok = dd4.init("B0:lab>el -> K+:loose pi-:loose");
    EXPECT_EQ(initok, false);
    EXPECT_EQ(dd4.getMother()->getName(), "");
    EXPECT_EQ(dd4.getMother()->getLabel(), "");

    initok = true;
    DecayDescriptor dd5;
    initok = dd5.init("B0:lab:el -> K+:loose pi-:loose");
    EXPECT_EQ(initok, false);
    EXPECT_EQ(dd5.getMother()->getName(), "");
    EXPECT_EQ(dd5.getMother()->getLabel(), "");
  }

  TEST(DecayDescriptorTest, BadGrammarTest)
  {
    DecayDescriptor dd1;
    bool initok = dd1.init("B0:label ---> K+:loose pi-:loose");
    EXPECT_EQ(initok, false);

    initok = true;
    DecayDescriptor dd2;
    initok = dd2.init("B0:label > K+:loose pi-:loose");
    EXPECT_EQ(initok, false);

    initok = true;
    DecayDescriptor dd3;
    initok = dd3.init("B0:label -> K+::loose pi-:loose");
    EXPECT_EQ(initok, false);

    initok = true;
    DecayDescriptor dd4;
    initok = dd4.init("B0:label K+:loose pi-:loose");
    EXPECT_EQ(initok, false);

    initok = true;
    DecayDescriptor dd5;
    initok = dd5.init("B0:label <- K+:loose pi-:loose");
    EXPECT_EQ(initok, false);
  }

  TEST(DecayDescriptorTest, B2ParticleInterface)
  {
    // need datastore for the particles StoreArray
    DataStore::Instance().setInitializeActive(true);
    StoreArray<Particle> particles;
    particles.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    // mock up a composite Belle2::Particle
    TLorentzVector zeroes(0, 0, 0, 0);
    Particle* Kp = particles.appendNew(Particle(zeroes, 321));    // 0
    Particle* pim1 = particles.appendNew(Particle(zeroes, -211)); // 1
    Particle* pim2 = particles.appendNew(Particle(zeroes, -211)); // 2
    Particle* pip = particles.appendNew(Particle(zeroes, 211));   // 3
    Particle* D0 = particles.appendNew(Particle(zeroes, 421));    // 4
    D0->appendDaughter(0);
    D0->appendDaughter(1);
    Particle* B0 = particles.appendNew(Particle(zeroes, 511));    // 5
    B0->appendDaughter(4);
    B0->appendDaughter(3);
    B0->appendDaughter(2);

    // ---
    DecayDescriptor dd;
    bool initok = dd.init("B0:B2Dzpipi -> [D0 -> K+:loose ^pi-:loose] ^pi+:loose pi-:loose");
    ASSERT_EQ(initok, true);

    std::vector<const Particle*> selectionparticles = dd.getSelectionParticles(B0);
    EXPECT_EQ(selectionparticles.size(), 2);
    EXPECT_EQ(selectionparticles[0], pim1);
    EXPECT_EQ(selectionparticles[1], pip);

    EXPECT_B2ERROR(dd.getSelectionParticles(D0));

    EXPECT_B2WARNING(dd.getSelectionParticles(pip));
    EXPECT_B2WARNING(dd.getSelectionParticles(Kp));
    EXPECT_B2WARNING(dd.getSelectionParticles(pim1));
    EXPECT_B2WARNING(dd.getSelectionParticles(pim2));

    EXPECT_EQ(dd.getSelectionParticles(D0).size(), 0);

    DataStore::Instance().reset();
  }
}
