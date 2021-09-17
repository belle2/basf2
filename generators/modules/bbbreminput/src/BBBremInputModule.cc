/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/bbbreminput/BBBremInputModule.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BBBremInput)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BBBremInputModule::BBBremInputModule() : Module(), m_initial(BeamParameters::c_smearVertex)
{
  //Set module properties
  setDescription("Generates low scattering angle radiative Bhabha events (Beam-Beam Bremsstrahlung).");

  //Parameter definition
  addParam("MinPhotonEnergyFraction", m_photonEFrac, "Fraction of the minimum photon energy.", 0.000001);
  addParam("Unweighted", m_unweighted, "Produce unweighted or weighted events.", true);
  addParam("MaxWeight", m_maxWeight, "The max weight (only for Unweighted=True).", 2000.0);
  addParam("DensityCorrectionMode", m_densityCorrectionMode, "Mode for bunch density correction (none=0, hard=1 (default), soft=2)",
           1);
  addParam("DensityCorrectionParameter", m_DensityCorrectionParameter, "Density correction parameter tc (=(hbarc/sigma_y)^2)",
           1.68e-17);
}


BBBremInputModule::~BBBremInputModule()
{

}


void BBBremInputModule::initialize()
{
  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();

  //Beam Parameters, initial particle - BBBREM cannot handle beam energy spread
  m_initial.initialize();

}


void BBBremInputModule::event()
{
  // Check if the BeamParameters have changed (if they do, abort the job! otherwise cross section calculation will be a nightmare.)
  if (m_beamParams.hasChanged()) {
    if (!m_initialized) {
      initializeGenerator();
    } else {
      B2FATAL("BBBremInputModule::event(): BeamParameters have changed within a job, this is not supported for BBBREM!");
    }
  }

  StoreObjPtr<EventMetaData> evtMetaData("EventMetaData", DataStore::c_Event);

  // initial particle from beam parameters
  const MCInitialParticles& initial = m_initial.generate();

  // true boost
  ROOT::Math::LorentzRotation boost = initial.getCMSToLab();

  // vertex
  TVector3 vertex = initial.getVertex();

  m_mcGraph.clear();
  double weight = m_generator.generateEvent(m_mcGraph, vertex, boost);

  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

  //store the weight (1.0 for unweighted events)
  evtMetaData->setGeneratedWeight(weight);
}


void BBBremInputModule::terminate()
{
  m_generator.term();

  B2RESULT("Cross-section (weighted):                   " << m_generator.getCrossSection() << " +/- " <<
           m_generator.getCrossSectionError() << " [mb]");
  B2RESULT("Maximum weight delivered:                   " << m_generator.getMaxWeightDelivered());
  B2RESULT("Overweight bias cross-section (unweighted): " << m_generator.getCrossSectionOver() << " +/- " <<
           m_generator.getCrossSectionErrorOver() << " [mb]");
}

void BBBremInputModule::initializeGenerator()
{

  const BeamParameters& nominal = m_initial.getBeamParameters();
  double centerOfMassEnergy = nominal.getMass();

  m_generator.init(centerOfMassEnergy, m_photonEFrac, m_unweighted, m_maxWeight, m_densityCorrectionMode,
                   m_DensityCorrectionParameter);

  m_initialized = true;

}
