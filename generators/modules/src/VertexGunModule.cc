/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/VertexGunModule.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreArray.h>
#include <boost/assign/std/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <numeric>

using namespace std;
using namespace Belle2;
using namespace boost::assign;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VertexGun)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VertexGunModule::VertexGunModule() : Module()
{
  //Set module properties
  setDescription("particle gun to generate simple tracks");
  setPropertyFlags(c_Input);

  const int nDefaultVertices = 2;

  //Parameter definition
  addParam("tracksPerVertex", m_tracksPerVertex,
           "A list of numbers giving the number of tracks for each vertex in one event. Its length determines the number of vertices in one event", vector<int>(nDefaultVertices, 2));

  vector<int> defaultPdgCodes;
  defaultPdgCodes.push_back(13);
  defaultPdgCodes.push_back(-13);
  defaultPdgCodes.push_back(13);
  defaultPdgCodes.push_back(-13);
  addParam("pdgCodes", m_pdgCodes,
           "PDG codes for generated particles", defaultPdgCodes);
//  addParam("varyNTracks", m_varyNumberOfTracks,
//           "Vary the number of tracks per vertex using a Possion distribution? Only "
//           "used if nTracks>0", vector<bool>(nDefaultVertices,false));

  addParam("momentumGeneration", m_pDists,
           "Distributions used for generation the momentum, one of fixed, uniform, "
           "uniformPt or normal. You have to give the distribution for every vertex", vector<string>(nDefaultVertices, "normal"));
  addParam("phiGeneration", m_phiDists,
           "Distribution used for generation the azimuth angle, one of fixed, "
           "uniform, or normal", vector<string>(nDefaultVertices, "uniform"));
  addParam("thetaGeneration", m_thetaDists,
           "Distribution used for generation the polar angle, one of fixed, uniform, "
           "uniformCosinus or normal", vector<string>(nDefaultVertices, "uniform"));
  addParam("vertexGeneration", m_vertexDists,
           "Distribution used for generation the vertex, one of fixed, uniform, "
           "or normal", vector<string>(nDefaultVertices, "uniform"));

  addParam("momentumParams1", m_pParams1,
           "List of the \"first\" parameters for the momentum generation. Meaning of the parameters "
           "depends on the chosen distribution", vector<double>(nDefaultVertices, 1));
  addParam("momentumParams2", m_pParams2,
           "List of the \"second\" parameters for the momentum generation. Meaning of the parameters "
           "depends on the chosen distribution", vector<double>(nDefaultVertices, 0.05));
  addParam("phiParams1", m_phiParams1,
           "List of the \"first\" parameters for the phi generation. Meaning of the parameters "
           "depends on the chosen distribution", vector<double>(nDefaultVertices, 0));
  addParam("phiParams2", m_phiParams2,
           "List of the \"second\" parameters for the phi generation. Meaning of the parameters "
           "depends on the chosen distribution", vector<double>(nDefaultVertices, 360));
  addParam("thetaParams1", m_thetaParams1,
           "List of the \"first\" parameters for the theta generation. Meaning of the parameters "
           "depends on the chosen distribution", vector<double>(nDefaultVertices, 17));
  addParam("thetaParams2", m_thetaParams2,
           "List of the \"second\" parameters for the theta generation. Meaning of the parameters "
           "depends on the chosen distribution", vector<double>(nDefaultVertices, 150));

  addParam("xVertexParams1", m_xParams1,
           "List of the \"first\" parameters for the x vertex generation. Meaning of the parameters "
           "depends on the chosen distribution", vector<double>(nDefaultVertices, -1));
  addParam("xVertexParams2", m_xParams2,
           "List of the \"second\" parameters for the x vertex generation. Meaning of the parameters "
           "depends on the chosen distribution", vector<double>(nDefaultVertices, 1));
  addParam("yVertexParams1", m_yParams1,
           "List of the \"first\" parameters for the y vertex generation. Meaning of the parameters "
           "depends on the chosen distribution", vector<double>(nDefaultVertices, -1));
  addParam("yVertexParams2", m_yParams2,
           "List of the \"second\" parameters for the y vertex generation. Meaning of the parameters "
           "depends on the chosen distribution", vector<double>(nDefaultVertices, 1));
  addParam("zVertexParams1", m_zParams1,
           "List of the \"first\" parameters for the z vertex generation. Meaning of the parameters "
           "depends on the chosen distribution", vector<double>(nDefaultVertices, -1));
  addParam("zVertexParams2", m_zParams2,
           "List of the \"second\" parameters for the z vertex generation. Meaning of the parameters "
           "depends on the chosen distribution", vector<double>(nDefaultVertices, 1));


}


ParticleGun::Distribution VertexGunModule::convertDistribution(std::string name)
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

void VertexGunModule::initialize()
{
  //check the consistency of the module parameters
  const unsigned nVertices = m_tracksPerVertex.size();
  const unsigned nTracksPerEvent = accumulate(m_tracksPerVertex.begin(), m_tracksPerVertex.end(), 0);
  if (m_pdgCodes.size() not_eq nTracksPerEvent) {
    B2FATAL("The number of PDG codes provided does not match the sum of all track numbers set in tracksPerVertex");
  }

  if (m_pDists.size() not_eq  nVertices) {
    B2FATAL("The number of momentum distribution identifiers does not match the number of vertices (length of tracksPerVertex list)");
  }
  if (m_phiDists.size() not_eq  nVertices) {
    B2FATAL("The number of phi distribution identifiers does not match the number of vertices (length of tracksPerVertex list)");
  }
  if (m_thetaDists.size() not_eq  nVertices) {
    B2FATAL("The number of theta distribution identifiers does not match the number of vertices (length of tracksPerVertex list)");
  }
  if (m_vertexDists.size() not_eq  nVertices) {
    B2FATAL("The number of vertex position distribution identifiers does not match the number of vertices (length of tracksPerVertex list)");
  }

  if (m_pParams1.size() not_eq  nVertices) {
    B2FATAL("The number of the first momentum parameters does not match the number of vertices (length of tracksPerVertex list)");
  }
  if (m_pParams2.size() not_eq  nVertices) {
    B2FATAL("The number of the second momentum parameters does not match the number of vertices (length of tracksPerVertex list)");
  }
  if (m_phiParams1.size() not_eq  nVertices) {
    B2FATAL("The number of the first phi parameters does not match the number of vertices (length of tracksPerVertex list)");
  }
  if (m_phiParams2.size() not_eq  nVertices) {
    B2FATAL("The number of the second phi parameters does not match the number of vertices (length of tracksPerVertex list)");
  }
  if (m_thetaParams1.size() not_eq  nVertices) {
    B2FATAL("The number of the first theta parameters does not match the number of vertices (length of tracksPerVertex list)");
  }
  if (m_thetaParams2.size() not_eq  nVertices) {
    B2FATAL("The number of the second theta parameters does not match the number of vertices (length of tracksPerVertex list)");
  }

  if (m_xParams1.size() not_eq  nVertices) {
    B2FATAL("The number of the first vertex x position parameters does not match the number of vertices (length of tracksPerVertex list)");
  }
  if (m_xParams2.size() not_eq  nVertices) {
    B2FATAL("The number of the second vertex x position parameters does not match the number of vertices (length of tracksPerVertex list)");
  }
  if (m_yParams1.size() not_eq  nVertices) {
    B2FATAL("The number of the first vertex y position parameters does not match the number of vertices (length of tracksPerVertex list)");
  }
  if (m_yParams2.size() not_eq  nVertices) {
    B2FATAL("The number of the second vertex y position parameters does not match the number of vertices (length of tracksPerVertex list)");
  }
  if (m_zParams1.size() not_eq  nVertices) {
    B2FATAL("The number of the first vertex z position parameters does not match the number of vertices (length of tracksPerVertex list)");
  }
  if (m_zParams2.size() not_eq  nVertices) {
    B2FATAL("The number of the second vertex z position parameters does not match the number of vertices (length of tracksPerVertex list)");
  }
//  //Initialize MCParticle collection
  StoreArray<MCParticle>::registerPersistent();

  //Convert degree to radian
  BOOST_FOREACH(double & angle, m_thetaParams1) angle *= Unit::deg;
  BOOST_FOREACH(double & angle, m_thetaParams2)   angle *= Unit::deg;
  BOOST_FOREACH(double & angle, m_phiParams1) angle *= Unit::deg;
  BOOST_FOREACH(double & angle, m_phiParams2)   angle *= Unit::deg;
  //take the parameters from the module's parameter list and put it into the particle gun parameters struct
  ParticleGun::Parameters parametersForOneVertex; /** Parameters of the particle gun */
  parametersForOneVertex.independentVertices = false;
  parametersForOneVertex.varyNumberOfTracks = false;
  parametersForOneVertex.momentumParams.resize(2);
  parametersForOneVertex.phiParams.resize(2);
  parametersForOneVertex.thetaParams.resize(2);
  parametersForOneVertex.xVertexParams.resize(2);
  parametersForOneVertex.yVertexParams.resize(2);
  parametersForOneVertex.zVertexParams.resize(2);
  int pdgCodeShift = 0;
  for (unsigned i = 0; i not_eq nVertices; ++i) {
    parametersForOneVertex.momentumDist = convertDistribution(m_pDists[i]);
    parametersForOneVertex.phiDist = convertDistribution(m_phiDists[i]);
    parametersForOneVertex.thetaDist = convertDistribution(m_thetaDists[i]);
    parametersForOneVertex.vertexDist = convertDistribution(m_vertexDists[i]);
    const int tracksPerVertex = m_tracksPerVertex[i];
    parametersForOneVertex.nTracks = tracksPerVertex;
    parametersForOneVertex.pdgCodes.resize(tracksPerVertex);
    for (int j = 0; j not_eq tracksPerVertex; ++j) {
      parametersForOneVertex.pdgCodes[j] = m_pdgCodes[pdgCodeShift + j];
    }
    parametersForOneVertex.momentumParams[0] = m_pParams1[i];
    parametersForOneVertex.momentumParams[1] = m_pParams2[i];
    parametersForOneVertex.phiParams[0] = m_phiParams1[i];
    parametersForOneVertex.phiParams[1] = m_phiParams2[i];
    parametersForOneVertex.thetaParams[0] = m_thetaParams1[i];
    parametersForOneVertex.thetaParams[1] = m_thetaParams2[i];
    parametersForOneVertex.xVertexParams[0] = m_xParams1[i];
    parametersForOneVertex.xVertexParams[1] = m_xParams2[i];
    parametersForOneVertex.yVertexParams[0] = m_yParams1[i];
    parametersForOneVertex.yVertexParams[1] = m_yParams2[i];
    parametersForOneVertex.zVertexParams[0] = m_zParams1[i];
    parametersForOneVertex.zVertexParams[1] = m_zParams2[i];
    m_parametersForAllVertices.push_back(parametersForOneVertex);
    pdgCodeShift += tracksPerVertex;
  }


}


void VertexGunModule::event()
{

  const int nVertex = m_tracksPerVertex.size();
  try {
    m_particleGraph.clear();
    for (int i = 0; i not_eq nVertex; ++i) {
      m_particleGun.setParameters(m_parametersForAllVertices[i]);
      m_particleGun.generateEvent(m_particleGraph);
    }
    m_particleGraph.generateList();
  } catch (runtime_error& e) {
    B2ERROR(e.what());
  }

}

