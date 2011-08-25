/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/PGunInputModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PGunInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PGunInputModule::PGunInputModule() : Module()
{
  vector<int> defaultPIDcodes;
  defaultPIDcodes.push_back(11);
  defaultPIDcodes.push_back(-11);

  //Set module properties
  setDescription("particle gun for tracks");
  setPropertyFlags(c_Input);

  //Parameter definition
  addParam("nTracks", m_nTracks, "The number of tracks to be generated per event", 1);
  addParam("pPar1", m_pPar1, "The first parameter for momentum distribution function", 0.2);
  addParam("pPar2", m_pPar2, "The second parameter for momentum distribution function", 1.0);
  addParam("phiPar1", m_phiPar1Deg, "The first parameter for the phi distribution function (Unit: Degrees)", 0.0);
  addParam("phiPar2", m_phiPar2Deg, "The second parameter for the phi distribution function (Unit: Degrees)", 360.0);
  addParam("thetaPar1", m_thetaPar1Deg, "The first parameter for theta distribution function (Unit: Degrees)", 17.0);
  addParam("thetaPar2", m_thetaPar2Deg, "The second parameter for theta distribution function (Unit: Degrees)", 150.0);
  addParam("xVertexPar1", m_xVertexPar1, "The first parameter for vertex x-coordinate distribution function", 0.0);
  addParam("xVertexPar2", m_xVertexPar2, "The second parameter for x-coordinate distribution distribution function", 0.7);
  addParam("yVertexPar1", m_yVertexPar1, "The first parameter for vertex y-coordinate distribution function", 0.0);
  addParam("yVertexPar2", m_yVertexPar2, "The second parameter for y-coordinate distribution distribution function", 0.7);
  addParam("zVertexPar1", m_zVertexPar1, "The first parameter for vertex z-coordinate distribution function", 0.0);
  addParam("zVertexPar2", m_zVertexPar2,  "The second parameter for z-coordinate distribution distribution function", 1.0);
  addParam("momentumGeneration", m_genMom, "Choice of distribution function for momentum generation", 0);
  addParam("vertexGeneration", m_genVert, "Choice of distribution function for vertex generation", 2);
  addParam("angleGeneration", m_genAngle, "Choice of distribution function for angle generation", 0);
  addParam("PIDcodes", m_PIDcodes, "PID codes for generated particles", defaultPIDcodes);
  addParam("Rseed", m_randomseed, "Randomseed for particle gun. (temporary solution)", 3452346);
  addParam("fixPt", m_fixedPt, "Set to true if you want to fix the Pt instead of the momentum.", false);
}


void PGunInputModule::initialize()
{
  if (m_nTracks > 0)
    m_pgun.m_nTracks = m_nTracks;
  else
    B2ERROR("Number of tracks to be generated should be larger than 0!");
  m_pgun.m_pPar1 = m_pPar1;
  m_pgun.m_pPar2 = m_pPar2;
  m_pgun.m_thetaPar1 = TMath::DegToRad() * m_thetaPar1Deg;
  m_pgun.m_thetaPar2 = TMath::DegToRad() * m_thetaPar2Deg;
  m_pgun.m_phiPar1 = TMath::DegToRad() * m_phiPar1Deg;
  m_pgun.m_phiPar2 = TMath::DegToRad() * m_phiPar2Deg;
  m_pgun.m_zVertexPar1 = m_zVertexPar1;
  m_pgun.m_zVertexPar2 = m_zVertexPar2;
  m_pgun.m_yVertexPar1 = m_yVertexPar1;
  m_pgun.m_yVertexPar2 = m_yVertexPar2;
  m_pgun.m_xVertexPar1 = m_xVertexPar1;
  m_pgun.m_xVertexPar2 = m_xVertexPar2;
  m_pgun.SetRandomSeed(m_randomseed);
  m_pgun.m_fixedPt     = m_fixedPt;
  if (m_PIDcodes.size() > 0)
    m_pgun.m_PIDcodes = m_PIDcodes;
  else {
    m_pgun.m_PIDcodes.push_back(11);
    m_pgun.m_PIDcodes.push_back(-11);
  }

  if (m_genMom<3 && m_genMom> -1)
    m_pgun.m_genMom = static_cast<ParticleGun::EgenerationOption>(m_genMom);
  else {
    m_genMom = 0;
    B2ERROR("Invalide option for random generation of momenta in particle gun!");
  }
  if (m_genVert<3 && m_genVert> -1)
    m_pgun.m_genVert = static_cast <ParticleGun::EgenerationOption>(m_genVert);
  else {
    m_genVert = 2;
    B2ERROR("Invalid option for random generation of vertices in particle gun!");
  }
  if (m_genAngle<3 && m_genAngle> -1)
    m_pgun.m_genAngle = static_cast <ParticleGun::EgenerationOption>(m_genAngle);
  else {
    m_genMom = 0;
    B2ERROR("Invalid option for random generation of angles in particle gun!");
  }

  //Initialize MCParticle collection
  StoreArray<MCParticle> MCParticles;
}


void PGunInputModule::event()
{
  try {
    mpg.clear();
    m_pgun.generateEvent(mpg);
    mpg.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);
  } catch (runtime_error &e) {
    B2ERROR(e.what());
  }
}

