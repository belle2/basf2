/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/RestOfEventPrinter/RestOfEventPrinterModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/RestOfEvent.h>
#include <mdst/dataobjects/MCParticle.h>

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
    addParam("fullPrint", m_fullPrint, "If true, print whole masks content.", false);
    addParam("unpackComposites", m_unpackComposites, "If true, replace composites by their daughters", true);
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

      int relatedPDG = part->getPDGCode();
      int relatedMCPDG;
      if (mcpart)
        relatedMCPDG = mcpart->getPDG();
      else
        relatedMCPDG = -1;

      // Start printing
      B2INFO(" - " << "ROE related to particle with PDG: " << relatedPDG);
      B2INFO(" - " << "ROE related to MC particle with PDG: " << relatedMCPDG);

      roe->print("", m_unpackComposites);

      for (const auto& maskName : m_maskNames) {
        B2INFO(" - " << "Info for ROEMask with name: \'" << maskName << "\'");
        roe->print(maskName, m_unpackComposites);

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
    for (auto* particle : maskParticles) {
      particle->print();
    }
  }
} // end Belle2 namespace

