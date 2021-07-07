/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>

using namespace Belle2;

CDCTriggerSegmentHit::CDCTriggerSegmentHit(unsigned short segmentID,
                                           unsigned short priorityPosition,
                                           unsigned short leftRight,
                                           short priorityTime,
                                           short fastestTime,
                                           short foundTime,
                                           short quadrant):
  m_segmentID(segmentID), m_priorityPosition(priorityPosition), m_leftRight(leftRight),
  m_priorityTime(priorityTime), m_fastestTime(fastestTime), m_foundTime(foundTime), m_quadrant(quadrant)
{
  // find super layer and local wire id
  std::array<int, 9> nWiresInSuperLayer = {
    160, 160, 192, 224, 256, 288, 320, 352, 384
  };
  unsigned short iSL = 0;
  unsigned short iWire = segmentID;
  while (iWire >= nWiresInSuperLayer[iSL]) {
    iWire -= nWiresInSuperLayer[iSL];
    iSL += 1;
  }
  // shift from center wire to priority wire
  if (priorityPosition == 1) {
    if (iWire == 0)
      iWire = nWiresInSuperLayer[iSL] - 1;
    else
      iWire -= 1;
  }
  unsigned short iLayer = (iSL == 0) ? 3 : 2;
  if (priorityPosition == 1 || priorityPosition == 2)
    iLayer += 1;
  m_eWire = WireID(iSL, iLayer, iWire).getEWire();
}

CDCTriggerSegmentHit::CDCTriggerSegmentHit(unsigned short iSL,
                                           unsigned short iWire,
                                           unsigned short priorityPosition,
                                           unsigned short leftRight,
                                           short priorityTime,
                                           short fastestTime,
                                           short foundTime,
                                           short quadrant):
  m_priorityPosition(priorityPosition), m_leftRight(leftRight),
  m_priorityTime(priorityTime), m_fastestTime(fastestTime), m_foundTime(foundTime), m_quadrant(quadrant)
{
  // calculate continuous segment ID
  std::array<int, 9> nWiresInSuperLayer = {
    160, 160, 192, 224, 256, 288, 320, 352, 384
  };
  m_segmentID = iWire;
  for (unsigned i = 0; i < iSL; ++i) {
    m_segmentID += nWiresInSuperLayer[i];
  }
  // shift from center wire to priority wire
  if (priorityPosition == 1) {
    if (iWire == 0)
      iWire = nWiresInSuperLayer[iSL] - 1;
    else
      iWire -= 1;
  }
  unsigned short iLayer = (iSL == 0) ? 3 : 2;
  if (priorityPosition == 1 || priorityPosition == 2)
    iLayer += 1;
  m_eWire = WireID(iSL, iLayer, iWire).getEWire();
}

CDCTriggerSegmentHit::CDCTriggerSegmentHit(const CDCHit& priorityHit,
                                           unsigned short segmentID,
                                           unsigned short priorityPosition,
                                           unsigned short leftRight,
                                           short priorityTime,
                                           short fastestTime,
                                           short foundTime,
                                           short quadrant):
  m_segmentID(segmentID), m_priorityPosition(priorityPosition), m_leftRight(leftRight),
  m_priorityTime(priorityTime), m_fastestTime(fastestTime), m_foundTime(foundTime),
  m_eWire(priorityHit.getID()), m_quadrant(quadrant)
{}

unsigned short
CDCTriggerSegmentHit::getIWireCenter() const
{
  std::array<int, 9> nWiresInSuperLayer = {
    160, 160, 192, 224, 256, 288, 320, 352, 384
  };
  unsigned short iWire = getIWire();
  unsigned short iSL = getISuperLayer();
  if (m_priorityPosition == 1) {
    iWire += 1;
    if (iWire == nWiresInSuperLayer[iSL]) {
      iWire = 0;
    }
  }
  return iWire;
}
