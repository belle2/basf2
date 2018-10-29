/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matic Lubej                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/RestOfEventPrinter/RestOfEventPrinterModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/RestOfEvent.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>


#include <iostream>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(RestOfEventPrinter)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  RestOfEventPrinterModule::RestOfEventPrinterModule() : Module()
  {
    // set module description (e.g. insert text)
    setDescription("Prints basic or detailed RestOfEvent info to screen. It is possible to print out ROEMasks for specific mask names as well.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    std::vector<std::string> emptyVector;

    addParam("maskNames", m_maskNames, "List of all mask names for which the info will be printed.", emptyVector);
    addParam("whichMask", m_whichMask, "Print Track mask (track), ECLCluster mask (cluster), or (both)?", std::string("both"));
    addParam("fullPrint", m_fullPrint, "True: Print whole masks content.", false);
  }

  void RestOfEventPrinterModule::initialize()
  {
    StoreArray<RestOfEvent>().isRequired();
  }

  void RestOfEventPrinterModule::event()
  {
    B2INFO("[RestOfEventPrinterModule] START ----------------------------------------");

    StoreObjPtr<RestOfEvent> roe("RestOfEvent");

    if (roe.isValid()) {

      const Particle* part = roe->getRelated<Particle>();
      const MCParticle* mcpart = part->getRelated<MCParticle>();

      unsigned int nAllTracks = roe->getNTracks();
      unsigned int nAllECLClusters = roe->getNECLClusters();
      unsigned int nAllKLMClusters = roe->getNKLMClusters();
      int relatedPDG = part->getPDGCode();
      int relatedMCPDG;
      if (mcpart)
        relatedMCPDG = mcpart->getPDG();
      else
        relatedMCPDG = -1;

      // Start printing
      B2INFO(" - " << "ROE related to particle with PDG: " << relatedPDG);
      B2INFO(" - " << "ROE related to MC particle with PDG: " << relatedMCPDG);
      B2INFO(" - " << "No. of Tracks in ROE: " << nAllTracks);
      B2INFO(" - " << "No. of ECLClusters in ROE: " << nAllECLClusters);
      B2INFO(" - " << "No. of KLMClusters in ROE: " << nAllKLMClusters);

      for (std::vector<std::string>::iterator it = m_maskNames.begin() ; it != m_maskNames.end(); ++it) {
        std::string maskName = *it;
        unsigned int nTracks = roe->getNTracks(maskName);
        unsigned int nECLClusters = roe->getNECLClusters(maskName);


        B2INFO(" - " << "Info for ROEMask with name: \'" << maskName << "\'");

        B2INFO("    o) " << "No. of Tracks which pass the mask: " << nTracks);

        B2INFO("    o) " << "No. of ECLClusters which pass the mask: " << nECLClusters);
        if (m_fullPrint) {
          printMaskParticles(roe->getParticles(maskName));
        }
      }
    } else
      B2ERROR("RestOfEvent object not valid! Did you build ROE?");

    B2INFO("[RestOfEventPrinterModule] END ------------------------------------------");
  }
  void RestOfEventPrinterModule::printMaskParticles(const std::vector<const Particle*>& maskParticles) const
  {
    std::string printout = "\n - -  Particles: ";
    for (auto* particle : maskParticles) {
      particle->print();
    }
  }
} // end Belle2 namespace

