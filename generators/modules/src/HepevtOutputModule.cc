/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 * Updated by R. Godang 11/26/13                                          *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/HepevtOutputModule.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dataobjects/MCParticle.h>

using namespace std;
using namespace Belle2;
using namespace boost;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HepevtOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

HepevtOutputModule::HepevtOutputModule() : Module()
{
  //Set module properties
  setDescription("HepEvt file output. This module loads an event record from the MCParticle collection and store the content back into the HEPEVT format. HEPEVT format is a standard event record format to contain an event record in a Monte Carlo-independent format.");

  //Parameter definition
  addParam("OutputFilename", m_filename, "The filename of the output file");
  addParam("MirrorPz", m_mirrorPz, "If the directions of HER and LER are switched, mirror Pz.", false);
  addParam("StoreVirtualParticles", m_storeVirtualParticles, "Store also virtual particles in the HEPEvt file.", false);
  addParam("FullFormat", m_fullFormat, "Write the full HepEvt format to file. Set it to false for a compact format.", true);
}


void HepevtOutputModule::initialize()
{
  m_fileStream.open(m_filename.c_str());
}


void HepevtOutputModule::event()
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

  m_fileStream << format("%10d%10d\n") % eventMetaDataPtr->getEvent() % (nPart - nVirtualPart);

  for (int iPart = 0; iPart < nPart; ++iPart) {
    MCParticle& mcPart = *mcPartCollection[iPart];
    if (!m_storeVirtualParticles && mcPart.isVirtual()) continue;

    TVector3 mom = mcPart.getMomentum();
    if (m_mirrorPz) mom.SetZ(-1.0 * mom.Z());

    if (m_fullFormat) {
      int isthep = 1;
      if (mcPart.getFirstDaughter() > 0) isthep = 2;

      int motherIndex = 0;
      if (mcPart.getMother() != NULL) motherIndex = mcPart.getMother()->getIndex();

      // cppcheck-suppress zerodiv
      m_fileStream << format("%5i%12i%10i%10i%10i%10i") % isthep % mcPart.getPDG() % motherIndex % motherIndex % mcPart.getFirstDaughter() % mcPart.getLastDaughter();
      m_fileStream << format("%15.6f%15.6f%15.6f%15.6f%15.6f") % mom.X() % mom.Y() % mom.Z() % mcPart.getEnergy() % mcPart.getMass();
      m_fileStream << format("%15.6f%15.6f%15.6f%15.6f\n") % mcPart.getVertex().X() % mcPart.getVertex().Y() % mcPart.getVertex().Z() % mcPart.getProductionTime();
    } else {
      m_fileStream << format("%15.6f%15.6f%15.6f%15.6f%15.6f%15i\n") % mom.X() % mom.Y() % mom.Z() % mcPart.getEnergy() % mcPart.getMass() % mcPart.getPDG();
    }
  }
}


void HepevtOutputModule::terminate()
{
  m_fileStream.close();
}
