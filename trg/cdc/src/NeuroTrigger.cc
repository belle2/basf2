/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/logging/Logger.h>
#include <trg/cdc/NeuroTrigger.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dbobjects/CDCTriggerNeuroConfig.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <string>
#include <cmath>
#include <TFile.h>

using namespace Belle2;
using namespace CDC;
using namespace std;

void
NeuroTrigger::initialize(const Parameters& p)
{
  // check parameters
  bool okay = true;
  // ensure that length of lists matches number of sectors
  if (p.nHidden.size() != 1 && p.nHidden.size() != p.nMLP) {
    B2ERROR("Number of nHidden lists should be 1 or " << p.nMLP);
    okay = false;
  }
  if (p.outputScale.size() != 1 && p.outputScale.size() != p.nMLP) {
    B2ERROR("Number of outputScale lists should be 1 or " << p.nMLP);
    okay = false;
  }
  bool rangeProduct = (p.phiRange.size() * p.invptRange.size() * p.thetaRange.size() * p.SLpattern.size() == p.nMLP);
  if (!rangeProduct) {
    if (p.phiRange.size() != 1 && p.phiRange.size() != p.nMLP) {
      B2ERROR("Number of phiRange lists should be 1 or " << p.nMLP);
      okay = false;
    }
    if (p.invptRange.size() != 1 && p.invptRange.size() != p.nMLP) {
      B2ERROR("Number of invptRange lists should be 1 or " << p.nMLP);
      okay = false;
    }
    if (p.thetaRange.size() != 1 && p.thetaRange.size() != p.nMLP) {
      B2ERROR("Number of thetaRange lists should be 1 or " << p.nMLP);
      okay = false;
    }
    if (p.SLpattern.size() != 1 && p.SLpattern.size() != p.nMLP) {
      B2ERROR("Number of SLpattern lists should be 1 or " << p.nMLP);
      okay = false;
    }
  }
  // ensure that length of maxHitsPerSL and SLpatternMask lists matches SLpattern
  if (p.maxHitsPerSL.size() != 1 && p.maxHitsPerSL.size() != p.SLpattern.size()) {
    B2ERROR("Number of maxHitsPerSL lists should be 1 or " << p.SLpattern.size());
    okay = false;
  }
  if (p.SLpatternMask.size() != 1 && p.SLpatternMask.size() != p.SLpattern.size()) {
    B2ERROR("Number of SLpatternMask lists should be 1 or " << p.SLpattern.size());
    okay = false;
  }
  // ensure that number of target nodes is valid
  unsigned short nTarget = int(p.targetZ) + int(p.targetTheta);
  if (nTarget < 1) {
    B2ERROR("No outputs! Turn on either targetZ or targetTheta.");
    okay = false;
  }
  // ensure that sector ranges are valid
  for (unsigned iPhi = 0; iPhi < p.phiRange.size(); ++iPhi) {
    if (p.phiRange[iPhi].size() != 2) {
      B2ERROR("phiRange should be exactly 2 values");
      okay = false;
      continue;
    }
    if (p.phiRange[iPhi][0] >= p.phiRange[iPhi][1]) {
      B2ERROR("phiRange[0] should be smaller than phiRange[1]");
      okay = false;
    }
    if (p.phiRange[iPhi][0] < -360. || p.phiRange[iPhi][1] > 360. ||
        (p.phiRange[iPhi][1] - p.phiRange[iPhi][0]) > 360.) {
      B2ERROR("phiRange should be in [-360, 360], with maximal width of 360");
      okay = false;
    }
  }
  for (unsigned iPt = 0; iPt < p.invptRange.size(); ++iPt) {
    if (p.invptRange[iPt].size() != 2) {
      B2ERROR("invptRange should be exactly 2 values");
      okay = false;
    }
    if (p.invptRange[iPt][0] >= p.invptRange[iPt][1]) {
      B2ERROR("invptRange[0] should be smaller than invptRange[1]");
      okay = false;
    }
  }
  for (unsigned iTheta = 0; iTheta < p.thetaRange.size(); ++iTheta) {
    if (p.thetaRange[iTheta].size() != 2) {
      B2ERROR("thetaRange should be exactly 2 values");
      okay = false;
      continue;
    }
    if (p.thetaRange[iTheta][0] >= p.thetaRange[iTheta][1]) {
      B2ERROR("thetaRange[0] should be smaller than thetaRange[1]");
      okay = false;
    }
    if (p.thetaRange[iTheta][0] < 0. || p.thetaRange[iTheta][1] > 180.) {
      B2ERROR("thetaRange should be in [0, 180]");
      okay = false;
    }
  }
  for (unsigned iScale = 0; iScale < p.outputScale.size(); ++iScale) {
    if (p.outputScale[iScale].size() != 2 * nTarget) {
      B2ERROR("outputScale should be exactly " << 2 * nTarget << " values");
      okay = false;
    }
  }
  // ensure that train sectors are valid
  if (p.phiRange.size() != p.phiRangeTrain.size()) {
    B2ERROR("Number of phiRange lists and phiRangeTrain lists should be equal.");
    okay = false;
  } else {
    for (unsigned iPhi = 0; iPhi < p.phiRange.size(); ++iPhi) {
      if (p.phiRangeTrain[iPhi].size() != 2) {
        B2ERROR("phiRangeTrain should be exactly 2 values.");
        okay = false;
      } else if (p.phiRangeTrain[iPhi][0] > p.phiRange[iPhi][0] ||
                 p.phiRangeTrain[iPhi][1] < p.phiRange[iPhi][1]) {
        B2ERROR("phiRangeTrain should be wider than phiRange or equal.");
        okay = false;
      }
    }
  }
  if (p.invptRange.size() != p.invptRangeTrain.size()) {
    B2ERROR("Number of invptRange lists and invptRangeTrain lists should be equal.");
    okay = false;
  } else {
    for (unsigned iPt = 0; iPt < p.invptRange.size(); ++iPt) {
      if (p.invptRangeTrain[iPt].size() != 2) {
        B2ERROR("invptRangeTrain should be exactly 2 values.");
        okay = false;
      } else if (p.invptRangeTrain[iPt][0] > p.invptRange[iPt][0] ||
                 p.invptRangeTrain[iPt][1] < p.invptRange[iPt][1]) {
        B2ERROR("invptRangeTrain should be wider than invptRange or equal.");
        okay = false;
      }
    }
  }
  if (p.thetaRange.size() != p.thetaRangeTrain.size()) {
    B2ERROR("Number of thetaRange lists and thetaRangeTrain lists should be equal.");
    okay = false;
  } else {
    for (unsigned iTheta = 0; iTheta < p.thetaRange.size(); ++iTheta) {
      if (p.thetaRangeTrain[iTheta].size() != 2) {
        B2ERROR("thetaRangeTrain should be exactly 2 values.");
        okay = false;
      } else if (p.thetaRangeTrain[iTheta][0] > p.thetaRange[iTheta][0] ||
                 p.thetaRangeTrain[iTheta][1] < p.thetaRange[iTheta][1]) {
        B2ERROR("thetaRangeTrain should be wider than thetaRange or equal.");
        okay = false;
      }
    }
  }

  if (!okay) return;

  // initialize MLPs
  m_MLPs.clear();
  for (unsigned iMLP = 0; iMLP < p.nMLP; ++iMLP) {
    //get indices for sector parameters
    vector<unsigned> indices = getRangeIndices(p, iMLP);
    //get number of nodes for each layer
    unsigned short maxHits = (p.maxHitsPerSL.size() == 1) ? p.maxHitsPerSL[0] : p.maxHitsPerSL[indices[3]];
    unsigned long SLpattern = p.SLpattern[indices[3]];
    unsigned long SLpatternMask = (p.SLpatternMask.size() == 1) ? p.SLpatternMask[0] : p.SLpatternMask[indices[3]];
    unsigned short nInput = 27 * maxHits;
    vector<float> nHidden = (p.nHidden.size() == 1) ? p.nHidden[0] : p.nHidden[iMLP];
    vector<unsigned short> nNodes = {nInput};
    for (unsigned iHid = 0; iHid < nHidden.size(); ++iHid) {
      if (p.multiplyHidden) {
        nNodes.push_back(nHidden[iHid] * nNodes[0]);
      } else {
        nNodes.push_back(nHidden[iHid]);
      }
    }
    nNodes.push_back(nTarget);
    unsigned short targetVars = int(p.targetZ) + (int(p.targetTheta) << 1);
    //get sector ranges (initially train ranges)
    vector<float> phiRange = p.phiRangeTrain[indices[0]];
    vector<float> invptRange = p.invptRangeTrain[indices[1]];
    vector<float> thetaRange = p.thetaRangeTrain[indices[2]];
    B2DEBUG(50, "Ranges for sector " << iMLP
            << ": phiRange [" << phiRange[0] << ", " << phiRange[1]
            << "], invptRange [" << invptRange[0] << ", " << invptRange[1]
            << "], thetaRange [" << thetaRange[0] << ", " << thetaRange[1]
            << "], SLpattern " << SLpattern);
    //get scaling values
    vector<float> outputScale = (p.outputScale.size() == 1) ? p.outputScale[0] : p.outputScale[iMLP];
    //convert phi and theta from degree to radian
    phiRange[0] *= Unit::deg;
    phiRange[1] *= Unit::deg;
    thetaRange[0] *= Unit::deg;
    thetaRange[1] *= Unit::deg;
    if (p.targetTheta) {
      outputScale[2 * int(p.targetZ)] *= Unit::deg;
      outputScale[2 * int(p.targetZ) + 1] *= Unit::deg;
    }
    //create new MLP
    m_MLPs.push_back(CDCTriggerMLP(nNodes, targetVars, outputScale,
                                   phiRange, invptRange, thetaRange,
                                   maxHits, SLpattern, SLpatternMask, p.tMax, p.T0fromHits,
                                   p.et_option));
  }
  // load some values from the geometry that will be needed for the input
  setConstants();
}

vector<unsigned>
NeuroTrigger::getRangeIndices(const Parameters& p, unsigned isector)
{
  std::vector<unsigned> indices = {0, 0, 0, 0};
  if (p.phiRange.size() * p.invptRange.size() * p.thetaRange.size() * p.SLpattern.size() == p.nMLP) {
    indices[0] = isector % p.phiRange.size();
    indices[1] = (isector / p.phiRange.size()) % p.invptRange.size();
    indices[2] = (isector / (p.phiRange.size() * p.invptRange.size())) % p.thetaRange.size();
    indices[3] = isector / (p.phiRange.size() * p.invptRange.size() * p.thetaRange.size());
  } else {
    indices[0] = (p.phiRange.size() == 1) ? 0 : isector;
    indices[1] = (p.invptRange.size() == 1) ? 0 : isector;
    indices[2] = (p.thetaRange.size() == 1) ? 0 : isector;
    indices[3] = (p.SLpattern.size() == 1) ? 0 : isector;
  }
  return indices;
}

void
NeuroTrigger::setConstants()
{
  CDCGeometryPar& cdc = CDCGeometryPar::Instance();
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
}

void
NeuroTrigger::initializeCollections(string hitCollectionName, string eventTimeName, const std::string& et_option)
{
  m_segmentHits.isRequired(hitCollectionName);
  if (!((et_option == "fastestpriority") || (et_option == "zero") || (et_option == "fastest2d"))) {
    m_eventTime.isRequired(eventTimeName);
  }
  m_hitCollectionName = hitCollectionName;
}

vector<int>
NeuroTrigger::selectMLPs(float phi0, float invpt, float theta)
{
  vector<int> indices = {};

  if (m_MLPs.size() == 0) {
    B2WARNING("Trying to select MLP before initializing MLPs.");
    return indices;
  }

  // find all matching sectors
  for (unsigned isector = 0; isector < m_MLPs.size(); ++isector) {
    if (m_MLPs[isector].inPhiRange(phi0) && m_MLPs[isector].inInvptRange(invpt)
        && m_MLPs[isector].inThetaRange(theta)) {
      indices.push_back(isector);
    }
  }

  if (indices.size() == 0) {
    B2DEBUG(150, "Track does not match any sector.");
    B2DEBUG(150, "invpt=" << invpt << ", phi=" << phi0 * 180. / M_PI << ", theta=" << theta * 180. / M_PI);
  }

  return indices;
}


int
NeuroTrigger::selectMLPbyPattern(std::vector<int>& MLPs, unsigned long pattern, const bool neurotrackinputmode)
{
  if (MLPs.size() == 0) {
    return -1;
  }

  int bestIndex = -1;
  for (unsigned i = 0; i < MLPs.size(); ++i) {
    int isector = MLPs[i];
    unsigned long sectorPattern = m_MLPs[isector].getSLpattern();
    B2DEBUG(250, "hitPattern " << pattern << " sectorPattern " << sectorPattern);
    // no hit pattern restriction -> keep looking for exact match
    if (sectorPattern == 0) {
      B2DEBUG(250, "found match for general sector");
      bestIndex = isector;
    }
    // exact match -> keep this sector and stop searching
    if (pattern == sectorPattern) {
      B2DEBUG(250, "found match for hit pattern " << pattern);
      bestIndex = isector;
      break;
    }
  }

  if (bestIndex < 0) {
    if (neurotrackinputmode) {
      B2DEBUG(150, "No sector found to match pattern, using sector 0" << pattern << ".");
      bestIndex = 0;
    } else {
      B2DEBUG(150, "No sector found to match pattern " << pattern << ".");
    }
  }
  return bestIndex;
}


void
NeuroTrigger::updateTrack(const CDCTriggerTrack& track)
{
  double omega = track.getOmega(); // signed track curvature
  for (int iSL = 0; iSL < 9; ++iSL) {
    for (int priority = 0; priority < 2; ++priority) {
      m_alpha[iSL][priority] = (m_radius[iSL][priority] * abs(omega) < 2.) ?
                               asin(m_radius[iSL][priority] * omega / 2.) :
                               M_PI_2;
      m_idRef[iSL][priority] = remainder(((track.getPhi0() - m_alpha[iSL][priority]) *
                                          (m_TSoffset[iSL + 1] - m_TSoffset[iSL]) / 2. / M_PI),
                                         (m_TSoffset[iSL + 1] - m_TSoffset[iSL]));
    }
  }
}

void
NeuroTrigger::updateTrackFix(const CDCTriggerTrack& track)
{
  unsigned precisionPhi = m_precision[0];
  unsigned precisionAlpha = m_precision[0];
  unsigned precisionScale = m_precision[1];
  unsigned precisionId = m_precision[2];

  double omega = track.getOmega();
  long phi = round(track.getPhi0() * (1 << precisionPhi));

  for (int iSL = 0; iSL < 9; ++iSL) {
    for (int priority = 0; priority < 2; ++priority) {
      // LUT, calculated on the fly here
      m_alpha[iSL][priority] =
        (m_radius[iSL][priority] * abs(omega) < 2) ?
        round(asin(m_radius[iSL][priority] * omega / 2) * (1 << precisionAlpha)) :
        round(M_PI_2 * (1 << precisionAlpha));
      long dphi = (precisionAlpha >= precisionPhi) ?
                  phi - (long(m_alpha[iSL][priority]) >> (precisionAlpha - precisionPhi)) :
                  phi - (long(m_alpha[iSL][priority]) << (precisionPhi - precisionAlpha));
      m_idRef[iSL][priority] =
        long(dphi * round((m_TSoffset[iSL + 1] - m_TSoffset[iSL]) / 2. / M_PI * (1 << precisionScale)) /
             (long(1) << (precisionPhi + precisionScale - precisionId)));
      // unscale after rounding
      m_alpha[iSL][priority] /= (1 << precisionAlpha);
      m_idRef[iSL][priority] /= (1 << precisionId);
    }
  }
}

double
NeuroTrigger::getRelId(const CDCTriggerSegmentHit& hit)
{
  int iSL = hit.getISuperLayer();
  int priority = hit.getPriorityPosition();
  // (((priority >> 1) & 1) - (priority & 1)) is 0, -1, 1, 0 for priority = 0, 1, 2, 3
  double relId = hit.getSegmentID() + 0.5 * (((priority >> 1) & 1) - (priority & 1))
                 - m_TSoffset[iSL] - m_idRef[iSL][int(priority < 3)];
  relId = remainder(relId, (m_TSoffset[iSL + 1] - m_TSoffset[iSL]));
  return relId;
}

void
NeuroTrigger::getEventTime(unsigned isector, const CDCTriggerTrack& track, std::string et_option, const bool neuroinputmode = false)
{
  if (et_option != m_MLPs[isector].get_et_option()) {
    B2WARNING("Used event time option is different to the one set in the MLP"
              << LogVar("et_option", et_option) << LogVar("isector", isector)
              << LogVar("et_option_mlp", m_MLPs[isector].get_et_option()));
  }
  if (et_option == "etf_or_fastestpriority") {
    bool hasT0 = m_eventTime->hasBinnedEventT0(Const::CDC);
    if (hasT0) {
      m_T0 = m_eventTime->getBinnedEventT0(Const::CDC);
      m_hasT0 = true;
    } else {
      m_T0 = 9999;
      // find shortest time of related and relevant axial hits
      RelationVector<CDCTriggerSegmentHit> axialHits =
        track.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
      for (unsigned ihit = 0; ihit < axialHits.size(); ++ihit) {
        // skip hits with negative relation weight (not selected in finder)
        if (axialHits.weight(ihit) < 0) continue;
        unsigned short iSL = axialHits[ihit]->getISuperLayer();
        // skip stereo hits (should not be related to track, but check anyway)
        if ((iSL % 2 == 1) && !neuroinputmode)  continue;
        // get shortest time of relevant hits
        double relId = getRelId(*axialHits[ihit]);
        if (m_MLPs[isector].isRelevant(relId, iSL) &&
            axialHits[ihit]->priorityTime() < m_T0) {
          m_T0 = axialHits[ihit]->priorityTime();
        }
      }
      if (!neuroinputmode) {
        // find shortest time of relevant stereo hits
        StoreArray<CDCTriggerSegmentHit> hits(m_hitCollectionName);
        for (int ihit = 0; ihit < hits.getEntries(); ++ihit) {
          unsigned short iSL = hits[ihit]->getISuperLayer();
          // skip axial hits
          if (iSL % 2 == 0) continue;
          // get shortest time of relevant hits
          double relId = getRelId(*hits[ihit]);
          if (m_MLPs[isector].isRelevant(relId, iSL) && hits[ihit]->priorityTime() < m_T0) {
            m_T0 = hits[ihit]->priorityTime();
          }
        }
      }
      if (m_T0 < 9999) {
        m_hasT0 = true;
      } else {
        m_T0 = 0;
        m_hasT0 = false;
      }
    }
  } else if (et_option == "etf_or_fastest2d") {
    bool hasT0 = m_eventTime->hasBinnedEventT0(Const::CDC);
    if (hasT0) {
      m_T0 = m_eventTime->getBinnedEventT0(Const::CDC);
      m_hasT0 = true;
    } else {
      m_T0 = 9999;
      // find shortest time of related and relevant axial hits
      RelationVector<CDCTriggerSegmentHit> axialHits =
        track.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
      for (unsigned ihit = 0; ihit < axialHits.size(); ++ihit) {
        // skip hits with negative relation weight (not selected in finder)
        if (axialHits.weight(ihit) < 0) continue;
        unsigned short iSL = axialHits[ihit]->getISuperLayer();
        // skip stereo hits (should not be related to track, but check anyway)
        if (iSL % 2 == 1) continue;
        // get shortest time of relevant hits
        double relId = getRelId(*axialHits[ihit]);
        if (m_MLPs[isector].isRelevant(relId, iSL) &&
            axialHits[ihit]->priorityTime() < m_T0) {
          m_T0 = axialHits[ihit]->priorityTime();
        }
      }
      if (m_T0 < 9999) {
        m_hasT0 = true;
      } else {
        m_T0 = 0;
        m_hasT0 = false;
      }
    }
  } else if (et_option == "fastestpriority") {
    B2DEBUG(200, "et_option is 'fastestpriority'");
    m_T0 = 9999;
    // find shortest time of related and relevant axial hits
    RelationVector<CDCTriggerSegmentHit> axialHits =
      track.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
    B2DEBUG(200, "looping over axials:");
    for (unsigned ihit = 0; ihit < axialHits.size(); ++ihit) {
      // skip hits with negative relation weight (not selected in finder)
      if (axialHits.weight(ihit) < 0) continue;
      unsigned short iSL = axialHits[ihit]->getISuperLayer();
      // skip stereo hits (should not be related to track, but check anyway)
      if ((iSL % 2 == 1) && !neuroinputmode)  continue;
      // get shortest time of relevant hits
      B2DEBUG(200, "  check drifttime: SL" + std::to_string(iSL) + ",ID = " + std::to_string(axialHits[ihit]->getSegmentID()) + ", t = " +
              std::to_string(axialHits[ihit]->priorityTime()));
      double relId = getRelId(*axialHits[ihit]);
      if (m_MLPs[isector].isRelevant(relId, iSL) &&
          axialHits[ihit]->priorityTime() < m_T0) {
        m_T0 = axialHits[ihit]->priorityTime();
        B2DEBUG(200, "    new t0: " << std::to_string(m_T0));
      }
    }
    if (!neuroinputmode) {
      // find shortest time of relevant stereo hits
      StoreArray<CDCTriggerSegmentHit> hits(m_hitCollectionName);
      B2DEBUG(200, "looping over stereos:");
      for (int ihit = 0; ihit < hits.getEntries(); ++ihit) {
        unsigned short iSL = hits[ihit]->getISuperLayer();
        // skip axial hits
        if (iSL % 2 == 0) continue;
        // get shortest time of relevant hits
        B2DEBUG(200, "  check drifttime: SL" + std::to_string(iSL) + ",ID = " + std::to_string(hits[ihit]->getSegmentID()) + ", t = " +
                std::to_string(hits[ihit]->priorityTime()));
        double relId = getRelId(*hits[ihit]);
        if (m_MLPs[isector].isRelevant(relId, iSL) && hits[ihit]->priorityTime() < m_T0) {
          m_T0 = hits[ihit]->priorityTime();
          B2DEBUG(200, "    new t0: " << std::to_string(m_T0));
        }
      }
    }
    if (m_T0 < 9999) {
      m_hasT0 = true;
    } else {
      m_T0 = 0;
      m_hasT0 = false;
    }

  } else if (et_option == "fastest2d") {
    m_T0 = 9999;
    // find shortest time of related and relevant axial hits
    RelationVector<CDCTriggerSegmentHit> axialHits =
      track.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
    for (unsigned ihit = 0; ihit < axialHits.size(); ++ihit) {
      // skip hits with negative relation weight (not selected in finder)
      if (axialHits.weight(ihit) < 0) continue;
      unsigned short iSL = axialHits[ihit]->getISuperLayer();
      // skip stereo hits (should not be related to track, but check anyway)
      if (iSL % 2 == 1) continue;
      // get shortest time of relevant hits
      double relId = getRelId(*axialHits[ihit]);
      if (m_MLPs[isector].isRelevant(relId, iSL) &&
          axialHits[ihit]->priorityTime() < m_T0) {
        m_T0 = axialHits[ihit]->priorityTime();
      }
    }
    if (m_T0 < 9999) {
      m_hasT0 = true;
    } else {
      m_T0 = 0;
      m_hasT0 = false;
    }
  } else if (et_option == "zero") {
    m_hasT0 = true;
    m_T0 = 0;
  } else if (et_option == "etf_only") {
    bool hasT0 = m_eventTime->hasBinnedEventT0(Const::CDC);
    if (hasT0) {
      m_T0 = m_eventTime->getBinnedEventT0(Const::CDC);
      m_hasT0 = true;
    } else {
      B2ERROR("No ETF output, but forced to use ETF!");
      m_hasT0 = false;
      m_T0 = 0;
    }
  } else if (et_option == "etf_or_zero") {
    bool hasT0 = m_eventTime->hasBinnedEventT0(Const::CDC);
    if (hasT0) {
      m_T0 = m_eventTime->getBinnedEventT0(Const::CDC);
      m_hasT0 = true;
    } else {
      m_hasT0 = true;
      m_T0 = 0;
    }
  } else {
    B2ERROR("No valid parameter for et_option (" << et_option << " )!");
  }

}
void
NeuroTrigger::getEventTime(unsigned isector, const CDCTriggerTrack& track)
{
  B2WARNING("This function is deprecated, it used the flag 'T0fromHits'. "
            << "Give additionally et_option as argument to use the new function. "
            << "et_option is now set for T0fromHits=True to 'etf_fastestpriority' "
            << "and for T0fromHits=false to 'etf_or_zero'.");
  if (m_MLPs[isector].getT0fromHits()) {
    getEventTime(isector, track, "etf_or_fastestpriority");
  } else {
    getEventTime(isector, track, "etf_or_zero");
  }
}

unsigned long
NeuroTrigger::getPureDriftThreshold(unsigned isector, const CDCTriggerTrack& track, const bool neurotrackinputmode)
{
  const CDCTriggerMLP& expert = m_MLPs[isector];
  unsigned long driftth = 0;
  vector<unsigned> nHits;
  nHits.assign(9, 0);
  // loop over axial hits related to input track
  RelationVector<CDCTriggerSegmentHit> axialHits =
    track.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
  for (unsigned ihit = 0; ihit < axialHits.size(); ++ ihit) {
    // skip hits with negative relation weight (not selected in finder)
    if (axialHits.weight(ihit) < 0) {
      continue;
    }
    unsigned short iSL = axialHits[ihit]->getISuperLayer();
    // // skip stereo hits (should not be related to track, but check anyway)
    if ((!neurotrackinputmode) && (iSL % 2 == 1)) continue;
    // get priority time
    int t = (m_hasT0) ? axialHits[ihit]->priorityTime() - m_T0 : 0;
    double relId = getRelId(*axialHits[ihit]);
    if (t < 0 || t > expert.getTMax()) {
      if (expert.isRelevant(relId, iSL)) {
        if (nHits[iSL] < expert.getMaxHitsPerSL()) {
          driftth |= 1 << (8 - iSL + 9 * nHits[iSL]);
          ++nHits[iSL];
        }
      }
    }
  }
  if (!neurotrackinputmode) {
    // loop over stereo hits
    for (int ihit = 0; ihit < m_segmentHits.getEntries(); ++ ihit) {
      unsigned short iSL = m_segmentHits[ihit]->getISuperLayer();
      // skip axial hits
      if (iSL % 2 == 0) continue;
      // get priority time
      int t = (m_hasT0) ? m_segmentHits[ihit]->priorityTime() - m_T0 : 0;
      double relId = getRelId(*m_segmentHits[ihit]);
      if (t < 0 || t > expert.getTMax()) {
        if (expert.isRelevant(relId, iSL)) {
          if (nHits[iSL] < expert.getMaxHitsPerSL()) {
            driftth |= 1 << (8 - iSL + 9 * nHits[iSL]);
            ++nHits[iSL];
          }
        }
      }
    }
  }
  return driftth;
}
unsigned long NeuroTrigger::getCompleteHitPattern(unsigned isector, const CDCTriggerTrack& track, const bool neurotrackinputmode)
{
  const CDCTriggerMLP& expert = m_MLPs[isector];
  unsigned long chitPattern = 0;
  vector<unsigned> nHits;
  nHits.assign(9, 0);
  // loop over axial hits related to input track
  RelationVector<CDCTriggerSegmentHit> axialHits =
    track.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
  for (unsigned ihit = 0; ihit < axialHits.size(); ++ ihit) {
    // skip hits with negative relation weight (not selected in finder)
    if (axialHits.weight(ihit) < 0) {
      continue;
    }
    unsigned short iSL = axialHits[ihit]->getISuperLayer();
    // // skip stereo hits (should not be related to track, but check anyway)
    if ((!neurotrackinputmode) && (iSL % 2 == 1)) continue;
    double relId = getRelId(*axialHits[ihit]);
    if (expert.isRelevant(relId, iSL)) {
      if (nHits[iSL] < expert.getMaxHitsPerSL()) {
        chitPattern |= 1 << (iSL + 9 * nHits[iSL]);
        ++nHits[iSL];
      }
    }
  }
  if (!neurotrackinputmode) {
    // loop over stereo hits
    for (int ihit = 0; ihit < m_segmentHits.getEntries(); ++ ihit) {
      unsigned short iSL = m_segmentHits[ihit]->getISuperLayer();
      // skip axial hits
      if (iSL % 2 == 0) continue;
      // get priority time
      double relId = getRelId(*m_segmentHits[ihit]);
      if (expert.isRelevant(relId, iSL)) {
        if (nHits[iSL] < expert.getMaxHitsPerSL()) {
          chitPattern |= 1 << (iSL + 9 * nHits[iSL]);
          ++nHits[iSL];
        }
      }
    }
  }
  return chitPattern;
}

unsigned long
NeuroTrigger::getInputPattern(unsigned isector, const CDCTriggerTrack& track, const bool neurotrackinputmode)
{
  CDCTriggerMLP& expert = m_MLPs[isector];
  unsigned long hitPattern = 0;
  vector<unsigned> nHits;
  nHits.assign(9, 0);
  // loop over axial hits related to input track
  RelationVector<CDCTriggerSegmentHit> axialHits =
    track.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
  for (unsigned ihit = 0; ihit < axialHits.size(); ++ ihit) {
    // skip hits with negative relation weight (not selected in finder)
    if (axialHits.weight(ihit) < 0) {
      continue;
    }
    unsigned short iSL = axialHits[ihit]->getISuperLayer();
    // // skip stereo hits (should not be related to track, but check anyway)
    if ((!neurotrackinputmode) && (iSL % 2 == 1)) continue;
    // get priority time
    int t = (m_hasT0) ? axialHits[ihit]->priorityTime() - m_T0 : 0;
    if (t < 0) {
      t = 0;
    } else if (t > expert.getTMax()) {
      t = expert.getTMax();
    }
    double relId = getRelId(*axialHits[ihit]);

    if (expert.isRelevant(relId, iSL)) {
      if (nHits[iSL] < expert.getMaxHitsPerSL()) {
        hitPattern |= 1 << (iSL + 9 * nHits[iSL]);
        ++nHits[iSL];
      }
      B2DEBUG(250, "hit in SL " << iSL);
    } else {
      B2DEBUG(250, "hit in SL " << iSL << " not relevant (relId = " << relId << ")");
    }
  }
  if (!neurotrackinputmode) {
    // loop over stereo hits
    for (int ihit = 0; ihit < m_segmentHits.getEntries(); ++ ihit) {
      unsigned short iSL = m_segmentHits[ihit]->getISuperLayer();
      // skip axial hits
      if (iSL % 2 == 0) continue;
      // get priority time
      int t = (m_hasT0) ? m_segmentHits[ihit]->priorityTime() - m_T0 : 0;
      if (t < 0) {
        t = 0;
      } else if (t > expert.getTMax()) {
        t = expert.getTMax();
      }
      double relId = getRelId(*m_segmentHits[ihit]);
      if (expert.isRelevant(relId, iSL)) {
        if (nHits[iSL] < expert.getMaxHitsPerSL()) {
          hitPattern |= 1 << (iSL + 9 * nHits[iSL]);
          ++nHits[iSL];
        }
        B2DEBUG(250, "hit in SL " << iSL);
      } else {
        B2DEBUG(250, "hit in SL " << iSL << " not relevant (relId = " << relId << ")");
      }
    }
  }
  B2DEBUG(250, "hitPattern " << hitPattern);
  return hitPattern & expert.getSLpatternMask();
}

vector<unsigned>
NeuroTrigger::selectHitsHWSim(unsigned isector, const CDCTriggerTrack& track)
{
  const CDCTriggerMLP& expert = m_MLPs[isector];
  vector<unsigned> selectedHitIds = {};
  // prepare vectors to keep best drift times, left/right and selected hit IDs
  vector<int> tMin;
  tMin.assign(expert.nNodesLayer(0), expert.getTMax());
  vector<bool> LRknown;
  LRknown.assign(expert.nNodesLayer(0), false);
  vector<int> hitIds;
  hitIds.assign(expert.nNodesLayer(0), -1);
  vector<unsigned> nHits;
  nHits.assign(9, 0);

  // loop over all hits related to input track
  RelationVector<CDCTriggerSegmentHit> allHits =
    track.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
  B2DEBUG(250, "start hit loop over all related hits");
  for (unsigned ihit = 0; ihit < allHits.size(); ++ihit) {
    // skip hits with negative relation weight (not selected in finder)
    if (allHits.weight(ihit) < 0) continue;
    unsigned short iSL = allHits[ihit]->getISuperLayer();
    if (expert.getSLpatternUnmasked() != 0 &&
        !((expert.getSLpatternUnmasked() >> iSL) & 1)) {
      B2DEBUG(250, "skipping hit in SL " << iSL);
      continue;
    }
    // get priority time and apply time window cut
    int t = (m_hasT0) ? allHits[ihit]->priorityTime() - m_T0 : 0;
    if (t < 0) {
      t = 0;
    } else if (t > expert.getTMax()) {
      t = expert.getTMax();
    }
    double relId = getRelId(*allHits[ihit]);
    if (expert.isRelevant(relId, iSL)) {
      // get reference hit (worst of existing hits)
      unsigned short iRef = iSL;
      if (expert.getMaxHitsPerSL() > 1) {
        if (nHits[iSL] < expert.getMaxHitsPerSL() &&
            (expert.getSLpatternUnmasked() >> (iSL + 9 * nHits[iSL])) & 1) {
          iRef += 9 * nHits[iSL];
          ++nHits[iSL];
        } else {
          for (unsigned compare = iSL; compare < iSL + 9 * nHits[iSL]; compare += 9) {
            if ((LRknown[iRef] && !LRknown[compare]) ||
                (LRknown[iRef] == LRknown[compare] && tMin[iRef] < tMin[compare]))
              iRef = compare;
          }
        }
      }
      // choose best hit (LR known before LR unknown, then shortest drift time)
      bool useHit = false;
      if (LRknown[iRef]) {
        useHit = (allHits[ihit]->LRknown() && t <= tMin[iRef]);
      } else {
        useHit = (allHits[ihit]->LRknown() || t <= tMin[iRef]);
      }
      B2DEBUG(250, "relevant wire SL " << iSL << " LR " << allHits[ihit]->getLeftRight()
              << " t " << t << " iRef " << iRef << " useHit " << useHit);
      if (useHit) {
        // keep drift time and LR
        LRknown[iRef] = allHits[ihit]->LRknown();
        tMin[iRef] = t;
        hitIds[iRef] = allHits[ihit]->getArrayIndex();
      }
    }
  }
  // save selected hit Ids
  for (unsigned iHit = 0; iHit < hitIds.size(); ++iHit) {
    if (hitIds[iHit] >= 0) selectedHitIds.push_back(hitIds[iHit]);
  }
  return selectedHitIds;
}
vector<unsigned>
NeuroTrigger::selectHits(unsigned isector, const CDCTriggerTrack& track,
                         bool returnAllRelevant)
{
  const CDCTriggerMLP& expert = m_MLPs[isector];
  vector<unsigned> selectedHitIds = {};
  // prepare vectors to keep best drift times, left/right and selected hit IDs
  vector<int> tMin;
  tMin.assign(expert.nNodesLayer(0), expert.getTMax());
  vector<bool> LRknown;
  LRknown.assign(expert.nNodesLayer(0), false);
  vector<int> hitIds;
  hitIds.assign(expert.nNodesLayer(0), -1);
  vector<unsigned> nHits;
  nHits.assign(9, 0);

  // loop over axial hits related to input track
  RelationVector<CDCTriggerSegmentHit> axialHits =
    track.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
  B2DEBUG(250, "start axial hit loop");
  for (unsigned ihit = 0; ihit < axialHits.size(); ++ihit) {
    // skip hits with negative relation weight (not selected in finder)
    if (axialHits.weight(ihit) < 0) continue;
    unsigned short iSL = axialHits[ihit]->getISuperLayer();
    // skip stereo hits (should not be related to track, but check anyway)
    if (iSL % 2 == 1) continue;
    if (expert.getSLpatternUnmasked() != 0 &&
        !((expert.getSLpatternUnmasked() >> iSL) & 1)) {
      B2DEBUG(250, "skipping hit in SL " << iSL);
      continue;
    }
    // get priority time and apply time window cut

    int t = (m_hasT0) ? axialHits[ihit]->priorityTime() - m_T0 : 0;
    if (t < 0) {
      t = 0;
    } else if (t > expert.getTMax()) {
      t = expert.getTMax();
    }
    double relId = getRelId(*axialHits[ihit]);
    if (expert.isRelevant(relId, iSL)) {
      // get reference hit (worst of existing hits)
      unsigned short iRef = iSL;
      if (expert.getMaxHitsPerSL() > 1) {
        if (nHits[iSL] < expert.getMaxHitsPerSL() &&
            (expert.getSLpatternUnmasked() >> (iSL + 9 * nHits[iSL])) & 1) {
          iRef += 9 * nHits[iSL];
          ++nHits[iSL];
        } else {
          for (unsigned compare = iSL; compare < iSL + 9 * nHits[iSL]; compare += 9) {
            if ((LRknown[iRef] && !LRknown[compare]) ||
                (LRknown[iRef] == LRknown[compare] && tMin[iRef] < tMin[compare]))
              iRef = compare;
          }
        }
      }
      // choose best hit (LR known before LR unknown, then shortest drift time)
      bool useHit = false;
      if (LRknown[iRef]) {
        useHit = (axialHits[ihit]->LRknown() && t <= tMin[iRef]);
      } else {
        useHit = (axialHits[ihit]->LRknown() || t <= tMin[iRef]);
      }
      B2DEBUG(250, "relevant wire SL " << iSL << " LR " << axialHits[ihit]->getLeftRight()
              << " t " << t << " iRef " << iRef << " useHit " << useHit);
      if (useHit) {
        // keep drift time and LR
        LRknown[iRef] = axialHits[ihit]->LRknown();
        tMin[iRef] = t;
        hitIds[iRef] = axialHits[ihit]->getArrayIndex();
      }
      if (returnAllRelevant) selectedHitIds.push_back(axialHits[ihit]->getArrayIndex());
    }
  }

  // loop over stereo hits, choosing up to MaxHitsPerSL per superlayer
  B2DEBUG(250, "start stereo hit loop");
  for (int ihit = 0; ihit < m_segmentHits.getEntries(); ++ ihit) {
    unsigned short iSL = m_segmentHits[ihit]->getISuperLayer();
    // skip axial hits
    if (iSL % 2 == 0) continue;
    if (expert.getSLpatternUnmasked() != 0 &&
        !((expert.getSLpatternUnmasked() >> iSL) & 1)) {
      B2DEBUG(250, "skipping hit in SL " << iSL);
      continue;
    }
    // get priority time and apply time window cut
    int t = (m_hasT0) ? m_segmentHits[ihit]->priorityTime() - m_T0 : 0;
    if (t < 0) {
      t = 0;
    } else if (t > expert.getTMax()) {
      t = expert.getTMax();
    }
    double relId = getRelId(*m_segmentHits[ihit]);
    if (expert.isRelevant(relId, iSL)) {
      // get reference hit (worst of existing hits)
      unsigned short iRef = iSL;
      if (expert.getMaxHitsPerSL() > 1) {
        if (nHits[iSL] < expert.getMaxHitsPerSL() &&
            (expert.getSLpatternUnmasked() >> (iSL + 9 * nHits[iSL])) & 1) {
          iRef += 9 * nHits[iSL];
          ++nHits[iSL];
        } else {
          for (unsigned compare = iSL; compare < iSL + 9 * nHits[iSL]; compare += 9) {
            if ((LRknown[iRef] && !LRknown[compare]) ||
                (LRknown[iRef] == LRknown[compare] && tMin[iRef] < tMin[compare]))
              iRef = compare;
          }
        }
      }
      // choose best hit (LR known before LR unknown, then shortest drift time)
      bool useHit = false;
      if (LRknown[iRef]) {
        useHit = (m_segmentHits[ihit]->LRknown() && t <= tMin[iRef]);
      } else {
        useHit = (m_segmentHits[ihit]->LRknown() || t <= tMin[iRef]);
      }
      B2DEBUG(250, "relevant wire SL " << iSL << " LR " << m_segmentHits[ihit]->getLeftRight()
              << " t " << t << " iRef " << iRef << " useHit " << useHit);
      if (useHit) {
        // keep drift time and LR
        LRknown[iRef] = m_segmentHits[ihit]->LRknown();
        tMin[iRef] = t;
        hitIds[iRef] = ihit;
      }
      if (returnAllRelevant) selectedHitIds.push_back(ihit);
    }
  }

  // save selected hit Ids
  if (!returnAllRelevant) {
    for (unsigned iHit = 0; iHit < hitIds.size(); ++iHit) {
      if (hitIds[iHit] >= 0) selectedHitIds.push_back(hitIds[iHit]);
    }
  }
  return selectedHitIds;
}

vector<float>
NeuroTrigger::getInputVector(unsigned isector, const vector<unsigned>& hitIds)
{
  const CDCTriggerMLP& expert = m_MLPs[isector];
  // prepare empty input vector and vectors to keep best drift times
  vector<float> inputVector;
  inputVector.assign(expert.nNodesLayer(0), 0.);
  // convert hits to input values
  vector<unsigned> nHits;
  nHits.assign(9, 0);
  for (unsigned ii = 0; ii < hitIds.size(); ++ii) {
    int ihit = hitIds[ii];
    unsigned short iSL = m_segmentHits[ihit]->getISuperLayer();
    unsigned short iRef = iSL + 9 * nHits[iSL];
    ++nHits[iSL];
    int t = (m_hasT0) ? m_segmentHits[ihit]->priorityTime() - m_T0 : 0;
    if (t < 0) {
      t = 0;
    } else if (t > expert.getTMax()) {
      t = expert.getTMax();
    }
    int LR = m_segmentHits[ihit]->getLeftRight();
    double relId = getRelId(*m_segmentHits[ihit]);
    int priority = m_segmentHits[ihit]->getPriorityPosition();
    // get scaled input values (scaling such that the absolute value of all inputs is < 1)
    inputVector[3 * iRef] = expert.scaleId(relId, iSL);
    float scaleT = pow(2, floor(log2(1. / expert.getTMax())));
    inputVector[3 * iRef + 1] = (((LR >> 1) & 1) - (LR & 1)) * t * scaleT;
    inputVector[3 * iRef + 2] = m_alpha[iSL][int(priority < 3)] * 0.5;
  }
  return inputVector;
}

vector<float>
NeuroTrigger::runMLP(unsigned isector, const vector<float>& input)
{
  const CDCTriggerMLP& expert = m_MLPs[isector];
  vector<float> weights = expert.getWeights();
  vector<float> layerinput = input;
  vector<float> layeroutput = {};
  unsigned iw = 0;
  for (unsigned il = 1; il < expert.nLayers(); ++il) {
    //add bias input
    layerinput.push_back(1.);
    //prepare output
    layeroutput.clear();
    layeroutput.assign(expert.nNodesLayer(il), 0.);
    //loop over outputs
    for (unsigned io = 0; io < layeroutput.size(); ++io) {
      //loop over inputs
      for (unsigned ii = 0; ii < layerinput.size(); ++ii) {
        layeroutput[io] += layerinput[ii] * weights[iw++];
      }
      //apply activation function
      layeroutput[io] = tanh(layeroutput[io] / 2.);
    }
    //output is new input
    layerinput = layeroutput;
  }
  return expert.unscaleTarget(layeroutput);
}

vector<float>
NeuroTrigger::runMLPFix(unsigned isector, vector<float> input)
{
  unsigned precisionInput = m_precision[3];
  unsigned precisionWeights = m_precision[4];
  unsigned precisionLUT = m_precision[5];
  unsigned precisionTanh = m_precision[3];
  unsigned dp = precisionInput + precisionWeights - precisionLUT;

  const CDCTriggerMLP& expert = m_MLPs[isector];
  // transform inputs to fixed point (cut off to input precision)
  vector<long> inputFix(input.size(), 0);
  for (unsigned ii = 0; ii < input.size(); ++ii) {
    inputFix[ii] = long(input[ii] * (1 << precisionInput));
  }
  // transform weights to fixed point (round to weight precision)
  vector<float> weights = expert.getWeights();
  vector<long> weightsFix(weights.size(), 0);
  for (unsigned iw = 0; iw < weights.size(); ++iw) {
    weightsFix[iw] = long(round(weights[iw] * (1 << precisionWeights)));
  }
  // maximum input value for the tanh LUT
  unsigned xMax = unsigned(ceil(atanh(1. - 1. / (1 << (precisionTanh + 1))) *
                                (1 << (precisionLUT + 1))));

  // run MLP
  vector<long> layerinput = inputFix;
  vector<long> layeroutput = {};
  unsigned iw = 0;
  for (unsigned il = 1; il < expert.nLayers(); ++il) {
    // add bias input
    layerinput.push_back(1 << precisionInput);
    // prepare output
    layeroutput.clear();
    layeroutput.assign(expert.nNodesLayer(il), 0);
    // loop over outputs
    for (unsigned io = 0; io < layeroutput.size(); ++io) {
      // loop over inputs
      for (unsigned ii = 0; ii < layerinput.size(); ++ii) {
        layeroutput[io] += layerinput[ii] * weightsFix[iw++];
      }
      // apply activation function -> LUT, calculated on the fly here
      unsigned long bin = abs(layeroutput[io]) >> dp;
      // correction to get symmetrical rounding errors
      float x = (bin + 0.5 - 1. / (1 << (dp + 1))) / (1 << precisionLUT);
      long tanhLUT = (bin < xMax) ? long(round(tanh(x / 2.) * (1 << precisionTanh))) : (1 << precisionTanh);
      layeroutput[io] = (layeroutput[io] < 0) ? -tanhLUT : tanhLUT;
    }
    // output is new input
    layerinput = layeroutput;
  }

  // transform output back to float before unscaling
  vector<float> output(layeroutput.size(), 0.);
  for (unsigned io = 0; io < output.size(); ++io) {
    output[io] = layeroutput[io] / float(1 << precisionTanh);
  }
  return expert.unscaleTarget(output);
}

void
NeuroTrigger::save(const string& filename, const string& arrayname)
{
  B2INFO("Saving networks to file " << filename << ", array " << arrayname);
  TFile datafile(filename.c_str(), "UPDATE");
  TObjArray* MLPs = new TObjArray(m_MLPs.size());
  for (unsigned isector = 0; isector < m_MLPs.size(); ++isector) {
    MLPs->Add(&m_MLPs[isector]);
  }
  MLPs->Write(arrayname.c_str(), TObject::kSingleKey | TObject::kOverwrite);
  datafile.Close();
  MLPs->Clear();
  delete MLPs;
}

bool
NeuroTrigger::load(const string& filename, const string& arrayname)
{
  if (filename.size() < 1) {
    m_MLPs.clear();
    m_MLPs = m_cdctriggerneuroconfig->getMLPs();
    if (m_MLPs.size() == 0) {
      B2ERROR("Could not load Neurotrigger weights from database!");
      return false;
    }
    B2DEBUG(2, "Loaded Neurotrigger MLP weights from database: " +  m_cdctriggerneuroconfig->getNNName());
    B2DEBUG(100, "loaded " << m_MLPs.size() << " networks from database");
    // load some values from the geometry that will be needed for the input
    setConstants();
    return true;
  } else {
    TFile datafile(filename.c_str(), "READ");
    if (!datafile.IsOpen()) {
      B2WARNING("Could not open file " << filename);
      return false;
    }

    TObjArray* MLPs = (TObjArray*)datafile.Get(arrayname.c_str());
    if (!MLPs) {
      datafile.Close();
      B2WARNING("File " << filename << " does not contain key " << arrayname);
      return false;
    }
    m_MLPs.clear();
    for (int isector = 0; isector < MLPs->GetEntriesFast(); ++isector) {
      CDCTriggerMLP* expert = dynamic_cast<CDCTriggerMLP*>(MLPs->At(isector));
      if (expert) m_MLPs.push_back(*expert);
      else B2WARNING("Wrong type " << MLPs->At(isector)->ClassName() << ", ignoring this entry.");
    }
    MLPs->Clear();
    delete MLPs;
    datafile.Close();
    B2DEBUG(100, "loaded " << m_MLPs.size() << " networks");

    // load some values from the geometry that will be needed for the input
    setConstants();

    return true;
  }
}
