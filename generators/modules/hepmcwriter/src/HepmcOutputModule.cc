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
// #include <framework/gearbox/Unit.h>
// #include <framework/core/Environment.h>
// #include <boost/format.hpp>
// #include <boost/foreach.hpp>

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
  addParam("OutputFilename", m_filename, "The filename of the output file");
}


void HepMCOutputModule::initialize()
{
  // don't know how to do that on the stack ...
  // (IO_GenEvent wants to be initialized with a filename)
  m_ascii_io = new HepMC::IO_GenEvent(m_filename, std::ios::out);
}


void HepMCOutputModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  StoreArray<MCParticle> mcPartCollection;

  int nPart = mcPartCollection.getEntries();

  HepMC::HEPEVT_Wrapper::set_event_number(eventMetaDataPtr->getEvent());
  HepMC::HEPEVT_Wrapper::set_number_entries(nPart);

  for (int iPart = 0; iPart < nPart; ++iPart) {
    MCParticle& mcPart = *mcPartCollection[iPart];

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

  HepMC::GenEvent* evt = m_hepevtio.read_next_event();
  *m_ascii_io << evt;
  delete evt;

  // HepMC::HEPEVT_Wrapper::set_event_number(4);
  B2INFO("Just read an event with" << evt->particles_size() << " particles");

}

void HepMCOutputModule::terminate()
{
  // here or better in destructor?
  if (m_ascii_io != 0) {
    delete m_ascii_io;
  }
}

