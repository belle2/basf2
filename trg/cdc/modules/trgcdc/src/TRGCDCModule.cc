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

using namespace std;

namespace Belle2 {

REG_MODULE(TRGCDC);

string
TRGCDCModule::version() const {
    return string("TRGCDCModule 5.07");
}

TRGCDCModule::TRGCDCModule()
    : Module::Module(),
      _debugLevel(0),
      _configFilename("TRGCDCConfig.dat"),
      _innerTSLUTDataFilename("undefined"),
      _outerTSLUTDataFilename("undefined"),
      _tsfLUTSL0DataFilename("undefined"),
      _tsfLUTSL1DataFilename("undefined"),
      _tsfLUTSL2DataFilename("undefined"),
      _tsfLUTSL3DataFilename("undefined"),
      _tsfLUTSL4DataFilename("undefined"),
      _tsfLUTSL5DataFilename("undefined"),
      _tsfLUTSL6DataFilename("undefined"),
      _tsfLUTSL7DataFilename("undefined"),
      _tsfLUTSL8DataFilename("undefined"),
      _rootTRGCDCFilename("undefined"),
      _rootFitter3DFilename("undefined"),
      _curlBackStop(0),
      _simulationMode(1),
      _fastSimulationMode(0),
      _firmwareSimulationMode(0),
      _fileTRGCDC(0),
      _perfect2DFinder(false),
      _perfect3DFinder(false),
//    _hFinderMeshX(96),
      _hFinderMeshX(180),
//    _hFinderMeshY(96),
      _hFinderMeshY(24),
      _hFinderPeakMin(5),
      _fLogicLUTTSF(0),
      _fLRLUT(1),
      _fevtTime(1),
      _wireHitInefficiency(0.),
      _fileTSF(0),
      _fileHough3D(0),
      _finder3DMode(0),
      _fileFitter3D(0),
      _trgCDCDataInputMode(0),
      _cdc(0),
      _sa(0) {

    string desc = "TRGCDCModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

    addParam("DebugLevel", _debugLevel, "TRGCDC debug level", _debugLevel);
    addParam("ConfigFile",
             _configFilename,
             "The filename of CDC trigger config file",
             _configFilename);
    addParam("InnerTSLUTDataFile",
             _innerTSLUTDataFilename,
             "The filename of LUT for the inner-most track segments",
             _innerTSLUTDataFilename);
    addParam("OuterTSLUTDataFile",
             _outerTSLUTDataFilename,
             "The filename of LUT for outer track segments",
             _outerTSLUTDataFilename);
    addParam("TSFLUTSL0DataFile",
             _tsfLUTSL0DataFilename,
             "The filename of LUT for TSF SL0",
             _tsfLUTSL0DataFilename);
    addParam("TSFLUTSL1DataFile",
             _tsfLUTSL1DataFilename,
             "The filename of LUT for TSF SL1",
             _tsfLUTSL1DataFilename);
    addParam("TSFLUTSL2DataFile",
             _tsfLUTSL2DataFilename,
             "The filename of LUT for TSF SL2",
             _tsfLUTSL2DataFilename);
    addParam("TSFLUTSL3DataFile",
             _tsfLUTSL3DataFilename,
             "The filename of LUT for TSF SL3",
             _tsfLUTSL3DataFilename);
    addParam("TSFLUTSL4DataFile",
             _tsfLUTSL4DataFilename,
             "The filename of LUT for TSF SL4",
             _tsfLUTSL4DataFilename);
    addParam("TSFLUTSL5DataFile",
             _tsfLUTSL5DataFilename,
             "The filename of LUT for TSF SL5",
             _tsfLUTSL5DataFilename);
    addParam("TSFLUTSL6DataFile",
             _tsfLUTSL6DataFilename,
             "The filename of LUT for TSF SL6",
             _tsfLUTSL6DataFilename);
    addParam("TSFLUTSL7DataFile",
             _tsfLUTSL7DataFilename,
             "The filename of LUT for TSF SL7",
             _tsfLUTSL7DataFilename);
    addParam("TSFLUTSL8DataFile",
             _tsfLUTSL8DataFilename,
             "The filename of LUT for TSF SL8",
             _tsfLUTSL8DataFilename);
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


    if (TRGDebug::level())
	cout << "TRGCDCModule ... created" << endl;
}

TRGCDCModule::~TRGCDCModule() {

    if (_cdc)
        TRGCDC::getTRGCDC("good-bye");

    //...Maybe G4RunManager delete it, so don't delete _sa.
//  if (_sa)
//         delete _sa;

    if (TRGDebug::level())
        cout << "TRGCDCModule ... destructed " << endl;
}

void
TRGCDCModule::initialize() {

    //...Stop curl buck...
    if (_curlBackStop) {
	G4RunManager* g4rm = G4RunManager::GetRunManager();
	if(g4rm!=0){
	    if(g4rm->GetUserPhysicsList()!=0){
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
}

void
TRGCDCModule::beginRun() {

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
				 _innerTSLUTDataFilename,
				 _outerTSLUTDataFilename,
				 _tsfLUTSL0DataFilename,
				 _tsfLUTSL1DataFilename,
				 _tsfLUTSL2DataFilename,
				 _tsfLUTSL3DataFilename,
				 _tsfLUTSL4DataFilename,
				 _tsfLUTSL5DataFilename,
				 _tsfLUTSL6DataFilename,
				 _tsfLUTSL7DataFilename,
				 _tsfLUTSL8DataFilename,
				 _rootTRGCDCFilename,
				 _rootFitter3DFilename,
				 _hFinderMeshX,
				 _hFinderMeshY,
				 _hFinderPeakMin,
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
TRGCDCModule::event() {

    if (TRGDebug::level()) {
//      _cdc->dump("geometry superLayers layers wires detail");
//      _cdc->dump("geometry superLayers layers detail");
    }

    //...CDC trigger simulation...
    _cdc->update(true);
    _cdc->simulate();
}

void
TRGCDCModule::endRun() {
    if (TRGDebug::level())
        cout << "TRGCDCModule ... endRun called " << endl;
}

void
TRGCDCModule::terminate() {

    _cdc->terminate();

    if (TRGDebug::level())
	cout << "TRGCDCModule ... terminate called " << endl;
}

} // namespace Belle2
