#include "trg/cdc/modules/fitting/CDCTrigger3DFitterModule.h"

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationVector.h>

#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/trg/dataobjects/TRGTiming.h>

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

  addParam("inputCollection", m_inputCollectionName,
           "Name of the StoreArray holding the input tracks from the 2D fitter.",
           string("Trg2DFitterTracks"));
  addParam("outputCollection", m_outputCollectionName,
           "Name of the StoreArray holding the 3D output tracks.",
           string("Trg3DFitterTracks"));
  addParam("xtSimple", m_xtSimple,
           "If true, use nominal drift velocity, otherwise use table "
           "for non-linear xt.",
           false);
}

void
CDCTrigger3DFitterModule::initialize()
{
  // register DataStore elements
  StoreArray<CDCTriggerTrack>::registerPersistent(m_outputCollectionName);
  StoreArray<CDCTriggerTrack>::required(m_inputCollectionName);
  StoreArray<CDCTriggerSegmentHit>::required();
  // register relations
  StoreArray<CDCTriggerTrack> tracks2D(m_inputCollectionName);
  StoreArray<CDCTriggerTrack> tracks3D(m_outputCollectionName);
  StoreArray<CDCTriggerSegmentHit> segmentHits;
  tracks3D.registerRelationTo(tracks2D);
  tracks3D.registerRelationTo(segmentHits);

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
  StoreArray<CDCTriggerTrack> tracks2D(m_inputCollectionName);
  StoreArray<CDCTriggerTrack> tracks3D(m_outputCollectionName);
  StoreArray<CDCTriggerSegmentHit> hits;
  StoreObjPtr<TRGTiming> eventTime("CDCTriggerEventTime");
  int T0 = eventTime->getTiming();

  for (int itrack = 0; itrack < tracks2D.getEntries(); ++itrack) {
    // select stereo hits
    int charge = tracks2D[itrack]->getChargeSign();
    double rho = 1. / abs(tracks2D[itrack]->getOmega());
    double fitPhi0 = tracks2D[itrack]->getPhi0() - charge * M_PI_2;
    vector<double> stAxPhi(4);
    for (int iSt = 0; iSt < 4; ++iSt) {
      stAxPhi[iSt] = Fitter3DUtility::calStAxPhi(charge, angleSt[iSt], zToStraw[iSt],
                                                 rr[iSt], rho, fitPhi0);
    }
    // get candidates
    vector<vector<int>> candidatesIndex(4, vector<int>());
    vector<vector<double>> candidatesPhi(4, vector<double>());
    vector<vector<double>> candidatesDiffStWires(4, vector<double>());
    for (int ihit = 0; ihit < hits.getEntries(); ++ihit) {
      // Reject second priority TSs.
      if (hits[ihit]->getPriorityPosition() != 3) continue;
      // only stereo hits
      unsigned iSL = hits[ihit]->getISuperLayer();
      if (iSL % 2 == 0) continue;
      // skip hits with too large radius
      if (2 * rho < rr[iSL / 2]) continue;
      // Find number of wire difference
      double wirePhi = hits[ihit]->getIWire() * 2. * M_PI / nWires[iSL / 2];
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
    vector<int> bestTSIndex(4, -1);
    vector<double> bestTSPhi(4, 9999);
    unsigned nHits = 0;
    for (int iSt = 0; iSt < 4; ++iSt) {
      double bestDiff = 9999;
      for (int iTS = 0; iTS < int(candidatesIndex[iSt].size()); ++iTS) {
        double diff = abs(abs(candidatesDiffStWires[iSt][iTS]) - meanWireDiff[iSt]);
        // Pick the better TS
        if (diff < bestDiff) {
          if (bestTSIndex[iSt] == -1) {
            nHits += 1;
          }
          bestDiff = diff;
          bestTSPhi[iSt] = candidatesPhi[iSt][iTS];
          bestTSIndex[iSt] = candidatesIndex[iSt][iTS];
        }
      } // TS loop
    } // Layer loop

    // do the fit and create a new track

    // Fill information for stereo layers
    vector<double> wirePhi(4, 9999);
    vector<unsigned> LR(4, 0);
    vector<int> driftTime(4, 9999);
    for (unsigned iSt = 0; iSt < 4; ++iSt) {
      if (bestTSIndex[iSt] != -1) {
        //m_mVector["tsId"][iSt * 2 + 1] = t_segment->localId();
        wirePhi[iSt] = bestTSPhi[iSt];
        LR[iSt] = hits[bestTSIndex[iSt]]->getLeftRight();
        driftTime[iSt] = hits[bestTSIndex[iSt]]->priorityTime();
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
                                        rr[iSt], phi3D[iSt], rho, fitPhi0);
        arcS[iSt] = Fitter3DUtility::calS(rho, rr[iSt]);
      }
    }

    // Fit3D
    double z0 = 0;
    double cot = 0;
    double zChi2 = 0;
    Fitter3DUtility::rSFit(&invZError2[0], &arcS[0], &zz[0], z0, cot, zChi2);

    // For failed fits. When cot is 0 or nan.
    if (cot == 0 || std::isnan(cot)) {
      B2INFO("3D fit failed");
      continue;
    }

    // save track
    CDCTriggerTrack* fittedTrack =
      tracks3D.appendNew(tracks2D[itrack]->getPhi0(), tracks2D[itrack]->getOmega(),
                         tracks2D[itrack]->getChi2D(),
                         z0, cot, zChi2);
    // make relation to 2D track
    fittedTrack->addRelationTo(tracks2D[itrack]);
    // make relation to hits
    for (unsigned iSt = 0; iSt < 4; ++iSt) {
      if (bestTSIndex[iSt] != -1)
        fittedTrack->addRelationTo(hits[bestTSIndex[iSt]]);
    }
    // add axial relations from 2D track
    RelationVector<CDCTriggerSegmentHit> axialHits =
      tracks2D[itrack]->getRelationsTo<CDCTriggerSegmentHit>();
    for (unsigned ihit = 0; ihit < axialHits.size(); ++ihit) {
      fittedTrack->addRelationTo(axialHits[ihit]);
    }
  }
}
