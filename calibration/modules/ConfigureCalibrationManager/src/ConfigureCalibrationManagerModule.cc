/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <calibration/modules/ConfigureCalibrationManager/ConfigureCalibrationManagerModule.h>
#include <calibration/CalibrationManager.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <boost/iostreams/detail/fstream.hpp>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ConfigureCalibrationManager)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ConfigureCalibrationManagerModule::ConfigureCalibrationManagerModule() : Module()
{
  // Set module properties
  setDescription("Internal module to to configure Calibration Manager");

  // Parameter definitions
  addParam("rangeExpMin", m_rangeExpMin, "Range: min Experiment", int(0));
  addParam("rangeRunMin", m_rangeRunMin, "Range: min Run", int(0));
  addParam("rangeExpMax", m_rangeExpMax, "Range: max Experiment", int(0));
  addParam("rangeRunMax", m_rangeRunMax, "Range: max Run", int(0));
  addParam("stateFile",   m_stateFile,   "Name of file with calibration state cache", string("calibration_cache.txt"));

  addParam("doCollection", m_doCollection, "Perform data collection", bool(true));
  addParam("doCalibration", m_doCalibration, "Perform calibration", bool(true));


}

void ConfigureCalibrationManagerModule::initialize()
{
  IntervalOfValidity iov(m_rangeExpMin, m_rangeRunMin, m_rangeExpMax, m_rangeRunMax);
  calibration::CalibrationManager::getInstance().setRange(iov);
  calibration::CalibrationManager::getInstance().setStateFileName(m_stateFile);
  calibration::CalibrationManager::getInstance().setJobType(m_doCollection, m_doCalibration);
}

void ConfigureCalibrationManagerModule::terminate()
{
  //calibration::CalibrationManager::getInstance().saveCachedStates();
  if (calibration::CalibrationManager::getInstance().done()) {
    B2RESULT("Calibration done.");
    B2INFO("Writing result file...");
    ofstream result("calibration_result.txt");
    result << "Calibration done." << endl;
    result.close();
  }
}

void ConfigureCalibrationManagerModule::beginRun()
{
  StoreObjPtr<EventMetaData> emd;
  if (calibration::CalibrationManager::getInstance().done()) {
    B2INFO("Calibration done. Stopping processing of events...");
    emd->setEndOfData();
  }
}
