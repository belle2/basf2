/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <iostream>
#include <sstream>

#include <string>
#include <trg/cdc/dataobjects/CDCTriggerMLP.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerMLPData.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerMLPData.h>
#include <framework/dataobjects/EventMetaData.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/gearbox/Unit.h>
#include <cmath>
#include <TFile.h>
#include <framework/geometry/B2Vector3.h>


using namespace Belle2;
namespace NeuroTrainer {
  std::vector<float> getTrainTargets(bool& trainonreco, CDCTriggerTrack* twodtrack, std::string targetcollectionname)
  {
    std::vector<float> ret;
    float phi0Target = 0;
    float invptTarget = 0;
    float thetaTarget = 0;
    float zTarget = 0;
    float isvalid = 1;
    if (trainonreco) {
      RecoTrack* recoTrack =
        twodtrack->getRelatedTo<RecoTrack>(targetcollectionname);
      if (!recoTrack) {
        B2DEBUG(150, "Skipping CDCTriggerTrack without relation to RecoTrack.");
        isvalid = 0;
      } else {
        // a RecoTrack has multiple representations for different particle hypothesis
        // -> just take the first one that does not give errors.
        const std::vector<genfit::AbsTrackRep*>& reps = recoTrack->getRepresentations();
        bool foundValidRep = false;
        for (unsigned irep = 0; irep < reps.size() && !foundValidRep; ++irep) {
          if (!recoTrack->wasFitSuccessful(reps[irep]))
            continue;
          // get state (position, momentum etc.) from hit closest to IP and
          // extrapolate to z-axis (may throw an exception -> continue to next representation)
          try {
            genfit::MeasuredStateOnPlane state =
              recoTrack->getMeasuredStateOnPlaneClosestTo(ROOT::Math::XYZVector(0, 0, 0), reps[irep]);
            reps[irep]->extrapolateToLine(state, TVector3(0, 0, -1000), TVector3(0, 0, 2000));
            // flip tracks if necessary, such that trigger tracks and reco tracks
            // point in the same direction
            if (state.getMom().Dot(B2Vector3D(twodtrack->getDirection())) < 0) {
              state.setPosMom(state.getPos(), -state.getMom());
              state.setChargeSign(-state.getCharge());
            }
            // get track parameters
            phi0Target = state.getMom().Phi();
            invptTarget = state.getCharge() / state.getMom().Pt();
            thetaTarget = state.getMom().Theta();
            zTarget = state.getPos().Z();
          } catch (...) {
            continue;
          }
          // break loop
          foundValidRep = true;
        }
      }
    } else {
      MCParticle* mcTrack =
        twodtrack->getRelatedTo<MCParticle>(targetcollectionname);
      if (not mcTrack) {
        B2DEBUG(150, "Skipping CDCTriggerTrack without relation to MCParticle.");
        isvalid = 0;
      } else {
        phi0Target = mcTrack->getMomentum().Phi();
        invptTarget = mcTrack->getCharge() / mcTrack->getMomentum().Rho();
        thetaTarget = mcTrack->getMomentum().Theta();
        zTarget = mcTrack->getProductionVertex().Z();
      }
    }
    ret.push_back(phi0Target);
    ret.push_back(invptTarget);
    ret.push_back(thetaTarget);
    ret.push_back(zTarget);
    ret.push_back(isvalid);
    return ret;
  }
  std::vector<float>
  getRelevantID(CDCTriggerMLPData& trainSet_prepare, double cutsum, double relevantcut)
  {
    std::vector<float> relevantID;
    relevantID.assign(18, 0.);
    CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();
    int layerId = 3;
    for (unsigned iSL = 0; iSL < 9; ++iSL) {
      int nWires = cdc.nWiresInLayer(layerId);
      layerId += (iSL > 0 ? 6 : 7);
      B2DEBUG(90, "SL " << iSL << " (" <<  nWires << " wires)");
      // get maximum hit counter
      unsigned maxCounter = 0;
      int maxId = 0;
      unsigned counterSum = 0;
      for (int iTS = 0; iTS < nWires; ++iTS) {
        if (trainSet_prepare.getHitCounter(iSL, iTS) > 0)
          B2DEBUG(90, iTS << " " << trainSet_prepare.getHitCounter(iSL, iTS));
        if (trainSet_prepare.getHitCounter(iSL, iTS) > maxCounter) {
          maxCounter = trainSet_prepare.getHitCounter(iSL, iTS);
          maxId = iTS;
        }
        counterSum += trainSet_prepare.getHitCounter(iSL, iTS);
      }
      // use maximum as starting range
      if (maxId > nWires / 2) maxId -= nWires;
      relevantID[2 * iSL] = maxId;
      relevantID[2 * iSL + 1] = maxId;
      if (cutsum) {
        // add neighboring wire with higher hit count
        // until sum over unused wires is less than relevantcut * sum over all wires
        double cut = relevantcut * counterSum;
        B2DEBUG(50, "Threshold on counterSum: " << cut);
        unsigned relevantSum = maxCounter;
        while (counterSum - relevantSum > cut) {
          int prev = trainSet_prepare.getHitCounter(iSL, relevantID[2 * iSL] - 1);
          int next = trainSet_prepare.getHitCounter(iSL, relevantID[2 * iSL + 1] + 1);
          if (prev > next ||
              (prev == next &&
               (relevantID[2 * iSL + 1] - maxId) > (maxId - relevantID[2 * iSL]))) {
            --relevantID[2 * iSL];
            relevantSum += prev;
            if (relevantID[2 * iSL] <= -nWires) break;
          } else {
            ++relevantID[2 * iSL + 1];
            relevantSum += next;
            if (relevantID[2 * iSL + 1] >= nWires - 1) break;
          }
        }
      } else {
        // add wires from both sides until hit counter drops below relevantcut * track counter
        double cut = relevantcut * trainSet_prepare.getTrackCounter();
        B2DEBUG(50, "Threshold on counter: " << cut);
        while (trainSet_prepare.getHitCounter(iSL, relevantID[2 * iSL] - 1) > cut) {
          --relevantID[2 * iSL];
          if (relevantID[2 * iSL] <= -nWires) break;
        }
        while (trainSet_prepare.getHitCounter(iSL, relevantID[2 * iSL + 1] + 1) > cut) {
          ++relevantID[2 * iSL + 1];
          if (relevantID[2 * iSL + 1] >= nWires - 1) break;
        }
      }
      // add +-0.5 to account for rounding during preparation
      relevantID[2 * iSL] -= 0.5;
      relevantID[2 * iSL + 1] += 0.5;
      B2DEBUG(50, "SL " << iSL << ": "
              << relevantID[2 * iSL] << " " << relevantID[2 * iSL + 1]);
    }
    return relevantID;
  }


}
