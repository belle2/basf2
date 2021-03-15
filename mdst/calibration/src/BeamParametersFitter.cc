/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <mdst/calibration/BeamParametersFitter.h>

/* Belle 2 headers. */
#include <framework/database/Database.h>
#include <framework/database/DBStore.h>

using namespace Belle2;

void BeamParametersFitter::setupDatabase()
{
  /* DataStore. */
  DataStore::Instance().setInitializeActive(true);
  StoreObjPtr<EventMetaData> eventMetaData;
  eventMetaData.registerInDataStore();
  DataStore::Instance().setInitializeActive(false);
  /* Database. */
  if (eventMetaData.isValid()) {
    eventMetaData->setExperiment(m_IntervalOfValidity.getExperimentLow());
    eventMetaData->setRun(m_IntervalOfValidity.getRunLow());
  } else {
    eventMetaData.construct(1, m_IntervalOfValidity.getRunLow(),
                            m_IntervalOfValidity.getExperimentLow());
  }
  DBStore& dbStore = DBStore::Instance();
  dbStore.update();
  dbStore.updateEvent();
}

void BeamParametersFitter::fit()
{
  setupDatabase();
}
