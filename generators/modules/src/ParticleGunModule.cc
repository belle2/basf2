/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz, Martin Ritter                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/ParticleGunModule.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreArray.h>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ParticleGun)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ParticleGunModule::ParticleGunModule() : Module()
{
  //Set module properties
  setDescription(
    "Particle gun to generate simple tracks.\n"
    "This module allows to generate simple events where all tracks have the same\n"
    "momentum, angular and vertex distributions.  Several distributions are\n"
    "available for momentum, phi, theta and vertex position generation:\n"
    "- fixed:       Fixed value, only one parameter has to be specified: [value]\n"
    "- uniform:     Uniform between two given values: [min, max]\n"
    "- uniformPt:   Generate flat transverse momentum pt: [min_pt, max_pt]\n"
    "- uniformCos:  Generate uniformly in the cosine, e.g. flat in cos(theta).\n"
    "               Parameters are still the minimum and maximum angle (not\n"
    "               cos()): [min_theta, max_theta]\n"
    "- normal:      Normal (Gaussian) distributed: [mean, width]\n"
    "- normalPt:    Generate normal distributed transverse momentum pt: [mean_pt, width_pt]\n"
    "- normalCos:   Generate normal distributed cosine of the angle: [mean, width]\n"
    "- polyline:    Generate according to a pdf given as polyline, first the sorted x\n"
    "               coordinates and then the non-negative y coordinates:\n"
    "               [x1, x2, x3, ... xn, y1, y2, y3, ..., yn]\n"
    "- polylinePt:  Like polyline but for pt, not p\n"
    "- polylineCos: Like polyline, but for the cos(), not the absolute value\n"
    "- inversePt:   Generate uniformly in the inverse of pt, that is uniform in\n"
    "               track curvature: [min_pt, max_pt]\n"
    "- discrete:    Discrete Spectrum given as a list of weights and values:\n"
    "               [weight1, value1, weight2, value2, ...]\n"
    "               (useful for radiactive sources)\n"
  );
  setPropertyFlags(c_Input);

  //Set default values for parameters
  m_parameters.pdgCodes       = { -11, 11};
  m_parameters.momentumParams = {0.05, 3.0};
  m_parameters.phiParams      = {0.0, 360.0};
  m_parameters.thetaParams    = {17.0, 150.0};
  m_parameters.xVertexParams  = {0.0, 0.7};
  m_parameters.yVertexParams  = {0.0, 0.7};
  m_parameters.zVertexParams  = {0.0, 1.0};

  //Parameter definition
  addParam("nTracks", m_parameters.nTracks,
           "The number of tracks to be generated per event. If <=0, one particle will "
           "be created for each entry in 'pdgCodes'. Otherwise N particles will be "
           "created and the Particle ID for each particle will be picked randomly "
           "from 'pdgCodes'", 1.0);
  addParam("pdgCodes", m_parameters.pdgCodes,
           "PDG codes for generated particles", m_parameters.pdgCodes);
  addParam("varyNTracks", m_parameters.varyNumberOfTracks,
           "If true, the number of tracks per event is varied using a Possion "
           "distribution. Only used if 'nTracks'>0", false);
  addParam("momentumGeneration", m_momentumDist,
           "Momentum distribution: one of fixed, uniform, normal, polyline, uniformPt, "
           "normalPt, inversePt, polylinePt or discrete", string("uniform"));
  addParam("phiGeneration", m_phiDist,
           "Phi distribution: one of fixed, uniform, normal, normalCos, polyline, uniformCos, "
           "polylineCos or discrete", string("uniform"));
  addParam("thetaGeneration", m_thetaDist,
           "Theta distribution: one of fixed, uniform, normal, normalCos, polyline, uniformCos, "
           "polylineCos or discrete", string("uniform"));
  addParam("vertexGeneration", m_vertexDist,
           "Vertex (x,y,z) distribution: one of fixed, uniform, normal, polyline or "
           "discrete", string("normal"));
  addParam("xVertexGeneration", m_xVertexDist,
           "X vertex distribution: same options as 'vertexGeneration'. If this parameter "
           "is not specified the value from 'vertexGeneration' is used", string(""));
  addParam("yVertexGeneration", m_yVertexDist,
           "Y vertex distribution: same options as 'vertexGeneration'. If this parameter "
           "is not specified the value from 'vertexGeneration' is used", string(""));
  addParam("zVertexGeneration", m_zVertexDist,
           "Z vertex distribution: same options as 'vertexGeneration'. If this parameter "
           "is not specified the value from 'vertexGeneration' is used", string(""));
  addParam("independentVertices", m_parameters.independentVertices,
           "If false, all tracks of one event will start from the same vertex, "
           "otherwise a new vertex is generated for every particle", false);
  addParam("momentumParams", m_parameters.momentumParams,
           "Parameters for the momentum generation. Meaning of the parameters "
           "depends on the chosen distribution", m_parameters.momentumParams);
  addParam("phiParams", m_parameters.phiParams,
           "Parameters for the phi generation. Meaning of the parameters "
           "depends on the chosen distribution", m_parameters.phiParams);
  addParam("thetaParams", m_parameters.thetaParams,
           "Parameters for the theta generation. Meaning of the parameters "
           "depends on the chosen distribution", m_parameters.thetaParams);
  addParam("xVertexParams", m_parameters.xVertexParams,
           "Parameters for the x vertex generation. Meaning of the parameters "
           "depends on the chosen distribution", m_parameters.xVertexParams);
  addParam("yVertexParams", m_parameters.yVertexParams,
           "Parameters for the y vertex generation. Meaning of the parameters "
           "depends on the chosen distribution", m_parameters.yVertexParams);
  addParam("zVertexParams", m_parameters.zVertexParams,
           "Parameters for the z vertex generation. Meaning of the parameters "
           "depends on the chosen distribution", m_parameters.zVertexParams);
}

ParticleGun::Distribution ParticleGunModule::convertDistribution(std::string name)
{
  boost::to_lower(name);
  boost::trim(name);
  if (name == "fixed")          return ParticleGun::fixedValue;
  if (name == "uniform")        return ParticleGun::uniformDistribution;
  if (name == "uniformpt")      return ParticleGun::uniformPtDistribution;
  if (name == "uniformcos")     return ParticleGun::uniformCosDistribution;
  if (name == "normal")         return ParticleGun::normalDistribution;
  if (name == "normalpt")       return ParticleGun::normalPtDistribution;
  if (name == "normalcos")      return ParticleGun::normalCosDistribution;
  if (name == "discrete")       return ParticleGun::discreteSpectrum;
  if (name == "inversept")      return ParticleGun::inversePtDistribution;
  if (name == "polyline")       return ParticleGun::polylineDistribution;
  if (name == "polylinept")     return ParticleGun::polylinePtDistribution;
  if (name == "polylinecos")    return ParticleGun::polylineCosDistribution;
  //TODO: backwards compatibility
  if (name == "uniformcosinus") {
    B2WARNING("Name for uniform cos() distribution changed to 'uniformCos'");
    return ParticleGun::uniformCosDistribution;
  }
  if (name == "discretespectrum") {
    B2WARNING("Name for discrete spectrum distribution changed to 'discrete'");
    return ParticleGun::discreteSpectrum;
  }
  B2ERROR("Unknown distribution '" << name << "', using fixed");
  return ParticleGun::fixedValue;
}

void ParticleGunModule::initialize()
{
  //Initialize MCParticle collection
  StoreArray<MCParticle>::registerPersistent();

  //Convert string representations to distribution values
  m_parameters.momentumDist = convertDistribution(m_momentumDist);
  m_parameters.phiDist      = convertDistribution(m_phiDist);
  m_parameters.thetaDist    = convertDistribution(m_thetaDist);
  m_parameters.xVertexDist  = convertDistribution(m_vertexDist);
  m_parameters.yVertexDist  = convertDistribution(m_vertexDist);
  m_parameters.zVertexDist  = convertDistribution(m_vertexDist);
  if (getParam<std::string>("xVertexGeneration").isSetInSteering()) {
    m_parameters.xVertexDist  = convertDistribution(m_xVertexDist);
  }
  if (getParam<std::string>("yVertexGeneration").isSetInSteering()) {
    m_parameters.yVertexDist  = convertDistribution(m_yVertexDist);
  }
  if (getParam<std::string>("zVertexGeneration").isSetInSteering()) {
    m_parameters.zVertexDist  = convertDistribution(m_zVertexDist);
  }

  //Convert degree to radian
  if (m_parameters.thetaDist != ParticleGun::polylineCosDistribution &&
      m_parameters.thetaDist != ParticleGun::normalCosDistribution) {
    for (double & angle : m_parameters.thetaParams) angle *= Unit::deg;
  }
  if (m_parameters.phiDist != ParticleGun::polylineCosDistribution &&
      m_parameters.phiDist != ParticleGun::normalCosDistribution) {
    for (double & angle : m_parameters.phiParams) angle *= Unit::deg;
  }

  //Assign parameters
  m_particleGun.setParameters(m_parameters);
}

void ParticleGunModule::event()
{
  try {
    m_particleGraph.clear();
    m_particleGun.generateEvent(m_particleGraph);
    m_particleGraph.generateList();
  } catch (runtime_error& e) {
    B2ERROR(e.what());
  }
}
