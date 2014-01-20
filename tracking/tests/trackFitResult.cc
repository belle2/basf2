#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/dataobjects/Track.h>
#include <tracking/dataobjects/TrackFitResult.h>
#include <genfit/Track.h>
#include <TMatrixF.h>
#include <genfit/RKTrackRep.h>


#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class TrackFitResultTest : public ::testing::Test {
  protected:
  };

  /** Test simple Getters. */
  TEST_F(TrackFitResultTest, SimpleGetters)
  {
    // choose some sample values with which the test will be run
    TVector3 pos(0.1, 0.1, 0.1);
    TVector3 mom(-1.0, 1.0, 1.0);
    TMatrixF errorMatrix(6, 6);
    double errorMatrixEntry(0.01);
    for (int ii = 0; ii < 6; ii++) {
      for (int jj = ii; jj < 6; jj++) {
        errorMatrix(ii, jj) = errorMatrix(jj, ii) = errorMatrixEntry;
        errorMatrixEntry += 0.01;
      }
    }

    short int charge(1);
    unsigned int pdg(11);
    float pValue(0.45);

    // set up class for testing
    TrackFitResult myResult(pos, mom, charge, errorMatrix, pdg, pValue, true);
    EXPECT_FLOAT_EQ(charge, myResult.getCharge());
    EXPECT_EQ(Const::ParticleType(pdg), myResult.getParticleType());
    EXPECT_FLOAT_EQ(pValue, myResult.getPValue());

  }

  /** Test Position/Momentum Vectors and respective error matrix input and retrieval.*/
  TEST_F(TrackFitResultTest, posMomErrorIO)
  {
    TVector3 pos(0.1, 0.1, 0.1);
    TVector3 mom(1.0, -1.0, -1.0);
    TMatrixF errorMatrix(6, 6);
    for (int ii = 0; ii < 6; ii++) {
      for (int jj = ii; jj < 6; jj++) {
        errorMatrix(ii, jj) = 0.01;
      }
    }

    short int charge(1);
    unsigned int pdg(11);
    float pValue(0.45);
    TrackFitResult myResult(pos, mom, charge, errorMatrix, pdg, pValue, true);

    //The following should be replaced with proper inspection,
    // if the TVector3 are compatible within the tolerance.
    //Position
    EXPECT_FLOAT_EQ(pos.Mag(), myResult.getPosition().Mag());

    EXPECT_FLOAT_EQ(pos.Phi(), myResult.getPosition().Phi());

    EXPECT_FLOAT_EQ(pos.Eta(), myResult.getPosition().Eta());

    //Momentum
    EXPECT_FLOAT_EQ(mom.Mag(), myResult.getMomentum().Mag());

    EXPECT_FLOAT_EQ(mom.Phi(), myResult.getMomentum().Phi());

    EXPECT_FLOAT_EQ(mom.Eta(), myResult.getMomentum().Eta());

    //Covariance
    //Dangerous, because expectation of bite-wise equality is unreasonable if there are internal
    // conversions.
    //EXPECT_EQ(errorMatrix, myResult.getCovariance6());

  }

  /** Test class with real data from GenFitter.
   * This will test if the cartesian coordinates can be extracted correctly*/
  TEST_F(TrackFitResultTest, realDataTest1)
  {
    TVector3 dataPos(0.014747, 0.009018, -0.032017);
    TVector3 dataMom(0.363594, 0.222542, 0.230146);
    TMatrixF dataCov6(6, 6);
    dataCov6(0, 0) = 0.0002724;
    dataCov6(0, 1) = dataCov6(1, 0) = -0.0001874;
    dataCov6(0, 2) = dataCov6(2, 0) = -0.0002549;
    dataCov6(0, 3) = dataCov6(3, 0) = -1.735e-05;
    dataCov6(0, 4) = dataCov6(4, 0) = 1.153e-05;
    dataCov6(0, 5) = dataCov6(5, 0) = 1.518e-05;
    dataCov6(1, 1) = 0.0004733;
    dataCov6(1, 2) = dataCov6(2, 1) = -0.000152;
    dataCov6(1, 3) = dataCov6(3, 1) = 1.329e-05;
    dataCov6(1, 4) = dataCov6(4, 1) = -2.946e-05;
    dataCov6(1, 5) = dataCov6(5, 1) = 9.085e-06;
    dataCov6(2, 2) = 0.0005497;
    dataCov6(2, 3) = dataCov6(3, 2) = 1.495e-05;
    dataCov6(2, 4) = dataCov6(4, 2) = 9.677e-06;
    dataCov6(2, 5) = dataCov6(5, 2) = -3.277e-05;
    dataCov6(3, 3) = 2.156e-06;
    dataCov6(3, 4) = dataCov6(4, 3) = -9.94e-07;
    dataCov6(3, 5) = dataCov6(5, 3) = -1.088e-06;
    dataCov6(4, 4) = 3.023e-06;
    dataCov6(4, 5) = dataCov6(5, 4) = -7.098e-07;
    dataCov6(5, 5) = 3.195e-06;
    short int charge(1);
    double pValue(1.000);
    int particleType(13);

    TrackFitResult myResult(dataPos, dataMom, charge, dataCov6, particleType, pValue, true);
    // Position test
    EXPECT_FLOAT_EQ(dataPos.X(), myResult.getX0());
    EXPECT_FLOAT_EQ(dataPos.Y(), myResult.getY0());
    EXPECT_FLOAT_EQ(dataPos.Z(), myResult.getZ0());
    // Momentum test
    EXPECT_FLOAT_EQ(dataMom.X(), myResult.getPx0());
    EXPECT_FLOAT_EQ(dataMom.Y(), myResult.getPy0());
    EXPECT_FLOAT_EQ(dataMom.Z(), myResult.getPz0());
    // Covariance matrix test
    TMatrixF cov6(myResult.getCovariance6());
    for (int ii = 0; ii < 6; ii++) {
      for (int jj = ii; jj < 6; jj++) {
        std::cout << "Matrix element (" << ii << "," << jj << ")" << std::endl;
        EXPECT_FLOAT_EQ(dataCov6(ii, jj), cov6(ii, jj));
      }
    }
    // Getters
    EXPECT_FLOAT_EQ(charge, myResult.getCharge());
    EXPECT_FLOAT_EQ(pValue, myResult.getPValue());
    EXPECT_EQ(Const::ParticleType(particleType), myResult.getParticleType());
  }

  /** Test class with real data from GenFitter.
   * This will test if the cartesian coordinates can be extracted correctly*/
  TEST_F(TrackFitResultTest, realDataTest2)
  {
    TVector3 dataPos(-0.003623, 0.003519, 0.002536);
    TVector3 dataMom(-0.517170, -0.823964, 0.395712);
    TMatrixF dataCov6(6, 6);
    dataCov6(0, 0) = 3.271e-06;
    dataCov6(0, 1) = dataCov6(1, 0) = -1.732e-06;
    dataCov6(0, 2) = dataCov6(2, 0) = 7.091e-07;
    dataCov6(0, 3) = dataCov6(3, 0) = -1.512e-06;
    dataCov6(0, 4) = dataCov6(4, 0) = 7.026e-07;
    dataCov6(0, 5) = dataCov6(5, 0) = -3.166e-07;
    dataCov6(1, 1) = 1.783e-06;
    dataCov6(1, 2) = dataCov6(2, 1) = 1.421e-06;
    dataCov6(1, 3) = dataCov6(3, 1) = 7.931e-07;
    dataCov6(1, 4) = dataCov6(4, 1) = -7.418e-07;
    dataCov6(1, 5) = dataCov6(5, 1) = -6.3e-07;
    dataCov6(2, 2) = 3.883e-06;
    dataCov6(2, 3) = dataCov6(3, 2) = -3.445e-07;
    dataCov6(2, 4) = dataCov6(4, 2) = -6.151e-07;
    dataCov6(2, 5) = dataCov6(5, 2) = -1.725e-06;
    dataCov6(3, 3) = 1.963e-06;
    dataCov6(3, 4) = dataCov6(4, 3) = 9.753e-07;
    dataCov6(3, 5) = dataCov6(5, 3) = -4.771e-07;
    dataCov6(4, 4) = 2.784e-06;
    dataCov6(4, 5) = dataCov6(5, 4) = -7.079e-07;
    dataCov6(5, 5) = 1.674e-06;
    short int charge(1);
    double pValue(1.000);
    int particleType(13);

    TrackFitResult myResult(dataPos, dataMom, charge, dataCov6, particleType, pValue, true);
    // Position test
    EXPECT_FLOAT_EQ(dataPos.X(), myResult.getX0());
    EXPECT_FLOAT_EQ(dataPos.Y(), myResult.getY0());
    EXPECT_FLOAT_EQ(dataPos.Z(), myResult.getZ0());
    // Momentum test
    EXPECT_FLOAT_EQ(dataMom.X(), myResult.getPx0());
    EXPECT_FLOAT_EQ(dataMom.Y(), myResult.getPy0());
    EXPECT_FLOAT_EQ(dataMom.Z(), myResult.getPz0());
    // Covariance matrix test
    TMatrixF cov6(myResult.getCovariance6());
    for (int ii = 0; ii < 6; ii++) {
      for (int jj = ii; jj < 6; jj++) {
        std::cout << "Matrix element (" << ii << "," << jj << ")" << std::endl;
        EXPECT_FLOAT_EQ(dataCov6(ii, jj), cov6(ii, jj));
      }
    }
    // Getters
    EXPECT_FLOAT_EQ(charge, myResult.getCharge());
    EXPECT_FLOAT_EQ(pValue, myResult.getPValue());
    EXPECT_EQ(Const::ParticleType(particleType), myResult.getParticleType());
  }

  /** Test class with real data from GenFitter.
   * This will test if the cartesian coordinates can be extracted correctly*/
  TEST_F(TrackFitResultTest, realDataTest3)
  {
    TVector3 dataPos(-0.000494, -0.002862, -0.015122);
    TVector3 dataMom(0.403883, 0.726547, -0.151006);
    TMatrixF dataCov6(6, 6);
    dataCov6(0, 0) = 4.089e-06;
    dataCov6(0, 1) = dataCov6(1, 0) = -2.141e-06;
    dataCov6(0, 2) = dataCov6(2, 0) = 4.986e-07;
    dataCov6(0, 3) = dataCov6(3, 0) = -1.943e-06;
    dataCov6(0, 4) = dataCov6(4, 0) = 1.139e-06;
    dataCov6(0, 5) = dataCov6(5, 0) = -2.377e-07;
    dataCov6(1, 1) = 1.401e-06;
    dataCov6(1, 2) = dataCov6(2, 1) = 1.091e-06;
    dataCov6(1, 3) = dataCov6(3, 1) = 1.019e-06;
    dataCov6(1, 4) = dataCov6(4, 1) = -7.184e-07;
    dataCov6(1, 5) = dataCov6(5, 1) = -4.394e-07;
    dataCov6(2, 2) = 6.586e-06;
    dataCov6(2, 3) = dataCov6(3, 2) = -2.272e-07;
    dataCov6(2, 4) = dataCov6(4, 2) = -4.509e-07;
    dataCov6(2, 5) = dataCov6(5, 2) = -2.75e-06;
    dataCov6(3, 3) = 1.748e-06;
    dataCov6(3, 4) = dataCov6(4, 3) = 2.295e-07;
    dataCov6(3, 5) = dataCov6(5, 3) = -4.704e-08;
    dataCov6(4, 4) = 2.171e-06;
    dataCov6(4, 5) = dataCov6(5, 4) = -9.02e-08;
    dataCov6(5, 5) = 1.714e-06;
    short int charge(1);
    double pValue(1.000);
    int particleType(13);

    TrackFitResult myResult(dataPos, dataMom, charge, dataCov6, particleType, pValue, true);
    // Position test
    EXPECT_FLOAT_EQ(dataPos.X(), myResult.getX0());
    EXPECT_FLOAT_EQ(dataPos.Y(), myResult.getY0());
    EXPECT_FLOAT_EQ(dataPos.Z(), myResult.getZ0());
    // Momentum test
    EXPECT_FLOAT_EQ(dataMom.X(), myResult.getPx0());
    EXPECT_FLOAT_EQ(dataMom.Y(), myResult.getPy0());
    EXPECT_FLOAT_EQ(dataMom.Z(), myResult.getPz0());
    // Covariance matrix test
    TMatrixF cov6(myResult.getCovariance6());
    for (int ii = 0; ii < 6; ii++) {
      for (int jj = ii; jj < 6; jj++) {
        std::cout << "Matrix element (" << ii << "," << jj << ")" << std::endl;
        EXPECT_FLOAT_EQ(dataCov6(ii, jj), cov6(ii, jj));
      }
    }
    // Getters
    EXPECT_FLOAT_EQ(charge, myResult.getCharge());
    EXPECT_FLOAT_EQ(pValue, myResult.getPValue());
    EXPECT_EQ(Const::ParticleType(particleType), myResult.getParticleType());
  }

  /** Test calculation of helix parameters. */
  /*  TEST_F(TrackFitResultTest, cartesianToPerigeeWithError)
    {
      // choose some sample values with which the test will be run
      TVector3 pos(0.1, 0.1, 0.1);
      TVector3 mom(-1.0, 1.0, 1.0);
      TMatrixF errorMatrix(6, 6);
      double errorMatrixEntry(0.01);
      for (int ii = 0; ii < 6; ii++) {
        for (int jj = ii; jj < 6; jj++) {
          errorMatrix(ii, jj) = errorMatrix(jj, ii) = errorMatrixEntry;
          errorMatrixEntry += 0.01;
        }
      }

      short int charge(1);
      unsigned int pdg(11);
      float pValue(0.45);
      // set up class for testing
      TrackFitResult myResult(pos, mom, charge, errorMatrix, pdg, pValue);

      // Test the conversion to helix parameters
      EXPECT_FLOAT_EQ(0.14142135623730953, myResult.getD0());
      EXPECT_FLOAT_EQ(2.356194490192345, myResult.getPhi0()); // 0.75*Pi
      EXPECT_FLOAT_EQ(0.0031797792000057487, myResult.getOmega());
      EXPECT_FLOAT_EQ(0.1, myResult.getZ0());
      EXPECT_FLOAT_EQ(0.707106781186547462, myResult.getCotTheta());
      // Test if cartesian coordinates are returned correctly
      EXPECT_FLOAT_EQ(pos.X(), myResult.getX0());
      EXPECT_FLOAT_EQ(pos.Y(), myResult.getY0());
      EXPECT_FLOAT_EQ(pos.Z(), myResult.getZ0());
      EXPECT_FLOAT_EQ(mom.Px(), myResult.getPx0());
      EXPECT_FLOAT_EQ(mom.Py(), myResult.getPy0());
      EXPECT_FLOAT_EQ(mom.Pz(), myResult.getPz0());
      // test charge
      EXPECT_EQ(charge, myResult.getCharge());

      // test 5x5 covariance matrix elements
      TMatrixF cov5(myResult.getCovariance5());
      EXPECT_NEAR(cov5(0, 0), 0.06, 1e-2);
      EXPECT_NEAR(cov5(0, 1), -0.0989949, 1e-7);
      EXPECT_NEAR(cov5(0, 2), -0.0000224844, 1e-10);
      EXPECT_NEAR(cov5(0, 3), 0.0777817, 1e-7);
      EXPECT_NEAR(cov5(0, 4), 0.08, 1e-2);
      EXPECT_NEAR(cov5(1, 1), 0.1725, 1e-4);
      EXPECT_NEAR(cov5(1, 2), 0.0000238483, 1e-10);
      EXPECT_NEAR(cov5(1, 3), -0.135, 1e-3);
      EXPECT_NEAR(cov5(1, 4), -0.129047, 1e-6);
      EXPECT_NEAR(cov5(2, 2), 2.52775e-8, 0.00001e-8);
      EXPECT_NEAR(cov5(2, 3), -0.0000158989, 1e-10);
      EXPECT_NEAR(cov5(2, 4), -0.0000168633, 1e-10);
      EXPECT_NEAR(cov5(3, 3), 0.12, 1e-2);
      EXPECT_NEAR(cov5(3, 4), 0.10253, 1e-5);
      EXPECT_NEAR(cov5(4, 4), 0.09625, 1e-5);

  // This test does not work because it can not work. The test matrix is no rank 5
      // test 6x6 covariance matrix elements
      TMatrixF cov6(myResult.getCovariance6());
      for (int ii = 0; ii < 6; ii++) {
        for (int jj = ii; jj < 6; jj++) {
          //std::cout << "Matrix element (" << ii << "," << jj << ")" << std::endl; // for better readability
          EXPECT_FLOAT_EQ(cov6(ii, jj), errorMatrix(ii, jj));
        }
      }

    }
  */

  /** Test if cartesian parameters are correctly caculated. */
  TEST_F(TrackFitResultTest, CartesianToPerigee)
  {
    std::vector<TVector3> pos;
    std::vector<TVector3> mom;
    TMatrixF errorMatrix(6, 6);
    for (int ii = 0; ii < 6; ii++) {
      for (int jj = ii; jj < 6; jj++) {
        errorMatrix(ii, jj) = 0.01;
      }
    }

    unsigned int pdg(11);
    float pValue(0.45);
    // choose some sample values with which the test will be run
    // Case1
    pos.push_back(TVector3(0.1, 0.1, 0.1));
    mom.push_back(TVector3(-1.0, 1.0, 1.0));
    // Case2
    pos.push_back(TVector3(0.1, 0.1, 0.1));
    mom.push_back(TVector3(1.0, -1.0, 1.0));
    // Case3
    pos.push_back(TVector3(-0.1, 0.1, 0.1));
    mom.push_back(TVector3(-1.0, -1.0, 1.0));
    // Case4
    pos.push_back(TVector3(-0.1, 0.1, 0.1));
    mom.push_back(TVector3(1.0, 1.0, 1.0));
    // Case5
    pos.push_back(TVector3(-0.1, -0.1, 0.1));
    mom.push_back(TVector3(1.0, -1.0, 1.0));
    // Case6
    pos.push_back(TVector3(-0.1, -0.1, 0.1));
    mom.push_back(TVector3(-1.0, 1.0, 1.0));
    // Case7
    pos.push_back(TVector3(0.1, -0.1, 0.1));
    mom.push_back(TVector3(1.0, 1.0, 1.0));
    // Case8
    pos.push_back(TVector3(0.1, -0.1, 0.1));
    mom.push_back(TVector3(-1.0, -1.0, 1.0));

    // set up class for testing
    for (short int charge = -1; charge <= 1; charge += 2) {
      for (unsigned int i = 0; i < pos.size(); ++i) {
        TrackFitResult myResult(pos[i], mom[i], charge, errorMatrix, pdg, pValue, true);

        EXPECT_FLOAT_EQ(pos[i].X(), myResult.getX0());
        EXPECT_FLOAT_EQ(pos[i].Y(), myResult.getY0());
        EXPECT_FLOAT_EQ(pos[i].Z(), myResult.getZ0());
        EXPECT_FLOAT_EQ(mom[i].Px(), myResult.getPx0());
        EXPECT_FLOAT_EQ(mom[i].Py(), myResult.getPy0());
        EXPECT_FLOAT_EQ(mom[i].Pz(), myResult.getPz0());
        EXPECT_EQ(charge, myResult.getCharge());
      } // loop over different vectors
    } // loop over charges

  }


  /** Test getters for poca position and momentum. */
  TEST_F(TrackFitResultTest, PocaPositionAndMomentumGetters)
  {
    // choose some sample values with which the test will be run
    TVector3 pos(0.1, 0.1, 0.1);
    TVector3 mom(-1.0, 1.0, 1.0);
    TMatrixF errorMatrix(6, 6);
    for (int ii = 0; ii < 6; ii++) {
      for (int jj = ii; jj < 6; jj++) {
        errorMatrix(ii, jj) = 0.01;
      }
    }

    unsigned int pdg(11);
    float pValue(0.45);
    // set up class for testing
    for (short int charge = -1; charge <= 1; charge += 2) {
      TrackFitResult myResult(pos, mom, charge, errorMatrix, pdg, pValue, true);
      // test whether position components are correctly
      EXPECT_FLOAT_EQ(pos.X(), myResult.getPosition()[0]);
      EXPECT_FLOAT_EQ(pos.Y(), myResult.getPosition()[1]);
      EXPECT_FLOAT_EQ(pos.Z(), myResult.getPosition()[2]);
      EXPECT_FLOAT_EQ(pos.X(), myResult.getPosition().X());
      EXPECT_FLOAT_EQ(pos.Y(), myResult.getPosition().Y());
      EXPECT_FLOAT_EQ(pos.Z(), myResult.getPosition().Z());
      // test whether momentum components are correctly
      EXPECT_FLOAT_EQ(mom.X(), myResult.getMomentum()[0]);
      EXPECT_FLOAT_EQ(mom.Y(), myResult.getMomentum()[1]);
      EXPECT_FLOAT_EQ(mom.Z(), myResult.getMomentum()[2]);
      EXPECT_FLOAT_EQ(mom.X(), myResult.getMomentum().X());
      EXPECT_FLOAT_EQ(mom.Y(), myResult.getMomentum().Y());
      EXPECT_FLOAT_EQ(mom.Z(), myResult.getMomentum().Z());
      // test charge
      EXPECT_EQ(charge, myResult.getCharge());
    } // loop over charges
  }

  /**
   * Test for the temporary solution
   */
  TEST_F(TrackFitResultTest, TemporaryGetterTests)
  {
    // choose some sample values with which the test will be run
    TVector3 pos(0.1, 0.1, 0.1);
    TVector3 mom(-1.0, 1.0, 1.0);
    TMatrixF errorMatrix(6, 6);
    double errorMatrixEntry(0.01);
    for (int ii = 0; ii < 6; ii++) {
      for (int jj = ii; jj < 6; jj++) {
        errorMatrix(ii, jj) = errorMatrix(jj, ii) = errorMatrixEntry;
        errorMatrixEntry += 0.01;
      }
    }
    short int charge(1);
    unsigned int pdg(11);
    float pValue(0.45);
    // set up class for testing
    TrackFitResult myResult(pos, mom, charge, errorMatrix, pdg, pValue, false);
    EXPECT_EQ(pos, myResult.getPosition());
    EXPECT_EQ(mom, myResult.getMomentum());
    TMatrixF covariance6(myResult.getCovariance6());
    for (unsigned int rr(0); rr < 6; ++rr) { // fill rows
      for (unsigned int cc(0); cc < 6; ++cc) { // fill columns
        EXPECT_FLOAT_EQ(errorMatrix(rr, cc), covariance6(rr, cc));
      } // column
    } // row

  }// namespace
}
