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
#include <eklm/calibration/EKLMDatabaseImporter.h>
#include <eklm/dbobjects/EKLMChannels.h>
#include <eklm/dbobjects/EKLMDigitizationParameters.h>
#include <eklm/dbobjects/EKLMReconstructionParameters.h>
#include <eklm/dbobjects/EKLMSimulationParameters.h>
#include <eklm/geometry/GeometryData.h>
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

void EKLMDatabaseImporter::importDigitizationParameters()
{
  DBImportObjPtr<EKLMDigitizationParameters> digPar;
  digPar.construct();
  GearDir dig("/Detector/DetectorComponent[@name=\"EKLM\"]/"
              "Content/DigitizationParams");
  digPar->setADCRange(dig.getInt("ADCRange"));
  digPar->setADCSamplingTime(dig.getDouble("ADCSamplingTime"));
  digPar->setNDigitizations(dig.getInt("nDigitizations"));
  digPar->setADCPedestal(dig.getDouble("ADCPedestal"));
  digPar->setADCPEAmplitude(dig.getDouble("ADCPEAmplitude"));
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
  IntervalOfValidity iov(0, 0, -1, -1);
  digPar.import(iov);
}

void EKLMDatabaseImporter::importReconstructionParameters()
{
  DBImportObjPtr<EKLMReconstructionParameters> recPar;
  recPar.construct();
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/"
             "Content/DigitizationParams");
  recPar->setTimeResolution(gd.getDouble("TimeResolution"));
  IntervalOfValidity iov(0, 0, -1, -1);
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
  IntervalOfValidity iov(0, 0, -1, -1);
  simPar.import(iov);
}

void EKLMDatabaseImporter::loadDefaultChannelData()
{
  EKLMChannelData channelData;
  m_Channels.construct();
  channelData.setActive(true);
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
            m_Channels->setChannelData(strip, &channelData);
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
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  stripGlobal = geoDat->stripNumber(endcap, layer, sector, plane, strip);
  m_Channels->setChannelData(stripGlobal, channelData);
}

void EKLMDatabaseImporter::importChannelData()
{
  IntervalOfValidity iov(0, 0, -1, -1);
  m_Channels.import(iov);
}

