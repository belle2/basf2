#include "trg/cdc/modules/fitting/CDCTrigger2DFitterModule.h"

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
REG_MODULE(CDCTrigger2DFitter);

CDCTrigger2DFitterModule::CDCTrigger2DFitterModule() : Module::Module()
{
  setDescription(
    "The 2D fitter module of the CDC trigger.\n"
    "Performs a circle fit on a given set of axial CDCTriggerSegmentHits.\n"
    "Requires a preceding track finder to sort hits to tracks.\n"
  );

  addParam("inputCollection", m_inputCollectionName,
           "Name of the StoreArray holding the input tracks from the track finder.",
           string("Trg2DFinderTracks"));
  addParam("outputCollection", m_outputCollectionName,
           "Name of the StoreArray holding the fitted output tracks.",
           string("Trg2DFitterTracks"));
  addParam("minHits", m_minHits,
           "Minimal number of hits required for the fitting.",
           unsigned(5));
  addParam("xtSimple", m_xtSimple,
           "If true, use nominal drift velocity, otherwise use table "
           "for non-linear xt.",
           false);
  addParam("useDriftTime", m_useDriftTime,
           "If true, use drift time, otherwise only wire position.",
           true);
}

void
CDCTrigger2DFitterModule::initialize()
{
  // register DataStore elements
  StoreArray<CDCTriggerTrack>::registerPersistent(m_outputCollectionName);
  StoreArray<CDCTriggerTrack>::required(m_inputCollectionName);
  StoreArray<CDCTriggerSegmentHit>::required();
  // register relations
  StoreArray<CDCTriggerTrack> finderTracks(m_inputCollectionName);
  StoreArray<CDCTriggerTrack> fitterTracks(m_outputCollectionName);
  StoreArray<CDCTriggerSegmentHit> segmentHits;
  fitterTracks.registerRelationTo(finderTracks);
  fitterTracks.registerRelationTo(segmentHits);

  // get geometry constants for first priority layers
  CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();
  // TODO: avoid hard coding the priority layers here
  vector<unsigned> iL = {3, 16, 28, 40, 52};
  for (int iAx = 0; iAx < 5; ++iAx) {
    nWires.push_back(cdc.nWiresInLayer(iL[iAx]));
    rr.push_back(cdc.senseWireR(iL[iAx]));
    vector<double> xt(512);
    for (unsigned iTick = 0; iTick < xt.size(); ++iTick) {
      double t = iTick * 2 * cdc.getTdcBinWidth();
      if (m_xtSimple) {
        xt[iTick] = cdc.getNominalDriftV() * t;
      } else {
        double driftLength_0 = cdc.getDriftLength(t, iL[iAx], 0);
        double driftLength_1 = cdc.getDriftLength(t, iL[iAx], 1);
        xt[iTick] = (driftLength_0 + driftLength_1) / 2;
      }
    }
    xtTables.push_back(xt);
  }
}

void
CDCTrigger2DFitterModule::event()
{
  StoreArray<CDCTriggerTrack> finderTracks(m_inputCollectionName);
  StoreArray<CDCTriggerTrack> fitterTracks(m_outputCollectionName);
  StoreObjPtr<TRGTiming> eventTime("CDCTriggerEventTime");
  int T0 = eventTime->getTiming();

  vector<double> wirePhi2DError({0.00085106,
                                 0.00039841,
                                 0.00025806,
                                 0.00019084,
                                 0.0001514
                                });
  vector<double> driftPhi2DError({0.00085106,
                                  0.00039841,
                                  0.00025806,
                                  0.00019084,
                                  0.0001514
                                 });

  for (int itrack = 0; itrack < finderTracks.getEntries(); ++itrack) {
    // get selected hits (positive relation weight)
    RelationVector<CDCTriggerSegmentHit> hits =
      finderTracks[itrack]->getRelationsTo<CDCTriggerSegmentHit>();
    unsigned nHits = 0;
    vector<double> tsId(5, -1.);
    vector<double> wirePhi(5, 0.);
    vector<int> driftTime(5, 0);
    vector<double> driftLength(5, 0.);
    vector<unsigned> LR(5, 0);
    vector<int> hitIds(5, -1);
    vector<double> useSL(5, 0.);
    for (unsigned ihit = 0; ihit < hits.size(); ++ihit) {
      if (hits.weight(ihit) > 0) {
        // use only first priority hits
        if (hits[ihit]->getPriorityPosition() != 3) continue;
        unsigned iSL = hits[ihit]->getISuperLayer();
        // skip stereo hits (should not be related, but check anyway)
        if (iSL % 2) continue;
        // the track finder should have selected a single hit per super layer
        // if that is not the case, select the fastest hit
        if (hitIds[iSL / 2] != -1) {
          B2WARNING("got multiple hits for SL " << iSL);
          if (hits[ihit]->priorityTime() > driftTime[iSL / 2])
            continue;
        } else {
          nHits += 1;
        }
        useSL[iSL / 2] = 1.;
        hitIds[iSL / 2] = ihit;
        tsId[iSL / 2] = hits[ihit]->getIWire();
        wirePhi[iSL / 2] = hits[ihit]->getIWire() * 2. * M_PI / nWires[iSL / 2];
        LR[iSL / 2] = hits[ihit]->getLeftRight();
        driftTime[iSL / 2] = hits[ihit]->priorityTime();
        int t = hits[ihit]->priorityTime() - T0;
        if (t < 0) t = 0;
        if (t > 511) t = 511;
        driftLength[iSL / 2] = xtTables[iSL / 2][t];
      }
    }
    if (nHits < m_minHits) {
      B2INFO("Not enough hits to do 2D fit (" << m_minHits << " needed, got " << nHits << ")");
      continue;
    }

    // Set phi2DError for 2D fit
    vector<double> phi2DInvError(5, 0.);
    for (unsigned iAx = 0; iAx < 5; ++iAx) {
      if (LR[iAx] == 0) continue;
      if (LR[iAx] != 3 && T0 != 9999)
        phi2DInvError[iAx] = 1. / driftPhi2DError[iAx];
      else
        phi2DInvError[iAx] = 1. / wirePhi2DError[iAx];
    }
    // Calculate phi2D using driftLength.
    vector<double> phi2D(5, 0.);
    for (unsigned iAx = 0; iAx < 5; ++iAx) {
      if (hitIds[iAx] < 0) continue;
      if (T0 == 9999 || !m_useDriftTime)
        phi2D[iAx] = wirePhi[iAx];
      else
        phi2D[iAx] = Fitter3DUtility::calPhi(wirePhi[iAx], driftLength[iAx], rr[iAx], LR[iAx]);
    }
    // Fit2D
    double rho = 0;
    double phi0 = 0;
    double chi2 = 0;
    Fitter3DUtility::rPhiFitter(&rr[0], &phi2D[0], &phi2DInvError[0], rho, phi0, chi2);
    double charge = finderTracks[itrack]->getChargeSign();
    double chargeFit = 0;
    Fitter3DUtility::chargeFinder(&nWires[0], &tsId[0], &useSL[0], phi0,
                                  charge, chargeFit);
    double omega = chargeFit / rho;

    // save track
    CDCTriggerTrack* fittedTrack =
      fitterTracks.appendNew(remainder(phi0 + chargeFit * M_PI_2, 2. * M_PI),
                             omega, chi2);
    // make relation to finder track
    fittedTrack->addRelationTo(finderTracks[itrack]);
    // make relation to hits
    for (unsigned iAx = 0; iAx < 5; ++iAx) {
      if (hitIds[iAx] >= 0)
        fittedTrack->addRelationTo(hits[hitIds[iAx]]);
    }
  }
}
