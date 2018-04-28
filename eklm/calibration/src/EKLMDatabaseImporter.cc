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

/* External headers. */
#include <TFile.h>
#include <TTree.h>

/* Belle2 headers. */
#include <eklm/calibration/EKLMDatabaseImporter.h>
#include <eklm/dataobjects/ElementNumbersSingleton.h>
#include <eklm/dbobjects/EKLMChannels.h>
#include <eklm/dbobjects/EKLMDigitizationParameters.h>
#include <eklm/dbobjects/EKLMElectronicsMap.h>
#include <eklm/dbobjects/EKLMReconstructionParameters.h>
#include <eklm/dbobjects/EKLMSimulationParameters.h>
#include <eklm/dbobjects/EKLMTimeConversion.h>
#include <eklm/geometry/AlignmentChecker.h>
#include <eklm/geometry/GeometryData.h>
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

void EKLMDatabaseImporter::importDigitizationParameters()
{
  DBImportObjPtr<EKLMDigitizationParameters> digPar;
  digPar.construct();
  GearDir dig("/Detector/DetectorComponent[@name=\"EKLM\"]/"
              "Content/DigitizationParams");
  digPar->setADCRange(dig.getInt("ADCRange"));
  digPar->setADCSamplingFrequency(dig.getDouble("ADCSamplingFrequency"));
  digPar->setNDigitizations(dig.getInt("nDigitizations"));
  digPar->setADCSaturation(dig.getDouble("ADCSaturation"));
  digPar->setNPEperMeV(dig.getDouble("nPEperMeV"));
  digPar->setMinCosTheta(cos(dig.getDouble("MaxTotalIRAngle") / 180.0 * M_PI));
  digPar->setMirrorReflectiveIndex(dig.getDouble("MirrorReflectiveIndex"));
  digPar->setScintillatorDeExcitationTime(dig.getDouble("ScintDeExTime"));
  digPar->setFiberDeExcitationTime(dig.getDouble("FiberDeExTime"));
  digPar->setFiberLightSpeed(dig.getDouble("FiberLightSpeed"));
  digPar->setAttenuationLength(dig.getDouble("AttenuationLength"));
  digPar->setPEAttenuationFrequency(dig.getDouble("PEAttenuationFreq"));
  digPar->setMeanSiPMNoise(dig.getDouble("MeanSiPMNoise"));
  digPar->setEnableConstBkg(dig.getDouble("EnableConstBkg") > 0);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  digPar.import(iov);
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
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/"
             "Content/SensitiveDetector");
  simPar->setHitTimeThreshold(
    Unit::convertValue(gd.getDouble("HitTimeThreshold") , "ns"));
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  simPar.import(iov);
}

void EKLMDatabaseImporter::loadChannelData(EKLMChannelData* channelData)
{
  m_Channels.construct();
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  int iEndcap, iLayer, iSector, iPlane, iStrip, strip;
  for (iEndcap = 1; iEndcap <= geoDat->getNEndcaps(); iEndcap++) {
    for (iLayer = 1; iLayer <= geoDat->getNDetectorLayers(iEndcap);
         iLayer++) {
      for (iSector = 1; iSector <= geoDat->getNSectors(); iSector++) {
        for (iPlane = 1; iPlane <= geoDat->getNPlanes(); iPlane++) {
          for (iStrip = 1; iStrip <= geoDat->getNStrips(); iStrip++) {
            strip = geoDat->stripNumber(iEndcap, iLayer, iSector, iPlane,
                                        iStrip);
            m_Channels->setChannelData(strip, channelData);
          }
        }
      }
    }
  }
}

void EKLMDatabaseImporter::setChannelData(
  int endcap, int layer, int sector, int plane, int strip,
  EKLMChannelData* channelData)
{
  int stripGlobal;
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  stripGlobal = elementNumbers->stripNumber(endcap, layer, sector, plane,
                                            strip);
  m_Channels->setChannelData(stripGlobal, channelData);
}

void EKLMDatabaseImporter::loadChannelDataCalibration(
  const char* calibrationData, int thresholdShift)
{
  int i, n;
  int copper, dataConcentrator, lane, asic, channel, threshold;
  int adjustmentVoltage;
  int endcap, layer, sector, plane, strip, stripGlobal;
  const int* sectorGlobal;
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  DBObjPtr<EKLMElectronicsMap> electronicsMap;
  EKLMChannelData channelData;
  EKLMDataConcentratorLane dataConcentratorLane;
  TFile* file;
  TTree* tree;
  channelData.setActive(true);
  channelData.setPedestal(0);
  channelData.setPhotoelectronAmplitude(0);
  channelData.setLookbackWindow(0);
  file = new TFile(calibrationData, "");
  tree = (TTree*)file->Get("tree");
  n = tree->GetEntries();
  tree->SetBranchAddress("copper", &copper);
  tree->SetBranchAddress("data_concentrator", &dataConcentrator);
  tree->SetBranchAddress("lane", &lane);
  tree->SetBranchAddress("asic", &asic);
  tree->SetBranchAddress("channel", &channel);
  tree->SetBranchAddress("threshold", &threshold);
  tree->SetBranchAddress("adjustment_voltage", &adjustmentVoltage);
  for (i = 0; i < n; i++) {
    tree->GetEntry(i);
    dataConcentratorLane.setCopper(copper);
    dataConcentratorLane.setDataConcentrator(dataConcentrator);
    dataConcentratorLane.setLane(lane);
    sectorGlobal = electronicsMap->getSectorByLane(&dataConcentratorLane);
    if (sectorGlobal == NULL) {
      B2FATAL("Wrong DAQ channel in calibration data: copper = " << copper <<
              ", data_concentrator = " << dataConcentrator << ", lane = " <<
              lane);
    }
    elementNumbers->sectorNumberToElementNumbers(*sectorGlobal, &endcap,
                                                 &layer, &sector);
    plane = asic / 5 + 1;
    strip = (asic % 5 * 15) + 1;
    stripGlobal = elementNumbers->stripNumber(endcap, layer, sector, plane,
                                              strip);
    channelData.setThreshold(threshold - thresholdShift);
    channelData.setAdjustmentVoltage(adjustmentVoltage);
    m_Channels->setChannelData(stripGlobal, &channelData);
  }
  delete tree;
  delete file;
}

void EKLMDatabaseImporter::importChannelData()
{
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  m_Channels.import(iov);
}

void EKLMDatabaseImporter::loadDefaultDisplacement()
{
  EKLMAlignmentData alignmentData(0., 0., 0.);
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  m_Displacement.construct();
  int iEndcap, iLayer, iSector, iPlane, iSegment, segment, sector;
  for (iEndcap = 1; iEndcap <= geoDat->getNEndcaps(); iEndcap++) {
    for (iLayer = 1; iLayer <= geoDat->getNDetectorLayers(iEndcap);
         iLayer++) {
      for (iSector = 1; iSector <= geoDat->getNSectors(); iSector++) {
        sector = geoDat->sectorNumber(iEndcap, iLayer, iSector);
        m_Displacement->setSectorAlignment(sector, &alignmentData);
        for (iPlane = 1; iPlane <= geoDat->getNPlanes(); iPlane++) {
          for (iSegment = 1; iSegment <= geoDat->getNSegments(); iSegment++) {
            segment = geoDat->segmentNumber(iEndcap, iLayer, iSector, iPlane,
                                            iSegment);
            m_Displacement->setSegmentAlignment(segment, &alignmentData);
          }
        }
      }
    }
  }
}

void EKLMDatabaseImporter::setSectorDisplacement(
  int endcap, int layer, int sector, float dx, float dy, float dalpha)
{
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  EKLMAlignmentData sectorAlignment(dx, dy, dalpha);
  EKLM::AlignmentChecker alignmentChecker(false);
  int sectorGlobal;
  sectorGlobal = geoDat->sectorNumber(endcap, layer, sector);
  if (!alignmentChecker.checkSectorAlignment(endcap, layer, sector,
                                             &sectorAlignment)) {
    B2ERROR("Incorrect displacement data (overlaps exist). "
            "The displacement is not changed");
    return;
  }
  m_Displacement->setSectorAlignment(sectorGlobal, &sectorAlignment);
}

void EKLMDatabaseImporter::setSegmentDisplacement(
  int endcap, int layer, int sector, int plane, int segment,
  float dx, float dy, float dalpha)
{
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  EKLMAlignmentData segmentAlignment(dx, dy, dalpha);
  EKLM::AlignmentChecker alignmentChecker(false);
  EKLMAlignmentData* sectorAlignment;
  int sectorGlobal, segmentGlobal;
  sectorGlobal = geoDat->sectorNumber(endcap, layer, sector);
  sectorAlignment = m_Displacement->getSectorAlignment(sectorGlobal);
  if (sectorAlignment == NULL)
    B2FATAL("Incomplete alignment data.");
  segmentGlobal = geoDat->segmentNumber(endcap, layer, sector, plane, segment);
  if (!alignmentChecker.checkSegmentAlignment(endcap, layer, sector, plane,
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

void EKLMDatabaseImporter::loadDefaultElectronicsMap()
{
  m_ElectronicsMap.construct();
}

void EKLMDatabaseImporter::addSectorLane(
  int endcap, int layer, int sector, int copper, int dataConcentrator, int lane)
{
  m_ElectronicsMap->addSectorLane(endcap, layer, sector,
                                  copper, dataConcentrator, lane);
}

void EKLMDatabaseImporter::importElectronicsMap()
{
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  m_ElectronicsMap.import(iov);
}

void EKLMDatabaseImporter::importTimeConversion()
{
  DBImportObjPtr<EKLMTimeConversion> timeConversion;
  timeConversion.construct();
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/"
             "Content/TimeConversion");
  timeConversion->setTDCFrequency(gd.getDouble("TDCFrequency"));
  timeConversion->setTimeOffset(gd.getDouble("TimeOffset"));
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  timeConversion.import(iov);
}

