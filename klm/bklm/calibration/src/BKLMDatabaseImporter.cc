/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/bklm/calibration/BKLMDatabaseImporter.h>

/* KLM headers. */
#include <klm/dbobjects/bklm/BKLMGeometryPar.h>
#include <klm/dbobjects/bklm/BKLMSimulationPar.h>

/* Basf2 headers. */
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

void BKLMDatabaseImporter::importSimulationPar()
{
  GearDir content(Gearbox::getInstance().getDetectorComponent("KLM"), "BKLM/SimulationParameters");
  BKLMSimulationPar bklmSimulationPar(content);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  Database::Instance().storeData("BKLMSimulationPar", &bklmSimulationPar, iov);
}
