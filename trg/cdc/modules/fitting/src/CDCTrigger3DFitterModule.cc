/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "trg/cdc/modules/fitting/CDCTrigger3DFitterModule.h"

#include <framework/datastore/RelationVector.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/geometry/CDCGeometryPar.h>

using namespace std;
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(CDCTrigger3DFitter);

CDCTrigger3DFitterModule::CDCTrigger3DFitterModule() : Module::Module()
{
  setDescription(
    "The 3D fitter module of the CDC trigger.\n"
    "1. Selects stereo hits around a given 2D track by Hough voting\n"
    "2. Performs a linear fit in the s-z plane (s: 2D arclength).\n"
  );
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("CDCHitCollectionName", m_CDCHitCollectionName,
           "Name of the input StoreArray of CDCHits.",
           string("CDCHits4Trg"));
  addParam("TSHitCollectionName", m_TSHitCollectionName,
           "Name of the input StoreArray of CDCTriggerSegmentHits.",
           string(""));
  addParam("EventTimeName", m_EventTimeName,
           "Name of the event time object.",
           string(""));
  addParam("inputCollectionName", m_inputCollectionName,
           "Name of the StoreArray holding the input tracks from the 2D finder.",
           string("TRGCDC2DFinderTracks"));
  addParam("outputCollectionName", m_outputCollectionName,
           "Name of the StoreArray holding the 3D output tracks.",
           string("TRGCDC3DFitterTracks"));
  addParam("minVoteCounts", m_minVoteCount,
           "Minimal number of votes in Hough voting.",
           8);
}

void
CDCTrigger3DFitterModule::initialize()
{
  // register DataStore elements
  m_tracks3D.registerInDataStore(m_outputCollectionName);
  m_tracks2D.isRequired(m_inputCollectionName);
  m_TSHits.isRequired(m_TSHitCollectionName);
  m_eventTime.isRequired(m_EventTimeName);
  // register relations
  m_tracks2D.registerRelationTo(m_tracks3D);
  m_tracks2D.requireRelationTo(m_TSHits);
  m_tracks3D.registerRelationTo(m_TSHits);


  // get CDC geometry constants

  // load parameters from database
  const CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();

  m_xtCurve.resize(m_nStereoLayer, vector<double>(m_maxDriftTime));
  m_nWire.resize(m_nStereoLayer, 0);
  m_rWire.resize(m_nStereoLayer, 0);
  m_phiBW.resize(m_nStereoLayer, vector<double>(cdc.getMaxNumberOfCellsPerLayer()));
  m_zBW.resize(m_nStereoLayer, 0);
  m_stereoAngle.resize(m_nStereoLayer, 0);

  for (int iLayer = 0; iLayer < m_nStereoLayer; ++iLayer) {
    // stereo layer ID (0--19) -> continuous layer ID (0--55)
    int iStereoSuperLayer = iLayer / m_nLayerInSuperLayer;
    int iLayerInSL = iLayer % m_nLayerInSuperLayer;
    int iCLayer = 8 + 12 * iStereoSuperLayer + iLayerInSL;

    // get x-t curve
    for (int iTick = 0; iTick < (int)m_xtCurve[iLayer].size(); ++iTick) {
      double t = iTick * 2 * cdc.getTdcBinWidth();
      // take the average of left & right
      double driftLength_left = cdc.getDriftLength(t, iCLayer, 0);
      double driftLength_right = cdc.getDriftLength(t, iCLayer, 1);
      m_xtCurve[iLayer][iTick] = (driftLength_left + driftLength_right) / 2;
    }

    // get the number of wires
    m_nWire[iLayer] = cdc.nWiresInLayer(iCLayer);

    // get the wire radius
    m_rWire[iLayer] = cdc.senseWireR(iCLayer);

    // get phi coordinate of the backward endpoint of the wire
    for (int iWire = 0; iWire < m_nWire[iLayer]; iWire++) {
      const B2Vector3D& wirePosBW = cdc.wireBackwardPosition(iCLayer, iWire);
      m_phiBW[iLayer][iWire] = wirePosBW.Phi();
    }

    // get z coordinate of the of the backward endpoint of the wire
    // get stereo angle
    const B2Vector3D& wirePosB = cdc.wireBackwardPosition(iCLayer, 0);
    const B2Vector3D& wirePosF = cdc.wireForwardPosition(iCLayer, 0);
    const B2Vector3D wireDirection = wirePosF - wirePosB;
    m_zBW[iLayer] = wirePosB.Z();
    m_stereoAngle[iLayer] = wireDirection.Theta();
    if (wirePosF.Phi() < wirePosB.Phi())m_stereoAngle[iLayer] *= -1;
  }
}

void
CDCTrigger3DFitterModule::event()
{
  for (int iTrack = 0; iTrack < m_tracks2D.getEntries(); ++iTrack) {
    // get 2D variable
    double phi0 = m_tracks2D[iTrack]->getPhi0(); // (rad)
    double omega = m_tracks2D[iTrack]->getOmega(); // curvature (cm^-1)

    vector<vector<CDCHit*>> preselectedHits = preselector(phi0, omega);

    vector<vector<double>> sCand = sConverter(preselectedHits, omega);
    vector<vector<double>> zCand = zConverter(preselectedHits, phi0, omega);

    HoughVoter voter = HoughVoter();
    voter.vote(sCand, zCand);
    auto [z0Voter, cotVoter, voteCount] = voter.findPeak();

    // skip if vote count is not enough
    if (voteCount < m_minVoteCount) {
      B2DEBUG(100, "vote count is lower than threshold (in 3D Fitter)");
      continue;
    }

    auto [s, z] = selector(sCand, zCand, z0Voter, cotVoter);

    auto [z0, cot] = fitter(s, z);

    // check if fit results are valid
    if (isnan(z0) || isnan(cot)) {
      B2DEBUG(100, "3D fit failed");
      continue;
    }

    // save track
    CDCTriggerTrack* fittedTrack =
      m_tracks3D.appendNew(m_tracks2D[iTrack]->getPhi0(), m_tracks2D[iTrack]->getOmega(),
                           m_tracks2D[iTrack]->getChi2D(),
                           z0, cot, 0);
    // make relation to 2D track
    m_tracks2D[iTrack]->addRelationTo(fittedTrack);
  } // end iTrack loop
}

// major functions

vector<vector<CDCHit*>>
                     CDCTrigger3DFitterModule::preselector(double phi0, double omega)
{
  vector<vector<CDCHit*>> preselectedHits(m_nStereoLayer, vector<CDCHit*>());

  vector<int> iWireBeginList = getIWireBegin(phi0, omega);

  // judge if each hit is in the preselection range
  for (int iTS = 0; iTS < m_TSHits.getEntries(); iTS++) {
    CDCTriggerSegmentHit* TS = m_TSHits[iTS];
    if (TS->getISuperLayer() % 2 == 0)continue; // skip axial TS

    RelationVector<CDCHit> hits = TS->getRelationsTo<CDCHit>(m_CDCHitCollectionName);

    vector<int> iHitInEachLayer = select5Cells(TS);

    for (int iHit : iHitInEachLayer) {
      if (iHit == -1)continue;

      int iLayer = getIStereoLayer(hits[iHit]->getICLayer());
      int iWire = hits[iHit]->getIWire();

      if (iWireBeginList[iLayer] == -1) continue;

      if (iWireBeginList[iLayer] + 10 - 1 < m_nWire[iLayer]) { // preselection range doesn't cross over iWire = 0
        if (iWireBeginList[iLayer] <= iWire && iWire < iWireBeginList[iLayer] + 10) {
          preselectedHits[iLayer].push_back(hits[iHit]);
        }
      } else { // preselection range crosses over iWire = 0
        if (iWireBeginList[iLayer] <= iWire || iWire < (iWireBeginList[iLayer] + 10) % m_nWire[iLayer]) {
          preselectedHits[iLayer].push_back(hits[iHit]);
        }
      }
    }
  }
  return preselectedHits;
}

vector<vector<double>>
                    CDCTrigger3DFitterModule::sConverter(const vector<vector<CDCHit*>>& preselectedHits, double omega)
{
  vector<vector<double>> s(m_nStereoLayer, vector<double>());

  for (int iLayer = 0; iLayer < m_nStereoLayer; iLayer++) {
    for (int iHit = 0; iHit < (int)preselectedHits[iLayer].size(); iHit++) {
      for (int lr = 1; lr <= 2; lr++) {
        double _s = 2 / omega * asin(1.0 / 2 * m_rWire[iLayer] * omega);
        s[iLayer].push_back(_s);
      }
    }
  }
  return s;
}

vector<vector<double>>
                    CDCTrigger3DFitterModule::zConverter(const vector<vector<CDCHit*>>& preselectedHits, double phi0, double omega)
{
  vector<vector<double>> z(m_nStereoLayer, vector<double>());

  for (int iLayer = 0; iLayer < m_nStereoLayer; iLayer++) {
    for (CDCHit* hit : preselectedHits[iLayer]) {
      double driftLength = getDriftLength(*hit);

      double phiCross = calPhiCross(m_rWire[iLayer], phi0, omega);
      for (int lr = 1; lr <= 2; lr++) {
        double phiHit;
        if (lr == 1) { // right
          phiHit = phiCross + driftLength / m_rWire[iLayer];
        } else if (lr == 2) { // left
          phiHit = phiCross - driftLength / m_rWire[iLayer];
        }
        double deltaPhi = normalizeAngle(phiHit - m_phiBW[iLayer][hit->getIWire()]);
        double _z = m_zBW[iLayer] + m_rWire[iLayer] * deltaPhi / tan(m_stereoAngle[iLayer]);
        z[iLayer].push_back(_z);
      }
    }
  }
  return z;
}

CDCTrigger3DFitterModule::HoughVoter::HoughVoter()
{
  votingCell.resize(nCellZ0, vector<int>(nCellZ0, 0));
}

void
CDCTrigger3DFitterModule::HoughVoter::vote(const vector<vector<double>>& sCand, const vector<vector<double>>& zCand)
{
  //              iCot=0(minCot) ... iCot=m(maxCot)
  // iZ0=0(maxZ0)
  // ...
  // iZ0=n(minZ0)
  for (int iLayer = 0; iLayer < m_nStereoLayer; iLayer++) {
    for (int iHit = 0; iHit < (int)sCand[iLayer].size(); iHit++) {
      double s = sCand[iLayer][iHit];
      double z = zCand[iLayer][iHit];

      for (int iCot = 0; iCot < nCellCot; iCot++) {
        int iZ0Shift = floor((z - (-cellWidthZ0 / 2)) / cellWidthZ0);

        double cotLower = getCotCellValue(iCot - 0.5);
        double z0Lower = -s * cotLower;
        int iZ0Lower = digitizeZ0(z0Lower) - iZ0Shift;

        double cotUpper = getCotCellValue(iCot + 0.5);
        double z0Upper = -s * cotUpper;
        int iZ0Upper = digitizeZ0(z0Upper) - iZ0Shift;

        for (int iZ0 = max(iZ0Lower, 0); iZ0 < min(iZ0Upper + 1, nCellZ0); iZ0++) {
          votingCell[iZ0][iCot] |= (1 << iLayer);
        }
      }
    }
  }
}

tuple<double, double, int>
CDCTrigger3DFitterModule::HoughVoter::findPeak()
{
  int maxCount = 0;
  double z0, cot;
  for (int iZ0 = 0; iZ0 < nCellZ0; iZ0++) {
    for (int iCot = nCellCot - 1; iCot >= 0; iCot--) {
      int count = __builtin_popcount(votingCell[iZ0][iCot]);
      if (maxCount <= count) {
        maxCount = count;
        z0 = getZ0CellValue(iZ0);
        cot = getCotCellValue(iCot);
      }
    }
  }
  return make_tuple(z0, cot, maxCount);
}

pair<vector<double>, vector<double>>
                                  CDCTrigger3DFitterModule::selector(const vector<vector<double>>& sCand, const vector<vector<double>>& zCand, double z0, double cot)
{
  vector<double> sSelected(m_nStereoLayer, -1); // s is always positive, so -1 means no hit
  vector<double> zSelected(m_nStereoLayer, -1);

  for (int iLayer = 0; iLayer < m_nStereoLayer; iLayer++) {
    double minDeltaZ = m_maxZSelection;
    for (int iHit = 0; iHit < (int)sCand[iLayer].size(); iHit++) {
      double zVoter = sCand[iLayer][iHit] * cot + z0;
      double deltaZ = abs(zCand[iLayer][iHit] - zVoter);
      if (deltaZ < minDeltaZ) {
        minDeltaZ = deltaZ;
        sSelected[iLayer] = sCand[iLayer][iHit];
        zSelected[iLayer] = zCand[iLayer][iHit];
      }
    }
  }

  return make_pair(sSelected, zSelected);
}

pair<double, double>
CDCTrigger3DFitterModule::fitter(const vector<double>& s, const vector<double>& z)
{
  // sigma parameter(=wire resolution) is set as 0 to reduce the calculation in firmware
  double nHit = 0, sum_s = 0, sum_z = 0, sum_sz = 0, sum_ss = 0;
  for (int iLayer = 0; iLayer < m_nStereoLayer; iLayer++) {
    // skip no hit layer
    if (s[iLayer] < 0)continue;

    nHit += 1;
    sum_s += s[iLayer];
    sum_z += z[iLayer];
    sum_sz += s[iLayer] * z[iLayer];
    sum_ss += s[iLayer] * s[iLayer];
  }

  double denominator = nHit * sum_ss - sum_s * sum_s;
  double z0 = (-sum_s * sum_sz + sum_ss * sum_z) / denominator;
  double cot = (nHit * sum_sz - sum_s * sum_z) / denominator;
  return make_pair(z0, cot);
}


// minor functions

int
CDCTrigger3DFitterModule::getIStereoLayer(int iContinuousLayer)
{
  int iStereoSuperLayer = (iContinuousLayer - 8) / 12;
  int iLayerInSL = iContinuousLayer - 8 - 12 * iStereoSuperLayer;
  return iStereoSuperLayer * m_nLayerInSuperLayer + iLayerInSL;
}

double
CDCTrigger3DFitterModule::normalizeAngle(double angle)
{
  return atan2(sin(angle), cos(angle));
}

double
CDCTrigger3DFitterModule::calPhiCross(double r, double phi0, double omega)
{
  // Based on Sara's thesis P84 eq.(6.4)
  // https://docs.belle2.org/record/823/
  double phiCross = phi0 - asin(1.0 / 2 * r * omega);
  return normalizeAngle(phiCross);
}

vector<int>
CDCTrigger3DFitterModule::getIWireBegin(double phi0, double omega)
{
  vector<int> iWireBeginList(m_nStereoLayer, -1);

  for (int iLayer = 0; iLayer < m_nStereoLayer; iLayer++) {
    int iStereoSuperLayer = iLayer / m_nLayerInSuperLayer;
    int iLayerInSL = iLayer % m_nLayerInSuperLayer;

    // Skip if track and hit don't intersect
    if (abs(1.0 / 2 * m_rWire[iLayer] * omega) > 1) continue;

    double phiCross = calPhiCross(m_rWire[iLayer], phi0, omega);
    if (phiCross < 0) phiCross += 2 * M_PI; // phiCross is in [0, 2pi]
    // convert phi -> wire ID
    double iWireCross = phiCross / (2 * M_PI) * m_nWire[iLayer];
    if (iLayerInSL % 2 == 1) iWireCross -= 0.5; // odd layer is shifted by the half cell

    // iWireBegin <= iWireCross < iWireEnd
    int iWireBegin;
    if (iStereoSuperLayer == 0 || iStereoSuperLayer == 2) { // phiFW > phiBW
      int iWireEnd;
      iWireEnd = (int)(ceil(iWireCross)) % m_nWire[iLayer];
      iWireBegin = (iWireEnd - 10 + m_nWire[iLayer]) % m_nWire[iLayer];
    } else { // phiBW > phiFW
      iWireBegin = (int)(ceil(iWireCross)) % m_nWire[iLayer];
    }
    iWireBeginList[iLayer] = iWireBegin;
  }
  return iWireBeginList;
}

vector<int>
CDCTrigger3DFitterModule::select5Cells(const CDCTriggerSegmentHit* TS)
{
  int priorityILayer = WireID(TS->getID()).getICLayer();
  int priorityIWire = TS->getIWire();
  RelationVector<CDCHit> hits = TS->getRelationsTo<CDCHit>(m_CDCHitCollectionName);

  // fill iHit of each cell
  vector<int> iHitInEachCell(m_nCellInTS, -1);
  for (int iHit = 0; iHit < (int)hits.size(); iHit++) {
    CDCHit hit = *hits[iHit];

    // get iTSCell
    // 10 9 8
    //   7 6
    //    5
    //   4 3
    //  2 1 0

    int iLayer = getIStereoLayer(hit.getICLayer());

    // get relative position of the wire to the priority wire
    int diffILayer = hit.getICLayer() - priorityILayer;
    int diffIWire = hit.getIWire() - priorityIWire;
    if (diffIWire < -2) diffIWire += m_nWire[iLayer];
    if (diffIWire > 2)  diffIWire -= m_nWire[iLayer];

    // 2nd priority case
    if (TS->getPriorityPosition() == 1) { // upper right
      diffILayer += 1;
      diffIWire -= 1;
    } else if (TS->getPriorityPosition() == 2) { // upper left
      diffILayer += 1;
    }

    int iTSCell = m_cellIDInTS[make_pair(diffILayer, diffIWire)];
    iHitInEachCell[iTSCell] = iHit;
  }

  // select
  vector<int> iHitInEachLayer(m_nLayerInSuperLayer, -1);
  vector<vector<int>> selectOrder{{1, 0, 2}, {3, 4}, {5}, {7, 6}, {9, 10, 8}};
  for (int iLayerInSL = 0; iLayerInSL < m_nLayerInSuperLayer; iLayerInSL++) {
    for (int iCell : selectOrder[iLayerInSL]) {
      if (iHitInEachCell[iCell] == -1) continue;
      iHitInEachLayer[iLayerInSL] = iHitInEachCell[iCell];
      break;
    }
  }

  return iHitInEachLayer;
}

double
CDCTrigger3DFitterModule::getDriftLength(const CDCHit hit)
{
  ;
  const CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();

  double driftTime = 0;
  if (m_eventTime->hasBinnedEventT0(Const::CDC)) {
    double T0 = m_eventTime->getBinnedEventT0(Const::CDC); // ns
    T0 = T0 / 2; // TRGCLK
    int hitTime = (int)floor((cdc.getT0(WireID(hit.getID())) / cdc.getTdcBinWidth() - hit.getTDCCount() + 0.5) / 2); // TRGCLK
    // reference: CDCTriggerTSFModule.cc:431
    driftTime = hitTime - T0;
  }
  if (driftTime < 0)driftTime = 0;
  if (driftTime >= m_maxDriftTime)driftTime = m_maxDriftTime - 1;

  int iLayer = getIStereoLayer(hit.getICLayer());
  return m_xtCurve[iLayer][driftTime];
}
