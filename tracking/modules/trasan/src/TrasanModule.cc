//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TrasanModule.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for CDC
//-----------------------------------------------------------------------------
// 0.00 : 2010/10/15 : First version
//-----------------------------------------------------------------------------

#include <iostream>
#include "tracking/modules/trasan/TrasanModule.h"

using namespace std;

namespace Belle2 {

  REG_MODULE(Trasan);

  TrasanModule::TrasanModule()
    : Module::Module(),
      _debugLevel(0),
      _gfTrackCandsName(""),
      _tra(Belle::Trasan())
  {

    string desc = "TrasanModule(" + _tra.version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

    addParam("DebugLevel", _debugLevel, "Debug level", 0);
    addParam("GFTrackCandidatesColName",
             _gfTrackCandsName,
             "Name of collection holding the GFTrackCandidates (output)",
             string(""));

    //ho
    //...Hough Finder...
    addParam("doHoughFinder", _tra.b_doHoughFinder,
	     "Hough Finder switch", 1);
    addParam("doHoughFinderCurlSearch", _tra.b_doHoughFinderCurlSearch,
	     "Hough Finder curl search switch", 1);
    //...Conformal finder...
    //    addParam("doConformalFinder", _tra.b_doConformalFinder,
    //		    "Conformal Finder switch", 0);
    //...CurlFinder...
    addParam("doCurlFinder", _tra.b_doCurlFinder,
	     "Curl Finder switch", 0);

    addParam("helixFitterNtrialMax", _tra.b_helixFitterNtrialMax,
	     "helix fitter max. # iterations", 20);
#ifdef TRASAN_DEBUG
    cout << "TrasanModule ... created" << endl;
#endif
  }

  TrasanModule::~TrasanModule()
  {
#ifdef TRASAN_DEBUG
    cout << "TrasanModule ... destructed " << endl;
#endif
  }

  void
  TrasanModule::initialize()
  {
    Belle2::StoreArray<GFTrackCand> trackCandidates(_gfTrackCandsName);
    _tra._gfTrackCandsName = _gfTrackCandsName;
    _tra.b_debugLevel = _debugLevel;
    _tra.initialize();
#ifdef TRASAN_DEBUG
//  _cdc->dump("geometry superLayers layers wires detail");
//  _cdc->dump("geometry superLayers layers detail");
    cout << "TrasanModule ... initialized" << endl;
#endif
  }

  void
  TrasanModule::beginRun()
  {
    _tra.beginRun();
#ifdef TRASAN_DEBUG
    cout << "TrasanModule ... beginRun called " << endl;
#endif
  }

  void
  TrasanModule::event()
  {
    _tra.event();
#ifdef TRASAN_DEBUG
    cout << "TrasanModule ... event called " << endl;
#endif
  }

  void
  TrasanModule::endRun()
  {
    _tra.endRun();
#ifdef TRASAN_DEBUG
    cout << "TrasanModule ... endRun called " << endl;
#endif
  }

  void
  TrasanModule::terminate()
  {
    _tra.terminate();
#ifdef TRASAN_DEBUG
    cout << "TrasanModule ... terminate called " << endl;
#endif
  }

} // namespace Belle2
