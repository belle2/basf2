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
#include <analysis/dataobjects/Particle.h>
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

      for (std::vector<std::string>::iterator it = m_maskNames.begin() ; it != m_maskNames.end(); ++it) {
        std::string maskName = *it;
        unsigned int nTracks = roe->getNTracks(maskName);
        unsigned int nECLClusters = roe->getNECLClusters(maskName);
        std::map<unsigned int, bool> trackMask = roe->getTrackMask(maskName);
        std::map<unsigned int, bool> eclClusterMask = roe->getECLClusterMask(maskName);


        B2INFO(" - " << "Info for ROEMask with name: \'" << maskName << "\'");

        B2INFO("    o) " << "No. of Tracks which pass the mask: " << nTracks);

        if ((m_whichMask == "track" or m_whichMask == "both") and m_fullPrint)
          printTrackMask(trackMask);

        B2INFO("    o) " << "No. of ECLClusters which pass the mask: " << nECLClusters);

        if ((m_whichMask == "cluster" or m_whichMask == "both") and m_fullPrint)
          printECLClusterMask(eclClusterMask);
      }
    } else
      B2ERROR("RestOfEvent object not valid! Did you build ROE?");

    B2INFO("[RestOfEventPrinterModule] END ------------------------------------------");
  }

  void RestOfEventPrinterModule::printTrackMask(std::map<unsigned int, bool> trackMask) const
  {
    StoreArray<Track> tracks;

    std::cout << std::endl << "           Track (ID, PDG, Mask): ";
    for (const auto& it : trackMask) {
      const Track* track = tracks[it.first];
      const MCParticle* mcp = track->getRelated<MCParticle>();
      int mcPDG;
      if (mcp)
        mcPDG = mcp->getPDG();
      else
        mcPDG = -1;
      std::cout << "(" << it.first << ", " << mcPDG << ", " << it.second << ")   ";
    }
    std::cout << std::endl << std::endl;
  }

  void RestOfEventPrinterModule::printECLClusterMask(std::map<unsigned int, bool> eclClusterMask) const
  {
    StoreArray<ECLCluster> eclClusters;

    std::cout << std::endl << "           ECLCluster (ID, PDG, Mask): ";
    for (const auto& it : eclClusterMask) {
      const ECLCluster* cluster = eclClusters[it.first];
      const MCParticle* mcp = cluster->getRelated<MCParticle>();
      int mcPDG;
      if (mcp)
        mcPDG = mcp->getPDG();
      else
        mcPDG = -1;
      std::cout << "(" << it.first << ", " << mcPDG << ", " << it.second << ")   ";
    }
    std::cout << std::endl << std::endl;
  }

} // end Belle2 namespace

