#include "trg/cdc/modules/fitting/CDCTrigger3DFitterModule.h"

#include <framework/datastore/RelationVector.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <trg/cdc/Fitter3DUtility.h>

using namespace std;
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(CDCTrigger3DFitter);

CDCTrigger3DFitterModule::CDCTrigger3DFitterModule() : Module::Module()
{
  setDescription(
    "The 3D fitter module of the CDC trigger.\n"
    "Selects stereo hits around a given 2D track and performs a linear fit "
    "in the s-z plane (s: 2D arclength).\n"
  );

  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the input StoreArray of CDCTriggerSegmentHits.",
           string(""));
  addParam("EventTimeName", m_EventTimeName,
           "Name of the event time object.",
           string(""));
  addParam("inputCollectionName", m_inputCollectionName,
           "Name of the StoreArray holding the input tracks from the 2D fitter.",
           string("TRGCDC2DFitterTracks"));
  addParam("outputCollectionName", m_outputCollectionName,
           "Name of the StoreArray holding the 3D output tracks.",
           string("TRGCDC3DFitterTracks"));
  addParam("minHits", m_minHits,
           "Minimal number of hits required for the fitting.",
           unsigned(2));
  addParam("xtSimple", m_xtSimple,
           "If true, use nominal drift velocity, otherwise use table "
           "for non-linear xt.",
           false);
}

void
CDCTrigger3DFitterModule::initialize()
{
  // register DataStore elements
  m_tracks3D.registerInDataStore(m_outputCollectionName);
  m_tracks2D.isRequired(m_inputCollectionName);
  m_hits.isRequired(m_hitCollectionName);
  m_eventTime.isRequired(m_EventTimeName);
  // register relations
  m_tracks2D.registerRelationTo(m_tracks3D);
  m_tracks2D.requireRelationTo(m_hits);
  m_tracks3D.registerRelationTo(m_hits);

  // get geometry constants for first priority layers
  CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();
  // TODO: avoid hard coding the priority layers here
  vector<unsigned> iL = {10, 22, 34, 46};
  for (int iSt = 0; iSt < 4; ++iSt) {
    nWires.push_back(cdc.nWiresInLayer(iL[iSt]));
    rr.push_back(cdc.senseWireR(iL[iSt]));
    zToStraw.push_back(cdc.senseWireBZ(iL[iSt]));
    nShift.push_back(cdc.nShifts(iL[iSt]));
    angleSt.push_back(2 * rr.back() * sin(M_PI * nShift.back() / (2 * nWires.back())) /
                      (cdc.senseWireFZ(iL[iSt]) - zToStraw.back()));
    vector<double> xt(512);
    for (unsigned iTick = 0; iTick < xt.size(); ++iTick) {
      double t = iTick * 2 * cdc.getTdcBinWidth();
      if (m_xtSimple) {
        xt[iTick] = cdc.getNominalDriftV() * t;
      } else {
        double driftLength_0 = cdc.getDriftLength(t, iL[iSt], 0);
        double driftLength_1 = cdc.getDriftLength(t, iL[iSt], 1);
        xt[iTick] = (driftLength_0 + driftLength_1) / 2;
      }
    }
    xtTables.push_back(xt);
  }
}

void
CDCTrigger3DFitterModule::event()
{
  for (int itrack = 0; itrack < m_tracks2D.getEntries(); ++itrack) {
    int charge = m_tracks2D[itrack]->getChargeSign();
    double rho = 1. / abs(m_tracks2D[itrack]->getOmega());
    double phi = m_tracks2D[itrack]->getPhi0() - charge * M_PI_2;

    // select stereo hits
    vector<int> bestTSIndex(4, -1);
    vector<double> bestTSPhi(4, 9999);
    finder(charge, rho, phi, bestTSIndex, bestTSPhi);

    // count the number of selected hits
    unsigned nHits = 0;
    for (unsigned iSt = 0; iSt < 4; ++iSt) {
      if (bestTSIndex[iSt] != -1) {
        nHits += 1;
      }
    }
    if (nHits < m_minHits) {
      B2DEBUG(100, "Not enough hits to do 3D fit (" << m_minHits << " needed, got " << nHits << ")");
      continue;
    }

    // do the fit and create a new track
    double z0 = 0;
    double cot = 0;
    double chi2 = 0;
    fitter(bestTSIndex, bestTSPhi, charge, rho, phi, z0, cot, chi2);

    // check if fit results are valid
    if (isnan(z0) || isnan(cot) || isnan(chi2)) {
      B2DEBUG(100, "3D fit failed");
      continue;
    }

    // save track
    CDCTriggerTrack* fittedTrack =
      m_tracks3D.appendNew(m_tracks2D[itrack]->getPhi0(), m_tracks2D[itrack]->getOmega(),
                           m_tracks2D[itrack]->getChi2D(),
                           z0, cot, chi2);
    // make relation to 2D track
    m_tracks2D[itrack]->addRelationTo(fittedTrack);
    // make relation to hits
    for (unsigned iSt = 0; iSt < 4; ++iSt) {
      if (bestTSIndex[iSt] != -1)
        fittedTrack->addRelationTo(m_hits[bestTSIndex[iSt]]);
    }
    // add axial relations from 2D track
    RelationVector<CDCTriggerSegmentHit> axialHits =
      m_tracks2D[itrack]->getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
    for (unsigned ihit = 0; ihit < axialHits.size(); ++ihit) {
      fittedTrack->addRelationTo(axialHits[ihit]);
    }
  }
}

void
CDCTrigger3DFitterModule::finder(int charge, double rho, double phi,
                                 vector<int>& bestTSIndex, vector<double>& bestTSPhi)
{
  vector<double> stAxPhi(4);
  for (int iSt = 0; iSt < 4; ++iSt) {
    stAxPhi[iSt] = Fitter3DUtility::calStAxPhi(charge, angleSt[iSt], zToStraw[iSt],
                                               rr[iSt], rho, phi);
  }
  // get candidates
  vector<vector<int>> candidatesIndex(4, vector<int>());
  vector<vector<double>> candidatesPhi(4, vector<double>());
  vector<vector<double>> candidatesDiffStWires(4, vector<double>());
  for (int ihit = 0; ihit < m_hits.getEntries(); ++ihit) {
    // Reject second priority TSs.
    if (m_hits[ihit]->getPriorityPosition() != 3) continue;
    // only stereo hits
    unsigned iSL = m_hits[ihit]->getISuperLayer();
    if (iSL % 2 == 0) continue;
    // skip hits with too large radius
    if (2 * rho < rr[iSL / 2]) continue;
    // Find number of wire difference
    double wirePhi = m_hits[ihit]->getIWire() * 2. * M_PI / nWires[iSL / 2];
    double tsDiffSt = stAxPhi[iSL / 2] - wirePhi;
    if (tsDiffSt > M_PI) tsDiffSt -= 2 * M_PI;
    if (tsDiffSt < -M_PI) tsDiffSt += 2 * M_PI;
    tsDiffSt = tsDiffSt / 2 / M_PI * nWires[iSL / 2];
    // Save index if condition is in 10 wires
    if ((iSL / 2) % 2 == 0) {
      if (tsDiffSt > 0 && tsDiffSt <= 10) {
        candidatesIndex[iSL / 2].push_back(ihit);
        candidatesPhi[iSL / 2].push_back(wirePhi);
        candidatesDiffStWires[iSL / 2].push_back(tsDiffSt);
      }
    } else {
      if (tsDiffSt < 0 && tsDiffSt >= -10) {
        candidatesIndex[iSL / 2].push_back(ihit);
        candidatesPhi[iSL / 2].push_back(wirePhi);
        candidatesDiffStWires[iSL / 2].push_back(tsDiffSt);
      }
    } // End of saving index
  } // Candidate loop

  // Pick middle candidate if multiple candidates
  // mean wire diff
  double meanWireDiff[4] = { 3.68186, 3.3542, 3.9099, 4.48263 };
  for (int iSt = 0; iSt < 4; ++iSt) {
    double bestDiff = 9999;
    for (int iTS = 0; iTS < int(candidatesIndex[iSt].size()); ++iTS) {
      double diff = abs(abs(candidatesDiffStWires[iSt][iTS]) - meanWireDiff[iSt]);
      // Pick the better TS
      if (diff < bestDiff) {
        bestDiff = diff;
        bestTSPhi[iSt] = candidatesPhi[iSt][iTS];
        bestTSIndex[iSt] = candidatesIndex[iSt][iTS];
      }
    } // TS loop
  } // Layer loop
}

void
CDCTrigger3DFitterModule::fitter(vector<int>& bestTSIndex, vector<double>& bestTSPhi,
                                 int charge, double rho, double phi,
                                 double& z0, double& cot, double& chi2)
{
  int T0 = (m_eventTime->hasBinnedEventT0(Const::CDC))
           ? m_eventTime->getBinnedEventT0(Const::CDC)
           : 9999;

  // Fill information for stereo layers
  vector<double> wirePhi(4, 9999);
  vector<unsigned> LR(4, 0);
  vector<int> driftTime(4, 9999);
  for (unsigned iSt = 0; iSt < 4; ++iSt) {
    if (bestTSIndex[iSt] != -1) {
      wirePhi[iSt] = bestTSPhi[iSt];
      LR[iSt] = m_hits[bestTSIndex[iSt]]->getLeftRight();
      driftTime[iSt] = m_hits[bestTSIndex[iSt]]->priorityTime();
    }
  } // End superlayer loop

  // Calculate phi3D.
  vector<double> phi3D(4, 9999);
  if (T0 == 9999) {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      phi3D[iSt] = wirePhi[iSt];
    }
  } else {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      if (bestTSIndex[iSt] != -1) {
        // Get drift length from table.
        int t = driftTime[iSt] - T0;
        if (t < 0) t = 0;
        if (t > 511) t = 511;
        double driftLength = xtTables[iSt][t];
        phi3D[iSt] = Fitter3DUtility::calPhi(wirePhi[iSt], driftLength, rr[iSt], LR[iSt]);
      }
    }
  }
  // Get zerror for 3D fit
  vector<double> driftZError({0.7676, 0.9753, 1.029, 1.372});
  vector<double> wireZError({0.7676, 0.9753, 1.029, 1.372});
  vector<double> zError(4, 9999);
  vector<double> invZError2(4, 0);
  for (unsigned iSt = 0; iSt < 4; ++iSt) {
    if (bestTSIndex[iSt] != -1) {
      // Check LR and eventTime
      if (LR[iSt] != 3 && T0 != 9999) zError[iSt] = driftZError[iSt];
      else zError[iSt] = wireZError[iSt];
      // Get inverse zerror ^ 2
      invZError2[iSt] = 1 / pow(zError[iSt], 2);
    }
  }

  // Calculate zz and arcS
  vector<double> zz(4, 0);
  vector<double> arcS(4, 0);
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    if (bestTSIndex[iSt] != -1) {
      zz[iSt] = Fitter3DUtility::calZ(charge, angleSt[iSt], zToStraw[iSt],
                                      rr[iSt], phi3D[iSt], rho, phi);
      arcS[iSt] = Fitter3DUtility::calS(rho, rr[iSt]);
    }
  }

  // Fit3D
  Fitter3DUtility::rSFit(&invZError2[0], &arcS[0], &zz[0], z0, cot, chi2);
}
