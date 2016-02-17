//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Module.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for CDC
//-----------------------------------------------------------------------------
// 0.00 : 2010/10/08 : First version
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include <stdlib.h>
#include <iostream>
#include "G4RunManager.hh"
#include "framework/core/ModuleManager.h"
#include "trg/cdc/modules/trgcdc/TRGCDCModule.h"
#include "trg/trg/Debug.h"

#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCHit.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>

using namespace std;

namespace Belle2 {

  REG_MODULE(TRGCDC);

  string
  TRGCDCModule::version() const
  {
    return string("TRGCDCModule 5.07");
  }

  TRGCDCModule::TRGCDCModule()
    : Module::Module(),
      _debugLevel(0),
      _configFilename("TRGCDCConfig.dat"),
      _innerTSLUTFilename("undefined"),
      _outerTSLUTFilename("undefined"),
      _rootTRGCDCFilename("undefined"),
      _rootFitter3DFilename("undefined"),
      _curlBackStop(0),
      _simulationMode(1),
      _fastSimulationMode(0),
      _firmwareSimulationMode(0),
      _fileTRGCDC(0),
      _perfect2DFinder(false),
      _perfect3DFinder(false),
      _hFinderMeshX(160),
      _hFinderMeshY(26),
      _hFinderPeakMin(5),
      _hFinderMappingFilePlus("HoughMappingPlus20160217.dat"),
      _hFinderMappingFileMinus("HoughMappingMinus20160217.dat"),
      _hFinderDoit(2),
      _fLogicLUTTSF(1),
      _fLRLUT(1),
      _fevtTime(1),
      _wireHitInefficiency(0.),
      _fileTSF(0),
      _fileHough3D(0),
      _finder3DMode(2),
      _fileFitter3D(0),
      _trgCDCDataInputMode(0),
      _cdc(0),
      _sa(0)
  {

    string desc = "TRGCDCModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("DebugLevel", _debugLevel, "TRGCDC debug level", _debugLevel);
    addParam("ConfigFile",
             _configFilename,
             "The filename of CDC trigger config file",
             _configFilename);
    addParam("InnerTSLUTFile",
             _innerTSLUTFilename,
             "The filename of LUT for inner-most track segments",
             _innerTSLUTFilename);
    addParam("OuterTSLUTFile",
             _outerTSLUTFilename,
             "The filename of LUT for outer track segments",
             _outerTSLUTFilename);
    addParam("RootTRGCDCFile",
             _rootTRGCDCFilename,
             "The filename of root file for TRGCDC",
             string("TRGCDC.root"));
    addParam("RootFitter3DFile",
             _rootFitter3DFilename,
             "The filename of root file for Fitter3D",
             string("Fitter3D.root"));
    addParam("CurlBackStop",
             _curlBackStop,
             "Curl back stop parameter",
             _curlBackStop);
    addParam("SimulationMode",
             _simulationMode,
             "TRGCDC simulation switch",
             _simulationMode);
    addParam("FastSimulationMode",
             _fastSimulationMode,
             "TRGCDC fast simulation mode",
             _fastSimulationMode);
    addParam("FirmwareSimulationMode",
             _firmwareSimulationMode,
             "TRGCDC firmware simulation mode",
             _firmwareSimulationMode);
    addParam("TRGCDCRootFile",
             _fileTRGCDC,
             "Flag for making TRGCDC.root",
             _fileTRGCDC);
    addParam("TSFLogicLUT",
             _fLogicLUTTSF,
             "Use logic or LUT for TSF",
             _fLogicLUTTSF);
    addParam("2DFinderPerfect",
             _perfect2DFinder,
             "2D finder perfect option",
             _perfect2DFinder);
    addParam("3DFinderPerfect",
             _perfect3DFinder,
             "3D finder perfect option",
             _perfect3DFinder);
    addParam("HoughFinderMeshX",
             _hFinderMeshX,
             "Hough finder # mesh in x",
             _hFinderMeshX);
    addParam("HoughFinderMeshY",
             _hFinderMeshY,
             "Hough finder # mesh in y",
             _hFinderMeshY);
    addParam("HoughFinderPeakMin",
             _hFinderPeakMin,
             "Hough finder min. peak height",
             _hFinderPeakMin);
    addParam("HoughFinderMappingFilePlus",
             _hFinderMappingFilePlus,
             "Hough finder cell mapping file for charge plus",
             _hFinderMappingFilePlus);
    addParam("HoughFinderMappingFileMinus",
             _hFinderMappingFileMinus,
             "Hough finder cell mapping file for charge minus",
             _hFinderMappingFileMinus);
    addParam("HoughFinderDoit",
             _hFinderDoit,
             "Hough finder doit() version",
             _hFinderDoit);
    addParam("Fitter3DLRLUT",
             _fLRLUT,
             "Using L/R LUT in fitter3D",
             _fLRLUT);
    addParam("EvtTime",
             _fevtTime,
             "Using event time in Segment",
             _fevtTime);
    addParam("Zierror",
             _fzierror,
             "Using new Zi error",
             _fzierror);
    addParam("MCLR",
             _fmclr,
             "Using MC L/R information",
             _fmclr);
    addParam("wireHitInefficiency",
             _wireHitInefficiency,
             "wire hit inefficiency",
             _wireHitInefficiency);
    addParam("TSFRootFile",
             _fileTSF,
             "Flag for making TSF.root",
             _fileTSF);
    addParam("Hough3DRootFile",
             _fileHough3D,
             "Flag for making Hough3D.root",
             _fileHough3D);
    addParam("Finder3DMode",
             _finder3DMode,
             "Mode for finder3D",
             _finder3DMode);
    addParam("Fitter3DRootFile",
             _fileFitter3D,
             "Flag for making Fitter3D.root",
             _fileFitter3D);
    addParam("TRGCDCDataInputMode",
             _trgCDCDataInputMode,
             "Flag for TRG CDC input mode",
             _trgCDCDataInputMode);
    addParam("2DfinderCollection", m_2DfinderCollectionName,
             "Name of the StoreArray holding the tracks made by the 2D finder.",
             string("Trg2DFinderTracks"));
    addParam("2DfitterCollection", m_2DfitterCollectionName,
             "Name of the StoreArray holding the tracks made by the 2D fitter.",
             string("Trg2DFitterTracks"));
    addParam("3DfitterCollection", m_3DfitterCollectionName,
             "Name of the StoreArray holding the tracks made by the 3D fitter.",
             string("Trg3DFitterTracks"));


    if (TRGDebug::level())
      cout << "TRGCDCModule ... created" << endl;
  }

  TRGCDCModule::~TRGCDCModule()
  {
    if (_cdc)
      TRGCDC::getTRGCDC("good-bye");

    if (TRGDebug::level())
      cout << "TRGCDCModule ... destructed " << endl;
  }

  void
  TRGCDCModule::initialize()
  {

    //...Stop curl buck...
    if (_curlBackStop) {
      G4RunManager* g4rm = G4RunManager::GetRunManager();
      if (g4rm != 0) {
        if (g4rm->GetUserPhysicsList() != 0) {
          _sa = new TCSAction();
          g4rm->SetUserAction(_sa);
        }
      }
    }
    TRGDebug::level(_debugLevel);

    if (TRGDebug::level()) {
      cout << "TRGCDCModule::initialize ... options" << endl;
      cout << TRGDebug::tab(4) << "debug level = " << TRGDebug::level()
           << endl;
      cout << TRGDebug::tab(4) << "back stop = " << _curlBackStop << endl;
      cout << TRGDebug::tab(4) << "2D finder perfect = " << _perfect2DFinder
           << endl;
      cout << TRGDebug::tab(4) << "Hough finder Mesh X = " << _hFinderMeshX
           << endl;
      cout << TRGDebug::tab(4) << "Hough finder Mesh Y = " << _hFinderMeshY
           << endl;
      cout << TRGDebug::tab(4) << "Hough finder Min. Peak = "
           << _hFinderPeakMin
           << endl;
    }

    // register DataStore elements
    StoreArray<CDCTriggerSegmentHit>::registerPersistent();
    StoreArray<CDCTriggerSegmentHit> segmentHits;
    StoreArray<CDCTriggerTrack>::registerPersistent(m_2DfinderCollectionName);
    StoreArray<CDCTriggerTrack>::registerPersistent(m_2DfitterCollectionName);
    StoreArray<CDCTriggerTrack>::registerPersistent(m_3DfitterCollectionName);
    StoreArray<CDCTriggerTrack> tracks2Dfinder(m_2DfinderCollectionName);
    StoreArray<CDCTriggerTrack> tracks2Dfitter(m_2DfitterCollectionName);
    StoreArray<CDCTriggerTrack> tracks3Dfitter(m_3DfitterCollectionName);
    StoreArray<CDCHit> cdcHits;
    StoreArray<MCParticle> mcparticles;
    segmentHits.registerRelationTo(cdcHits);
    mcparticles.registerRelationTo(segmentHits);
    tracks2Dfinder.registerRelationTo(segmentHits); // hits related over Hough cell
    tracks2Dfitter.registerRelationTo(segmentHits); // hits used for the 2D fit
    tracks3Dfitter.registerRelationTo(segmentHits); // hits used for the 2D and 3D fit
    tracks2Dfinder.registerRelationTo(tracks2Dfitter);
    tracks2Dfinder.registerRelationTo(tracks3Dfitter);
  }

  void
  TRGCDCModule::beginRun()
  {
    //...CDC trigger config. name...
    static string cfn = _configFilename;

    //...CDC trigger...
    if ((cfn != _configFilename) || (_cdc == 0))
      _cdc = TRGCDC::getTRGCDC(_configFilename,
                               _simulationMode,
                               _fastSimulationMode,
                               _firmwareSimulationMode,
                               _fileTRGCDC,
                               _perfect2DFinder,
                               _perfect3DFinder,
                               _innerTSLUTFilename,
                               _outerTSLUTFilename,
                               _rootTRGCDCFilename,
                               _rootFitter3DFilename,
                               _hFinderMeshX,
                               _hFinderMeshY,
                               _hFinderPeakMin,
                               _hFinderMappingFilePlus,
                               _hFinderMappingFileMinus,
                               _hFinderDoit,
                               _fLogicLUTTSF,
                               _fLRLUT,
                               _fevtTime,
                               _fzierror,
                               _fmclr,
                               _wireHitInefficiency,
                               _fileTSF,
                               _fileHough3D,
                               _finder3DMode,
                               _fileFitter3D,
                               _trgCDCDataInputMode);

    if (TRGDebug::level())
      cout << "TRGCDCModule ... beginRun called " << endl;
  }

  void
  TRGCDCModule::event()
  {
    //...CDC trigger simulation...
    _cdc->update(true);
    _cdc->simulate();

    // save the results to DataStore
    _cdc->storeSimulationResults(m_2DfinderCollectionName,
                                 m_2DfitterCollectionName,
                                 m_3DfitterCollectionName);
  }

  void
  TRGCDCModule::endRun()
  {
    if (TRGDebug::level())
      cout << "TRGCDCModule ... endRun called " << endl;
  }

  void
  TRGCDCModule::terminate()
  {

    _cdc->terminate();

    if (TRGDebug::level())
      cout << "TRGCDCModule ... terminate called " << endl;
  }

} // namespace Belle2
