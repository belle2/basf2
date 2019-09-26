#include <generators/particlegun/ParticleGun.h>
#include <gtest/gtest.h>
#include <iostream>
using namespace std;

namespace Belle2 {
  /** The fixture for testing the Particlegun. */
  class ParticleGunTest : public ::testing::Test {
  protected:
    /** Set up parameters which work for all distributions */
    virtual void SetUp()
    {
      parameters.momentumParams = {1, 1};
      parameters.xVertexParams  = {1, 1};
      parameters.yVertexParams  = {1, 1};
      parameters.zVertexParams  = {1, 1};
      parameters.thetaParams    = {1, 1};
      parameters.phiParams      = {1, 1};
    }
    /** Variable of pgun */
    ParticleGun pgun;
    /** Variable parameters */
    ParticleGun::Parameters parameters;

    /** check one of the variables given a list of allowed and excluded distributions */
    void checkVariable(const std::string& name, const std::map<int, bool>& allowed, ParticleGun::EDistribution& dist,
                       std::vector<double>& pars);
  };

  void ParticleGunTest::checkVariable(const std::string& name, const std::map<int, bool>& allowed, ParticleGun::EDistribution& dist,
                                      std::vector<double>& pars)
  {
    parameters = ParticleGun::Parameters();
    parameters.pdgCodes = {11};
    for (auto item : allowed) {
      dist = (ParticleGun::EDistribution) item.first;
      //We need at least for parameters for some distributions.
      pars = {1, 1, 1, 1};
      //Check if distribution is accepted
      ASSERT_EQ(item.second, pgun.setParameters(parameters)) << name << ": " << item.first << ", " << item.second;
      if (item.second) {
        //And if it is accepted, check number of parameters
        for (int i = 0; i < 10; ++i) {
          pars.resize(i, 1);
          int minpar = (item.first == ParticleGun::c_fixedValue) ? 1 : 2;
          //Polyline needs at least two points, so four parameters
          if (dist == ParticleGun::c_polylineDistribution || dist == ParticleGun::c_polylinePtDistribution
              || dist == ParticleGun::c_polylineCosDistribution) minpar = 4;
          bool valid = i >= minpar;
          //discrete and polyline need even number of parameters
          if (dist == ParticleGun::c_discreteSpectrum || dist == ParticleGun::c_polylineDistribution ||
              dist == ParticleGun::c_polylinePtDistribution || dist == ParticleGun::c_polylineCosDistribution) {
            valid &= i % 2 == 0;
          }
          //Check if numberof parameters is accepted
          ASSERT_EQ(valid, pgun.setParameters(parameters)) << name << " " << pars.size();
        }
      }
      //Check that zeros are forbidden for inversePt spectrum
      if (item.first == ParticleGun::c_inversePtDistribution && item.second) {
        pars = {1, 1};
        ASSERT_TRUE(pgun.setParameters(parameters));
        pars = {1, 0};
        ASSERT_FALSE(pgun.setParameters(parameters)) << name << ": "  << parameters.momentumParams[0] << ", " <<
                                                     parameters.momentumParams[1];
        pars = {0, 1};
        ASSERT_FALSE(pgun.setParameters(parameters)) << name << ": "  << parameters.momentumParams[0] << ", " <<
                                                     parameters.momentumParams[1];
      }
      //Check polyline stuff
      if ((dist == ParticleGun::c_polylineDistribution || dist == ParticleGun::c_polylinePtDistribution
           || dist == ParticleGun::c_polylineCosDistribution) && item.second) {
        pars = {0, 1, 0, 1};
        ASSERT_TRUE(pgun.setParameters(parameters));
        //x needs to be ascending
        pars = {1, 0, 0, 1};
        ASSERT_FALSE(pgun.setParameters(parameters));
        //at least one positive y value
        pars = {0, 1, 0, 0};
        ASSERT_FALSE(pgun.setParameters(parameters));
        //no negative y values
        pars = {0, 1, 1, -1};
        ASSERT_FALSE(pgun.setParameters(parameters));
      }
      //Check discrete spectrum has non-negative weights
      if (dist == ParticleGun::c_discreteSpectrum) {
        pars = {0, 1, 0, 1};
        ASSERT_TRUE(pgun.setParameters(parameters));
        //at least one weight must be positive
        pars = {0, 1, 0, 0};
        ASSERT_FALSE(pgun.setParameters(parameters));
        //no negative weights
        pars = {0, 1, 1, -1};
        ASSERT_FALSE(pgun.setParameters(parameters));
      }
    }
    std::cout << std::endl;
  }

  /** Tests momentum generation parameters */
  TEST_F(ParticleGunTest, MomentumDistributions)
  {
    std::map<int, bool> distributions = {
      {ParticleGun::c_fixedValue,                 true},
      {ParticleGun::c_uniformDistribution,        true},
      {ParticleGun::c_uniformPtDistribution,      true},
      {ParticleGun::c_uniformCosDistribution,    false},
      {ParticleGun::c_normalDistribution,         true},
      {ParticleGun::c_normalPtDistribution,       true},
      {ParticleGun::c_normalCosDistribution,     false},
      {ParticleGun::c_discreteSpectrum,           true},
      {ParticleGun::c_inversePtDistribution,      true},
      {ParticleGun::c_polylineDistribution,       true},
      {ParticleGun::c_polylinePtDistribution,     true},
      {ParticleGun::c_polylineCosDistribution,   false}
    };
    checkVariable("momentum", distributions, parameters.momentumDist, parameters.momentumParams);
  }

  /** Tests vertex generation parameters */
  TEST_F(ParticleGunTest, VertexDistributions)
  {
    std::map<int, bool> distributions = {
      {ParticleGun::c_fixedValue,                 true},
      {ParticleGun::c_uniformDistribution,        true},
      {ParticleGun::c_uniformPtDistribution,     false},
      {ParticleGun::c_uniformCosDistribution,    false},
      {ParticleGun::c_normalDistribution,         true},
      {ParticleGun::c_normalPtDistribution,      false},
      {ParticleGun::c_normalCosDistribution,     false},
      {ParticleGun::c_discreteSpectrum,           true},
      {ParticleGun::c_inversePtDistribution,     false},
      {ParticleGun::c_polylineDistribution,       true},
      {ParticleGun::c_polylinePtDistribution,    false},
      {ParticleGun::c_polylineCosDistribution,   false}
    };
    checkVariable("xvertex", distributions, parameters.xVertexDist, parameters.xVertexParams);
    checkVariable("yvertex", distributions, parameters.yVertexDist, parameters.yVertexParams);
    checkVariable("zvertex", distributions, parameters.zVertexDist, parameters.zVertexParams);
  }

  /** Tests angular generation parameters */
  TEST_F(ParticleGunTest, AngularDistributions)
  {
    std::map<int, bool> distributions = {
      {ParticleGun::c_fixedValue,                 true},
      {ParticleGun::c_uniformDistribution,        true},
      {ParticleGun::c_uniformPtDistribution,     false},
      {ParticleGun::c_uniformCosDistribution,     true},
      {ParticleGun::c_normalDistribution,         true},
      {ParticleGun::c_normalPtDistribution,      false},
      {ParticleGun::c_normalCosDistribution,      true},
      {ParticleGun::c_discreteSpectrum,           true},
      {ParticleGun::c_inversePtDistribution,     false},
      {ParticleGun::c_polylineDistribution,       true},
      {ParticleGun::c_polylinePtDistribution,    false},
      {ParticleGun::c_polylineCosDistribution,    true}
    };
    checkVariable("theta", distributions, parameters.thetaDist, parameters.thetaParams);
    checkVariable("phi", distributions, parameters.phiDist, parameters.phiParams);
  }
}  // namespace
