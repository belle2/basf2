/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/cdc/modules/houghETF/CDCTriggerHoughETFModule.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <cmath>
#include <algorithm>
#include <iterator>

#include <root/TMatrix.h>

/* defines */
#define CDC_SUPER_LAYERS 9

using namespace Belle2;
using namespace Belle2::CDC;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCTriggerHoughETF);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCTriggerHoughETFModule::CDCTriggerHoughETFModule() : Module()
{
  //Set module properties
  setDescription("Hough tracking algorithm for CDC trigger.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Define module parameters
  // 2D ETF specified parameters
  // ----- PARAMETERS : GRL t0 finder with fastest priority timing (current at 2020.2.7)-----
  // t0CalcMethod = 0
  // usePriorityPosition = true
  // arrivalOrder = 0
  //
  addParam("outputEventTimeName", m_EventTimeName,
           "Name of the output StoreObjPtr.",
           std::string(""));
  addParam("storeTracks", m_storeTracks,
           "store tracks",
           false);
  addParam("usePriorityTiming", m_usePriorityTiming,
           "Use priority timing instead of fastest timing.",
           true);
  addParam("useHighPassTimingList", m_useHighPassTimingList,
           "Use associated fastest timings track-by-track",
           true);
  addParam("t0CalcMethod", m_t0CalcMethod,
           "0: Nth fastest fastest time."
           "1: median of all timings."
           "2: median of timings in timing window.",
           (unsigned)(0));
  addParam("arrivalOrder", m_arrivalOrder,
           "When t0CalcMethod == 0: Nth fastest ft is used as T0. (i.e. 0 is fastest)",
           (unsigned)(0));
  addParam("timeWindowBegin", m_timeWindowBegin,
           "When t0CalcMethod == 2: start time of time window relative to median. (in ns)",
           (short)(40));
  addParam("timeWindowEnd", m_timeWindowEnd,
           "When t0CalcMethod == 2: end time of time window relative to median. (in ns)",
           (short)(0));

  //common as CDCTrigger2DFinderModule
  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the input StoreArray of CDCTriggerSegmentHits.",
           std::string(""));
  addParam("outputCollectionName", m_outputCollectionName,
           "Name of the StoreArray holding the tracks found in the Hough tracking.",
           std::string("TRGCDCETFTracks"));
  addParam("clusterCollectionName", m_clusterCollectionName,
           "Name of the StoreArray holding the clusters formed in the Hough plane.",
           std::string(""));
  addParam("m_nCellsPhi", m_nCellsPhi,
           "Number of Hough cells in phi (limits: [-180, 180]). Must be an even number.",
           (unsigned)(160));
  addParam("m_nCellsR", m_nCellsR,
           "Number of Hough cells in 1/r. Must be an even number.",
           (unsigned)(34));
  addParam("minPt", m_minPt,
           "Minimum Pt [GeV]. "
           "Hough plane limits in 1/r are [-1/r(minPt), 1/r(minPt)]", (double)(0.3));
  addParam("shiftPt", m_shiftPt,
           "Shift the Hough plane by 1/4 cell size in 1/r to avoid "
           "curvature 0 tracks (<0: shift in negative direction, "
           "0: no shift, >0: shift in positive direction).", 0);

  addParam("minHits", m_minHits,
           "Minimum hits from different super layers required in a peak cell.",
           (unsigned)(4));
  addParam("minHitsShort", m_minHitsShort,
           "Minimum hits required required in a peak cell for a short track"
           " (must be in the first minHitsShort super layers).",
           (unsigned)(4));
  addParam("mim_nCells", m_minCells,
           "Peaks with less than mim_nCells connected cells are ignored.",
           (unsigned)(2));
  addParam("onlyLocalMax", m_onlyLocalMax,
           "Switch to remove cells connected to a cell with higher super layer count.",
           false);
  addParam("connect", m_connect,
           "Definition for connected cells. 4: direct (left/right/top/bottom), "
           "6: direct + 2 diagonal (top right/bottom left), "
           "8: direct + all diagonal (top right/top left/bottom right/bottom left)",
           (unsigned)(6));
  addParam("ignore2ndPriority", m_ignore2nd,
           "Switch to skip second priority hits.", false);
  addParam("usePriorityPosition", m_usePriority,
           "If true, use wire position of priority cell in track segment, "
           "otherwise use wire position of center cell.", true);
  addParam("requireSL0", m_requireSL0,
           "Switch to check separately for a hit in the innermost superlayer.", false);
  addParam("storeHoughPlane", m_storePlane,
           "Switch for saving Hough plane as TMatrix in DataStore. "
           "0: don't store anything, 1: store only peaks, 2: store full plane "
           "(will increase runtime).", (unsigned)(0));
  addParam("clusterPattern", m_clusterPattern,
           "use nested pattern algorithm to find clusters", true);
  addParam("clusterSizeX", m_clusterSizeX,
           "maximum number of 2 x 2 squares in cluster for pattern algorithm",
           (unsigned)(3));
  addParam("clusterSizeY", m_clusterSizeY,
           "maximum number of 2 x 2 squares in cluster for pattern algorithm",
           (unsigned)(3));
  addParam("hitRelationsFromCorners", m_hitRelationsFromCorners,
           "Switch for creating relations to hits in the pattern algorithm. "
           "If true, create relations from cluster corners, otherwise "
           "from estimated cluster center (might not have relations).", false);

  addParam("m_testFilename", m_testFilename,
           "If not empty, a file with input (hits) and output (tracks) "
           "for each event is written (for firmware debugging).", std::string(""));

  addParam("suppressClone", m_suppressClone,
           "Switch to send only the first found track and suppress the "
           "subsequent clones.", false);

  addParam("offset", m_offset,
           "Set certain time offset for ETFHough simulation"
           "Default as -10", -10);
}

void
CDCTriggerHoughETFModule::initialize()
{
  // register DataStore elements
  m_eventTime.registerInDataStore(m_EventTimeName);
  m_segmentHits.isRequired(m_hitCollectionName);

  if (m_storeTracks) {
    m_tracks.registerInDataStore(m_outputCollectionName);
    m_clusters.registerInDataStore(m_clusterCollectionName);

    m_tracks.registerRelationTo(m_segmentHits);
    m_tracks.registerRelationTo(m_clusters);
  }

  if (m_storePlane > 0) m_houghPlane.registerInDataStore("HoughPlane");

  const CDCGeometryPar& cdc = CDCGeometryPar::Instance();
  int layerId = 3;
  int nTS = 0;
  for (int iSL = 0; iSL < 9; ++iSL) {
    m_TSoffset[iSL] = nTS;
    nTS += cdc.nWiresInLayer(layerId);
    m_TSoffset[iSL + 1] = nTS;
    for (int priority = 0; priority < 2; ++ priority) {
      m_radius[iSL][priority] = cdc.senseWireR(layerId + priority);
    }
    layerId += (iSL > 0 ? 6 : 7);
  }

  for (int sl = 1; sl < 9; sl++) {
    m_NSecOffset[sl] = m_NSEC[sl - 1] + m_NSecOffset[sl - 1];
  }

  if (m_testFilename != "") {
    m_testFile.open(m_testFilename);
  }

  if (m_suppressClone) {
    B2INFO("2D finder will exit with the first track candidate in time.");
  }

  if (m_t0CalcMethod != 0 && m_t0CalcMethod != 1) B2WARNING("t0CalcMethod is invalid. calculate as default.");

  /* Extent the Hough plane such that the cell number is a power of 2 (same for x and y).
   * This is for the fast peak finder, which divides the Hough plane in half in each step.
   * Peaks found outside of the actual limits are ignored. */
  m_maxIterations = std::ceil(std::log2(std::max(m_nCellsPhi, m_nCellsR))) - 1;
  m_nCells = std::pow(2, m_maxIterations + 1);
  /* limits in phi: [-pi, pi] + extra cells */
  m_rectX = M_PI * m_nCells / m_nCellsPhi;
  /* limits in R: [-R(minPt), R(minPt)] + extra cells + shift */
  m_maxR = 0.5 * Const::speedOfLight * 1.5e-4 / m_minPt;
  m_rectY = m_maxR * m_nCells / m_nCellsR;
  m_shiftR = 0;
  if (m_shiftPt < 0) {
    m_shiftR = -m_maxR / 2. / m_nCellsR;
  } else if (m_shiftPt > 0) {
    m_shiftR = m_maxR / 2. / m_nCellsR;
  }
}

void
CDCTriggerHoughETFModule::event()
{
  /* Clean hits */
  m_hitMap.clear();
  m_houghCand.clear();
  m_associatedTSHitsList.clear();

  if (!m_eventTime.isValid()) m_eventTime.create();
  /* set default return value */
  setReturnValue(true);

  if (m_testFilename != "") {
    m_testFile << StoreObjPtr<EventMetaData>()->getEvent() << " "
               << m_segmentHits.getEntries() << std::endl;
  }

  if (m_segmentHits.getEntries() == 0) {
    //B2WARNING("CDCTracking: tsHitsCollection is empty!");
    return;
  }

  /* get CDCHits coordinates in conformal space */
  for (int iHit = 0; iHit < m_segmentHits.getEntries(); iHit++) {
    unsigned short iSL = m_segmentHits[iHit]->getISuperLayer();
    if (m_testFilename != "") {
      m_testFile << iSL << " " << m_segmentHits[iHit]->getSegmentID() - m_TSoffset[iSL] << " "
                 << m_segmentHits[iHit]->getPriorityPosition() << std::endl;
    }
    if (iSL % 2) continue;
    if (m_ignore2nd && m_segmentHits[iHit]->getPriorityPosition() < 3) continue;
    double phi = m_segmentHits[iHit]->getSegmentID() - m_TSoffset[iSL];
    if (m_usePriority) {
      phi += 0.5 * (((m_segmentHits[iHit]->getPriorityPosition() >> 1) & 1)
                    - (m_segmentHits[iHit]->getPriorityPosition() & 1));
    }
    phi = phi * 2. * M_PI / (m_TSoffset[iSL + 1] - m_TSoffset[iSL]);
    double r = m_radius[iSL][int(m_usePriority &&
                                 m_segmentHits[iHit]->getPriorityPosition() < 3)];
    ROOT::Math::XYVector pos(std::cos(phi) / r, std::sin(phi) / r);
    m_hitMap.insert(std::make_pair(iHit, std::make_pair(iSL, pos)));
  }

  B2DEBUG(50, "extending Hough plane to " << m_maxIterations << " iterations, "
          << m_nCells << " cells: phi in ["
          << -m_rectX * 180. / M_PI << ", " << m_rectX * 180. / M_PI
          << "] deg, 1/r in [" << -m_rectY + m_shiftR << ", " << m_rectY + m_shiftR << "] /cm");

  /* prepare matrix for storing the Hough plane */
  if (m_storePlane > 0) {
    m_houghPlane.create();
    m_houghPlane->ResizeTo(m_nCellsPhi, m_nCellsR);
  }

  // hit map containing only the early hits
  CDCHitMap fastHitMap;
  if (m_suppressClone && !m_hitMap.empty()) {
    // find the first track candidates in Hough plane
    // only for z0 resolution study with single-track events
    // This will surely fail with multi-track ones,
    // in which case we really need tick-by-tick simulation for all hits.

    /** Pair of <counter, CDCPair>, for hits with indices */
    typedef std::pair<int, CDCPair> CDCHitPair;
    // sequential hit map, ordered by TS found time
    typedef std::vector<CDCHitPair> CDCSeqMap;
    CDCSeqMap seqHitMap;
    // copy hit map to sequential hit map and sort it by TS found time
    seqHitMap.reserve(m_hitMap.size());
    for (auto hit : m_hitMap) {
      seqHitMap.push_back(hit);
    }
    sort(seqHitMap.begin(), seqHitMap.end(), [this](CDCHitPair i, CDCHitPair j) {
      return m_segmentHits[i.first]->foundTime() < m_segmentHits[j.first]->foundTime();
    });
    auto seqHitItr = seqHitMap.begin();
    /* layer filter */
    std::vector<bool> layerHit(CDC_SUPER_LAYERS, false);
    // data clock cycle in unit of 2ns
    short period = 16;
    short firstTick = m_segmentHits[(*seqHitMap.begin()).first]->foundTime() / period + 1;
    short lastTick = m_segmentHits[(*(seqHitMap.end() - 1)).first]->foundTime() / period + 1;
    // add TS hits in every clock cycle until a track candidate is found
    for (auto tick = firstTick * period; tick < lastTick * period; tick += period) {
      int nHitInCycle = 0;
      for (auto itr = seqHitItr; itr < seqHitMap.end(); ++itr) {
        CDCHitPair currentHit = *itr;
        // start from the first hit over SL threshold
        if (count(layerHit.begin(), layerHit.end(), true) >= m_minHits &&
            m_segmentHits[currentHit.first]->foundTime() > tick) {
          break;
        }
        nHitInCycle++;
        layerHit[m_segmentHits[currentHit.first]->getISuperLayer()] = true;
      }
      copy_n(seqHitItr, nHitInCycle, inserter(fastHitMap, fastHitMap.end()));
      fastInterceptFinder(fastHitMap, -m_rectX, m_rectX, -m_rectY + m_shiftR, m_rectY + m_shiftR, 0, 0, 0);
      B2DEBUG(20, "at tick " << tick << ", number of candidates: " << m_houghCand.size());
      if (!m_houghCand.empty()) {
        B2DEBUG(10, "found a track at clock " << tick << " with "
                << fastHitMap.size() << "hits");
        break;
      }
      advance(seqHitItr, nHitInCycle);
    }
  } else {
    /* find track candidates in Hough plane using all TS hits */
    fastInterceptFinder(m_hitMap, -m_rectX, m_rectX, -m_rectY + m_shiftR, m_rectY + m_shiftR, 0, 0, 0);
    if (!m_houghCand.empty()) {
      B2DEBUG(10, "found a track with " << m_hitMap.size() << "hits");
    }
  }

  /* merge track candidates */
  if (m_clusterPattern) {
    if (patternClustering(fastHitMap))
      m_eventTime->addBinnedEventT0(calcEventTiming() + m_offset, Const::CDC);
  } else {
    connectedRegions();
  }

  if (m_testFilename != "") {
    m_testFile << m_tracks.getEntries() << std::endl;
    for (int i = 0; i < m_tracks.getEntries(); ++i) {
      float ix = (m_tracks[i]->getPhi0() - M_PI_4) * m_nCellsPhi / 2. / M_PI - 0.5;
      float iy = (m_tracks[i]->getOmega() / 2. + m_maxR - m_shiftR) * m_nCellsR / 2. / m_maxR - 0.5;
      m_testFile << round(2 * ix) / 2. << " " << round(2 * iy) / 2. << " "
                 << m_tracks[i]->getChargeSign() << std::endl;
      RelationVector<CDCTriggerSegmentHit> hits =
        m_tracks[i]->getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
      m_testFile << hits.size() << std::endl;
      for (unsigned ihit = 0; ihit < hits.size(); ++ihit) {
        unsigned short iSL = hits[ihit]->getISuperLayer();
        m_testFile << iSL << " " << hits[ihit]->getSegmentID() - m_TSoffset[iSL] << " "
                   << hits[ihit]->getPriorityPosition() << " "
                   << hits.weight(ihit) << std::endl;
      }
    }
  }
}

void
CDCTriggerHoughETFModule::terminate()
{
  if (m_testFilename != "") m_testFile.close();
}
