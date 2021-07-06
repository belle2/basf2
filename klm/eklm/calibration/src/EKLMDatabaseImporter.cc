/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/calibration/EKLMDatabaseImporter.h>

/* KLM headers. */
#include <klm/dbobjects/eklm/EKLMSimulationParameters.h>

/* Belle 2 headers. */
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;

EKLMDatabaseImporter::EKLMDatabaseImporter()
{
}

EKLMDatabaseImporter::~EKLMDatabaseImporter()
{
}

void EKLMDatabaseImporter::setIOV(int experimentLow, int runLow,
                                  int experimentHigh, int runHigh)
{
  m_ExperimentLow = experimentLow;
  m_RunLow = runLow;
  m_ExperimentHigh = experimentHigh;
  m_RunHigh = runHigh;
}

void EKLMDatabaseImporter::importSimulationParameters()
{
  DBImportObjPtr<EKLMSimulationParameters> simPar;
  simPar.construct();
  GearDir gd(Gearbox::getInstance().getDetectorComponent("KLM"));
  gd.append("/EKLM/SensitiveDetector");
  simPar->setHitTimeThreshold(
    Unit::convertValue(gd.getDouble("HitTimeThreshold") , "ns"));
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  simPar.import(iov);
}
