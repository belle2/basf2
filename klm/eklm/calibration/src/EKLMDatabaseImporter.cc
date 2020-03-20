/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/calibration/EKLMDatabaseImporter.h>

/* KLM headers. */
#include <klm/dbobjects/eklm/EKLMReconstructionParameters.h>
#include <klm/dbobjects/eklm/EKLMSimulationParameters.h>

/* Belle 2 headers. */
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;

EKLMDatabaseImporter::EKLMDatabaseImporter()
{
  m_ExperimentLow = 0;
  m_RunLow = 0;
  m_ExperimentHigh = -1;
  m_RunHigh = -1;
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

void EKLMDatabaseImporter::importReconstructionParameters()
{
  DBImportObjPtr<EKLMReconstructionParameters> recPar;
  recPar.construct();
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/"
             "Content/DigitizationParams");
  recPar->setTimeResolution(gd.getDouble("TimeResolution"));
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  recPar.import(iov);
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

void EKLMDatabaseImporter::importElectronicsMap(
  const EKLMElectronicsMap* electronicsMap)
{
  DBImportObjPtr<EKLMElectronicsMap> electronicsMapImport;
  electronicsMapImport.construct(*electronicsMap);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  electronicsMapImport.import(iov);
}
