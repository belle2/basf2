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
#include <background/modules/BeamBkgTagSetter/BeamBkgTagSetterModule.h>

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// SimHits
#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <top/dataobjects/TOPSimHit.h>
#include <arich/dataobjects/ARICHSimHit.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>
#include <bklm/dataobjects/BKLMSimHit.h>
#include <eklm/dataobjects/EKLMSimHit.h>

// MetaData
#include <framework/dataobjects/EventMetaData.h>
#include <background/dataobjects/BackgroundMetaData.h>

#include <unordered_map>
using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(BeamBkgTagSetter)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  BeamBkgTagSetterModule::BeamBkgTagSetterModule() : Module(),
    m_backgroundTag(SimHitBase::bg_none)

  {
    // set module description (e.g. insert text)
    setDescription("Sets beam background tag variable in SimHits; returns true if at least one of the SimHit store arrays has entries. Return value can be used to discard empty events at output.");

    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("backgroundType", m_backgroundType, "one of: Coulomb_LER, Coulomb_HER, RBB_LER, RBB_HER, Touschek_LER, Touschek_HER, twoPhoton, other");
    addParam("realTime", m_realTime,
             "real time in nano seconds that corresponds to background samle");

  }

  BeamBkgTagSetterModule::~BeamBkgTagSetterModule()
  {
  }

  void BeamBkgTagSetterModule::initialize()
  {
    if (m_realTime <= 0) B2FATAL("invalid realTime: " << m_realTime);

    std::unordered_map<std::string, SimHitBase::BG_TAG> tag;
    tag["Coulomb_LER"] = SimHitBase::bg_Coulomb_LER;
    tag["Coulomb_HER"] = SimHitBase::bg_Coulomb_HER;
    tag["RBB_LER"] = SimHitBase::bg_RBB_LER;
    tag["RBB_HER"] = SimHitBase::bg_RBB_HER;
    tag["Touschek_LER"] = SimHitBase::bg_Touschek_LER;
    tag["Touschek_HER"] = SimHitBase::bg_Touschek_HER;
    tag["twoPhoton"] = SimHitBase::bg_twoPhoton;
    tag["other"] = SimHitBase::bg_other;

    m_backgroundTag = tag[m_backgroundType];

    if (m_backgroundTag == 0) {
      B2WARNING("Unknown beam background: " << m_backgroundType <<
                ", will be tagged as SimHitBase::bg_other");
      m_backgroundTag = SimHitBase::bg_other;
    }

    StoreObjPtr<BackgroundMetaData> bkgMetaData("", DataStore::c_Persistent);
    bkgMetaData.registerAsPersistent();
    bkgMetaData.create();
    bkgMetaData->setBackgroundType(m_backgroundType);
    bkgMetaData->setBackgroundTag(m_backgroundTag);
    bkgMetaData->setRealTime(m_realTime);

    StoreArray<PXDSimHit>::optional();
    StoreArray<SVDSimHit>::optional();
    StoreArray<CDCSimHit>::optional();
    StoreArray<TOPSimHit>::optional();
    StoreArray<ARICHSimHit>::optional();
    StoreArray<ECLSimHit>::optional();
    StoreArray<ECLHit>::optional();
    StoreArray<BKLMSimHit>::optional();
    StoreArray<EKLMSimHit>::optional();
  }

  void BeamBkgTagSetterModule::beginRun()
  {
  }

  void BeamBkgTagSetterModule::event()
  {
    StoreArray<PXDSimHit> pxdSimHits;
    StoreArray<SVDSimHit> svdSimHits;
    StoreArray<CDCSimHit> cdcSimHits;
    StoreArray<TOPSimHit> topSimHits;
    StoreArray<ARICHSimHit> arichSimHits;
    StoreArray<ECLSimHit> eclSimHits;
    StoreArray<ECLHit> eclHits;
    StoreArray<BKLMSimHit> bklmSimHits;
    StoreArray<EKLMSimHit> eklmSimHits;

    int n = setBackgroundTag(pxdSimHits);
    n += setBackgroundTag(svdSimHits);
    n += setBackgroundTag(cdcSimHits);
    n += setBackgroundTag(topSimHits);
    n += setBackgroundTag(arichSimHits);
    n += setBackgroundTag(eclSimHits);
    n += setBackgroundTag(eclHits);
    n += setBackgroundTag(bklmSimHits);
    n += setBackgroundTag(eklmSimHits);

    setReturnValue(n > 0);

    StoreObjPtr<EventMetaData> evtMetaData;
    B2INFO("Exp " << evtMetaData->getExperiment() <<
           " Run " << evtMetaData->getRun() <<
           " Event " << evtMetaData->getEvent() <<
           " number of SimHits = " << n);

  }


  void BeamBkgTagSetterModule::endRun()
  {
  }

  void BeamBkgTagSetterModule::terminate()
  {
  }

  void BeamBkgTagSetterModule::printModuleParams() const
  {
  }


} // end Belle2 namespace

