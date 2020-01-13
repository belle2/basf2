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

int KLMDatabaseImporter::getEKLMStripFirmwareBySoftware(int stripSoftware) const
{
  int segment, strip;
  segment = (stripSoftware - 1) / EKLMElementNumbers::getNStripsSegment();
  /* Order of segment readout boards in the firmware is opposite. */
  segment = 4 - segment;
  strip = segment * EKLMElementNumbers::getNStripsSegment() +
          (stripSoftware - 1) % EKLMElementNumbers::getNStripsSegment() + 1;
  return strip;
}

void KLMDatabaseImporter::getEKLMAsicChannel(
  int plane, int strip, int* asic, int* channel) const
{
  int stripFirmware = getEKLMStripFirmwareBySoftware(strip);
  int asicMod5 = (stripFirmware - 1) / EKLMElementNumbers::getNStripsSegment();
  *channel = (stripFirmware - 1) % EKLMElementNumbers::getNStripsSegment();
  *asic = asicMod5 +
          EKLMElementNumbers::getMaximalSegmentNumber() * (plane - 1);
}

void KLMDatabaseImporter::addEKLMElectronicsMapLane(
  int section, int sector, int layer, int copper, int slot, int axis)
{
}

void KLMDatabaseImporter::loadEKLMElectronicsMap(int version, bool mc)
{
  const int minimalVersion = 1;
  const int maximalVersion = 2;
  if (version < minimalVersion || version > maximalVersion) {
    B2FATAL("Incorrect version (" << version << ") of EKLM electronics map. "
            "It must be from " << minimalVersion << " to " << maximalVersion);
  }
  /* Backward section. */
  /* Sector 1. */
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 1, 1, 3, 0, 1);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 1, 2, 3, 0, 2);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 1, 3, 3, 0, 3);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 1, 4, 3, 0, 4);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 1, 5, 3, 0, 5);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 1, 6, 3, 0, 6);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 1, 7, 3, 1, 1);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 1, 8, 3, 1, 2);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 1, 9, 3, 1, 3);
  /* Wrong connection was fixed between phase 2 and phase 3. */
  if (mc || (version >= 2)) {
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 1, 10, 3, 1, 4);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 1, 11, 3, 1, 5);
  } else {
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 1, 10, 3, 1, 5);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 1, 11, 3, 1, 4);
  }
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 1, 12, 3, 1, 6);
  /* Sector 2. */
  if (version == 1) {
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 1, 3, 2, 1);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 2, 3, 2, 2);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 3, 3, 2, 3);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 4, 3, 2, 4);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 5, 3, 2, 5);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 6, 3, 2, 6);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 7, 3, 3, 1);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 8, 3, 3, 2);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 9, 3, 3, 3);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 10, 3, 3, 4);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 11, 3, 3, 5);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 12, 3, 3, 6);
  } else {
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 1, 3, 3, 6);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 2, 3, 3, 5);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 3, 3, 3, 4);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 4, 3, 3, 3);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 5, 3, 3, 2);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 6, 3, 3, 1);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 7, 3, 2, 6);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 8, 3, 2, 5);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 9, 3, 2, 4);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 10, 3, 2, 3);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 11, 3, 2, 2);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 2, 12, 3, 2, 1);
  }
  /* Sector 3. */
  if (version == 1) {
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 1, 4, 0, 1);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 2, 4, 0, 2);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 3, 4, 0, 3);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 4, 4, 0, 4);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 5, 4, 0, 5);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 6, 4, 0, 6);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 7, 4, 1, 1);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 8, 4, 1, 2);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 9, 4, 1, 3);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 10, 4, 1, 4);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 11, 4, 1, 5);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 12, 4, 1, 6);
  } else {
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 1, 4, 1, 6);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 2, 4, 1, 5);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 3, 4, 1, 4);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 4, 4, 1, 3);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 5, 4, 1, 2);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 6, 4, 1, 1);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 7, 4, 0, 6);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 8, 4, 0, 5);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 9, 4, 0, 4);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 10, 4, 0, 3);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 11, 4, 0, 2);
    addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 3, 12, 4, 0, 1);
  }
  /* Sector 4. */
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 4, 1, 4, 2, 1);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 4, 2, 4, 2, 2);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 4, 3, 4, 2, 3);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 4, 4, 4, 2, 4);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 4, 5, 4, 2, 5);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 4, 6, 4, 2, 6);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 4, 7, 4, 3, 1);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 4, 8, 4, 3, 2);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 4, 9, 4, 3, 3);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 4, 10, 4, 3, 4);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 4, 11, 4, 3, 5);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_BackwardSection, 4, 12, 4, 3, 6);
  /* Forward section. */
  /* Sector 1. */
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 1, 1, 1, 2, 1);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 1, 2, 1, 2, 2);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 1, 3, 1, 2, 3);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 1, 4, 1, 2, 4);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 1, 5, 1, 2, 5);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 1, 6, 1, 2, 6);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 1, 7, 1, 2, 7);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 1, 8, 1, 3, 1);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 1, 9, 1, 3, 2);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 1, 10, 1, 3, 3);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 1, 11, 1, 3, 4);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 1, 12, 1, 3, 5);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 1, 13, 1, 3, 6);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 1, 14, 1, 3, 7);
  /* Sector 2. */
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 2, 1, 1, 0, 1);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 2, 2, 1, 0, 2);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 2, 3, 1, 0, 3);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 2, 4, 1, 0, 4);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 2, 5, 1, 0, 5);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 2, 6, 1, 0, 6);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 2, 7, 1, 0, 7);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 2, 8, 1, 1, 1);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 2, 9, 1, 1, 2);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 2, 10, 1, 1, 3);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 2, 11, 1, 1, 4);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 2, 12, 1, 1, 5);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 2, 13, 1, 1, 6);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 2, 14, 1, 1, 7);
  /* Sector 3. */
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 3, 1, 2, 2, 1);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 3, 2, 2, 2, 2);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 3, 3, 2, 2, 3);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 3, 4, 2, 2, 4);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 3, 5, 2, 2, 5);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 3, 6, 2, 2, 6);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 3, 7, 2, 2, 7);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 3, 8, 2, 3, 1);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 3, 9, 2, 3, 2);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 3, 10, 2, 3, 3);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 3, 11, 2, 3, 4);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 3, 12, 2, 3, 5);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 3, 13, 2, 3, 6);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 3, 14, 2, 3, 7);
  /* Sector 4. */
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 4, 1, 2, 0, 1);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 4, 2, 2, 0, 2);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 4, 3, 2, 0, 3);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 4, 4, 2, 0, 4);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 4, 5, 2, 0, 5);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 4, 6, 2, 0, 6);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 4, 7, 2, 0, 7);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 4, 8, 2, 1, 1);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 4, 9, 2, 1, 2);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 4, 10, 2, 1, 3);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 4, 11, 2, 1, 4);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 4, 12, 2, 1, 5);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 4, 13, 2, 1, 6);
  addEKLMElectronicsMapLane(EKLMElementNumbers::c_ForwardSection, 4, 14, 2, 1, 7);

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
