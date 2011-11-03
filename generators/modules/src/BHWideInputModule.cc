/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/BHWideInputModule.h>
#include <generators/utilities/cm2LabBoost.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/RunMetaData.h>
#include <framework/gearbox/Unit.h>

#include <TLorentzVector.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BHWideInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BHWideInputModule::BHWideInputModule() : Module()
{
  //Set module properties
  setDescription("Generates radiative BhaBha scattering events with BHWide.");

  //Parameter definition
  addParam("BoostMode", m_boostMode, "The mode of the boost (0 = no boost, 1 = Belle II, 2 = Belle)", 0);
  addParam("ScatteringAngleRangePositron", m_ScatteringAngleRangePositron, "Min [0] and Max [1] value for the scattering angle [deg] of the positron.", make_vector(17.0, 150.0));
  addParam("ScatteringAngleRangeElectron", m_ScatteringAngleRangeElectron, "Min [0] and Max [1] value for the scattering angle [deg] of the electron.", make_vector(17.0, 150.0));
}


BHWideInputModule::~BHWideInputModule()
{

}


void BHWideInputModule::initialize()
{
  StoreObjPtr<RunMetaData> runMetaDataPtr("RunMetaData", DataStore::c_Run);

  //Depending on the settings, use the Belle II or Belle boost
  if (m_boostMode == 1) {
    m_generator.setBoost(getBoost(runMetaDataPtr->getEnergyHER(), runMetaDataPtr->getEnergyLER(),
                                  runMetaDataPtr->getCrossingAngle(), runMetaDataPtr->getAngleLER()));
  } else {
    if (m_boostMode == 2) {

      //electron and positron beam energies (magic numbers from Jeremy)
      double electronBeamEnergy = 7.998213; // [GeV]
      double positronBeamEnergy = 3.499218; // [GeV]
      double crossingAngle = 22.0; //[mrad]

      double pzP = sqrt(positronBeamEnergy * positronBeamEnergy - 0.000510998918 * 0.000510998918);
      double pE  = sqrt(electronBeamEnergy * electronBeamEnergy - 0.000510998918 * 0.000510998918);
      TLorentzVector boostVector(pE*sin(crossingAngle * 0.001), 0., pE * cos(crossingAngle * 0.001) - pzP, electronBeamEnergy + positronBeamEnergy);
      m_generator.setBoost(boostVector.BoostVector());
    }
  }

  m_generator.enableBoost(m_boostMode > 0);
  m_generator.setScatAnglePositron(vectorToPair(m_ScatteringAngleRangePositron, "ScatteringAngleRangePositron"));
  m_generator.setScatAngleElectron(vectorToPair(m_ScatteringAngleRangeElectron, "ScatteringAngleRangeElectron"));
  m_generator.init();
}


void BHWideInputModule::event()
{
  m_mcGraph.clear();
  m_generator.generateEvent(m_mcGraph);
  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);
}


void BHWideInputModule::terminate()
{
  m_generator.term();

  B2INFO(">>> Total cross section: " << m_generator.getCrossSection() * 0.001 << " nb +- " << m_generator.getCrossSection() * m_generator.getCrossSectionError() * 0.001 << " nb")
}
