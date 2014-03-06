/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticlePrinter/ParticlePrinterModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticlePrinter)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticlePrinterModule::ParticlePrinterModule() : Module()

  {
    // set module description (e.g. insert text)
    setDescription("Prints particle list to screen (usefull for debugging)");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("ListName", m_listName, "name of particle list", string(""));
    addParam("FullPrint", m_fullPrint, "reduced or full printout", true);

  }

  ParticlePrinterModule::~ParticlePrinterModule()
  {
  }

  void ParticlePrinterModule::initialize()
  {
  }

  void ParticlePrinterModule::beginRun()
  {
  }

  void ParticlePrinterModule::event()
  {
    StoreObjPtr<ParticleList> plist(m_listName);
    if (!plist) {
      B2ERROR("ParticleList " << m_listName << " not found");
      return;
    }

    std::cout << std::endl;
    plist->print();
    if (!m_fullPrint) return;

    std::cout << std::endl;
    for (unsigned i = 0; i < plist->getListSize(); i++) {
      plist->getParticle(i)->print();
    }

  }


  void ParticlePrinterModule::endRun()
  {
  }

  void ParticlePrinterModule::terminate()
  {
  }

  void ParticlePrinterModule::printModuleParams() const
  {
  }


} // end Belle2 namespace

