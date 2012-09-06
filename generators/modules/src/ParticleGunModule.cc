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
#include <boost/assign/std/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;
using namespace boost::assign;

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
  setDescription("particle gun to generate simple tracks");
  setPropertyFlags(c_Input);

  //Set default values for parameters
  m_parameters.pdgCodes       += -11, 11;
  m_parameters.momentumParams += 0.05, 3.0;
  m_parameters.phiParams      += 0.0, 360.0;
  m_parameters.thetaParams    += 17.0, 150.0;
  m_parameters.xVertexParams  += 0.0, 0.7;
  m_parameters.yVertexParams  += 0.0, 0.7;
  m_parameters.zVertexParams  += 0.0, 1.0;

  //Parameter definition
  addParam("nTracks", m_parameters.nTracks,
           "The number of tracks to be generated per event. If <=0, one particle will "
           "be created for each entry in pdgCodes. Otherwise N particles will be "
           "created and the Particle ID for each particle will be picked randomly "
           "from pdgCodes", 1.0);
  addParam("pdgCodes", m_parameters.pdgCodes,
           "PDG codes for generated particles", m_parameters.pdgCodes);
  addParam("varyNTracks", m_parameters.varyNumberOfTracks,
           "Vary the number of tracks per event using a Possion distribution? Only "
           "used if nTracks>0", false);

  addParam("momentumGeneration", m_momentumDist,
           "Distribution used for generation the momentum, one of fixed, uniform, "
           "uniformPt or normal", string("uniform"));
  addParam("phiGeneration", m_phiDist,
           "Distribution used for generation the azimuth angle, one of fixed, "
           "uniform, or normal", string("uniform"));
  addParam("thetaGeneration", m_thetaDist,
           "Distribution used for generation the polar angle, one of fixed, uniform, "
           "uniformCosinus or normal", string("uniform"));
  addParam("vertexGeneration", m_vertexDist,
           "Distribution used for generation the vertex, one of fixed, uniform, "
           "or normal", string("normal"));
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
  if (name == "uniformcosinus") return ParticleGun::uniformCosinusDistribution;
  if (name == "normal")         return ParticleGun::normalDistribution;
  if (name == "normalpt")       return ParticleGun::normalPtDistribution;
  if (name == "discretespectrum") return ParticleGun::discreteSpectrum;
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
  m_parameters.vertexDist   = convertDistribution(m_vertexDist);

  //Convert degree to radian
  BOOST_FOREACH(double & angle, m_parameters.thetaParams) angle *= Unit::deg;
  BOOST_FOREACH(double & angle, m_parameters.phiParams)   angle *= Unit::deg;

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

