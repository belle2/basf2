#include <iostream>
#include <sstream>

#include <string>
#include <trg/cdc/dataobjects/CDCTriggerMLP.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerMLPData.h>
#include <framework/dataobjects/EventMetaData.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/gearbox/Unit.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <TFile.h>
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/multi_array.hpp"
#define BOOST_MULTI_ARRAY_NO_GENERATORS

using namespace Belle2;
namespace NeuroTrainer {
  struct HeaderSet {
    float relID[18];
    unsigned exPert;
    HeaderSet() {}
    HeaderSet(unsigned expert, std::vector<float> relevantID)
    {
      if (relevantID.size() == 18) {
        for (unsigned i = 0; i < 18; ++i) {relID[i] = relevantID[i];}
      } else {std::cout << "ERROR! wrong length of relID vector!" << std::endl;}
      exPert = expert;
    }
    friend std::ostream& operator << (std::ostream& out, const HeaderSet& hset)
    {
      out << hset.exPert << '\t';
      for (unsigned i = 0; i < 18; ++i) { out << hset.relID[i] << '\t';}
      return out;
    }
    friend std::istream& operator >> (std::istream& in, HeaderSet& hset)
    {
      std::string help;
      std::string helpline;
      help = "";
      helpline = "";
      std::getline(in, helpline, '\n');
      if (helpline.length() < 2) {return in;}
      std::stringstream ss(helpline);
      std::getline(ss, help, '\t');

      hset.exPert = std::stoul(help);
      for (unsigned i = 0; i < 18; ++i) {
        help = "";
        std::getline(ss, help, '\t');
        hset.relID[i] = std::stof(help);
      }
      return in;
    }


  };
  /** Struct to keep one set of training data for either training, validation or testing.
   * Having this struct makes it easier to save the data to an output filestream.
   */
  template <const unsigned inLen, const unsigned outLen>
  struct NeuroSet {
    float input[inLen];
    float target[outLen];
    int exp;
    int run;
    int subrun;
    int evt;
    int track;
    unsigned expert;
    NeuroSet() {}
    NeuroSet(float xin[inLen], float xout[outLen],  int& xexp, int& xrun, int& xsubrun, int& xevt, int& xtrack, unsigned& xexpert)
    {
      for (unsigned i = 0; i < inLen; ++i) {
        input[i] = xin[i];
      }
      for (unsigned i = 0; i < outLen; ++i) {
        target[i] = xout[i];
      }
      exp = xexp;
      run = xrun;
      subrun = xsubrun;
      evt = xevt;
      track = xtrack;
      expert = xexpert;
    }
    friend std::ostream& operator << (std::ostream& out, const NeuroSet& dset)
    {
      out << dset.exp << '\t' << dset.run << '\t' << dset.subrun << '\t' << dset.evt << '\t' << dset.track << '\t' << dset.expert << '\t'
          << inLen << '\t' << outLen << '\t';
      for (auto indata : dset.input) {out << indata << '\t';}
      for (auto outdata : dset.target) {out << outdata << '\t';}
      return out;
    }
    friend std::istream& operator >> (std::istream& in, NeuroSet& dset)
    {
      std::string help;
      help = "";
      std::getline(in, help, '\t');
      if (help.length() < 2) {return in;}
      dset.exp = std::stoi(help);
      help = "";
      std::getline(in, help, '\t');
      dset.run = std::stoi(help);
      help = "";
      std::getline(in, help, '\t');
      dset.subrun = std::stoi(help);
      help = "";
      std::getline(in, help, '\t');
      dset.evt = std::stoi(help);
      help = "";
      std::getline(in, help, '\t');
      dset.track = std::stoi(help);
      help = "";
      std::getline(in, help, '\t');
      dset.expert = std::stoul(help);
      // check if input and target size match:
      std::string insize = "";
      std::string outsize = "";
      std::getline(in, insize, '\t');
      if (std::stoul(insize) != inLen) {
        B2ERROR("Input and output format of neurotrigger training data does not  match!");
      }
      std::getline(in, outsize, '\t');
      if (std::stoul(outsize) != outLen) {
        B2ERROR("Input and output format of neurotrigger training data does not  match!");
      }
      for (unsigned i = 0; i < inLen; ++i) {
        help = "";
        std::getline(in, help, '\t');
        dset.input[i] = std::stof(help);
      }
      for (unsigned i = 0; i < outLen; ++i) {
        help = "";
        std::getline(in, help, '\t');
        dset.target[i] = std::stof(help);
      }
      return in;
    }
  };
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
              recoTrack->getMeasuredStateOnPlaneClosestTo(TVector3(0, 0, 0), reps[irep]);
            reps[irep]->extrapolateToLine(state, TVector3(0, 0, -1000), TVector3(0, 0, 2000));
            // flip tracks if necessary, such that trigger tracks and reco tracks
            // point in the same direction
            if (state.getMom().Dot(twodtrack->getDirection()) < 0) {
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
        invptTarget = mcTrack->getCharge() / mcTrack->getMomentum().Pt();
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
