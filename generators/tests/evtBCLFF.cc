#include <generators/evtgen/models/EvtBCLFF.h>
#include <generators/evtgen/EvtGenInterface.h>
#include <framework/utilities/FileSystem.h>

#include <EvtGenBase/EvtId.hh>
#include <EvtGenBase/EvtPDL.hh>

#include <gtest/gtest.h>

#include <string>

namespace Belle2 {

  /** The fixture for testing the EvtBCLFF. */
  class EvtBCLFFTest : public ::testing::Test {
  protected:
    static void SetUpTestCase()
    {
      std::string decayFileName = FileSystem::findFile("generators/evtgen/decayfiles/DECAY_BELLE2.DEC");
      s_evtgen = EvtGenInterface::createEvtGen(decayFileName);
    }

    static void TearDownTestCase()
    {
      delete s_evtgen;
      s_evtgen = nullptr;
    }

    static EvtGen* s_evtgen;
  };

  EvtGen* EvtBCLFFTest::s_evtgen = nullptr;

  /// Test calculation of scalar BCL FF.
  TEST_F(EvtBCLFFTest, Scalar)
  {
    EvtId B0 = EvtPDL::getId("B0");
    EvtId M = EvtPDL::getId("pi+");
    const auto mB = EvtPDL::getMeanMass(B0);
    const auto mB2 = mB * mB;
    const auto mM = EvtPDL::getMeanMass(M);
    const auto mM2 = mM * mM;
    const auto q2min = 0.0;
    const auto q2max = mB2 + mM2 - 2 * mB * mM;

    int nArguments = 8;
    double arguments[] = {0.419, -0.495, -0.43, 0.22, 0.510, -1.700, 1.53, 4.52};

    EvtBCLFF bclff(nArguments, arguments);

    double fplus = 0;
    double fzero = 0;

    bclff.getscalarff(B0, M, q2min, 0, &fplus, &fzero);

    ASSERT_NEAR(0.253, fplus, 0.003);
    ASSERT_NEAR(0.253, fzero, 0.003);

    bclff.getscalarff(B0, M, q2max, 0, &fplus, &fzero);

    ASSERT_NEAR(7.614, fplus, 0.003);
    ASSERT_NEAR(1.006, fzero, 0.003);


  } // Scalar Tests

  /// Test calculation of vector BCL FF.
  TEST_F(EvtBCLFFTest, Vector)
  {
    EvtId B0 = EvtPDL::getId("B0");
    EvtId M = EvtPDL::getId("rho+");
    const auto mB = EvtPDL::getMeanMass(B0);
    const auto mB2 = mB * mB;
    const auto mM = EvtPDL::getMeanMass(M);
    const auto mM2 = mM * mM;
    const auto q2min = 0.0;
    const auto q2max = mB2 + mM2 - 2 * mB * mM;

    int nArguments = 11;
    double arguments[] = { -0.833,  1.331,  0.262,  0.394,  0.163,  0.297,  0.759,  0.465,  0.327, -0.86 ,  1.802};

    EvtBCLFF bclFF(nArguments, arguments);

    double A0 = 0;
    double A1 = 0;
    double A2 = 0;
    double V = 0;

    bclFF.getvectorff(B0, M, q2min, 0, &A1, &A2, &V, &A0);

    ASSERT_NEAR(0.356, A0, 0.003);
    ASSERT_NEAR(0.262, A1, 0.003);
    ASSERT_NEAR(0.327, V, 0.003);

    bclFF.getvectorff(B0, M, q2max, 0, &A1, &A2, &V, &A0);

    ASSERT_NEAR(2.123, A0, 0.003);
    ASSERT_NEAR(0.497, A1, 0.003);
    ASSERT_NEAR(2.014, V, 0.003);

  } // Vector Tests

}