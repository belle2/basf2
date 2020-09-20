#pragma once

#include <vector>
#include <tuple>
#include "TVector3.h"
#include "TMatrixDSym.h"
#include "TTree.h"


struct ExpRunEvt {
  int exp, run, evt;
  ExpRunEvt(int Exp, int Run, int Evt) : exp(Exp), run(Run), evt(Evt) {}
};


// track parameters (neglecting curvature)
struct track {
  double d0, z0, phi0, tanlambda;
};

//Event from two tracks
struct event {
  int exp, run, evtNo;
  track mu0, mu1;

  int nBootStrap; //random bootstrap weight (n=1 -> original sample)
  bool isSig;     // is not removed?
  double t;       // time of event (relative)
  double tAbs;       // time of event (absolute)
  void toMicroM()   //from [cm] to [um]
  {
    mu0.d0 *= 1e4;
    mu1.d0 *= 1e4;
    mu0.z0 *= 1e4;
    mu1.z0 *= 1e4;
  }
};


//functions
std::vector<event> getEvents(TTree* tr);
std::tuple<std::vector<TVector3>, std::vector<TMatrixDSym>, TMatrixDSym>  runBeamSpotAnalysis(std::vector<event> evts,
    std::vector<double> splitPoints);
std::vector<ExpRunEvt> convertSplitPoints(const std::vector<event>& events, std::vector<double> splitPoints);
ExpRunEvt getPosition(const std::vector<event>& events, double tEdge);

