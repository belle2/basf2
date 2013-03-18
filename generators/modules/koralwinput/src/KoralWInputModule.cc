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
#include <generators/utilities/cm2LabBoost.h>

#include <framework/core/Environment.h>
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

KoralWInputModule::KoralWInputModule() : Module()
{
  //Set module properties
  setDescription("Generates four fermion final state events with KoralW.");

  //Parameter definition
  addParam("BoostMode", m_boostMode, "The mode of the boost (0 = no boost, 1 = Belle II, 2 = Belle)", 0);
  addParam("DataPath",  m_dataPath, "The path to the KoralW input data files.",
           Environment::Instance().getDataSearchPath() + "/generators/koralw");
  addParam("UserDataFile",  m_userDataFile, "The filename of the user KoralW input data file.",
           Environment::Instance().getDataSearchPath() + "/generators/koralw/KoralW_ee.data");
  addParam("RandomSeed", m_seed, "The random seed of the generator.", 1227);
}


KoralWInputModule::~KoralWInputModule()
{

}


void KoralWInputModule::initialize()
{
  StoreArray<MCParticle>::registerPersistent();

  m_generator.init(m_dataPath, m_userDataFile, m_seed);

  //Depending on the settings use the Belle II or Belle boost
  if (m_boostMode == 1) {
    GearDir ler("/Detector/SuperKEKB/LER/");
    GearDir her("/Detector/SuperKEKB/HER/");

    m_generator.setBoost(getBoost(her.getDouble("energy"), ler.getDouble("energy"),
                                  her.getDouble("angle") - ler.getDouble("angle"), her.getDouble("angle")));
  } else {
    if (m_boostMode == 2) {

      //electron and positron beam energies (magic numbers from Jeremy)
      double electronBeamEnergy = 7.998213; // [GeV]
      double positronBeamEnergy = 3.499218; // [GeV]
      double crossingAngle = 22.0; //[mrad]

      double pzP = sqrt(positronBeamEnergy * positronBeamEnergy - 0.000510998918 * 0.000510998918);
      double pE  = sqrt(electronBeamEnergy * electronBeamEnergy - 0.000510998918 * 0.000510998918);
      TLorentzVector boostVector(pE * sin(crossingAngle * 0.001), 0., pE * cos(crossingAngle * 0.001) - pzP, electronBeamEnergy + positronBeamEnergy);
      m_generator.setBoost(boostVector.BoostVector());
    }
  }

  m_generator.enableBoost(m_boostMode > 0);
}


void KoralWInputModule::event()
{
  m_mcGraph.clear();
  m_generator.generateEvent(m_mcGraph);
  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);
}


void KoralWInputModule::terminate()
{
  m_generator.term();

  B2INFO(">>> Total cross section: " << m_generator.getCrossSection() << " pb +- " << m_generator.getCrossSectionError() << " pb")
}



