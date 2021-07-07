/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/hepmcwriter/HepmcOutputModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <mdst/dataobjects/MCParticle.h>

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
    HepMC::HEPEVT_Wrapper::set_momentum(iPart,
                                        mom.X() * Unit::GeV,
                                        mom.Y() * Unit::GeV,
                                        mom.Z() * Unit::GeV,
                                        mcPart.getEnergy() * Unit::GeV);
    HepMC::HEPEVT_Wrapper::set_mass(iPart, mcPart.getMass() * Unit::GeV);
    HepMC::HEPEVT_Wrapper::set_position(iPart,
                                        vert.X() * Unit::cm,
                                        vert.Y() * Unit::cm,
                                        vert.Z() * Unit::cm,
                                        mcPart.getProductionTime() * Const::speedOfLight * Unit::cm);
  }

  // read from buffers and write event to disk
  HepMC::GenEvent* evt = m_hepevtio.read_next_event();
  evt->use_units(HepMC::Units::GEV, HepMC::Units::CM);
  *m_ascii_io << evt;
  delete evt;

}

void HepMCOutputModule::terminate()
{

}
