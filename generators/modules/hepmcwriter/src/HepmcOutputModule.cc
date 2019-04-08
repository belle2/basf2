/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nikolai Hartmann                                         *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/hepmcwriter/HepmcOutputModule.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HepMCOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

HepMCOutputModule::HepMCOutputModule() : Module()
{
  setDescription("HepMC file output. This module loads an event record from the MCParticle collection and store the content back into the HepMC (2) format. HepMC format is a standard event record format to contain an event record in a Monte Carlo-independent format.");

  addParam("OutputFilename", m_filename, "The filename of the output file");
  addParam("StoreVirtualParticles", m_storeVirtualParticles, "Store also virtual particles in the HepMC file.", false);
}


void HepMCOutputModule::initialize()
{
  m_ascii_io = std::make_unique<HepMC::IO_GenEvent>(m_filename, std::ios::out);
}


void HepMCOutputModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  StoreArray<MCParticle> mcPartCollection;

  int nPart = mcPartCollection.getEntries();

  //Find number of virtual particles
  int nVirtualPart = 0;
  if (!m_storeVirtualParticles) {
    for (int iPart = 0; iPart < nPart; ++iPart) {
      MCParticle& mcPart = *mcPartCollection[iPart];
      if (mcPart.isVirtual()) nVirtualPart++;
    }
  }

  // The following fills values into the HEPEVT_Wrapper buffers
  // The procedure is similar to the code in HepevtOutputModule

  HepMC::HEPEVT_Wrapper::zero_everything();
  HepMC::HEPEVT_Wrapper::set_event_number(eventMetaDataPtr->getEvent());
  HepMC::HEPEVT_Wrapper::set_number_entries(nPart - nVirtualPart);

  // Note: Particle numbering in HepMC starts at 1
  for (int iPart = 1; iPart <= nPart; ++iPart) {
    // but this is a normal array, starting at 0
    MCParticle& mcPart = *mcPartCollection[iPart - 1];

    if (!m_storeVirtualParticles && mcPart.isVirtual()) {
      continue;
    }

    TVector3 mom = mcPart.getMomentum();
    TVector3 vert = mcPart.getVertex();

    int isthep = 1;
    if (mcPart.getFirstDaughter() > 0) isthep = 2;
    if (mcPart.isInitial()) isthep = 2;

    int motherIndex = 0;
    if (mcPart.getMother() != NULL) motherIndex = mcPart.getMother()->getIndex();

    HepMC::HEPEVT_Wrapper::set_status(iPart, isthep);
    HepMC::HEPEVT_Wrapper::set_id(iPart, mcPart.getPDG());
    HepMC::HEPEVT_Wrapper::set_parents(iPart, motherIndex, motherIndex);
    HepMC::HEPEVT_Wrapper::set_children(iPart, mcPart.getFirstDaughter(), mcPart.getLastDaughter());
    HepMC::HEPEVT_Wrapper::set_momentum(iPart, mom.X(), mom.X(), mom.Z(), mcPart.getEnergy());
    HepMC::HEPEVT_Wrapper::set_mass(iPart, mcPart.getMass());
    HepMC::HEPEVT_Wrapper::set_position(iPart, vert.X(), vert.Y(), vert.Z(), mcPart.getProductionTime());
  }

  // read from buffers and write event to disk
  HepMC::GenEvent* evt = m_hepevtio.read_next_event();
  *m_ascii_io << evt;
  delete evt;

}

void HepMCOutputModule::terminate()
{

}
