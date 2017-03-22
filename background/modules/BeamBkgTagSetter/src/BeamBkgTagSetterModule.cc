/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Yuri Solovier, Igal Jaegle                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgTagSetter/BeamBkgTagSetterModule.h>


// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

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

// BEAST SimHits
#include <beast/beamabort/dataobjects/BeamabortSimHit.h>
#include <beast/claw/dataobjects/ClawSimHit.h>
#include <beast/claws/dataobjects/CLAWSSimHit.h>
#include <beast/fangs/dataobjects/FANGSSimHit.h>
#include <beast/plume/dataobjects/PlumeSimHit.h>
#include <beast/pindiode/dataobjects/PindiodeSimHit.h>
#include <beast/he3tube/dataobjects/He3tubeSimHit.h>
#include <beast/microtpc/dataobjects/MicrotpcSimHit.h>
#include <beast/qcsmonitor/dataobjects/QcsmonitorSimHit.h>

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
    m_backgroundTag(SimHitBase::bg_none), m_fileType(BackgroundMetaData::c_Usual)

  {
    // set module description (e.g. insert text)
    setDescription("Sets beam background tag variable in SimHits and "
                   "adds BackgroundMetaData branch in persistent tree; "
                   "returns true if at least one of the SimHit store arrays "
                   "has entries. Return value can be used to discard empty "
                   "events at output.");

    // parallel processing certificate
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("backgroundType", m_backgroundType,
             "one of: " + m_bgTypes.getBGTypes());
    addParam("realTime", m_realTime,
             "equivalent time of superKEKB running in [ns] to obtain this sample");
    addParam("specialFor", m_specialFor,
             "tag ordinary file (default) or additional file ('ECL' or 'PXD')",
             string(""));
    addParam("Phase", m_phase,
             "specify the phase: 1 for phase 1, 2 for phase 2, 3 for physics run or phase 3");

  }

  BeamBkgTagSetterModule::~BeamBkgTagSetterModule()
  {
  }

  void BeamBkgTagSetterModule::initialize()
  {
    if (m_realTime <= 0) B2FATAL("invalid realTime: " << m_realTime);

    m_backgroundTag = m_bgTypes.getTag(m_backgroundType);
    if (m_backgroundTag == 0) {
      B2ERROR("Unknown beam background type: " << m_backgroundType << "\n"
              "Possible are: " + m_bgTypes.getBGTypes());
      m_backgroundTag = SimHitBase::bg_other;
    }

    if (m_specialFor != "") {
      if (m_specialFor == "ECL") {m_fileType = BackgroundMetaData::c_ECL;}
      else if (m_specialFor == "PXD") {m_fileType = BackgroundMetaData::c_PXD;}
      else {B2ERROR("specialFor " << m_specialFor << "not supported");}
    }

    StoreObjPtr<BackgroundMetaData> bkgMetaData("", DataStore::c_Persistent);
    bkgMetaData.registerInDataStore();
    if (!bkgMetaData.isValid())
      bkgMetaData.create();
    bkgMetaData->setBackgroundType(m_backgroundType);
    bkgMetaData->setBackgroundTag(m_backgroundTag);
    bkgMetaData->setRealTime(m_realTime);
    bkgMetaData->setFileType(m_fileType);

    StoreArray<PXDSimHit>::optional();
    StoreArray<SVDSimHit>::optional();
    StoreArray<CDCSimHit>::optional();
    StoreArray<TOPSimHit>::optional();
    StoreArray<ARICHSimHit>::optional();
    StoreArray<ECLSimHit>::optional();
    StoreArray<ECLHit>::optional();
    StoreArray<BKLMSimHit>::optional();
    StoreArray<EKLMSimHit>::optional();

    // BEAST StoreArray
    StoreArray<BeamabortSimHit>::optional();
    StoreArray<CLAWSSimHit>::optional();
    StoreArray<ClawSimHit>::optional();
    StoreArray<FANGSSimHit>::optional();
    StoreArray<PlumeSimHit>::optional();
    StoreArray<PindiodeSimHit>::optional();
    StoreArray<He3tubeSimHit>::optional();
    StoreArray<MicrotpcSimHit>::optional();
    StoreArray<QcsmonitorSimHit>::optional();
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

    // BEAST addition
    StoreArray<BeamabortSimHit> diaSimHits;
    StoreArray<CLAWSSimHit> clw2SimHits;
    StoreArray<ClawSimHit> clw1SimHits;
    StoreArray<FANGSSimHit> fngSimHits;
    StoreArray<PlumeSimHit> plmSimHits;
    StoreArray<PindiodeSimHit> pinSimHits;
    StoreArray<He3tubeSimHit> he3SimHits;
    StoreArray<MicrotpcSimHit> tpcSimHits;
    StoreArray<QcsmonitorSimHit> sciSimHits;

    int n = 0;
    if (m_phase == 2 || m_phase == 3) {
      n += setBackgroundTag(pxdSimHits);
      n += setBackgroundTag(svdSimHits);
      n += setBackgroundTag(cdcSimHits);
      n += setBackgroundTag(topSimHits);
      n += setBackgroundTag(arichSimHits);
      n += setBackgroundTag(eclSimHits);
      n += setBackgroundTag(eclHits);
      n += setBackgroundTag(bklmSimHits);
      n += setBackgroundTag(eklmSimHits);
    }
    // BEAST addition
    if (m_phase == 1 || m_phase == 2) {
      n += setBackgroundTag(diaSimHits);
      if (m_phase == 1)
        n += setBackgroundTag(clw1SimHits);
      if (m_phase == 2) {
        n += setBackgroundTag(clw2SimHits);
        n += setBackgroundTag(fngSimHits);
        n += setBackgroundTag(plmSimHits);
      }
      n += setBackgroundTag(pinSimHits);
      n += setBackgroundTag(he3SimHits);
      n += setBackgroundTag(tpcSimHits);
      n += setBackgroundTag(sciSimHits);
    }

    if (m_fileType == BackgroundMetaData::c_ECL) n = eclHits.getEntries();
    if (m_fileType == BackgroundMetaData::c_PXD) n = pxdSimHits.getEntries();

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


} // end Belle2 namespace

