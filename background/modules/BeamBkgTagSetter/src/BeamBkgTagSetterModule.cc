/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <background/modules/BeamBkgTagSetter/BeamBkgTagSetterModule.h>


// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// MetaData
#include <framework/dataobjects/EventMetaData.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(BeamBkgTagSetter);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BeamBkgTagSetterModule::BeamBkgTagSetterModule() : Module(),
  m_backgroundTag(BackgroundMetaData::bg_none), m_fileType(BackgroundMetaData::c_Usual)

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
           "specify the Phase: 1 for Phase 1, 2 for Phase 2, 3 for Physics Run or Phase 3", 3);

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
    m_backgroundTag = BackgroundMetaData::bg_other;
  }

  if (m_specialFor != "") {
    if (m_specialFor == "ECL") {m_fileType = BackgroundMetaData::c_ECL;}
    else if (m_specialFor == "PXD") {m_fileType = BackgroundMetaData::c_PXD;}
    else {B2ERROR("specialFor " << m_specialFor << "not supported");}
  }

  // set BackgroundMetaData
  StoreObjPtr<BackgroundMetaData> bkgMetaData("", DataStore::c_Persistent);
  bkgMetaData.registerInDataStore();
  if (!bkgMetaData.isValid())
    bkgMetaData.create();
  bkgMetaData->setBackgroundType(m_backgroundType);
  bkgMetaData->setBackgroundTag(m_backgroundTag);
  bkgMetaData->setRealTime(m_realTime);
  bkgMetaData->setFileType(m_fileType);

  // registration of detector simHits
  m_pxdSimHits.isOptional();
  m_svdSimHits.isOptional();
  m_cdcSimHits.isOptional();
  m_topSimHits.isOptional();
  m_arichSimHits.isOptional();
  m_eclSimHits.isOptional();
  m_eclHits.isOptional();
  m_klmSimHits.isOptional();

  // registration of beast simHits
  m_diaSimHits.isOptional();
  m_clw2SimHits.isOptional();
  m_clw1SimHits.isOptional();
  m_fngSimHits.isOptional();
  m_plmSimHits.isOptional();
  m_pinSimHits.isOptional();
  m_he3SimHits.isOptional();
  m_tpcSimHits.isOptional();
  m_sciSimHits.isOptional();
  m_bgoSimHits.isOptional();
  m_csiSimHits.isOptional();
}

void BeamBkgTagSetterModule::beginRun()
{
}

void BeamBkgTagSetterModule::event()
{
  int n = 0;
  if (m_phase == 2 || m_phase == 3) {
    n += setBackgroundTag(m_pxdSimHits);
    n += setBackgroundTag(m_svdSimHits);
    n += setBackgroundTag(m_cdcSimHits);
    n += setBackgroundTag(m_topSimHits);
    n += setBackgroundTag(m_arichSimHits);
    n += setBackgroundTag(m_eclSimHits);
    n += setBackgroundTag(m_eclHits);
    n += setBackgroundTag(m_klmSimHits);
  }
  // BEAST addition
  if (m_phase == 1 || m_phase == 2) {
    n += setBackgroundTag(m_diaSimHits);
    if (m_phase == 1) {
      n += setBackgroundTag(m_clw1SimHits);
      n += setBackgroundTag(m_csiSimHits);
      n += setBackgroundTag(m_bgoSimHits);
    }
    if (m_phase == 2) {
      n += setBackgroundTag(m_clw2SimHits);
      n += setBackgroundTag(m_fngSimHits);
      n += setBackgroundTag(m_plmSimHits);
    }
    n += setBackgroundTag(m_pinSimHits);
    n += setBackgroundTag(m_he3SimHits);
    n += setBackgroundTag(m_tpcSimHits);
    n += setBackgroundTag(m_sciSimHits);
  }

  if (m_fileType == BackgroundMetaData::c_ECL) n = m_eclHits.getEntries();
  if (m_fileType == BackgroundMetaData::c_PXD) n = m_pxdSimHits.getEntries();

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
