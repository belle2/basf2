/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/koralwinput/KoralWInputModule.h>

#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/GearDir.h>

#include <TLorentzVector.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(KoralWInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

KoralWInputModule::KoralWInputModule() : Module(), m_initial(BeamParameters::c_smearVertex)
{
  //Set module properties
  setDescription("Generates four fermion final state events with KoralW.");

  //Parameter definition
  addParam("DataPath",  m_dataPath, "The path to the KoralW input data files.",
           FileSystem::findFile("/data/generators/koralw"));
  addParam("UserDataFile",  m_userDataFile, "The filename of the user KoralW input data file.",
           FileSystem::findFile("/data/generators/koralw/KoralW_ee.data"));
  addParam("RandomSeed", m_seed, "The random seed of the generator.", 1227);
}


KoralWInputModule::~KoralWInputModule()
{

}


void KoralWInputModule::initialize()
{
  StoreArray<MCParticle>::registerPersistent();

  //Beam Parameters, initial particle - KORALW cannot handle beam energy spread
  m_initial.initialize();
  const BeamParameters& nominal = m_initial.getBeamParameters();
  double ecm = nominal.getMass();
  m_generator.setCMSEnergy(ecm);

  m_generator.init(m_dataPath, m_userDataFile, m_seed);

}


void KoralWInputModule::event()
{
  // initial particle from beam parameters
  MCInitialParticles& initial = m_initial.generate();

  // true boost
  TLorentzRotation boost = initial.getCMSToLab();

  // vertex
  TVector3 vertex = initial.getVertex();

  m_mcGraph.clear();
  m_generator.generateEvent(m_mcGraph, vertex, boost);
  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);
}


void KoralWInputModule::terminate()
{
  m_generator.term();

  B2INFO(">>> Total cross section: " << m_generator.getCrossSection() << " pb +- " << m_generator.getCrossSectionError() << " pb")
}



