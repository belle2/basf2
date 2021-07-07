/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/****************************************************************
 * This test files used to be the continuumSuppression test.    *
 ****************************************************************/

#include <analysis/ContinuumSuppression/Thrust.h>
#include <analysis/ContinuumSuppression/CleoCones.h>
#include <analysis/ContinuumSuppression/FoxWolfram.h>
#include <analysis/ContinuumSuppression/HarmonicMoments.h>
#include <analysis/ContinuumSuppression/SphericityEigenvalues.h>

#include <TVector3.h>
#include <TRandom3.h>
#include <TMath.h>
#include <boost/math/special_functions/legendre.hpp>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  class eventShapeCoreAlgorithmTest : public ::testing::Test {
  protected:
  };

  /** Test the calculation of a thrust axis */
  TEST_F(eventShapeCoreAlgorithmTest, Thrust)
  {
    std::vector<TVector3> momenta;
    // random generated numbers
    momenta.emplace_back(0.5935352844151847, 0.28902324918117417, 0.9939000705771412);
    momenta.emplace_back(0.7097025137911714, 0.5118418422879152, 0.44501044145648994);
    momenta.emplace_back(0.6005771199332856, 0.12366608492454145, 0.7541373665256832);
    momenta.emplace_back(0.8548902083897467, 0.6887268865943484, 0.34301136659215437);
    momenta.emplace_back(0.26863521039211535, 0.011148638667487942, 0.96186334199901);

    const TVector3 thrustB = Thrust::calculateThrust(momenta);

    EXPECT_FLOAT_EQ(0.925838, thrustB.Mag());
    EXPECT_FLOAT_EQ(0.571661, thrustB.X());
    EXPECT_FLOAT_EQ(0.306741, thrustB.Y());
    EXPECT_FLOAT_EQ(0.660522, thrustB.Z());
  }

  /** Test the calculation of the CleoClones variables */
  TEST_F(eventShapeCoreAlgorithmTest, CleoCones)
  {
    const bool use_all = true;
    const bool use_roe = true;
    std::vector<TVector3> momenta;
    std::vector<TVector3> sig_side_momenta;
    std::vector<TVector3> roe_side_momenta;

    // "Signal Side"
    sig_side_momenta.emplace_back(0.5429965262452898, 0.37010582077332344, 0.0714978744529432);
    sig_side_momenta.emplace_back(0.34160659934755344, 0.6444967896760643, 0.18455766323674105);
    sig_side_momenta.emplace_back(0.9558442475237068, 0.3628892505037786, 0.545225050633818);
    sig_side_momenta.emplace_back(0.8853521332124835, 0.340704481181513, 0.34728211023189237);
    sig_side_momenta.emplace_back(0.3155615844988947, 0.8307541128801257, 0.45701302024212986);

    // "ROE Side"
    roe_side_momenta.emplace_back(0.6100164897524695, 0.5077455724845565, 0.06639458334119974);
    roe_side_momenta.emplace_back(0.5078972239903029, 0.9196504908351234, 0.3710366834603026);
    roe_side_momenta.emplace_back(0.06252858849289977, 0.4680168989606487, 0.4056055050148607);
    roe_side_momenta.emplace_back(0.61672460498333, 0.4472311336875816, 0.31288581834261064);
    roe_side_momenta.emplace_back(0.18544654870476218, 0.0758107751704592, 0.31909701462121065);

    // "All momenta"
    momenta.emplace_back(0.5429965262452898, 0.37010582077332344, 0.0714978744529432);
    momenta.emplace_back(0.34160659934755344, 0.6444967896760643, 0.18455766323674105);
    momenta.emplace_back(0.9558442475237068, 0.3628892505037786, 0.545225050633818);
    momenta.emplace_back(0.8853521332124835, 0.340704481181513, 0.34728211023189237);
    momenta.emplace_back(0.3155615844988947, 0.8307541128801257, 0.45701302024212986);
    momenta.emplace_back(0.6100164897524695, 0.5077455724845565, 0.06639458334119974);
    momenta.emplace_back(0.5078972239903029, 0.9196504908351234, 0.3710366834603026);
    momenta.emplace_back(0.06252858849289977, 0.4680168989606487, 0.4056055050148607);
    momenta.emplace_back(0.61672460498333, 0.4472311336875816, 0.31288581834261064);
    momenta.emplace_back(0.18544654870476218, 0.0758107751704592, 0.31909701462121065);


    // Calculate thrust from "Signal Side"
    const TVector3 thrustB = Thrust::calculateThrust(sig_side_momenta);

    CleoCones myCleoCones(momenta, roe_side_momenta, thrustB, use_all, use_roe);

    const auto& cleo_cone_with_all = myCleoCones.cleo_cone_with_all();
    const auto& cleo_cone_with_roe = myCleoCones.cleo_cone_with_roe();

    EXPECT_FLOAT_EQ(0.823567, cleo_cone_with_all[0]);
    EXPECT_FLOAT_EQ(4.7405558, cleo_cone_with_all[1]);
    EXPECT_FLOAT_EQ(1.7517139, cleo_cone_with_all[2]);
    EXPECT_FLOAT_EQ(0.37677661, cleo_cone_with_all[3]);
    EXPECT_FLOAT_EQ(0.622467, cleo_cone_with_all[4]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_all[5]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_all[6]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_all[7]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_all[8]);

    EXPECT_FLOAT_EQ(0.823567, cleo_cone_with_roe[0]);
    EXPECT_FLOAT_EQ(1.9106253, cleo_cone_with_roe[1]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_roe[2]);
    EXPECT_FLOAT_EQ(0.37677661, cleo_cone_with_roe[3]);
    EXPECT_FLOAT_EQ(0.622467, cleo_cone_with_roe[4]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_roe[5]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_roe[6]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_roe[7]);
    EXPECT_FLOAT_EQ(0, cleo_cone_with_roe[8]);
  }

  /** Test the calculation of the Fox-Wolfram moments */
  TEST_F(eventShapeCoreAlgorithmTest, FoxWolfram)
  {
    std::vector<TVector3> momenta;

    momenta.emplace_back(0.5429965262452898, 0.37010582077332344, 0.0714978744529432);
    momenta.emplace_back(0.34160659934755344, 0.6444967896760643, 0.18455766323674105);
    momenta.emplace_back(0.9558442475237068, 0.3628892505037786, 0.545225050633818);
    momenta.emplace_back(0.8853521332124835, 0.340704481181513, 0.34728211023189237);
    momenta.emplace_back(0.3155615844988947, 0.8307541128801257, 0.45701302024212986);
    momenta.emplace_back(0.6100164897524695, 0.5077455724845565, 0.06639458334119974);
    momenta.emplace_back(0.5078972239903029, 0.9196504908351234, 0.3710366834603026);
    momenta.emplace_back(0.06252858849289977, 0.4680168989606487, 0.4056055050148607);
    momenta.emplace_back(0.61672460498333, 0.4472311336875816, 0.31288581834261064);
    momenta.emplace_back(0.18544654870476218, 0.0758107751704592, 0.31909701462121065);

    FoxWolfram FW(momenta);
    FW.calculateBasicMoments();
    EXPECT_FLOAT_EQ(0.63011014, FW.getR(2));

    // Tests using the analytic values provided by Fox & Wolfram
    // for simple distributions
    // first: Back-to-back versors
    momenta.clear();
    momenta.emplace_back(1., 0., 0.);
    momenta.emplace_back(-1., 0., 0.);

    FW.setMomenta(momenta);
    FW.calculateBasicMoments();

    EXPECT_TRUE(TMath::Abs(FW.getR(0) - 1) < 0.001);
    EXPECT_TRUE(TMath::Abs(FW.getR(1)) < 0.001);
    EXPECT_TRUE(TMath::Abs(FW.getR(2) - 1) < 0.001);
    EXPECT_TRUE(TMath::Abs(FW.getR(3)) < 0.001);
    EXPECT_TRUE(TMath::Abs(FW.getR(4) - 1) < 0.001);

    // second: equatorial line
    momenta.clear();
    TRandom3 rnd;
    for (int i = 0; i < 10000; i++) {
      double phi = rnd.Uniform(0., 2 * TMath::Pi());
      momenta.emplace_back(TMath::Cos(phi), TMath::Sin(phi), 0.);
    }
    FW.setMomenta(momenta);
    FW.calculateBasicMoments();

    EXPECT_TRUE(TMath::Abs(FW.getR(0) - 1.) < 0.001);
    EXPECT_TRUE(TMath::Abs(FW.getR(1)) < 0.001);
    EXPECT_TRUE(TMath::Abs(FW.getR(2) - 0.25) < 0.001);
    EXPECT_TRUE(TMath::Abs(FW.getR(3)) < 0.001);
    EXPECT_TRUE(TMath::Abs(FW.getR(4) - 0.14) < 0.001);

  }



  /** Test the calculation of the Harmonic moments */
  TEST_F(eventShapeCoreAlgorithmTest, HarmonicMoments)
  {

    float dummySqrtS = 10.;
    std::vector<TVector3> partMom;

    partMom.emplace_back(0.5429965262452898, 0.37010582077332344, 0.0714978744529432);
    partMom.emplace_back(0.34160659934755344, 0.6444967896760643, 0.18455766323674105);
    partMom.emplace_back(0.9558442475237068, 0.3628892505037786, 0.545225050633818);
    partMom.emplace_back(0.8853521332124835, 0.340704481181513, 0.34728211023189237);
    partMom.emplace_back(0.3155615844988947, 0.8307541128801257, 0.45701302024212986);
    partMom.emplace_back(0.6100164897524695, 0.5077455724845565, 0.06639458334119974);
    partMom.emplace_back(0.5078972239903029, 0.9196504908351234, 0.3710366834603026);
    partMom.emplace_back(0.06252858849289977, 0.4680168989606487, 0.4056055050148607);
    partMom.emplace_back(0.61672460498333, 0.4472311336875816, 0.31288581834261064);
    partMom.emplace_back(0.18544654870476218, 0.0758107751704592, 0.31909701462121065);

    TVector3 axis(0., 0., 1.);
    // repeats the calculation
    double moment[9] = {0.};
    for (auto& p : partMom) {
      double pMag = p.Mag();
      double cTheta = p.Dot(axis) / pMag;
      for (short i = 0; i < 9; i++)
        moment[i] += pMag * boost::math::legendre_p(i, cTheta) / dummySqrtS;
    }

    HarmonicMoments MM(partMom, axis);
    MM.calculateAllMoments();

    for (short i = 0; i < 9; i++)
      EXPECT_FLOAT_EQ(moment[i], MM.getMoment(i, dummySqrtS));
  }




  /** Test the calculation of the Sphericity eigenvalues and eigenvectors */
  TEST_F(eventShapeCoreAlgorithmTest, Sphericity)
  {
    std::vector<TVector3> partMom;

    partMom.emplace_back(0.5429965262452898, 0.37010582077332344, 0.0714978744529432);
    partMom.emplace_back(0.34160659934755344, 0.6444967896760643, 0.18455766323674105);
    partMom.emplace_back(0.9558442475237068, 0.3628892505037786, 0.545225050633818);
    partMom.emplace_back(0.8853521332124835, 0.340704481181513, 0.34728211023189237);
    partMom.emplace_back(0.3155615844988947, 0.8307541128801257, 0.45701302024212986);
    partMom.emplace_back(0.6100164897524695, 0.5077455724845565, 0.06639458334119974);
    partMom.emplace_back(0.5078972239903029, 0.9196504908351234, 0.3710366834603026);
    partMom.emplace_back(0.06252858849289977, 0.4680168989606487, 0.4056055050148607);
    partMom.emplace_back(0.61672460498333, 0.4472311336875816, 0.31288581834261064);
    partMom.emplace_back(0.18544654870476218, 0.0758107751704592, 0.31909701462121065);


    SphericityEigenvalues Sph(partMom);
    EXPECT_FLOAT_EQ(0., Sph.getEigenvalue(0));
    EXPECT_FLOAT_EQ(0., Sph.getEigenvalue(1));
    EXPECT_FLOAT_EQ(0., Sph.getEigenvalue(2));

    Sph.calculateEigenvalues();

    EXPECT_FLOAT_EQ(0.87272972, Sph.getEigenvalue(0));
    EXPECT_FLOAT_EQ(0.095489956, Sph.getEigenvalue(1));
    EXPECT_FLOAT_EQ(0.031780295, Sph.getEigenvalue(2));
    EXPECT_FLOAT_EQ(1., Sph.getEigenvalue(0) + Sph.getEigenvalue(1) + Sph.getEigenvalue(2));
  }


}  // namespace
