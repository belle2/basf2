/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/bbbreminput/BBBremInputModule.h>
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
REG_MODULE(BBBremInput)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BBBremInputModule::BBBremInputModule() : Module()
{
  //Set module properties
  setDescription("Generates low scattering angle radiative Bhabha events (Beam-Beam Bremsstrahlung).");

  //Parameter definition
  addParam("BoostMode", m_boostMode, "The mode of the boost (0 = no boost, 1 = Belle II, 2 = Belle)", 0);
  addParam("MinPhotonEnergyFraction", m_photonEFrac, "Fraction of the minimum photon energy.", 0.000001);
  addParam("Unweighted", m_unweighted, "Produce unweighted or weighted events.", true);
  addParam("MaxWeight", m_maxWeight, "The max weight (only for Unweighted=True).", 2000.0);
}


BBBremInputModule::~BBBremInputModule()
{

}


void BBBremInputModule::initialize()
{
  GearDir ler("/Detector/SuperKEKB/LER/");
  GearDir her("/Detector/SuperKEKB/HER/");

  double centerOfMassEnergy = 2.0 * sqrt(her.getDouble("energy") * ler.getDouble("energy"));

  m_generator.init(centerOfMassEnergy, m_photonEFrac, m_unweighted, m_maxWeight);

  //Depending on the settings use the Belle II or Belle boost
  if (m_boostMode == 1) {
    m_generator.setBoost(getBoost(her.getDouble("energy"), ler.getDouble("energy"),
                                  her.getDouble("angle") - ler.getDouble("angle"), her.getDouble("angle")));
  } else {
    if (m_boostMode == 2) {
      m_generator.setBoost(getBoost(7.998213, 3.499218, 22.0 * Unit::mrad, 0.0));
    }
  }

  m_generator.enableBoost(m_boostMode > 0);
}


void BBBremInputModule::event()
{
  StoreObjPtr<EventMetaData> evtMetaData("EventMetaData", DataStore::c_Event);

  m_mcGraph.clear();
  double weight = m_generator.generateEvent(m_mcGraph);
  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

  evtMetaData->setGeneratedWeight(weight);
}


void BBBremInputModule::terminate()
{
  m_generator.term();

  B2INFO("Maximum weight delivered: " << m_generator.getMaxWeightDelivered())
  B2INFO("Cross-section:            " << m_generator.getCrossSection() << " +- " << m_generator.getCrossSectionError() << " [mb]")
}
