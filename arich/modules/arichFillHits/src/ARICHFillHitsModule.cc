/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <arich/modules/arichFillHits/ARICHFillHitsModule.h>



// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// Dataobject classes
#include <arich/dataobjects/ARICHDigit.h>
#include <arich/dataobjects/ARICHHit.h>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHFillHits)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHFillHitsModule::ARICHFillHitsModule() : Module(),
    m_arichgp(ARICHGeometryPar::Instance())
  {
    // set module description (e.g. insert text)
    setDescription("Fills ARICHHits collection from ARICHDigits");
    setPropertyFlags(c_ParallelProcessingCertified);

  }

  ARICHFillHitsModule::~ARICHFillHitsModule()
  {
  }

  void ARICHFillHitsModule::initialize()
  {

    StoreArray<ARICHDigit> digits;
    digits.isRequired();

    StoreArray<ARICHHit> arichHits;
    arichHits.registerInDataStore();

    if (!m_arichgp->isInit()) {
      GearDir content("/Detector/DetectorComponent[@name='ARICH']/Content");
      m_arichgp->Initialize(content);
    }
    if (!m_arichgp->isInit()) B2ERROR("Component ARICH not found in Gearbox");

  }

  void ARICHFillHitsModule::beginRun()
  {
  }

  void ARICHFillHitsModule::event()
  {

    StoreArray<ARICHDigit> digits;
    StoreArray<ARICHHit> arichHits;

    for (const auto& digit : digits) {
      int chID = digit.getChannelID();
      int modID = digit.getModuleID();
      TVector3 hitpos = m_arichgp->getChannelCenterGlob(modID, chID);
      arichHits.appendNew(hitpos, modID, chID);
    }

  }


  void ARICHFillHitsModule::endRun()
  {
  }

  void ARICHFillHitsModule::terminate()
  {
  }


} // end Belle2 namespace

