/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui Guan, Vipin Gaur,                                *
 *               Zachary S. Stottler, Giacomo De Pietro                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/bklm/calibration/BKLMDatabaseImporter.h>

/* KLM headers. */
#include <klm/dbobjects/bklm/BKLMGeometryPar.h>
#include <klm/dbobjects/bklm/BKLMSimulationPar.h>

/* Belle 2 headers. */
#include <framework/database/Database.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/gearbox/GearDir.h>

using namespace Belle2;

BKLMDatabaseImporter::BKLMDatabaseImporter()
{
}

void BKLMDatabaseImporter::setIOV(int experimentLow, int runLow,
                                  int experimentHigh, int runHigh)
{
  m_ExperimentLow = experimentLow;
  m_RunLow = runLow;
  m_ExperimentHigh = experimentHigh;
  m_RunHigh = runHigh;
}

void BKLMDatabaseImporter::importGeometryPar()
{
  GearDir content(Gearbox::getInstance().getDetectorComponent("KLM"));
  BKLMGeometryPar bklmGeometryPar(content);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  Database::Instance().storeData("BKLMGeometryPar", &bklmGeometryPar, iov);

}

void BKLMDatabaseImporter::importSimulationPar()
{
  GearDir content(Gearbox::getInstance().getDetectorComponent("KLM"), "BKLM/SimulationParameters");
  BKLMSimulationPar bklmSimulationPar(content);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  Database::Instance().storeData("BKLMSimulationPar", &bklmSimulationPar, iov);
}

void BKLMDatabaseImporter::importADCThreshold(BKLMADCThreshold* inputThreshold)
{
  DBImportObjPtr<BKLMADCThreshold> adcThreshold;
  adcThreshold.construct(*inputThreshold);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  adcThreshold.import(iov);
}
