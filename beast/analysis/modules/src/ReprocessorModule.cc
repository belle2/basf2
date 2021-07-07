/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/analysis/modules/ReprocessorModule.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/logging/Logger.h>

//c++
#include <boost/foreach.hpp>
#include <string>
#include <fstream>

using namespace std;
using namespace Belle2;
//using namespace reprocessor;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Reprocessor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ReprocessorModule::ReprocessorModule() : Module()
{
  // Set module properties
  setDescription("Reprocessor module");

  addParam("input_TPC_PDG", m_input_TPC_PDG, "Set the PDG particle to re-launch");
  addParam("input_TPC_Ntimes", m_input_TPC_Ntimes, "Re-launch the PDG particle N times");
  addParam("input_HE3_PDG", m_input_HE3_PDG, "Set the PDG particle to re-launch");
  addParam("input_HE3_Ntimes", m_input_HE3_Ntimes, "Re-launch the PDG particle N times");
}

ReprocessorModule::~ReprocessorModule()
{
}

void ReprocessorModule::initialize()
{
  B2INFO("Reprocessor: Initializing");

  m_mcParticle.registerInDataStore();

  m_evtMetaData.registerInDataStore();

  MetaHits.isRequired();
  mc_he3_parts.isOptional();
  mc_tpc_parts.isOptional();
}

void ReprocessorModule::beginRun()
{
}

void ReprocessorModule::event()
{
  //skip events with no HE3G4 and TPCG4 particles stored
  if (mc_he3_parts.getEntries() == 0 && mc_tpc_parts.getEntries() == 0 && m_input_TPC_Ntimes != 0 && m_input_HE3_Ntimes != 0) {
    return;
  }
  if (mc_he3_parts.getEntries() == 0 && m_input_HE3_Ntimes != 0) {
    return;
  }
  if (mc_tpc_parts.getEntries() == 0 && m_input_TPC_Ntimes != 0) {
    return;
  }

  //Look at the meta data to extract IR rate and scattering ring section
  double rate = 0;
  for (const auto& MetaHit : MetaHits) {
    rate = MetaHit.getrate();
  }

  if (m_input_HE3_Ntimes != 0 && m_input_TPC_Ntimes == 0) rate /= (double)m_input_HE3_Ntimes;
  if (m_input_TPC_Ntimes != 0 && m_input_HE3_Ntimes == 0) rate /= (double)m_input_TPC_Ntimes;

  // if not already existed, create MCParticles data store
  StoreArray<MCParticle> MCParticles;
  MCParticles.clear();

  StoreObjPtr<EventMetaData> evtMetaData;
  //evtMetaData.clear();
  if (!evtMetaData.isValid()) evtMetaData.create();

  if (m_input_TPC_Ntimes != 0) {
    for (const auto& mcpart : mc_tpc_parts) { // start loop over all Tracks
      const int PDG = mcpart.getPDG();
      const double energy = mcpart.getEnergy();
      TVector3 Momentum = mcpart.getMomentum();
      TVector3 ProductionVertex = mcpart.getProductionVertex();
      if (m_input_TPC_PDG == PDG) {
        for (int i = 0; i < m_input_TPC_Ntimes; i ++) {
          // store generated particle
          MCParticle* particle = MCParticles.appendNew();
          particle->setStatus(MCParticle::c_PrimaryParticle);
          particle->setPDG(PDG);
          particle->setMassFromPDG();
          particle->setMomentum(Momentum);
          particle->setProductionVertex(ProductionVertex);
          particle->setProductionTime(0.0);
          particle->setEnergy(energy);
          particle->setValidVertex(true);
        }
      }
    }
  }

  if (m_input_HE3_Ntimes != 0) {
    for (const auto& mcpart : mc_he3_parts) { // start loop over all Tracks
      const int PDG = mcpart.getPDG();
      const double energy = mcpart.getEnergy();
      TVector3 Momentum = mcpart.getMomentum();
      TVector3 ProductionVertex = mcpart.getProductionVertex();
      if (m_input_HE3_PDG == PDG) {
        for (int i = 0; i < m_input_HE3_Ntimes; i ++) {
          // store generated particle
          MCParticle* particle = MCParticles.appendNew();
          particle->setStatus(MCParticle::c_PrimaryParticle);
          particle->setPDG(PDG);
          particle->setMassFromPDG();
          particle->setMomentum(Momentum);
          particle->setProductionVertex(ProductionVertex);
          particle->setProductionTime(0.0);
          particle->setEnergy(energy);
          particle->setValidVertex(true);
        }
      }
    }
  }

  evtMetaData->setGeneratedWeight(rate);
}



void ReprocessorModule::endRun()
{
}

void ReprocessorModule::terminate()
{
}


