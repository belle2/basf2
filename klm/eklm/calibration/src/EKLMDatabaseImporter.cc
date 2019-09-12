/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* C++ headers. */
#include <cmath>

/* Belle2 headers. */
#include <klm/eklm/calibration/EKLMDatabaseImporter.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>
#include <klm/eklm/dbobjects/EKLMReconstructionParameters.h>
#include <klm/eklm/dbobjects/EKLMSimulationParameters.h>
#include <klm/eklm/geometry/AlignmentChecker.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMDatabaseImporter::EKLMDatabaseImporter() :
  m_Displacement("EKLMDisplacement")
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

void EKLMDatabaseImporter::loadDefaultDisplacement()
{
  EKLMAlignmentData alignmentData(0., 0., 0.);
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  m_Displacement.construct();
  int iSection, iLayer, iSector, iPlane, iSegment, segment, sector;
  for (iSection = 1; iSection <= geoDat->getNSections(); iSection++) {
    for (iLayer = 1; iLayer <= geoDat->getNDetectorLayers(iSection);
         iLayer++) {
      for (iSector = 1; iSector <= geoDat->getNSectors(); iSector++) {
        sector = geoDat->sectorNumber(iSection, iLayer, iSector);
        m_Displacement->setSectorAlignment(sector, &alignmentData);
        for (iPlane = 1; iPlane <= geoDat->getNPlanes(); iPlane++) {
          for (iSegment = 1; iSegment <= geoDat->getNSegments(); iSegment++) {
            segment = geoDat->segmentNumber(iSection, iLayer, iSector, iPlane,
                                            iSegment);
            m_Displacement->setSegmentAlignment(segment, &alignmentData);
          }
        }
      }
    }
  }
}

void EKLMDatabaseImporter::setSectorDisplacement(
  int section, int layer, int sector, float dx, float dy, float dalpha)
{
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  EKLMAlignmentData sectorAlignment(dx, dy, dalpha);
  EKLM::AlignmentChecker alignmentChecker(false);
  int sectorGlobal;
  sectorGlobal = geoDat->sectorNumber(section, layer, sector);
  if (!alignmentChecker.checkSectorAlignment(section, layer, sector,
                                             &sectorAlignment)) {
    B2ERROR("Incorrect displacement data (overlaps exist). "
            "The displacement is not changed");
    return;
  }
  m_Displacement->setSectorAlignment(sectorGlobal, &sectorAlignment);
}

void EKLMDatabaseImporter::setSegmentDisplacement(
  int section, int layer, int sector, int plane, int segment,
  float dx, float dy, float dalpha)
{
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  EKLMAlignmentData segmentAlignment(dx, dy, dalpha);
  EKLM::AlignmentChecker alignmentChecker(false);
  const EKLMAlignmentData* sectorAlignment;
  int sectorGlobal, segmentGlobal;
  sectorGlobal = geoDat->sectorNumber(section, layer, sector);
  sectorAlignment = m_Displacement->getSectorAlignment(sectorGlobal);
  if (sectorAlignment == nullptr)
    B2FATAL("Incomplete alignment data.");
  segmentGlobal = geoDat->segmentNumber(section, layer, sector, plane, segment);
  if (!alignmentChecker.checkSegmentAlignment(section, layer, sector, plane,
                                              segment, sectorAlignment,
                                              &segmentAlignment, false)) {
    B2ERROR("Incorrect displacement data (overlaps exist). "
            "The displacement is not changed");
    return;
  }
  m_Displacement->setSegmentAlignment(segmentGlobal, &segmentAlignment);
}

void EKLMDatabaseImporter::importDisplacement()
{
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  m_Displacement.import(iov);
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
