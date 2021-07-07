/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/calibration/KLMElectronicsMapImporter.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>
#include <klm/dbobjects/KLMElectronicsMap.h>

/* Belle 2 headers. */
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/logging/Logger.h>
#include <rawdata/dataobjects/RawCOPPERFormat.h>

using namespace Belle2;

KLMElectronicsMapImporter::KLMElectronicsMapImporter() :
  m_ElementNumbers(&(KLMElementNumbers::Instance()))
{
}

KLMElectronicsMapImporter::~KLMElectronicsMapImporter()
{
}

void KLMElectronicsMapImporter::setIOV(int experimentLow, int runLow,
                                       int experimentHigh, int runHigh)
{
  m_ExperimentLow = experimentLow;
  m_RunLow = runLow;
  m_ExperimentHigh = experimentHigh;
  m_RunHigh = runHigh;
}

void KLMElectronicsMapImporter::clearElectronicsMap()
{
  m_ChannelMap.clear();
}

void KLMElectronicsMapImporter::loadBKLMElectronicsMap(int version)
{
  const int minimalVersion = 1;
  const int maximalVersion = 2;
  if (version < minimalVersion || version > maximalVersion) {
    B2FATAL("Incorrect version (" << version << ") of BKLM electronics map. "
            "It must be from " << minimalVersion << " to " << maximalVersion);
  }
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
              if (version == 1) {
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

      KLMChannelNumber detectorChannel = m_ElementNumbers->channelNumberBKLM(
                                           section, sector, layer, plane, iStrip);
      m_ChannelMap.insert(
        std::pair<KLMChannelNumber, KLMElectronicsChannel>(
          detectorChannel,
          KLMElectronicsChannel(copperId, slotId, laneId, axisId, channelId)));
    }
  }
}

int KLMElectronicsMapImporter::getEKLMStripFirmwareBySoftware(int stripSoftware) const
{
  int segment, strip;
  /* Segment is 0-based here. */
  segment = (stripSoftware - 1) / EKLMElementNumbers::getNStripsSegment();
  /* Order of segment readout boards in the firmware is opposite. */
  segment = 4 - segment;
  strip = segment * EKLMElementNumbers::getNStripsSegment() +
          (stripSoftware - 1) % EKLMElementNumbers::getNStripsSegment() + 1;
  return strip;
}

void KLMElectronicsMapImporter::addEKLMLane(
  int section, int sector, int layer, int copper, int slot, int lane)
{
  for (int plane = 1; plane <= EKLMElementNumbers::getMaximalPlaneNumber(); ++plane) {
    for (int strip = 1; strip <= EKLMElementNumbers::getMaximalStripNumber(); ++strip) {
      int axis = plane - 1;
      int channel = getEKLMStripFirmwareBySoftware(strip);
      KLMChannelNumber detectorChannel = m_ElementNumbers->channelNumberEKLM(
                                           section, sector, layer, plane, strip);
      m_ChannelMap.insert(
        std::pair<KLMChannelNumber, KLMElectronicsChannel>(
          detectorChannel,
          KLMElectronicsChannel(EKLM_ID + copper, slot, lane, axis, channel)));
    }
  }
}

void KLMElectronicsMapImporter::setChannelsEKLMSegment(
  int section, int sector, int layer, int plane, int segment,
  int firmwareSegment)
{
  std::map<KLMChannelNumber, KLMElectronicsChannel>:: iterator it;
  for (int strip = 1; strip <= EKLMElementNumbers::getNStripsSegment(); ++strip) {
    int stripPlane = strip + EKLMElementNumbers::getNStripsSegment() * (segment - 1);
    KLMChannelNumber detectorChannel = m_ElementNumbers->channelNumberEKLM(
                                         section, sector, layer, plane, stripPlane);
    it = m_ChannelMap.find(detectorChannel);
    if (it == m_ChannelMap.end())
      B2FATAL("The KLM electronics map is not loaded or incomplete.");
    int channel = (firmwareSegment - 1) * EKLMElementNumbers::getNStripsSegment() +
                  strip;
    it->second.setChannel(channel);
  }
}

void KLMElectronicsMapImporter::loadEKLMElectronicsMap(int version, bool mc)
{
  const int minimalVersion = 1;
  const int maximalVersion = 2;
  if (version < minimalVersion || version > maximalVersion) {
    B2FATAL("Incorrect version (" << version << ") of EKLM electronics map. "
            "It must be from " << minimalVersion << " to " << maximalVersion);
  }
  /* Backward section. */
  /* Sector 1. */
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 1, 1, 3, 1, 1);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 1, 2, 3, 1, 2);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 1, 3, 3, 1, 3);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 1, 4, 3, 1, 4);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 1, 5, 3, 1, 5);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 1, 6, 3, 1, 6);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 1, 7, 3, 2, 1);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 1, 8, 3, 2, 2);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 1, 9, 3, 2, 3);
  /* Wrong connection was fixed between phase 2 and phase 3. */
  if (mc || (version >= 2)) {
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 1, 10, 3, 2, 4);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 1, 11, 3, 2, 5);
  } else {
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 1, 10, 3, 2, 5);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 1, 11, 3, 2, 4);
  }
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 1, 12, 3, 2, 6);
  /* Sector 2. */
  if (version == 1) {
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 1, 3, 3, 1);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 2, 3, 3, 2);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 3, 3, 3, 3);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 4, 3, 3, 4);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 5, 3, 3, 5);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 6, 3, 3, 6);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 7, 3, 4, 1);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 8, 3, 4, 2);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 9, 3, 4, 3);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 10, 3, 4, 4);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 11, 3, 4, 5);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 12, 3, 4, 6);
  } else {
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 1, 3, 4, 6);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 2, 3, 4, 5);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 3, 3, 4, 4);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 4, 3, 4, 3);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 5, 3, 4, 2);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 6, 3, 4, 1);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 7, 3, 3, 6);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 8, 3, 3, 5);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 9, 3, 3, 4);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 10, 3, 3, 3);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 11, 3, 3, 2);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 2, 12, 3, 3, 1);
  }
  /* Sector 3. */
  if (version == 1) {
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 1, 4, 1, 1);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 2, 4, 1, 2);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 3, 4, 1, 3);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 4, 4, 1, 4);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 5, 4, 1, 5);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 6, 4, 1, 6);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 7, 4, 2, 1);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 8, 4, 2, 2);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 9, 4, 2, 3);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 10, 4, 2, 4);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 11, 4, 2, 5);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 12, 4, 2, 6);
  } else {
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 1, 4, 2, 6);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 2, 4, 2, 5);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 3, 4, 2, 4);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 4, 4, 2, 3);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 5, 4, 2, 2);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 6, 4, 2, 1);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 7, 4, 1, 6);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 8, 4, 1, 5);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 9, 4, 1, 4);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 10, 4, 1, 3);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 11, 4, 1, 2);
    addEKLMLane(EKLMElementNumbers::c_BackwardSection, 3, 12, 4, 1, 1);
  }
  /* Sector 4. */
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 4, 1, 4, 3, 1);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 4, 2, 4, 3, 2);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 4, 3, 4, 3, 3);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 4, 4, 4, 3, 4);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 4, 5, 4, 3, 5);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 4, 6, 4, 3, 6);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 4, 7, 4, 4, 1);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 4, 8, 4, 4, 2);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 4, 9, 4, 4, 3);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 4, 10, 4, 4, 4);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 4, 11, 4, 4, 5);
  addEKLMLane(EKLMElementNumbers::c_BackwardSection, 4, 12, 4, 4, 6);
  /* Forward section. */
  /* Sector 1. */
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 1, 1, 1, 3, 1);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 1, 2, 1, 3, 2);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 1, 3, 1, 3, 3);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 1, 4, 1, 3, 4);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 1, 5, 1, 3, 5);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 1, 6, 1, 3, 6);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 1, 7, 1, 3, 7);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 1, 8, 1, 4, 1);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 1, 9, 1, 4, 2);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 1, 10, 1, 4, 3);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 1, 11, 1, 4, 4);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 1, 12, 1, 4, 5);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 1, 13, 1, 4, 6);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 1, 14, 1, 4, 7);
  /* Sector 2. */
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 2, 1, 1, 1, 1);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 2, 2, 1, 1, 2);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 2, 3, 1, 1, 3);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 2, 4, 1, 1, 4);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 2, 5, 1, 1, 5);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 2, 6, 1, 1, 6);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 2, 7, 1, 1, 7);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 2, 8, 1, 2, 1);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 2, 9, 1, 2, 2);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 2, 10, 1, 2, 3);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 2, 11, 1, 2, 4);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 2, 12, 1, 2, 5);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 2, 13, 1, 2, 6);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 2, 14, 1, 2, 7);
  /* Sector 3. */
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 3, 1, 2, 3, 1);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 3, 2, 2, 3, 2);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 3, 3, 2, 3, 3);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 3, 4, 2, 3, 4);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 3, 5, 2, 3, 5);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 3, 6, 2, 3, 6);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 3, 7, 2, 3, 7);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 3, 8, 2, 4, 1);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 3, 9, 2, 4, 2);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 3, 10, 2, 4, 3);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 3, 11, 2, 4, 4);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 3, 12, 2, 4, 5);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 3, 13, 2, 4, 6);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 3, 14, 2, 4, 7);
  /* Sector 4. */
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 4, 1, 2, 1, 1);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 4, 2, 2, 1, 2);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 4, 3, 2, 1, 3);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 4, 4, 2, 1, 4);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 4, 5, 2, 1, 5);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 4, 6, 2, 1, 6);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 4, 7, 2, 1, 7);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 4, 8, 2, 2, 1);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 4, 9, 2, 2, 2);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 4, 10, 2, 2, 3);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 4, 11, 2, 2, 4);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 4, 12, 2, 2, 5);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 4, 13, 2, 2, 6);
  addEKLMLane(EKLMElementNumbers::c_ForwardSection, 4, 14, 2, 2, 7);
  /* Switch of internal cables. */
  if (!mc) {
    setChannelsEKLMSegment(1, 1, 5, 1, 1, 4);
    setChannelsEKLMSegment(1, 1, 5, 1, 2, 5);
    setChannelsEKLMSegment(1, 1, 5, 2, 1, 4);
    setChannelsEKLMSegment(1, 1, 5, 2, 2, 5);
    setChannelsEKLMSegment(2, 1, 10, 2, 3, 2);
    setChannelsEKLMSegment(2, 1, 10, 2, 4, 3);
  }
}

void KLMElectronicsMapImporter::setLane(
  int subdetector, int section, int sector, int layer, int lane)
{
  std::map<KLMChannelNumber, KLMElectronicsChannel>::iterator it;
  int minimalPlane = m_ElementNumbers->getMinimalPlaneNumber(subdetector);
  KLMChannelIndex klmChannel(subdetector, section, sector, layer, minimalPlane, 1);
  KLMChannelIndex klmModule(klmChannel);
  klmModule.setIndexLevel(KLMChannelIndex::c_IndexLevelLayer);
  KLMChannelIndex klmNextModule(klmModule);
  ++klmNextModule;
  for (; klmChannel != klmNextModule; ++klmChannel) {
    KLMChannelNumber channel = klmChannel.getKLMChannelNumber();
    it = m_ChannelMap.find(channel);
    if (it == m_ChannelMap.end())
      B2FATAL("The KLM electronics map is not loaded or incomplete.");
    it->second.setLane(lane);
  }
}

void KLMElectronicsMapImporter::setLane(
  int subdetector, int section, int sector, int layer, int plane, int lane)
{
  std::map<KLMChannelNumber, KLMElectronicsChannel>::iterator it;
  KLMChannelIndex klmChannel(subdetector, section, sector, layer, plane, 1);
  KLMChannelIndex klmPlane(klmChannel);
  klmPlane.setIndexLevel(KLMChannelIndex::c_IndexLevelPlane);
  KLMChannelIndex klmNextPlane(klmPlane);
  ++klmNextPlane;
  for (; klmChannel != klmNextPlane; ++klmChannel) {
    KLMChannelNumber channel = klmChannel.getKLMChannelNumber();
    it = m_ChannelMap.find(channel);
    if (it == m_ChannelMap.end())
      B2FATAL("The KLM electronics map is not loaded or incomplete.");
    it->second.setLane(lane);
  }
}

void KLMElectronicsMapImporter::importElectronicsMap()
{
  DBImportObjPtr<KLMElectronicsMap> electronicsMap;
  electronicsMap.construct();
  std::map<KLMChannelNumber, KLMElectronicsChannel>::iterator it;
  for (it = m_ChannelMap.begin(); it != m_ChannelMap.end(); ++it) {
    electronicsMap->addChannel(
      it->first,
      it->second.getCopper(),
      it->second.getSlot(),
      it->second.getLane(),
      it->second.getAxis(),
      it->second.getChannel());
  }
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  electronicsMap.import(iov);
}
