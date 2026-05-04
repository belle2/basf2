/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/cdc/NDFinderPeakFinder.h"

#include <vector>
#include <utility>
#include <array>
#include <cmath>

using namespace Belle2;

// Find all the peaks in the Hough space sections
std::vector<HoughPeak> NDFinderPeakFinder::findPeaks()
{
  std::vector<HoughPeak> candidatePeaks;
  c3array houghSpaceBackup = *m_houghSpace;
  for (unsigned short quadrant = 0; quadrant < 4; ++quadrant) {
    for (unsigned short section = 0; section < 4; ++section) {
      std::array<c3index, 2> sectionBounds = getSectionBounds(quadrant, section);
      iterateOverSection(sectionBounds, candidatePeaks);
      if (m_peakFindingParams.iterations > 1) *m_houghSpace = houghSpaceBackup;
    }
  }
  return candidatePeaks;
}

// Get the phi bounds of one quadrant section
std::array<c3index, 2> NDFinderPeakFinder::getSectionBounds(const unsigned short quadrant, const unsigned section)
{
  c3index quadrantSize = m_peakFindingParams.nPhi / 4;
  c3index quadrantOffset = m_peakFindingParams.nPhi / 8;
  c3index sectionSize = quadrantSize / 4;

  c3index quadrantStart = quadrant * quadrantSize + quadrantOffset;
  c3index sectionStart = quadrantStart + section * sectionSize;
  c3index sectionEnd = sectionStart + sectionSize;

  return {sectionStart, sectionEnd};
}

// Iterate m_peakFindingParams.iterations times over one section
void NDFinderPeakFinder::iterateOverSection(const std::array<c3index, 2>& sectionBounds,
                                            std::vector<HoughPeak>& candidatePeaks)
{
  for (unsigned short _ = 0; _ < m_peakFindingParams.iterations; ++_) {
    HoughPeak sectionPeak = getSectionPeak(sectionBounds);
    if (sectionPeak.weight > 0) candidatePeaks.push_back(sectionPeak);
    if (m_peakFindingParams.iterations > 1) deletePeakSurroundings(sectionPeak);
  }
}

// Returns the global section peak index and weight
HoughPeak NDFinderPeakFinder::getSectionPeak(const std::array<c3index, 2>& sectionBounds)
{
  unsigned int peakValue = 0;
  cell_index peakCell = {0, 0, 0};
  for (c3index omegaIdx = 0; omegaIdx < m_peakFindingParams.nOmega; ++omegaIdx) {
    for (c3index phiIdx = sectionBounds[0]; phiIdx < sectionBounds[1]; ++phiIdx) {
      c3index phiIdxMod = phiIdx % m_peakFindingParams.nPhi;
      for (c3index cotIdx = 0; cotIdx < m_peakFindingParams.nCot; ++cotIdx) {
        if ((*m_houghSpace)[omegaIdx][phiIdxMod][cotIdx] > peakValue) {
          peakValue = (*m_houghSpace)[omegaIdx][phiIdxMod][cotIdx];
          peakCell = {omegaIdx, phiIdxMod, cotIdx};
        }
      }
    }
  }
  HoughPeak peak;
  peak.cell = peakCell;
  peak.weight = peakValue;
  return peak;
}

// Deletes the surroundings of the peak index with a butterfly cutout
void NDFinderPeakFinder::deletePeakSurroundings(const HoughPeak& peak)
{
  c3index omegaPeak = peak.cell[0];
  c3index phiPeak = peak.cell[1];

  c3index omegaLowerBound = std::max<unsigned short>(0, omegaPeak - m_peakFindingParams.omegaTrim);
  c3index omegaUpperBound = std::min<unsigned short>(m_peakFindingParams.nOmega, omegaPeak + m_peakFindingParams.omegaTrim + 1);

  for (c3index cotIdx = 0; cotIdx < m_peakFindingParams.nCot; ++cotIdx) {
    for (c3index omegaIdx = omegaLowerBound; omegaIdx < omegaUpperBound; ++omegaIdx) {
      c3index phiCell = phiPeak + omegaPeak - omegaIdx;
      c3index relativePhi = phiCell - phiPeak;

      if (relativePhi > 0) {
        DeletionBounds firstBounds = {
          phiCell - m_peakFindingParams.phiTrim,
          static_cast<c3index>(phiCell + m_peakFindingParams.phiTrim + std::floor(2.4 * relativePhi)),
          omegaIdx,
          cotIdx
        };
        clearHoughSpaceRow(firstBounds);
      } else if (relativePhi < 0) {
        DeletionBounds secondBounds = {
          static_cast<c3index>(phiCell - m_peakFindingParams.phiTrim + std::ceil(2.4 * relativePhi)),
          phiCell + m_peakFindingParams.phiTrim + 1,
          omegaIdx,
          cotIdx
        };
        clearHoughSpaceRow(secondBounds);
      } else {
        DeletionBounds thirdBounds = {
          phiCell - m_peakFindingParams.phiTrim,
          phiCell + m_peakFindingParams.phiTrim + 1,
          omegaIdx,
          cotIdx
        };
        clearHoughSpaceRow(thirdBounds);
      }
    }
  }
}

// Clears a single omega row
void NDFinderPeakFinder::clearHoughSpaceRow(const DeletionBounds& bounds)
{
  for (c3index phiIdx = bounds.phiLowerBound; phiIdx < bounds.phiUpperBound; ++phiIdx) {
    c3index phiIdxMod = (phiIdx + m_peakFindingParams.nPhi) % m_peakFindingParams.nPhi;
    (*m_houghSpace)[bounds.omega][phiIdxMod][bounds.cot] = 0;
  }
}
