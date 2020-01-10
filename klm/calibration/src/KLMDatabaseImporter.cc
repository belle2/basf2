/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Giacomo De Pietro                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/calibration/KLMDatabaseImporter.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

/* Belle 2 headers. */
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/logging/Logger.h>
#include <rawdata/dataobjects/RawCOPPERFormat.h>

/* ROOT headers. */
#include <TFile.h>
#include <TTree.h>

/* C++ headers. */
#include <string>

using namespace Belle2;

KLMDatabaseImporter::KLMDatabaseImporter() :
  m_ExperimentLow(0),
  m_RunLow(0),
  m_ExperimentHigh(-1),
  m_RunHigh(-1)
{
  m_ElementNumbers = &(KLMElementNumbers::Instance());
}

KLMDatabaseImporter::~KLMDatabaseImporter()
{
}

void KLMDatabaseImporter::setIOV(int experimentLow, int runLow,
                                 int experimentHigh, int runHigh)
{
  m_ExperimentLow = experimentLow;
  m_RunLow = runLow;
  m_ExperimentHigh = experimentHigh;
  m_RunHigh = runHigh;
}

void KLMDatabaseImporter::importChannelStatus(
  const KLMChannelStatus* channelStatus)
{
  DBImportObjPtr<KLMChannelStatus> channelStatusImport;
  channelStatusImport.construct(*channelStatus);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  channelStatusImport.import(iov);
}

void KLMDatabaseImporter::importScintillatorDigitizationParameters(
  const KLMScintillatorDigitizationParameters* digitizationParameters)
{
  DBImportObjPtr<KLMScintillatorDigitizationParameters> digPar;
  digPar.construct(*digitizationParameters);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  digPar.import(iov);
}

void KLMDatabaseImporter::importTimeConversion(
  const KLMTimeConversion* timeConversion)
{
  DBImportObjPtr<KLMTimeConversion> timeConversionImport;
  timeConversionImport.construct(*timeConversion);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  timeConversionImport.import(iov);
}

void KLMDatabaseImporter::loadStripEfficiency(
  KLMStripEfficiency* stripEfficiency, std::string fileName)
{
  TFile* file = TFile::Open(fileName.c_str(), "r");
  if (!file) {
    B2ERROR("KLMDatabaseImporter: calibration file " << fileName << " *** failed to open");
  } else {
    TTree* tree = (TTree*)file->Get("tree");
    if (!tree) {
      B2ERROR("KLMDatabaseImporter: calibration file " << fileName << " *** no tree named 'tree' found");
      file->Close();
    } else {
      int isBarrel = 0;
      tree->SetBranchAddress("isBarrel", &isBarrel);
      int section = 0;
      tree->SetBranchAddress("isForward", &section);
      int sector = 0;
      tree->SetBranchAddress("sector", &sector);
      int layer = 0;
      tree->SetBranchAddress("layer", &layer);
      int plane = 0;
      tree->SetBranchAddress("plane", &plane);
      int strip = 0;
      tree->SetBranchAddress("strip", &strip);
      float efficiency = 1.;
      tree->SetBranchAddress("efficiency", &efficiency);
      float efficiencyError = 0.;
      tree->SetBranchAddress("efficiencyError", &efficiencyError);

      for (int i = 0; i < tree->GetEntries(); i++) {
        tree->GetEntry(i);
        if (isBarrel)
          stripEfficiency->setBarrelEfficiency(section, sector, layer, plane, strip, efficiency, efficiencyError);
        else
          stripEfficiency->setEndcapEfficiency(section, sector, layer, plane, strip, efficiency, efficiencyError);
      }
    }
    file->Close();
  }
}

void KLMDatabaseImporter::importStripEfficiency(
  const KLMStripEfficiency* stripEfficiency)
{
  DBImportObjPtr<KLMStripEfficiency> stripEfficiencyImport;
  stripEfficiencyImport.construct(*stripEfficiency);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  stripEfficiencyImport.import(iov);
}

void KLMDatabaseImporter::importBKLMAlignment(
  const BKLMAlignment* bklmAlignment, bool displacement)
{
  std::string payloadName;
  if (displacement)
    payloadName = "BKLMDisplacement";
  else
    payloadName = "BKLMAlignment";
  DBImportObjPtr<BKLMAlignment> bklmAlignmentImport(payloadName);
  bklmAlignmentImport.construct(*bklmAlignment);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  bklmAlignmentImport.import(iov);
}

void KLMDatabaseImporter::importEKLMAlignment(
  const EKLMAlignment* eklmAlignment, bool displacement)
{
  std::string payloadName;
  if (displacement)
    payloadName = "EKLMDisplacement";
  else
    payloadName = "EKLMAlignment";
  DBImportObjPtr<EKLMAlignment> eklmAlignmentImport(payloadName);
  eklmAlignmentImport.construct(*eklmAlignment);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  eklmAlignmentImport.import(iov);
}

void KLMDatabaseImporter::importEKLMSegmentAlignment(
  const EKLMSegmentAlignment* eklmSegmentAlignment, bool displacement)
{
  std::string payloadName;
  if (displacement)
    payloadName = "EKLMSegmentDisplacement";
  else
    payloadName = "EKLMSegmentAlignment";
  DBImportObjPtr<EKLMSegmentAlignment> eklmSegmentAlignmentImport(payloadName);
  eklmSegmentAlignmentImport.construct(*eklmSegmentAlignment);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  eklmSegmentAlignmentImport.import(iov);
}

void KLMDatabaseImporter::importAlignment(
  const BKLMAlignment* bklmAlignment, const EKLMAlignment* eklmAlignment,
  const EKLMSegmentAlignment* eklmSegmentAlignment, bool displacement)
{
  importBKLMAlignment(bklmAlignment, displacement);
  importEKLMAlignment(eklmAlignment, displacement);
  importEKLMSegmentAlignment(eklmSegmentAlignment, displacement);
}

void KLMDatabaseImporter::loadBKLMElectronicsMap(bool isExperiment10)
{
  // Clear the vector: needed if we want to load two different maps
  // in the same steering file.
  if (m_ElectronicsChannels.size() > 0)
    m_ElectronicsChannels.clear();

  int copperId = 0;
  int slotId = 0;
  int laneId;
  int axisId = 0;
  KLMChannelIndex bklmPlanes(KLMChannelIndex::c_IndexLevelPlane);
  for (KLMChannelIndex bklmPlane = bklmPlanes.beginBKLM();
       bklmPlane != bklmPlanes.endBKLM(); ++bklmPlane) {
    int section = bklmPlane.getSection();
    int sector = bklmPlane.getSector();
    int layer = bklmPlane.getLayer();
    int plane = bklmPlane.getPlane();

    if (section == BKLMElementNumbers::c_ForwardSection) {
      if (sector == 3 || sector == 4 || sector == 5 || sector == 6)
        copperId = 1 + BKLM_ID;
      if (sector == 1 || sector == 2 || sector == 7 || sector == 8)
        copperId = 2 + BKLM_ID;
    }
    if (section == BKLMElementNumbers::c_BackwardSection) {
      if (sector == 3 || sector == 4 || sector == 5 || sector == 6)
        copperId = 3 + BKLM_ID;
      if (sector == 1 || sector == 2 || sector == 7 || sector == 8)
        copperId = 4 + BKLM_ID;
    }

    if (sector == 3 || sector == 4 || sector == 5 || sector == 6)
      slotId = sector - 2;
    if (sector == 1 || sector == 2)
      slotId = sector + 2;
    if (sector == 7 || sector == 8)
      slotId = sector - 6;

    if (layer >= BKLMElementNumbers::c_FirstRPCLayer) {
      laneId = layer + 5;
      axisId = plane;
    } else {
      laneId = layer;
      if (plane == BKLMElementNumbers::c_ZPlane)
        axisId = 1;
      if (plane == BKLMElementNumbers::c_PhiPlane)
        axisId = 0;
    }

    int MaxiChannel = BKLMElementNumbers::getNStrips(
                        section, sector, layer, plane);

    bool dontFlip = false;
    if (section == BKLMElementNumbers::c_ForwardSection &&
        (sector == 7 ||  sector == 8 || sector == 1 || sector == 2))
      dontFlip = true;
    if (section == BKLMElementNumbers::c_BackwardSection &&
        (sector == 4 ||  sector == 5 || sector == 6 || sector == 7))
      dontFlip = true;

    for (int iStrip = 1; iStrip <= MaxiChannel; iStrip++) {
      int channelId = iStrip;

      if (!(dontFlip && layer >= BKLMElementNumbers::c_FirstRPCLayer && plane == BKLMElementNumbers::c_PhiPlane))
        channelId = MaxiChannel - iStrip + 1;

      if (plane == BKLMElementNumbers::c_PhiPlane) {
        // Start settings for exp. 10.
        if (isExperiment10) {
          if (layer < BKLMElementNumbers::c_FirstRPCLayer) {
            if (sector == 1 || sector == 2 || sector == 4 || sector == 5 || sector == 6 || sector == 8) {
              channelId = MaxiChannel - channelId + 1;
              if (layer == 1)
                channelId += -2;
              if (layer == 2)
                channelId += 1;
            }
          }
        } // End settings for exp. 10.
        if (layer == 1)
          channelId += 4;
        if (layer == 2)
          channelId += 2;
      }

      if (plane == BKLMElementNumbers::c_ZPlane) {
        if (layer < BKLMElementNumbers::c_FirstRPCLayer) {
          int channelCheck = channelId;
          if (section == BKLMElementNumbers::c_BackwardSection
              && sector == BKLMElementNumbers::c_ChimneySector) {
            if (layer == 1) {
              if (!isExperiment10) {
                if (channelCheck > 0 && channelCheck < 9)
                  channelId = 9 - channelId;
                if (channelCheck > 8 && channelCheck < 24)
                  channelId = 54 - channelId;
                if (channelCheck > 23 && channelCheck < 39)
                  channelId = 54 - channelId;
              } else {
                if (channelCheck > 0 && channelCheck < 9)
                  channelId = 9 - channelId; // 8 : 1
                if (channelCheck > 8 && channelCheck < 24)
                  channelId = 39 - channelId; // 30 : 16
                if (channelCheck > 23 && channelCheck < 39)
                  channelId = 69 - channelId; // 45 : 31
              }
            }
            if (layer == 2) {
              if (channelCheck > 0 && channelCheck < 10)
                channelId = 10 - channelId;
              if (channelCheck > 9 && channelCheck < 24)
                channelId = 40 - channelId;
              if (channelCheck > 23 && channelCheck < 39)
                channelId = 69 - channelId;
            }
          } else { // All the sectors except the chimney one
            if (channelCheck > 0 && channelCheck < 10)
              channelId = 10 - channelId;
            if (channelCheck > 9 && channelCheck < 25)
              channelId = 40 - channelId;
            if (channelCheck > 24 && channelCheck < 40)
              channelId = 70 - channelId;
            if (channelCheck > 39 && channelCheck < 55)
              channelId = 100 - channelId;
          }
        }
      }

      uint16_t detectorChannel = m_ElementNumbers->channelNumberBKLM(
                                   section, sector, layer, plane, iStrip);
      m_ElectronicsChannels.push_back(
        std::pair<uint16_t, KLMElectronicsChannel>(
          detectorChannel,
          KLMElectronicsChannel(copperId, slotId, laneId, axisId, channelId)));
    }
  }
}

void KLMDatabaseImporter::setElectronicsMapLane(
  int subdetector, int section, int sector, int layer, int lane)
{
  int channelSubdetector, channelSection, channelSector, channelLayer;
  int plane, strip;
  unsigned int n = m_ElectronicsChannels.size();
  for (unsigned int i = 0; i < n; ++i) {
    uint16_t channel = m_ElectronicsChannels[i].first;
    m_ElementNumbers->channelNumberToElementNumbers(
      channel, &channelSubdetector, &channelSection, &channelSector,
      &channelLayer, &plane, &strip);
    if ((channelSubdetector == subdetector) &&
        (channelSection == section) &&
        (channelSector == sector) &&
        (channelLayer == layer))
      m_ElectronicsChannels[i].second.setLane(lane);
  }
}

void KLMDatabaseImporter::setElectronicsMapLane(
  int subdetector, int section, int sector, int layer, int plane, int lane)
{
  int channelSubdetector, channelSection, channelSector, channelLayer;
  int channelPlane, strip;
  unsigned int n = m_ElectronicsChannels.size();
  for (unsigned int i = 0; i < n; ++i) {
    uint16_t channel = m_ElectronicsChannels[i].first;
    m_ElementNumbers->channelNumberToElementNumbers(
      channel, &channelSubdetector, &channelSection, &channelSector,
      &channelLayer, &channelPlane, &strip);
    if ((channelSubdetector == subdetector) &&
        (channelSection == section) &&
        (channelSector == sector) &&
        (channelLayer == layer) &&
        (channelPlane == plane))
      m_ElectronicsChannels[i].second.setLane(lane);
  }
}

void KLMDatabaseImporter::importElectronicsMap()
{
  DBImportObjPtr<KLMElectronicsMap> electronicsMap;
  electronicsMap.construct();
  unsigned int n = m_ElectronicsChannels.size();
  for (unsigned int i = 0; i < n; ++i) {
    electronicsMap->addChannel(
      m_ElectronicsChannels[i].first,
      m_ElectronicsChannels[i].second.getCopper(),
      m_ElectronicsChannels[i].second.getSlot(),
      m_ElectronicsChannels[i].second.getLane(),
      m_ElectronicsChannels[i].second.getAxis(),
      m_ElectronicsChannels[i].second.getChannel());
  }
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  electronicsMap.import(iov);
}
