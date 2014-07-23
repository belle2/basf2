//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGCDC.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "framework/datastore/StoreArray.h"
#include "framework/datastore/RelationArray.h"
#include "mdst/dataobjects/MCParticle.h"
#include "cdc/geometry/CDCGeometryPar.h"
#include "cdc/dataobjects/CDCHit.h"
#include "cdc/dataobjects/CDCSimHit.h"
#include "rawdata/dataobjects/RawDataBlock.h"
#include "rawdata/dataobjects/RawCOPPER.h"   
#include "trg/trg/Debug.h"
#include "trg/trg/Time.h"
#include "trg/trg/State.h"
#include "trg/trg/Signal.h"
#include "trg/trg/Channel.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/TrackMC.h"
#include "trg/cdc/Relation.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/LUT.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"
#include "trg/cdc/TrackSegmentFinder.h"
#include "trg/cdc/PerfectFinder.h"
#include "trg/cdc/HoughFinder.h"
#include "trg/cdc/Hough3DFinder.h"
#include "trg/cdc/Fitter3D.h"
#include "trg/cdc/Link.h"
#include "trg/cdc/Relation.h"
#include "trg/cdc/EventTime.h"

#define NOT_USE_SOCKETLIB
//#define NOT_SEND
//#define DUMMY_DATA
#define TIME_MONITOR
//#define MULTIPLE_SEND
//#define MEMCPY_TO_ONE_BUFFER
#define SEND_BY_WRITEV

#ifdef TRGCDC_DISPLAY
#include "trg/cdc/DisplayRphi.h"
#include "trg/cdc/DisplayHough.h"
namespace Belle2_TRGCDC {
Belle2::TRGCDCDisplayRphi * D = 0;
}
using namespace Belle2_TRGCDC;
#endif

#define P3D HepGeom::Point3D<double>

using namespace std;

namespace Belle2 {

string
TRGCDC::name(void) const {
    return "TRGCDC";
}

string
TRGCDC::version(void) const {
    return string("TRGCDC 5.39");
}

TRGCDC *
TRGCDC::_cdc = 0;

TRGCDC*
TRGCDC::getTRGCDC(const string & configFile,
                  unsigned simulationMode,
                  unsigned fastSimulationMode,
                  unsigned firmwareSimulationMode,
                  bool makeRootFile,
                  bool perfect2DFinder,
                  bool perfect3DFinder,
                  const string & innerTSLUTDataFile,
                  const string & outerTSLUTDataFile,
		  const string & innerTSLUTFile,
		  const string & outerTSLUTFile,
                  const string & tsfLUTSL0DataFile,
                  const string & tsfLUTSL1DataFile,
                  const string & tsfLUTSL2DataFile,
                  const string & tsfLUTSL3DataFile,
                  const string & tsfLUTSL4DataFile,
                  const string & tsfLUTSL5DataFile,
                  const string & tsfLUTSL6DataFile,
                  const string & tsfLUTSL7DataFile,
                  const string & tsfLUTSL8DataFile,
                  const string & rootTRGCDCFile,
                  const string & rootFitter3DFile,
                  unsigned houghFinderMeshX,
                  unsigned houghFinderMeshY,
                  unsigned houghFinderPeakMin,
                  bool fLogicLUTTSF,
                  bool fLRLUT,
                  bool fevtTime,
                  bool fzierror,
                  bool fmclr,
                  double inefficiency,
                  bool fileTSF,
                  bool fileHough3D,
                  int finder3DMode,
                  bool fileFitter3D,
                  int trgCDCDataInputMode) {
    if (_cdc) {
        //delete _cdc;
        _cdc = 0;
    }

    if (configFile != "good-bye") {
        _cdc = new TRGCDC(configFile,
                          simulationMode,
                          fastSimulationMode,
                          firmwareSimulationMode,
                          makeRootFile,
                          perfect2DFinder,
                          perfect3DFinder,
                          innerTSLUTDataFile,
                          outerTSLUTDataFile,
			  innerTSLUTFile,
			  outerTSLUTFile,
                          tsfLUTSL0DataFile,
                          tsfLUTSL1DataFile,
                          tsfLUTSL2DataFile,
                          tsfLUTSL3DataFile,
                          tsfLUTSL4DataFile,
                          tsfLUTSL5DataFile,
                          tsfLUTSL6DataFile,
                          tsfLUTSL7DataFile,
                          tsfLUTSL8DataFile,
                          rootTRGCDCFile,
                          rootFitter3DFile,
                          houghFinderMeshX,
                          houghFinderMeshY,
                          houghFinderPeakMin,
                          fLogicLUTTSF,
                          fLRLUT,
                          fevtTime,
                          fzierror,
                          fmclr,
                          inefficiency,
                          fileTSF,
                          fileHough3D,
                          finder3DMode,
                          fileFitter3D,
                          trgCDCDataInputMode);
    }
    else {
        cout << "TRGCDC::getTRGCDC ... good-bye" << endl;
        //        delete _cdc;
        _cdc = 0;
    }

    return _cdc;
}

TRGCDC*
TRGCDC::getTRGCDC(void) {
    if (! _cdc)
        cout << "TRGCDC::getTRGCDC !!! TRGCDC is not created yet" << endl;
    return _cdc;
}

TRGCDC::TRGCDC(const string & configFile,
               unsigned simulationMode,
               unsigned fastSimulationMode,
               unsigned firmwareSimulationMode,
               bool makeRootFile,
               bool perfect2DFinder,
               bool perfect3DFinder,
               const string & innerTSLUTDataFile,
               const string & outerTSLUTDataFile,
	       const string & innerTSLUTFile,
	       const string & outerTSLUTFile,
               const string & tsfLUTSL0DataFile,
               const string & tsfLUTSL1DataFile,
               const string & tsfLUTSL2DataFile,
               const string & tsfLUTSL3DataFile,
               const string & tsfLUTSL4DataFile,
               const string & tsfLUTSL5DataFile,
               const string & tsfLUTSL6DataFile,
               const string & tsfLUTSL7DataFile,
               const string & tsfLUTSL8DataFile,
               const string & rootTRGCDCFile,
               const string & rootFitter3DFile,
               unsigned houghFinderMeshX,
               unsigned houghFinderMeshY,
               unsigned houghFinderPeakMin,
               bool fLogicLUTTSF,
               bool fLRLUT,
               bool fevtTime,
               bool fzierror,
               bool fmclr,
               double inefficiency,
               bool fileTSF,
               bool fileHough3D,
               int finder3DMode,
               bool fileFitter3D,
               int trgCDCDataInputMode)
    : _debugLevel(0),
      _configFilename(configFile),
      _simulationMode(simulationMode),
      _fastSimulationMode(fastSimulationMode),
      _firmwareSimulationMode(firmwareSimulationMode),
      _makeRootFile(makeRootFile),
      _perfect2DFinder(perfect2DFinder),
      _perfect3DFinder(perfect3DFinder),
      _innerTSLUTDataFilename(innerTSLUTDataFile),
      _outerTSLUTDataFilename(outerTSLUTDataFile),
      _innerTSLUTFilename(innerTSLUTFile),
      _outerTSLUTFilename(outerTSLUTFile),
      _tsfLUTSL0DataFilename(tsfLUTSL0DataFile),
      _tsfLUTSL1DataFilename(tsfLUTSL1DataFile),
      _tsfLUTSL2DataFilename(tsfLUTSL2DataFile),
      _tsfLUTSL3DataFilename(tsfLUTSL3DataFile),
      _tsfLUTSL4DataFilename(tsfLUTSL4DataFile),
      _tsfLUTSL5DataFilename(tsfLUTSL5DataFile),
      _tsfLUTSL6DataFilename(tsfLUTSL6DataFile),
      _tsfLUTSL7DataFilename(tsfLUTSL7DataFile),
      _tsfLUTSL8DataFilename(tsfLUTSL8DataFile),
      _rootTRGCDCFilename(rootTRGCDCFile),
      _rootFitter3DFilename(rootFitter3DFile),
      _fLogicLUTTSF(fLogicLUTTSF),
      _fLRLUT(fLRLUT),
      _fevtTime(fevtTime),
      _fzierror(fzierror),
      _fmclr(fmclr),
      _inefficiency(inefficiency),
      _fileTSF(fileTSF),
      _fileHough3D(fileHough3D),
      _finder3DMode(finder3DMode),
      _fileFitter3D(fileFitter3D),
      _fudgeFactor(1.),
      _width(0),
      _r(0),
      _r2(0),
      _clock("CDCTrigger system clock", Belle2_GDL::GDLSystemClock, 1),
      _clockFE("CDCFE TDC clock", Belle2_GDL::GDLSystemClock, 8),
      _clockD("CDCTrigger data clock", Belle2_GDL::GDLSystemClock, 1, 4),
      _clockUser3125("CDCTrigger Aurora user clock (3.125Gbps)",
                     Belle2_GDL::GDLSystemClock,
                     25,
                     20),
      _clockUser6250("CDCTrigger Aurora user clock (6.250Gbps)",
                     Belle2_GDL::GDLSystemClock,
                     50,
                     20),
      _offset(5.3),
      _pFinder(0),
      _hFinder(0),
      _h3DFinder(0),
      _fitter3D(0),
      _eventTime(0),
      _trgCDCDataInputMode(trgCDCDataInputMode) {

	cout << "		-------------A TRGCDC is declared-------------" << endl;


#ifdef TRGCDC_DISPLAY
    int argc = 0;
    char** argv = 0;
    Gtk::Main main_instance(argc, argv);
    if (! D)
        D = new TCDisplayRphi();
    D->clear();
    D->show();
    cout << "TRGCDC ... GTK initialized" << endl;
#endif

    if (TRGDebug::level()) {
        cout << "TRGCDC ... TRGCDC initializing with " << _configFilename
             << endl
             << "           mode=0x" << hex << _simulationMode << dec << endl;
    }

    initialize(houghFinderMeshX, houghFinderMeshY, houghFinderPeakMin);

    if (TRGDebug::level()) {
        cout << "TRGCDC ... TRGCDC created with " << _configFilename << endl;
/*
        Belle2_GDL::GDLSystemClock.dump();
        _clock.dump();
        _clockFE.dump();
        _clockD.dump();
*/
    }

        cout << "               -------------A TRGCDC is ending declared-------------"<< endl;

}

void
TRGCDC::initialize(unsigned houghFinderMeshX,
                   unsigned houghFinderMeshY,
                   unsigned houghFinderPeakMin) {

        cout << "               -------------A TRGCDC is initialized!!"<< endl;

    //...CDC...
    Belle2::CDC::CDCGeometryPar & cdc2 =
        Belle2::CDC::CDCGeometryPar::Instance();
    const unsigned nLayers = cdc2.nWireLayers();

    //...Loop over layers...
    int superLayerId = -1;
    vector<TRGCDCLayer *> * superLayer;
    unsigned lastNWires = 0;
    int lastShifts = -1000;
    int ia = -1;
    int is = -1;
    int ias = -1;
    int iss = -1;
    unsigned nWires = 0;
    float fwrLast = 0;
    unsigned axialStereoSuperLayerId = 0;
    for (unsigned i = 0; i < nLayers; i++) {
        const unsigned nWiresInLayer = cdc2.nWiresInLayer(i);


        //...Axial or stereo?...
        int nShifts = cdc2.nShifts(i);
        bool axial = true;
        if (nShifts != 0)
            axial = false;

        unsigned axialStereoLayerId = 0;
        if (axial) {
            ++ia;
            axialStereoLayerId = ia;
        } else {
            ++is;
            axialStereoLayerId = is;
        }

	//cout << "No "  << i << " nWiresInLayer: "  << nWiresInLayer << " axial: " << axial << " nShifts: " << nShifts << endl;

        //...Is this in a new super layer?...
        if ((lastNWires != nWiresInLayer) || (lastShifts != nShifts)) {
            ++superLayerId;
            superLayer = new vector<TRGCDCLayer *>;
            _superLayers.push_back(superLayer);
            if (axial) {
                ++ias;
                axialStereoSuperLayerId = ias;
                _axialSuperLayers.push_back(superLayer);
            } else {
                ++iss;
                axialStereoSuperLayerId = iss;
                _stereoSuperLayers.push_back(superLayer);
            }
            lastNWires = nWiresInLayer;
            lastShifts = nShifts;
        }

	//cout <<"(lastNWires,nWiresInLayer) " << lastNWires << " " << nWiresInLayer << endl;
	//cout << "(lastShifts,nShifts) " << lastShifts << " " << nShifts << endl;
	//cout << "superLayerId: " << superLayerId << endl;

	//cout << "ia: " << ia << " is: " << is << " ias: " << ias << " iss: " << iss << endl;

        //...Calculate radius...
        const float swr = cdc2.senseWireR(i);
        float fwr = cdc2.fieldWireR(i);
        if (i == nLayers - 2)
            fwrLast = fwr;
        else if (i == nLayers - 1)
            fwr = swr + (swr - fwrLast);
        const float innerRadius = swr - (fwr - swr);
        const float outerRadius = swr + (fwr - swr);
/*
        if (TRGDebug::level() > 9)
            cout << "lyr " << i << ", in=" << innerRadius << ", out="  << outerRadius << ", swr=" << swr << ", fwr" << fwr << endl;
*/
        //...New layer...
        TRGCDCLayer * layer = new TRGCDCLayer(i,
                                              superLayerId,
                                              _superLayers[superLayerId]->size(),
                                              axialStereoLayerId,
                                              axialStereoSuperLayerId,
                                              cdc2.zOffsetWireLayer(i),
                                              nShifts,
                                              M_PI * cdc2.senseWireR(i)
                                              * cdc2.senseWireR(i)
                                              / double(nWiresInLayer),
                                              nWiresInLayer,
                                              innerRadius,
                                              outerRadius);
        _layers.push_back(layer);
        superLayer->push_back(layer);
        if (axial)
            _axialLayers.push_back(layer);
        else
            _stereoLayers.push_back(layer);

        //...Loop over all wires in a layer...
        for (unsigned j = 0; j < nWiresInLayer; j++) {
            const P3D fp = P3D(cdc2.wireForwardPosition(i, j).x(),
                               cdc2.wireForwardPosition(i, j).y(),
                               cdc2.wireForwardPosition(i, j).z());
            const P3D bp = P3D(cdc2.wireBackwardPosition(i, j).x(),
                               cdc2.wireBackwardPosition(i, j).y(),
                               cdc2.wireBackwardPosition(i, j).z());
            TCWire * tw = new TCWire(nWires++, j, * layer, fp, bp, _clockFE);
            _wires.push_back(tw);
            layer->push_back(tw);
        }
    }

//******* Will be removed -KT
    //...LUT for LR decision : common to all TS... 
    _luts.push_back(new TCLUT("TS LUTs", * this));
    _luts.back()->initialize(_innerTSLUTDataFilename);
    _luts.back()->initialize(_outerTSLUTDataFilename);
    _luts.back()->initialize(_tsfLUTSL0DataFilename);
    _luts.back()->initialize(_tsfLUTSL1DataFilename);
    _luts.back()->initialize(_tsfLUTSL2DataFilename);
    _luts.back()->initialize(_tsfLUTSL3DataFilename);
    _luts.back()->initialize(_tsfLUTSL4DataFilename);
    _luts.back()->initialize(_tsfLUTSL5DataFilename);
    _luts.back()->initialize(_tsfLUTSL6DataFilename);
    _luts.back()->initialize(_tsfLUTSL7DataFilename);
    _luts.back()->initialize(_tsfLUTSL8DataFilename);

    //...event Time...
    _eventTime.push_back(new TCEventTime(*this));
    _eventTime.back()->initialize();

    //  for (unsigned i = 0; _luts.size(); i++)
    //  _luts[i]->doit();

    TRGCDCSegment *tstmp;

    //...Make TSF's...
    const unsigned nWiresInTS[2] = {15, 11};
    const int shape[2][30] = {
        {
            //-2,  0,  // relative layer id, relative wire id
            //-1, -1,  // assuming layer offset 0.0, not 0.5
            //-1,  0,
            //0,  -1,
            //0,   0,
            //0,   1,
            //1,  -2,
            //1,  -1,
            //1,   0,
            //1,   1,
            //2,  -2,
            //2,  -1,
            //2,   0,
            //2,   1,
            //2,   2
            -2,  0,  // relative layer id, relative wire id
            -1,  0,  // assuming layer offset 0.5, not 0.0
            -1,  1,
            0,  -1,
            0,   0,
            0,   1,
            1,  -1,
            1,  -0,
            1,   1,
            1,   2,
            2,  -2,
            2,  -1,
            2,   0,
            2,   1,
            2,   2
        },
        {
            -2, -1,
            -2,  0,
            -2,  1,
            -1, -1,
            -1,  0,
            0,   0,
            1,  -1,
            1,   0,
            2,  -1,
            2,   0,
            2,   1,
            0,   0,
            0,   0,
            0,   0,
            0,   0
        }
    };
    const int layerOffset[2] = {4, 2};
    unsigned id = 0;
    unsigned idTS = 0;
    for (unsigned i = 0; i < nSuperLayers(); i++) {
        unsigned tsType = 0;
        if (i)
            tsType = 1;

        const unsigned nLayers = _superLayers[i]->size();
        if (nLayers < 5) {
            cout << "TRGCDC !!! can not create TS because "
                 << "#layers is less than 5 in super layer " << i
                 << endl;
            continue;
        }

        //...TS layer... w is a central wire
        const TCCell & ww = *(* _superLayers[i])[layerOffset[tsType]]->front();
        TRGCDCLayer * layer = new TRGCDCLayer(id++, ww);
        _tsLayers.push_back(layer);

        //...Loop over all wires in a central wire layer...
        const unsigned nWiresInLayer = ww.layer().nCells();
        for (unsigned j = 0; j < nWiresInLayer; j++) {
            const TCWire & w =
                * (TCWire*)(* (* _superLayers[i])[layerOffset[tsType]])[j];

            const unsigned localId = w.localId();
            const unsigned layerId = w.layerId();
            vector<const TCWire*> cells;

            for (unsigned k = 0; k < nWiresInTS[tsType]; k++) {
                const unsigned laid = layerId + shape[tsType][k * 2];
                const unsigned loid = localId + shape[tsType][k * 2 + 1];

                const TCWire * c = wire(laid, loid);
                if (! c)
                    cout << "TRGCDC !!! no such a wire for TS : "
                         << "layer id=" << laid << ", local id=" << loid
                         << endl;

                cells.push_back(c);
            }

            //...Create a track segment...
//            TRGCDCSegment * ts = new TRGCDCSegment(idTS++,
//                                                   * layer,
//                                                   w,
//                                                   _luts.back(),
//                                                   _clockD,
//                                                   _eventTime.back(),
//						   _innerTSLUTFilename,
//						   _outerTSLUTFilename,
//                                                   cells);
    
            TRGCDCSegment * ts;
	if(w.superLayerId()){

            ts = new TRGCDCSegment(idTS++,
                                   * layer,
                                   w,
                                   _luts.back(),  //Will be Removed.
                                   _clockD,
                                   _eventTime.back(),
				   _outerTSLUTFilename,
                                   cells);
	}

	else{
            ts = new TRGCDCSegment(idTS++,
                                   * layer,
                                   w,
                                   _luts.back(),  // Will be removed.
                                   _clockD,
                                   _eventTime.back(),
				   _innerTSLUTFilename,
                                   cells);
  	}
    	    ts->initialize();
            tstmp=ts;

            //...Store it...
            _tss.push_back(ts);
            _tsSL[i].push_back(ts);
            layer->push_back(ts);
        }
    }

    tstmp->initialize(_fevtTime);

    //...Fill caches...
    if (_width) delete [] _width;
    if (_r) delete [] _r;
    if (_r2) delete [] _r2;
    _width = new float[nSuperLayers()];
    _r = new float[nSuperLayers() + 1];
    _r2 = new float[nSuperLayers() + 1];
    for (unsigned i = 0; i < nSuperLayers(); i++) {
        const vector<TRGCDCLayer*> & slayer = * _superLayers[i];
        _width[i] = M_PI * 2 / float(slayer.back()->nCells());
        _r[i] = slayer[0]->innerRadius();
        _r2[i] = _r[i] * _r[i];
        if (i == (nSuperLayers() - 1)) {
            _r[i + 1] = slayer.back()->outerRadius();
            _r2[i + 1] = _r[i + 1] * _r[i + 1];
        }

        if (TRGDebug::level() > 9) {
            const TCCell & wi = * slayer[0]->front();
            const unsigned layerId = wi.layerId();
            //cout << layerId << "," << cdc2.senseWireR(layerId) << "," << cdc2.fieldWireR(layerId) << endl;
            //cout << "    super layer " << i << " radius=" << _r[i] << "(r^2=" << _r2[i] << ")" << endl;
        }
    }

//*-----------------comment_begin-------------------

    //...Track Segment Finder...
    _tsFinder = new TSFinder(*this, _fileTSF, _fLogicLUTTSF);

   // _tsFinder->initialize(_innerTSLUTFilename, _outerTSLUTFilename);

    //...Perfect 2D Finder...
    _pFinder = new TCPFinder("Perfect2DFinder", * this);

    //...Perfect 3D Finder...
    _p3DFinder = 0;

    //...Hough 2D Finder...
    _hFinder = new TCHFinder("HoughFinder",
                             * this,
                             houghFinderMeshX,
                             houghFinderMeshY,
                             houghFinderPeakMin);

    //...Hough 3D Finder...
    _h3DFinder = new TCH3DFinder(*this, _fileHough3D, _finder3DMode);

    //...3D fitter...
    map<string, bool> flags = {{"fLRLUT",_fLRLUT},{"fEvtTime",_fevtTime},{"fzierror",_fzierror},
                               {"fmcLR",_fmclr},{"fRootFile",_fileFitter3D}};
    _fitter3D = new TCFitter3D("Fitter3D",
                               _rootFitter3DFilename,
                               * this,
                               flags);
    _fitter3D->initialize();


//----------------comment_end----------------*/

    //...For module simulation (Front-end)...
    configure();

    //...Initialize event number...
    m_eventNum = 1;


//----------------comment_begin-------------------

    //...Initialize root file...
    if(_makeRootFile) m_file = new TFile((char*)_rootTRGCDCFilename.c_str(), "RECREATE");
//m_file = new TFile("TRGCDC.root", "RECREATE");
    m_tree = new TTree("m_tree", "tree");
    m_treeAllTracks = new TTree("m_treeAllTracks", "treeAllTracks");

    m_fitParameters = new TClonesArray("TVectorD");
    m_mcParameters = new TClonesArray("TVectorD");
    m_mcTrack4Vector = new TClonesArray("TLorentzVector");
    m_mcTrackVertexVector = new TClonesArray("TVector3");
    m_mcTrackStatus = new TClonesArray("TVectorD");
    //    m_parameters2D = new TClonesArray("TVectorD");

    m_tree->Branch("fitParameters", &m_fitParameters,32000,0);
    m_tree->Branch("mcParameters", &m_mcParameters,32000,0);
    m_treeAllTracks->Branch("mcTrack4Vector", &m_mcTrack4Vector,32000,0);
    m_treeAllTracks->Branch("mcTrackVertexVector", &m_mcTrackVertexVector,32000,0);
    m_treeAllTracks->Branch("mcTrackStatus", &m_mcTrackStatus,32000,0);

    m_evtTime = new TClonesArray("TVectorD");
    m_treeAllTracks->Branch("evtTime", &m_evtTime,32000,0);

    _tree2D = new TTree("tree2D", "2D Tracks");
    _tracks2D = new TClonesArray("TVectorD");
    _tree2D->Branch("track parameters", & _tracks2D,32000,0);


    //...Initialize firmware ROOT input
    //m_minCDCTdc = 9999;
    //m_maxCDCTdc = 0;
    //m_minTRGTdc = 9999;
    //m_maxTRGTdc = 0;

    m_treeROOTInput = new TTree("m_treeROOTInput", "treeRootInput");
    //m_CDCTRGTimeMatch = new TClonesArray("TVectorD");
    m_rootCDCHitInformation = new TClonesArray("TVectorD");
    m_rootTRGHitInformation = new TClonesArray("TVectorD");
    m_rootTRGRawInformation = new TClonesArray("TObjString");
    //m_treeROOTInput->Branch("CDCTRGTimeMatch", &m_CDCTRGTimeMatch,32000,0);
    m_treeROOTInput->Branch("rootCDCHitInformation", &m_rootCDCHitInformation,32000,0);
    m_treeROOTInput->Branch("rootTRGHitInformation", &m_rootTRGHitInformation,32000,0);
    m_treeROOTInput->Branch("rootTRGRawInformation", &m_rootTRGRawInformation,32000,0);

//----------------comment_end----------------



        cout << "               -------------End TRGCDC initialization-------------"<< endl;

}

void
TRGCDC::terminate(void) {

        cout << "               -------------A TRGCDC is terminated-------------"<< endl;
///*
    if(_tsFinder) {
        _tsFinder->terminate();
    }
    if(_fitter3D) {
        _fitter3D->terminate();
    }
    if(_h3DFinder) {
        _h3DFinder->terminate();
    }
    if(_makeRootFile) {
        m_file->Write();
        m_file->Close();
    }
//*/
        cout << "               -------------End of termination-------------"<< endl;

}

void
TRGCDC::dump(const string & msg) const {
cout << "               -------------A TRGCDC is dumped-------------"<< endl;

    if (msg.find("name")    != string::npos ||
        msg.find("version") != string::npos ||
        msg.find("detail")  != string::npos ||
        msg == "") {
        cout << name() << "(CDC version=" << versionCDC() << ", "
             << version() << ") ";
    }
    if (msg.find("detail") != string::npos ||
        msg.find("state") != string::npos) {
        cout << "Debug Level=" << _debugLevel;
    }
    cout << endl;

    string tab("        ");

    if (msg == "" || msg.find("geometry") != string::npos) {

        //...Get information..."
        unsigned nLayer = _layers.size();
        cout << "    version    : " << version() << endl;
        cout << "    cdc version: " << versionCDC() << endl;
        cout << "    # of wires : " << _wires.size() << endl;
        cout << "    # of layers: " << nLayer << endl;
        cout << "    super layer information" << endl;
        cout << "        # of super layers() = "
             << nSuperLayers() << endl;
        cout << "        # of Axial super layers = "
             << nAxialSuperLayers() << endl;
        cout << "        # of Stereo super layers = "
             << nStereoSuperLayers() << endl;

        if (msg.find("superLayers") != string::npos) {
            cout << "        super layer detail" << endl;
            cout << "            id #layers (stereo type)" << endl;
            for (unsigned i = 0; i < nSuperLayers(); ++i) {
                const unsigned n = _superLayers[i]->size();
                cout << "            " << i << "  " << n << " (";
                for (unsigned j = 0; j < n; j++) {
                    cout << (* _superLayers[i])[0]->stereoType();
                }
                cout << ")" << endl;
            }
        }

        cout << "    layer information" << endl;
        cout << "        # of Axial layers = "
             << nAxialLayers() << endl;
        cout << "        # of Stereo layers = "
             << nStereoLayers() << endl;

        if (msg.find("layers") != string::npos) {
            cout << "        layer detail" << endl;
            cout << "            id type sId #wires lId asId assId"
                 << endl;
            for (unsigned int i = 0; i < nLayers(); ++i) {
                const TRGCDCLayer & l = * _layers[i];
                cout << "            " << i
                     << " " << l.stereoType()
                     << " " << l.superLayerId()
                     << " " << l.nCells()
                     << " " << l.localLayerId()
                     << " " << l.axialStereoLayerId()
                     << " " << l.axialStereoSuperLayerId()
                     << endl;
            }
        }

        if (msg.find("wires") != string::npos) {
            cout << "    wire information" << endl;
            for (unsigned i = 0; i < nWires(); i++)
                (_wires[i])->dump("neighbor", tab);
        }

        return;
    }
    if (msg.find("hits") != string::npos) {
        cout << "    hits : " << _hits.size() << endl;
        for (unsigned i = 0; i < (unsigned) _hits.size(); i++)
            _hits[i]->dump("mc drift", tab);
    }
    if (msg.find("axialHits") != string::npos) {
        cout << "    hits : " << _axialHits.size() << endl;
        for (unsigned i = 0; i < (unsigned) _axialHits.size(); i++)
            _axialHits[i]->dump("mc drift", tab);
    }
    if (msg.find("stereoHits") != string::npos) {
        cout << "    hits : " << _stereoHits.size() << endl;
        for (unsigned i = 0; i < (unsigned) _stereoHits.size(); i++)
            _stereoHits[i]->dump("mc drift", tab);
    }
    if (msg.find("trgWireHits") != string::npos) {
        const string dumpOption = "trigger detail";
        cout << "    wire hits" << endl;
        for (unsigned i = 0; i < nWires(); i++) {
            const TCWire & w = * wire(i);
            if (w.signal().active())
                w.dump(dumpOption, TRGDebug::tab(4));
        }
    }
    if (msg.find("trgWireCentralHits") != string::npos) {
        const string dumpOption = "trigger detail";
        cout << "    wire hits" << endl;
        for (unsigned i = 0; i < nSegments(); i++) {
            const TCSegment & s = segment(i);
            if (s.wires()[5]->signal().active())
                s.wires()[5]->dump(dumpOption, TRGDebug::tab(4));
        }
    }
    if (msg.find("trgTSHits") != string::npos) {
        const string dumpOption = "trigger detail";
        cout << "    TS hits" << endl;
        for (unsigned i = 0; i < nSegments(); i++) {
            const TCSegment & s = segment(i);
            if (s.signal().active())
                s.dump(dumpOption, TRGDebug::tab(4));
        }
    }

cout << "               -------------End of dump-------------"<< endl;
}

const TCWire *
TRGCDC::wire(unsigned id) const {
    if (id < nWires())
        return _wires[id];
    return 0;
}

const TCWire *
TRGCDC::wire(unsigned layerId, int localId) const {
    if (layerId < nLayers())
        return (TCWire*) & _layers[layerId]->cell(localId);
    return 0;
}

// const TCWire *
// TRGCDC::wire(const HepGeom::Point3D<double> & p) const {
//     float r = p.mag();
//     float phi = p.phi();
//     return wire(r, phi);
// }

const TCWire *
TRGCDC::wire(float , float) const {

    //...Not implemented yet...
    return _wires[0];

    // //     cout << "r,phi = " << r << "," << p << endl;

    // //     unsigned id = 25;
    // //     bool ok = false;
    // //     const TRGCDCLayer * l;
    // //     while (! ok) {
    // //         l = layer(id);
    // //         if (! l) return 0;

    // //         const geocdc_layer * geo = l->geocdc();
    // //         if (geo->m_r + geo->m_rcsiz2 < r) ++id;
    // //         else if (geo->m_r - geo->m_rcsiz1 > r) --id;
    // //         else ok = true;
    // //     }
    // //     float dPhi = 2. * M_PI / float(l->nCells());
    // //     if (l->geocdc()->m_offset > 0.) p -= dPhi / 2.;
    // //     unsigned localId = unsigned(phi(p) / dPhi);
    // //     return l->wire(localId);
    // }
}

void
TRGCDC::clear(void) {

        cout << "               -------------A TRGCDC is cleared-------------"<< endl;
    TCWHit::removeAll();
    TCSHit::removeAll();
    TCLink::removeAll();

    //     for (unsigned i = 0; i < _hits.size(); i++)
    //         delete _hits[i];
    for (unsigned i = 0; i < _hitsMC.size(); i++)
        delete _hitsMC[i];
    //     for (unsigned i = 0; i < _badHits.size(); i++)
    //         delete _badHits[i];
    //     for (unsigned i = 0; i < _segmentHits.size(); i++)
    //         delete _segmentHits[i];

    for(unsigned i = 0; i < _wires.size(); i++){
        TCWire * w = _wires[i];
        w->clear();
    }
    for(unsigned i = 0; i< _tss.size(); i++){
        TCSegment * s = _tss[i];
        s->clear();
    }
    _hitWires.clear();
    _hits.clear();
    _axialHits.clear();
    _stereoHits.clear();
    _badHits.clear();
    _hitsMC.clear();
    _segmentHits.clear();
    for (unsigned i = 0; i < 9; i++)
        _segmentHitsSL[i].clear();

        cout << "               -------------End of clear-------------"<< endl;
}

void
TRGCDC::fastClear(void) {
}

void
TRGCDC::update(bool) {

        cout << "               -------------A TRGCDC is updated-------------"<< endl;

    if(_trgCDCDataInputMode != 0) updateByData(_trgCDCDataInputMode);
    else {

        TRGDebug::enterStage("TRGCDC update");

        //...Clear old information...
        //  fastClear();
        clear();

        //...CDCSimHit...
        StoreArray<CDCSimHit> SimHits;
        if (! SimHits) {
            cout << "TRGCDC !!! can not access to CDCSimHits" << endl;
            TRGDebug::leaveStage("TRGCDC update");
            return;
        }
        const unsigned n = SimHits.getEntries();

        //...CDCHit...
        StoreArray<CDCHit> CDCHits("CDCHits");
        if (! CDCHits) {
            cout << "TRGCDC !!! can not access to CDCHits" << endl;
            TRGDebug::leaveStage("TRGCDC update");
            return;
        }
        const unsigned nHits = CDCHits.getEntries();

        //...MCParticle...
        StoreArray<MCParticle> mcParticles;
        if (! mcParticles) {
            cout << "TRGCDC !!! can not access to MCParticles" << endl;
            TRGDebug::leaveStage("TRGCDC update");
            return;
        }

        //...Relations...
        RelationArray rels(SimHits, CDCHits);
        const unsigned nRels = rels.getEntries();
        RelationArray relsMC(mcParticles, CDCHits);
        const unsigned nRelsMC = relsMC.getEntries();

        //...Loop over CDCHits...
        for (unsigned i = 0; i < nHits; i++) {
            const CDCHit & h = * CDCHits[i];
            double tmp = rand() / (double(RAND_MAX));
            if (tmp < _inefficiency)
                continue;

            //      //...Check validity (skip broken channel)...
            //      if (! (h->m_stat & CellHitFindingValid)) continue;

            //...Get CDCSimHit... This is expensive. Should be moved outside.
            unsigned iSimHit = 0;
            for (unsigned j = 0; j < nRels; j++) {
                const unsigned k = rels[j].getToIndices().size();
                for (unsigned l = 0; l < k; l++) {
                    if (rels[j].getToIndex(l) == i)
                        iSimHit = rels[j].getFromIndex();
                }

                if (TRGDebug::level())
                    if (k > 1)
                        cout << "TRGCDC::update !!! CDCSimHit[" << iSimHit
                             << "] has multiple CDCHit(" << k << " hits)" << endl;
            }


            //...Get MCParticle... This is expensive, again.
            //   (Getting the first MCParticle only)
            unsigned iMCPart = 0;
            for (unsigned j = 0; j < nRelsMC; j++) {
                const unsigned k = relsMC[j].getToIndices().size();
                for (unsigned l = 0; l < k; l++) {
                    if (relsMC[j].getToIndex(l) == i) {
                        iMCPart = relsMC[j].getFromIndex();
                        break;
                    }
                }

                if (TRGDebug::level())
                    if (k > 1)
                        cout << "TRGCDC::update !!! MCParticle[" << iMCPart
                             << "] has multiple CDCHit(" << k << " hits)" << endl;
            }


            //...Wire...
            int t_layerId;
            if (h.getISuperLayer() == 0) t_layerId = h.getILayer();
            else t_layerId = h.getILayer() + 6 * h.getISuperLayer() + 2;
            const unsigned layerId = t_layerId;
            const unsigned wireId = h.getIWire();
            TCWire & w = * (TCWire*) wire(layerId, wireId);


            //...TDC count...
            const int tdcCount = h.getTDCCount();

            //...Drift length from TDC...
            const float driftLength = tdcCount * (40. / 10000.);
            const float driftLengthError = 0.013;

            //...Trigger timing...
            TRGTime rise = TRGTime(tdcCount, true, _clockFE, w.name());
            TRGTime fall = rise;
            fall.shift(1).reverse();
            w._signal = TRGSignal(rise & fall);
            w._signal.name(w.name());

            //...Left/right...
            const int LRflag = SimHits[iSimHit]->getPosFlag();

            //...TCWireHit...
            TCWHit * hit = new TCWHit(w,
                                      i,
                                      iSimHit,
                                      iMCPart,
                                      driftLength,
                                      driftLengthError,
                                      driftLength,
                                      driftLengthError,
                                      LRflag,
                                      1);
            hit->state(CellHitFindingValid | CellHitFittingValid);

            //...Store a hit...
            ((TCWire *)(* _layers[layerId])[wireId])->hit(hit);
            _hits.push_back(hit);
            if (w.axial()) _axialHits.push_back(hit);
            else           _stereoHits.push_back(hit);

            //...Debug...
/*
            if (TRGDebug::level() > 2) {
                w._signal.dump("", TRGDebug::tab());
                std::cout << TRGDebug::tab(4) << "CDCHit TDC count=" << h.getTDCCount() << std::endl;
            }
*/
        }

        //...Track segment... This part is moved to ::simulate().

        //...Hit classification...
        //  _hits.sort(TCWHit::sortByWireId);
        classification();

        if (TRGDebug::level() > 2) {
            StoreArray<CDCSimHit> simHits("CDCSimHits");
/*
            if (TRGDebug::level()) {
                cout << TRGDebug::tab() << "#CDCSimHit=" << n << ",#CDCHit="
                     << nHits << endl;
            }
*/
            //_clock.dump("detail", TRGDebug::tab());
            //_clockFE.dump("detail", TRGDebug::tab());
            if (TRGDebug::level() > 10) {
                for (unsigned i = 0; i < _hits.size(); i++) {
                    const TCWHit & h = * _hits[i];
                    //h.dump("detail", TRGDebug::tab(4));
                }
            }
            else {
                const unsigned n = 10;
                cout << TRGDebug::tab() << "Dump of the first " << n
                     << " hits of a wire" << endl;
                for (unsigned i = 0; i < n; i++) {
                    const TCWHit & h = * _hits[i];
                    //h.dump("detail", TRGDebug::tab(4));
                }
            }
        }

        TRGDebug::leaveStage("TRGCDC update");
    }

	        cout << "               -------------End of updating-------------"<< endl;

}


void
TRGCDC::updateByData(int inputMode) {

        cout << "               -------------A TRGCDC is updateByData-------------"<< endl;


    TRGDebug::enterStage("TRGCDC update");
    //...Clear old information...
    clear();
    //cout<<"Event: "<<m_eventNum<<endl;

    // There is a 1 window difference between slow control parameter and actual data.
    // For SPrint-8 data
    int cdcDelay = 101;
    int nCDCWindows = 28+1;
    // For Fuji hall data
    //int cdcDelay = 87;
    //int nCDCWindows = 32+1;
    // For Fuji hall matching data
    //int cdcDelay = 79;
    //int nCDCWindows = 47+1;
    int nCdcBitsInWindow = 1536;

    // For TRG parameters
    unsigned nTRGWindows = 48;
    int nTrgBitsInWindow = 352;

    vector<string> trgInformations;

    // ##### Store data into a common binary format #####
    // inBinaryData[iBoard][iWord]
    vector<vector<unsigned> > inBinaryData; 
    // Get data using Belle2Link
    if(inputMode == 1) {

        //[FIXME] Getting data from ROOT file is fixed to 2 CDC FE, 1 TRG board.
        // Data should be in CDCFE0, CDCFE1, TRG sequence.

        // For Fuji data
        // Get data from root file.
        StoreArray<RawDataBlock> raw_datablkarray;
        // One block is one event. Take only first block = event.
        // Copper board 0 is for two CDC FE and one TRG.
        int iBlock = 0;
        int iCopper = 0;
        //cout<<"Number of blocks: "<<raw_cdcarray.getEntries()<<endl;
        //cout<<"Number of Copper: "<<raw_cdcarray[iBlock]->GetNumEntries()<<endl;
        int* temp_buf = raw_datablkarray[ iBlock ]->GetBuffer(iCopper);
        int nwords = raw_datablkarray[ iBlock ]->GetBlockNwords(iCopper);
        int malloc_flag = 0;
        int num_nodes = 1;
        int num_events = 1;
        RawCOPPER* raw_copper;
        RawCOPPER raw_copper_buf;
        raw_copper_buf.SetBuffer(temp_buf, nwords, malloc_flag, num_nodes, num_events);
        raw_copper = &raw_copper_buf;

        //// For SPring-8 data
        //StoreArray<RawCDC> raw_cdcarray;
        //int iBlock = 0;
        //int iCopper = 0;
        //RawCOPPER* raw_copper = raw_cdcarray[iBlock];

        //// Print the data
        //printf("*******Start of BODY**********\n");
        //printf("\n%.8d : ", 0);
        //for (int j = 0; j < raw_copper->GetBlockNwords(iCopper); j++) {
        //  printf("0x%.8x ", (raw_copper->GetBuffer(iCopper))[ j ]);
        //  if ((j + 1) % 10 == 0) {
        //    printf("\n%.8d : ", j + 1);
        //  }
        //}
        //printf("*******End of BODY**********\n");

        // Number of words for data
        int daqHeader = 32;
        int cdcHeader = 5;
        int cdcTrailer = 2;
        int trgHeader = 1;
        //int trgTrailer = 1;
        // 3 for event data. -1 for lost word in DAQ. Each window is 48 words.
        int widthCDCFEData = 3 - 1 + nCdcBitsInWindow/32*nCDCWindows;
        int widthTRGData = nTrgBitsInWindow/32*nTRGWindows;


        // Remove headers and trailer from data.
        // rawBinaryData[iBoard][iWord]
        // CDCFE0, CDCFE1, TSF sequence.
        vector<vector<unsigned> > rawBinaryData;
        for(int iFe=0; iFe<2; iFe++){
            int startCDCData = daqHeader + cdcHeader + (iFe*(widthCDCFEData+cdcTrailer+cdcHeader)) + 1;
            // 1 is due to missing data.
            vector<unsigned> t_feData(widthCDCFEData+1);
            // Loop over all the words for CDC FE data
            for(int iWord=0; iWord<widthCDCFEData; iWord++){
                t_feData[iWord] = (raw_copper->GetBuffer(iCopper))[iWord+startCDCData];
            }
            // Fill with 0 for last missing data
            t_feData[widthCDCFEData] = 0;
            rawBinaryData.push_back(t_feData);
        }
        vector<unsigned> t_trgData(widthTRGData);
        // Loop over all the words for TRG data
        int startTRGData = daqHeader + 2*(cdcHeader + widthCDCFEData + cdcTrailer) + trgHeader + 1;
        for(int iWord=0; iWord<widthTRGData; iWord++){
            t_trgData[iWord] = (raw_copper->GetBuffer(iCopper))[iWord+startTRGData];
        }
        rawBinaryData.push_back(t_trgData);
        //// Print data of rawBinaryData
        //for(unsigned iBoard=0; iBoard<rawBinaryData.size(); iBoard++){
        //  cout<<"Board"<<iBoard<<endl;
        //  for(unsigned iWord=0; iWord<rawBinaryData[iBoard].size(); iWord++){
        //    cout<<setfill('0')<<setw(8)<<hex<<rawBinaryData[iBoard][iWord];
        //  }
        //  cout<<dec<<endl;
        //}


        // Shift all binary data by 8 bits to the right for two CDC FE data
        for(unsigned iBoard=0; iBoard<2; iBoard++){
            unsigned t_buf=0;
            unsigned t_value;
            // 1 is due to missing data
            vector<unsigned> t_feData(widthCDCFEData+1);
            for(unsigned iWord=0; iWord<rawBinaryData[iBoard].size(); iWord++){
                t_value = t_buf + (rawBinaryData[iBoard][iWord] >> 8);
                t_buf = rawBinaryData[iBoard][iWord] << 24;
                t_feData[iWord] = t_value;
            }
            inBinaryData.push_back(t_feData);
        }
        inBinaryData.push_back(rawBinaryData[2]);
        //// Print data of inBinaryData
        //for(unsigned iBoard=0; iBoard<inBinaryData.size(); iBoard++){
        //  cout<<"Board"<<iBoard<<endl;
        //  for(unsigned iWord=0; iWord<inBinaryData[iBoard].size(); iWord++){
        //    cout<<setfill('0')<<setw(8)<<hex<<inBinaryData[iBoard][iWord];
        //  }
        //  cout<<dec<<endl;
        //}


    } else {
        cout<<"[ERROR} TRGCDCDataInputMode is incorrect! No simulation will be done."<<endl;
        return;
    }


    // #####Separate binary into meaningful data#####
    // CDC
    // cdcTrgTiming[iFE]
    vector<unsigned> cdcTrgTiming(2);
    for(unsigned iFe=0; iFe<2; iFe++){
        cdcTrgTiming[iFe] = (inBinaryData[iFe][1])>>16;
        if(cdcTrgTiming[iFe] >= 32768) cout<<"CDC trigger timing error. TDC is larger than 0x8000"<<endl;
    }
    //// Print cdc trigger timing
    //for(unsigned iFe=0; ieE<2; iFe++){
    //  cout<<"FE"<<iFe<<" TRG Timing: "<<setfill('0')<<setw(8)<<hex<<cdcTrgTiming[0]<<dec<<endl;
    //}
    // [iHit]  [0]: layerId, [1]: wireId, [2]: adc, [3]: tdc, [4]: FE trg timing
    vector<vector<unsigned> > hitCdcData;
    // Store information according to window,FE,Wire
    for(int iWindow=0; iWindow<nCDCWindows; iWindow++){
        for(int iFE=0; iFE<2; iFE++){
            for(int iWire=0; iWire<48; iWire++){
                unsigned t_adc;
                unsigned t_tdc;
                unsigned t_layerId = 3*iFE + ((iWire/8)%3);
                unsigned t_wireId = 8*(iWire/24) + iWire%8;
                // Take 16 bits according to wire.
                t_adc = ((inBinaryData[iFE][iWire/2+iWindow*48+3] << (16*(iWire%2))) >> 16);
                t_tdc = ((inBinaryData[iFE][iWire/2+iWindow*48+24+3] << (16*(iWire%2))) >> 16);
                if(t_tdc !=0) {
                    if(t_tdc >= 32768) {
                        // Calculate TDC.
                        t_tdc -= 32768;
                        // Round down and go back cdcDelay+nCDCWindows windows
                        unsigned startTiming;
                        if(unsigned(cdcTrgTiming[iFE]/32)*32 > (unsigned)(cdcDelay+nCDCWindows)*32){
                            startTiming = unsigned(cdcTrgTiming[iFE]/32)*32 - (cdcDelay+nCDCWindows)*32;
                        } else {
                            startTiming = unsigned(cdcTrgTiming[iFE]/32)*32 + 32768 - (cdcDelay+nCDCWindows)*32;
                        }
                        if(t_tdc > startTiming) t_tdc -= startTiming;
                        else t_tdc += 32768 - startTiming;
                        // Fill data
                        vector<unsigned> t_hitCdcData(5);
                        t_hitCdcData[0] = t_layerId;
                        t_hitCdcData[1] = t_wireId;
                        t_hitCdcData[2] = t_adc;
                        t_hitCdcData[3] = t_tdc;
                        t_hitCdcData[4] = cdcTrgTiming[iFE];
                        hitCdcData.push_back(t_hitCdcData);
                    } else {
                        cout<<"CDC TDC data error. TDC is smaller than 0x8000. "<<hex<<t_tdc<<dec<<endl;
                        continue;
                    }
                } // end tdc hit 
            } // wire loop
        } // fe loop
    } // window loop
    //// Print hit information
    //for(unsigned iHit=0; iHit<hitCdcData.size(); iHit++){
    //  cout<<"["<<hitCdcData[iHit][0]<<"]["<<hitCdcData[iHit][1]<<"]: "<<hitCdcData[iHit][2]<<", "<<hitCdcData[iHit][3]<<endl;
    //}
    // TRG
    // Save data into string
    stringstream t_trgWindow;
    for(unsigned iWord=0; iWord<inBinaryData[2].size(); iWord++){
        t_trgWindow << setw(8)<<setfill('0')<<hex<<inBinaryData[2][iWord];
        if(iWord%11 == 10) {
            trgInformations.push_back(t_trgWindow.str());
            t_trgWindow.str("");
        }
    }
    // Save data into hits
    // [iHit]  [0]: layerId, [1]: wireId, [2]: window timing, [3]: priority timing
    // priority timing = -1 : No timing
    vector<vector<int> > hitTrgData;
    for(unsigned iWindow=0; iWindow<nTRGWindows; iWindow++){
        // Change binary into TRGState
        vector<unsigned> t_trgData(inBinaryData[2].begin()+11*iWindow, inBinaryData[2].begin()+11*iWindow+11);
        TRGState t_trgState(t_trgData,1);
        TRGState t_mergerState = t_trgState.subset(96, 256);
        TRGState t_hitMap = t_mergerState.subset(0,80);
        TRGState t_priorityMap = t_mergerState.subset(32,16);
        TRGState t_secondPriorityMap = t_mergerState.subset(48,16);
        TRGState t_secondHitFlag= t_mergerState.subset(208,16);
        TRGState t_priorityTiming = t_mergerState.subset(80,64);
        TRGState t_fastestTiming = t_mergerState.subset(144,64);
        //cout<<"["<<iWindow<<"]  HM: "<<t_hitMap<<endl;
        //cout<<"["<<iWindow<<"] PHM: "<<t_priorityMap<<endl;
        //cout<<"["<<iWindow<<"] SHM: "<<t_secondPriorityMap<<endl;
        //cout<<"["<<iWindow<<"] SPF: "<<t_secondHitFlag<<endl;
        //cout<<"["<<iWindow<<"]  PT: "<<setfill('0')<<setw(16)<<hex<<(unsigned long long) (t_priorityTiming)<<dec<<endl;
        //cout<<"["<<iWindow<<"]  FT: "<<setfill('0')<<setw(16)<<hex<<(unsigned long long) (t_fastestTiming)<<dec<<endl;
        for(unsigned iWire=0; iWire<t_hitMap.size(); iWire++){
            unsigned t_layerId = iWire/16;
            unsigned t_wireId = iWire%16; 
            if(t_hitMap[iWire] == 1) {
                vector<int> t_hitTrgData(4);
                t_hitTrgData[0] = t_layerId;
                t_hitTrgData[1] = t_wireId;
                t_hitTrgData[2] = iWindow;
                // Calculate priority timing
                if(t_layerId==2){
                    t_hitTrgData[3] = (unsigned)t_priorityTiming.subset(4*t_wireId,4);
                    //cout<<"["<<t_layerId<<"]["<<t_wireId<<"]: "<<iWindow<<", "<<t_hitTrgData[3]<<endl;
                } else if(t_layerId==3) {
                    // [1] Priority hit, [0] secondaryLR
                    TRGState t_leftInformation(2);
                    TRGState t_rightInformation(2);
                    TRGState t_priorityHits(2);
                    TRGState t_secondaryLR(2);
                    if(t_wireId != 15){
                        t_priorityHits = t_priorityMap.subset(t_wireId, 2);
                        t_secondaryLR = t_secondHitFlag.subset(t_wireId,2);
                        t_leftInformation.set(1, t_priorityMap.subset(t_wireId+1,1));
                        t_leftInformation.set(0, t_secondHitFlag.subset(t_wireId+1,1));
                        t_rightInformation.set(1, t_priorityMap.subset(t_wireId,1));
                        t_rightInformation.set(0, t_secondHitFlag.subset(t_wireId,1));
                    } else {
                        t_priorityHits.set(0, t_priorityMap.subset(t_wireId, 1));
                        t_priorityHits.set(1,0);
                        t_secondaryLR.set(0, t_secondHitFlag.subset(t_wireId,1));
                        t_secondaryLR.set(1, 0);
                        t_leftInformation.set(1, 1);
                        t_leftInformation.set(0, 0);
                        t_rightInformation.set(1, t_priorityMap.subset(t_wireId,1));
                        t_rightInformation.set(0, t_secondHitFlag.subset(t_wireId,1));
                    }
                    //cout<<"["<<iWindow<<"]["<<t_layerId<<"]["<<t_wireId<<"] PriorityHits: "<<t_priorityHits<<endl;
                    //cout<<"["<<iWindow<<"]["<<t_layerId<<"]["<<t_wireId<<"] SecondaryLR: "<<t_secondaryLR<<endl;
                    //cout<<"["<<iWindow<<"]["<<t_layerId<<"]["<<t_wireId<<"] Combine0: "<<t_leftInformation<<endl;
                    //cout<<"["<<iWindow<<"]["<<t_layerId<<"]["<<t_wireId<<"] Combine1: "<<t_rightInformation<<endl;
                    int secondaryFlag = -1;
                    if((unsigned)t_leftInformation == (unsigned)TRGState("00",0)) secondaryFlag = 0;
                    if((unsigned)t_leftInformation == (unsigned)TRGState("01",0)) secondaryFlag = 1;
                    //cout<<"["<<iWindow<<"]["<<t_layerId<<"]["<<t_wireId<<"] SecondaryFlag: "<<secondaryFlag<<" timing: ";
                    if(secondaryFlag!=-1){
                        if(t_wireId+(1-secondaryFlag)<16){
                            //int t_priorityTiming = strtol(priorityTiming.substr(15-t_wireId-(1-secondaryFlag),1).c_str(),0,16);
                            unsigned t_secondaryTiming = (unsigned)t_priorityTiming.subset(4*(t_wireId+(1-secondaryFlag)),4);
                            t_hitTrgData[3] = t_secondaryTiming;
                            //cout<<t_secondaryTiming<<endl;
                        } else {
                            cout<<"Error in TRGDATA for secondary priority hits"<<endl;
                        }
                        //cout<<"Layer3end"<<endl;
                    } else {
                        t_hitTrgData[3] = -1;
                        //cout<<"-1"<<endl;
                    }
                } else {
                    t_hitTrgData[3] = -1;
                }
                hitTrgData.push_back(t_hitTrgData);
            } // wire is hit
        } // wire loop
    } // window loop
    //// Print hitWires from Trigger
    //if(hitTrgData.size()) cout<<"TRG: "<<endl;
    //for(unsigned iHit=0; iHit<hitTrgData.size(); iHit++){
    //  cout<<"["<<hitTrgData[iHit][0]<<"]["<<hitTrgData[iHit][1]<<"]: "<<hitTrgData[iHit][2]*32+hitTrgData[iHit][3]*2<<"("<<hitTrgData[iHit][2]<<","<<hitTrgData[iHit][3]<<")"<<endl;;
    //}
    //if(hitTrgData.size()) {
    //  for(unsigned iWindow=0; iWindow<nTRGWindows; iWindow++){
    //    vector<unsigned> t_trgData(inBinaryData[2].begin()+11*iWindow, inBinaryData[2].begin()+11*iWindow+11);
    //    TRGState t_trgState(t_trgData,1);
    //    TRGState t_mergerState = t_trgState.subset(96, 256);
    //    TRGState t_hitMap = t_mergerState.subset(0,80);
    //    TRGState t_priorityMap = t_mergerState.subset(32,16);
    //    TRGState t_secondPriorityMap = t_mergerState.subset(48,16);
    //    TRGState t_secondHitFlag= t_mergerState.subset(208,16);
    //    TRGState t_priorityTiming = t_mergerState.subset(80,64);
    //    TRGState t_fastestTiming = t_mergerState.subset(144,64);
    //    cout<<"["<<iWindow<<"]  HM: "<<t_hitMap<<endl;
    //    cout<<"["<<iWindow<<"] PHM: "<<t_priorityMap<<endl;
    //    cout<<"["<<iWindow<<"] SHM: "<<t_secondPriorityMap<<endl;
    //    cout<<"["<<iWindow<<"] SPF: "<<t_secondHitFlag<<endl;
    //    cout<<"["<<iWindow<<"]  PT: "<<setfill('0')<<setw(16)<<hex<<(unsigned long long) (t_priorityTiming)<<dec<<endl;
    //    cout<<"["<<iWindow<<"]  FT: "<<setfill('0')<<setw(16)<<hex<<(unsigned long long) (t_fastestTiming)<<dec<<endl;
    //  }
    //}




    int trgOutput = 2;
    // Store TRG data to TRGSignalBundle.
    // 1 is FE output. 2 is Merger output. 3 is TRG output.
    //...Clock...
    const TRGClock & dClock = TRGCDC::getTRGCDC()->dataClock();
    // Create signal bundle
    TRGSignalBundle* allTrgData;
    allTrgData = new TRGSignalBundle(string("TRGData"), dClock);
    // Create signal vector. Need to create signals for of signal vector.
    TRGSignalVector* trgData; 
    trgData = new TRGSignalVector(string("TRGData"), dClock, nTrgBitsInWindow);
    // Loop over data and fill TRGSignalVector
    for(unsigned iWindow=0; iWindow<nTRGWindows; iWindow++){
        vector<unsigned> t_trgData(inBinaryData[2].begin()+11*iWindow, inBinaryData[2].begin()+11*iWindow+11);
        TRGState t_trgState(t_trgData,1);
        trgData->set(t_trgState, iWindow);
    }
    allTrgData->push_back(trgData);
    // Clean up memory
    if(allTrgData) {
        for(unsigned i=0; i<allTrgData->size();i++) delete (* allTrgData)[i];
        delete allTrgData;
    }


    // Save CDC hit data and TRG hit data to ROOT file.
    saveTRGRawInformation(trgInformations);
    saveCDCHitInformation(hitCdcData);
    if(trgOutput == 2) {
        saveTRGHitInformation(hitTrgData);
    }
    m_treeROOTInput->Fill();

    // Update CDC wires using data. Start from layer 50.
    for(unsigned iHit=0; iHit<hitCdcData.size(); iHit++){
        unsigned t_layerId = hitCdcData[iHit][0] + 50;
        unsigned t_wireId = hitCdcData[iHit][1];
        int t_rawTdc = hitCdcData[iHit][2];
        TCWire& currentWire = *(TCWire*) wire(t_layerId, t_wireId);

        // Update currentWire._signal
        TRGTime rise = TRGTime(t_rawTdc, true, _clockFE, currentWire.name());
        TRGTime fall = rise;
        fall.shift(1).reverse();
        if(currentWire._signal.active()){
            currentWire._signal = currentWire.signal() | TRGSignal(rise & fall);
            currentWire._signal.name(currentWire.name());
        }else{
            currentWire._signal = TRGSignal(rise & fall);
            currentWire._signal.name(currentWire.name());

            // Make TCWireHit and store in _layers.
            // [FIXME] TCWireHit can not accept wire being hit two times.
            TCWHit * hit = new TCWHit(currentWire, 0, 0, 0, t_rawTdc, 0, t_rawTdc, 0, 0, 1);
            hit->state(CellHitFindingValid | CellHitFittingValid);
            ((TCWire *)(* _layers[t_layerId])[t_wireId])->hit(hit);
            _hits.push_back(hit);
            if (currentWire.axial()) _axialHits.push_back(hit);
            else           _stereoHits.push_back(hit);
        }
        //cout<<"iLayer, iWire, iHit: "<<t_layerId<<" "<<t_wireId<<" "<<iHit<<endl;
        //currentWire.signal().dump();

    } // End updating CDC wires



    //...Hit classification...
    classification();

    m_eventNum++;

    TRGDebug::leaveStage("TRGCDC update");

        cout << "               -------------End of updateByData-------------"<< endl;

}



void
TRGCDC::classification(void) {
	cout << "               -------------classification-------------"<< endl;

    unsigned n = _hits.size();

    for (unsigned i = 0; i < n; i++) {
        TCWHit * h = _hits[i];
        const TCWire & w = h->wire();
        unsigned state = h->state();

        //...Cache pointers to a neighbor...
        const TCWire * neighbor[7];
        for (unsigned j = 0; j < 7; j++) neighbor[j] = w.neighbor(j);

        //...Decide hit pattern...
        unsigned pattern = 0;
        for (unsigned j = 0; j < 7; j++) {
            if (neighbor[j])
                if (neighbor[j]->hit())
                    pattern += (1 << j);
        }
        state |= (pattern << CellHitNeighborHit);

        //...Check isolation...
        const TCWHit * hr1 = neighbor[2]->hit();
        const TCWHit * hl1 = neighbor[3]->hit();
        if ((hr1 == 0) && (hl1 == 0)) {
            state |= CellHitIsolated;
        } else {
            const TCWHit * hr2 = neighbor[2]->neighbor(2)->hit();
            const TCWHit * hl2 = neighbor[3]->neighbor(3)->hit();
            if (((hr2 == 0) && (hr1 != 0) && (hl1 == 0)) ||
                ((hl2 == 0) && (hl1 != 0) && (hr1 == 0)))
                state |= CellHitIsolated;
        }

        //...Check continuation...
        //        unsigned superLayer = w.superLayerId();
        bool previous = false;
        bool next = false;
        if (neighbor[0] == 0) previous = true;
        else {
            if ((neighbor[0]->hit()) || neighbor[1]->hit())
                previous = true;
            //             if (m_smallcell && w.layerId() == 3)
            //                 if (neighbor[6]->hit())
            //                     previous = true;
        }
        if (neighbor[5] == 0) next = true;
        else {
            if ((neighbor[4]->hit()) || neighbor[5]->hit())
                next = true;
        }
        // if (previous && next) state |= CellHitContinuous;
        if (previous || next) state |= CellHitContinuous;

        //...Solve LR locally...
        if ((pattern == 34) || (pattern == 42) ||
            (pattern == 40) || (pattern == 10) ||
            (pattern == 35) || (pattern == 50))
            state |= CellHitPatternRight;
        else if ((pattern == 17) || (pattern == 21) ||
                 (pattern == 20) || (pattern ==  5) ||
                 (pattern == 19) || (pattern == 49))
            state |= CellHitPatternLeft;

        //...Store it...
        h->state(state);
    }

cout << "               -------------End of classification-------------"<< endl;
}

vector<const TCWHit *>
TRGCDC::axialHits(void) const {
    vector<const TCWHit *> t;
    t.assign(_axialHits.begin(), _axialHits.end());
    return t;

    //     if (! mask) return _axialHits;
    //     else if (mask == CellHitFindingValid) return _axialHits;
    //     cout << "TRGCDC::axialHits !!! unsupported mask given" << endl;
    //  return _axialHits;
}

vector<const TCWHit *>
TRGCDC::stereoHits(void) const {
    vector<const TCWHit *> t;
    t.assign(_stereoHits.begin(), _stereoHits.end());
    return t;

    //     if (! mask) return _stereoHits;
    //     else if (mask == CellHitFindingValid) return _stereoHits;
    //     cout << "TRGCDC::stereoHits !!! unsupported mask given" << endl;
    //     return _stereoHits;
}

vector<const TCWHit *>
TRGCDC::hits(void) const {
    vector<const TCWHit *> t;
    t.assign(_hits.begin(), _hits.end());
    return t;

    //     if (! mask) return _hits;
    //     else if (mask == CellHitFindingValid) return _hits;
    //     cout << "TRGCDC::hits !!! unsupported mask given" << endl;
    //     return _hits;
}

// vector<const TCWHit *>
// TRGCDC::badHits(void) const {
//     vector<const TCWHit *> t;
//     t.assign(_badHits.begin(), _badHits.end());
//     return t;

// //cnv     if (! updated()) update();
// //     if (_badHits.length()) return _badHits;

// //     //...Loop over RECCDC_WIRHIT bank...
// //    x unsigned nReccdc = BsCouTab(RECCDC_WIRHIT);
// //     for (unsigned i = 0; i < nReccdc; i++) {
// //         x struct reccdc_wirhit * h =
// //             (struct reccdc_wirhit *)
// //             BsGetEnt(RECCDC_WIRHIT, i + 1, BBS_No_Index);

// //         //...Check validity...
// //         if (h->m_stat & CellHitFindingValid) continue;

// //         //...Obtain a pointer to GEOCDC...
// //         x geocdc_wire * g =
// //             (geocdc_wire *) BsGetEnt(GEOCDC_WIRE, h->m_geo, BBS_No_Index);

// //         //...Get a pointer to a TRGCDCWire...
// //         TCWire * w = _wires[g->m_ID - 1];

// //         //...Create TCWHit...
// //         _badHits.append(new TCWHit(w, h, _fudgeFactor));
// //     }

// //     return _badHits;
// }

vector<const TCWHitMC*>
TRGCDC::hitsMC(void) const {
    vector<const TCWHitMC*> t;
    t.assign(_hitsMC.begin(), _hitsMC.end());
    return t;
}

string
TRGCDC::wireName(unsigned wireId) const {
    string as = "-";
    const TCWire * const w = wire(wireId);
    if (w) {
        if (w->stereo())
            as = "=";
    } else {
        return "invalid_wire(" + TRGUtil::itostring(wireId) + ")";
    }
    return TRGUtil::itostring(layerId(wireId)) + as + TRGUtil::itostring(localId(wireId));
}

unsigned
TRGCDC::localId(unsigned id) const {
    cout << "TRGCDC::localId !!! this function is not tested yet"
         << endl;
    unsigned iLayer = 0;
    unsigned nW = 0;
    unsigned nWLast = 0;
    bool nextLayer = true;
    while (nextLayer) {
        nWLast = nW;
        nW += layer(iLayer++)->nCells();
        if (id < (nW - 1))
            return id - nWLast;
        if (nW >= nWires())
            nextLayer = false;
    }
    cout << "TRGCDC::localId !!! no such a wire (id=" << id << endl;
    return TRGCDC_UNDEFINED;
}

unsigned
TRGCDC::layerId(unsigned id) const {
    cout << "TRGCDC::layerId !!! this function is not tested yet"
         << endl;
    unsigned iLayer = 0;
    unsigned nW = 0;
    bool nextLayer = true;
    while (nextLayer) {
        nW += layer(iLayer++)->nCells();
        if (id < (nW - 1))
            return iLayer - 1;
        if (nW >= nWires())
            nextLayer = false;
    }
    cout << "TRGCDC::layerId !!! no such a wire (id=" << id << endl;
    return TRGCDC_UNDEFINED;
}

unsigned
TRGCDC::layerId(unsigned , unsigned) const {
    cout << "TRGCDC::layerId !!! this function is not implemented yet"
         << endl;
    return TRGCDC_UNDEFINED;
}

unsigned
TRGCDC::superLayerId(unsigned id) const {
    unsigned iLayer = 0;
    unsigned nW = 0;
    bool nextLayer = true;
    while (nextLayer) {
        const vector<TRGCDCLayer*> & sl = * superLayer(iLayer);
        const unsigned nLayers = sl.size();
        for (unsigned i = 0; i < nLayers; i++)
            nW += sl[i]->nCells();

        if (id < (nW - 1))
            return iLayer;
        if (nW >= nWires())
            nextLayer = false;
    }
    cout << "TRGCDC::superLayerId !!! no such a wire (id=" << id
         << endl;
    return TRGCDC_UNDEFINED;
}

unsigned
TRGCDC::localLayerId(unsigned id) const {
    unsigned iLayer = 0;
    unsigned nW = 0;
    bool nextLayer = true;
    while (nextLayer) {
        const vector<TRGCDCLayer*> & sl = * superLayer(iLayer);
        const unsigned nLayers = sl.size();
        for (unsigned i = 0; i < nLayers; i++) {
            nW += sl[i]->nCells();
            if (id < (nW - 1))
                return i;
        }

        if (nW >= nWires())
            nextLayer = false;
    }
    cout << "TRGCDC::localLayerId !!! no such a wire (id=" << id
         << endl;
    return TRGCDC_UNDEFINED;
}

unsigned
TRGCDC::axialStereoSuperLayerId(unsigned aors, unsigned i) const {
    unsigned is = 99;
    //    cout << "aors,i= " << aors <<" "<< i << std::endl;
    if       (aors == 0) { //axial
        if       ( i <=  7) {
            is = 0;
        } else if( i <= 13) {
            is = 1;
        } else if( i <= 19) {
            is = 2;
        } else if( i <= 25) {
            is = 3;
        } else if( i <= 31) {
            is = 4;
        }      

    } else if(aors == 1) { //stereo
        if       ( i <=  5) {
            is = 0;
        } else if( i <= 11) {
            is = 1;
        } else if( i <= 17) {
            is = 2;
        } else if( i <= 23) {
            is = 3;
        }      
    }

    assert(is != 99);
    return is;
}

// void
// TRGCDC::driftDistance(TLink & l,
//                     const TTrack & t,
//                     unsigned flag,
//                     float t0Offset) {

//     //...No correction...
//     if (flag == 0) {
//         if (l.hit()) {
// //             l.drift(0, l.hit()->drift(0));
// //             l.drift(1, l.hit()->drift(1));
// //             l.dDrift(0, l.hit()->dDrift(0));
// //             l.dDrift(1, l.hit()->dDrift(1));
//             l.drift(l.hit()->drift(0), 0);
//             l.drift(l.hit()->drift(1), 1);
//             l.dDrift(l.hit()->dDrift(0), 0);
//             l.dDrift(l.hit()->dDrift(1), 1);
//         }
//         else {
// //             l.drift(0, 0.);
// //             l.drift(1, 0.);
// //             l.dDrift(0, 0.);
// //             l.dDrift(1, 0.);
//             l.drift(0., 0);
//             l.drift(0., 1);
//             l.dDrift(0., 0);
//             l.dDrift(0., 1);
//         }

//         return;
//     }

//     //...TOF correction...
//     float tof = 0.;
//     if (flag && 1) {
//         int imass = 3;
//         float tl = t.helix().a()[4];
//         float f = sqrt(1. + tl * tl);
//         float s = fabs(t.helix().curv()) * fabs(l.dPhi()) * f;
//         float p = f / fabs(t.helix().a()[2]);
//         calcdc_tof2_(& imass, & p, & s, & tof);
//     }

//     //...T0 correction....
//     if (! (flag && 2)) t0Offset = 0.;

//     //...Propagation corrections...
//     const TCWHit & h = * l.hit();
//     int wire = h.wire()->id();
//     HepGeom::Vector3D<double> tp = t.helix().momentum(l.dPhi());
//     float p[3] = {tp.x(), tp.y(), tp.z()};
//     const HepGeom::Point3D<double> & onWire = l.positionOnWire();
//     float x[3] = {onWire.x(), onWire.y(), onWire.z()};
// //cnv    float time = h.reccdc()->m_tdc + t0Offset - tof;
//     float time = 0;
//     float dist;
//     float edist;
//     int prop = (flag & 4);

//     //...Calculation with left side...
//     int side = -1;
//     if (side == 0) side = -1;
//     calcdc_driftdist_(& prop,
//                       & wire,
//                       & side,
//                       p,
//                       x,
//                       & time,
//                       & dist,
//                       & edist);
// //     l.drift(0, dist);
// //     l.dDrift(0, edist);
//     l.drift(dist, 0);
//     l.dDrift(edist, 0);

//     //...Calculation with left side...
//     side = 1;
//     calcdc_driftdist_(& prop,
//                       & wire,
//                       & side,
//                       p,
//                       x,
//                       & time,
//                       & dist,
//                       & edist);
// //     l.drift(1, dist);
// //     l.dDrift(1, edist);
//     l.drift(dist, 1);
//     l.dDrift(edist, 1);

//     //...tan(lambda) correction...
//     if (flag && 8) {
//         float tanl = abs(p[2]) / tp.perp();
//         float c;
//         if ((tanl >= 0.0) && (tanl < 0.5))      c = -0.48 * tanl + 1.3;
//         else if ((tanl >= 0.5) && (tanl < 1.0)) c = -0.28 * tanl + 1.2;
//         else if ((tanl >= 1.0) && (tanl < 1.5)) c = -0.16 * tanl + 1.08;
//         else                                    c =  0.84;

// //         l.dDrift(0, l.dDrift(0) * c);
// //         l.dDrift(1, l.dDrift(1) * c);
//         l.dDrift(l.dDrift(0) * c, 0);
//         l.dDrift(l.dDrift(1) * c, 1);
//     }
// }

TRGCDC::~TRGCDC() {

        cout << "               -------------A TRGCDC is destructed-------------"<< endl;
    clear();

    delete [] _width;
    delete [] _r;
    delete [] _r2;

    if(_tsFinder) delete _tsFinder;
    if (_hFinder)
        delete _hFinder;
    if (_h3DFinder)
        delete _h3DFinder;
    if (_fitter3D)
        delete _fitter3D;

//******* Will be removed -KT
    for (unsigned i = 0; i < _luts.size(); i++)
        delete _luts[i];

#ifdef TRGCDC_DISPLAY
    if (D)
        delete D;
    cout << "TRGCDC ... rphi displays deleted" << endl;
#endif
}

bool
TRGCDC::neighbor(const TCWire & w0, const TCWire & w1) const {
    const int lyr0 = w0.layerId();
    const int lyr1 = w1.layerId();
    const int lyr = lyr0 - lyr1;

    if (abs(lyr) > 1) return false;
    if (w0.superLayerId() != w1.superLayerId()) return false;

    for (unsigned i = 0; i < 7; i++) {
        if (w0.neighbor(i)) {
            if (w0.neighbor(i)->id() == w1.id())
                return true;
        }
    }
    return false;
}

void
TRGCDC::simulate(void) {
        cout << "               -------------A TRGCDC is simulated-------------"<< endl;

    const bool fast = (_simulationMode & 1);
    const bool firm = (_simulationMode & 2);
    if (fast)
        fastSimulation();
    if (firm)
        firmwareSimulation();

        cout << "               -------------End of simulate-------------"<< endl;
}

void
TRGCDC::fastSimulation(void) {
        cout << "               -------------A TRGCDC is fastsimulated-------------"<< endl;

#ifdef TRGCDC_DISPLAY
    D->beginningOfEvent();
#endif

    TRGDebug::enterStage("TRGCDC simulation");

    //...Options...
    const bool trackSegmentSimulationOnly = _fastSimulationMode & 1;
    const bool trackSegmentClockSimulation = _fastSimulationMode & 2;

    _tsFinder->doit(_tss, trackSegmentClockSimulation, _segmentHits, _segmentHitsSL);

    //    //...Store TS hits...
    //    const unsigned n = _tss.size();
    //    for (unsigned i = 0; i < n; i++) {
    //	TCSegment & s = * _tss[i];
    //	s.simulate(trackSegmentClockSimulation);
    //	if (s.signal().active()) {
    //            TCSHit * th = new TCSHit(s);
    //            s.hit(th);
    //            _segmentHits.push_back(th);
    //            _segmentHitsSL[s.layerId()].push_back(th);
    //
    //	    //...Create TCShit...
    ////	    unsigned j = 0;
    ////	    const TCWire * w = s[j];
    ////	    TCSHit * th = 0;
    ////	    while (w) {
    ////		const TCWHit * h = w->hit();
    ////		if (h) {
    ////		    if (! th) {
    ////			th = new TCSHit(s);
    ////			s.hit(th);
    ////			_segmentHits.push_back(th);
    ////			_segmentHitsSL[s.layerId()].push_back(th);
    ////		    }
    ////		    s._hits.push_back(h);
    ////		}
    ////		w = s[++j];
    ////	    }
    //	}
    //    }
    //
    //    if (TRGDebug::level() > 1) {
    //	cout << TRGDebug::tab() << "TS hit list" << endl;
    //	string dumpOption = "trigger";
    //	if (TRGDebug::level() > 2)
    //	    dumpOption = "detail";
    //	for (unsigned i = 0; i < nSegments(); i++) {
    //	    const TCSegment & s = segment(i);
    //	    if (s.signal().active())
    //		s.dump(dumpOption, TRGDebug::tab(4));
    //	}
    //
    //	cout << TRGDebug::tab() << "TS hit list (2)" << endl;
    //	if (TRGDebug::level() > 2)
    //	    dumpOption = "detail";
    //	for (unsigned i = 0; i < _segmentHits.size(); i++) {
    //	    const TCSHit & s = * _segmentHits[i];
    //	    s.dump(dumpOption, TRGDebug::tab(4));
    //	}
    //
    //	cout << TRGDebug::tab() << "TS hit list (3)" << endl;
    //	if (TRGDebug::level() > 2)
    //	    dumpOption = "detail";
    //	for (unsigned j = 0; j < _superLayers.size(); j++) {
    //	    for (unsigned i = 0; i < _segmentHitsSL[j].size(); i++) {
    //		const vector<TCSHit*> & s = _segmentHitsSL[j];
    //		for (unsigned k = 0; k < s.size(); k++)
    //		    s[k]->dump(dumpOption, TRGDebug::tab(4));
    //	    }
    //	}
    //    }


    if (trackSegmentSimulationOnly) {
        TRGDebug::leaveStage("TRGCDC simulation");
        return;
    }

    //...2D tracker : Hough finder...

    vector<TCTrack *> trackList;
/*
    if (_perfect2DFinder)
        _pFinder->doit(trackList);
    else
        _hFinder->doit(trackList);
*/


    //...Perfect position test...
    if (trackList.size()) {
        vector<HepGeom::Point3D<double> > ppos =
            trackList[0]->perfectPosition();
        if (TRGDebug::level()) {
            if (ppos.size() != 9) {
                cout << TRGDebug::tab() << "There are only " << ppos.size()
                     << " perfect positions" << endl;
            }
        }
    }

    //...Stereo finder...
    //_h3DFinder->doit(trackList, m_eventNum);

    //...Check tracks...
    if (TRGDebug::level()) {
        for (unsigned i = 0; i < trackList.size(); i++) {
            const TCTrack & t = * trackList[i];
            cout << ">   links=" << t.links().size() << endl;
            t.dump();
        }
    }

    //...Check relations...
    if (TRGDebug::level()) {
        for (unsigned i = 0; i < trackList.size(); i++) {
            //          trackList[i]->relation().dump();
            trackList[i]->dump();
        }
    }

    //...Event Time... In ns scale.
    // [FIXME] This does nothing.
    _eventTime.back()->getT0();

    //...3D tracker...
    vector<TCTrack*> trackList3D;
    //_fitter3D->doit(trackList, trackList3D);

    //...End of simulation...

    //:::Fill root file:::
    //...Event Time...
    TClonesArray & evtTime = *m_evtTime;
    evtTime.Clear();
    TVectorD tempEvtTime(1);
    tempEvtTime[0] = _eventTime.back()->getT0();
    new(evtTime[0]) TVectorD(tempEvtTime);
    //...MCParticle...
    StoreArray<MCParticle> mcParticles;
    //if (! mcParticles) {
    //  cout << "TRGCDC !!! can not access to MCParticles" << endl;
    //  TRGDebug::leaveStage("TRGCDC simulation");
    //  return;
    //}
    TClonesArray & fitParameters = *m_fitParameters;
    TClonesArray & mcParameters = *m_mcParameters;
    TClonesArray & mcTrack4Vector = *m_mcTrack4Vector;
    TClonesArray & mcTrackVertexVector = *m_mcTrackVertexVector;
    TClonesArray & mcTrackStatus = *m_mcTrackStatus;
    fitParameters.Clear();
    mcParameters.Clear();
    mcTrack4Vector.Clear();
    mcTrackVertexVector.Clear();
    mcTrackStatus.Clear();
    int iFit = 0;
    for (unsigned i = 0; i < trackList3D.size(); i++) {
        const TCTrack & aTrack = *trackList3D[i];
        if (aTrack.fitted()) {
            double fitPt = aTrack.pt();
            double fitPhi0 = aTrack.helix().phi0();
            if(aTrack.charge()<0) {
                fitPhi0 -= M_PI;
                if (fitPhi0 < 0) fitPhi0 += 2 * M_PI;
            }
            double fitZ0 = aTrack.helix().dz();
            double fitCot = aTrack.helix().tanl();
            // pT, phi0, z0, cot
            TVectorD tempFitParameters(4);
            tempFitParameters[0] = fitPt;
            tempFitParameters[1] = fitPhi0;
            tempFitParameters[2] = fitZ0;
            tempFitParameters[3] = fitCot;
            new(fitParameters[iFit]) TVectorD(tempFitParameters);

            const TCRelation & trackRelation = aTrack.relation();
            const MCParticle & trackMCParticle = trackRelation.mcParticle(0);
            //iw        const TCRelation & trackRelation3D = aTrack.relation3D();

            double mcPt = trackMCParticle.getMomentum().Pt();
            double mcPhi0;
            if (trackMCParticle.getCharge() > 0) mcPhi0 = trackMCParticle.getMomentum().Phi() - M_PI / 2;
            if (trackMCParticle.getCharge() < 0) mcPhi0 = trackMCParticle.getMomentum().Phi() + M_PI / 2;
            // Change range to [0,2pi]
            if (mcPhi0 < 0) mcPhi0 += 2 * M_PI;
            double mcZ0 = trackMCParticle.getVertex().Z() / 100;
            double mcCot = trackMCParticle.getMomentum().Pz() / trackMCParticle.getMomentum().Pt();

            tempFitParameters[0] = mcPt;
            tempFitParameters[1] = mcPhi0;
            tempFitParameters[2] = mcZ0;
            tempFitParameters[3] = mcCot;
            new(mcParameters[iFit]) TVectorD(tempFitParameters);
            iFit += 1;
        } // if fitted
    } // trackList loop

    // Store trackIDs that make a TS in the superlayer.
    vector<vector<unsigned > > trackIDsLayer;
    // Loop over all layers
    for(unsigned iLayers=0; iLayers<nSegmentLayers(); iLayers++){
        trackIDsLayer.push_back(vector<unsigned>());
        // Loop over all TS in superlayer.
        vector<const TCSHit *> const & hits = segmentHits(iLayers);
        for(unsigned j=0; j<hits.size(); j++) {
            const TCWHit * wh = hits[j]->segment().center().hit();
            if(! wh) continue;
            const unsigned trackID = wh->iMCParticle();
            //cout<<"Found trackID "<<trackID<<" in Layer["<<iLayers<<"]? "<<binary_search(trackIDsLayer[iLayers].begin(), trackIDsLayer[iLayers].end(), trackID)<<endl;
            if(! binary_search(trackIDsLayer[iLayers].begin(), trackIDsLayer[iLayers].end(), trackID)){
                trackIDsLayer[iLayers].push_back(trackID);
                //cout<<"Layer: "<<iLayers<<" trackID: "<<trackID<<endl;
            }
            // For comparing
            sort(trackIDsLayer[iLayers].begin(),trackIDsLayer[iLayers].end());
        } // End of loop over all TS in superlayer
    } // End of loop over all layers
    // Find common trackIDs.
    vector<unsigned > trackIDs;

    // Loop over last layer candidates
    for(unsigned iTS=0; iTS<trackIDsLayer[nSegmentLayers()-1].size(); iTS++){
        unsigned tempID = trackIDsLayer[nSegmentLayers()-1][iTS];
        bool trackOK = 1;
        // Loop over rest of the layers to compare
        for(unsigned iLayers=0; iLayers<nSegmentLayers()-1; iLayers++){
            //cout<<"Find "<<tempID<<" in Layer["<<iLayers<<"]: "<<binary_search(trackIDsLayer[iLayers].begin(), trackIDsLayer[iLayers].end(), tempID)<<endl;
            if(! binary_search(trackIDsLayer[iLayers].begin(), trackIDsLayer[iLayers].end(), tempID)){
                trackOK *= 0; 
            }
        } // End of loop over rest of layers
        if(trackOK==1) {
            trackIDs.push_back(tempID);
            //cout<<"trackID OK: "<<tempID<<endl;
        }
    } // End of loop over last layer candidates

#ifdef TRGCDC_DISPLAY
    if (trackList.size() == 0) {
        cout << "!!! No track found. Display will be stopped" << endl;
        D->stop();
    }
#endif
    _tracks2D->Clear();
    for (unsigned i = 0; i < trackList.size(); i++) {
        const TCTrack & trk = * trackList[i];

        TCHelix h = trk.helix();
        static const HepGeom::Point3D<double> Origin(0, 0, 0);
        h.pivot(Origin);
        const double chg = trk.charge();
        const double pt = h.momentum().perp();
        const double phi = h.phi0();
        const double dr = h.dr();

        const TCRelation & trkRelation = trk.relation();
        const MCParticle & trkMC = trkRelation.mcParticle(0);
        TCHelix hMC = TCHelix(
            HepGeom::Point3D<double>(trkMC.getProductionVertex().x(),
                                     trkMC.getProductionVertex().y(),
                                     trkMC.getProductionVertex().z()),
            CLHEP::Hep3Vector(trkMC.getMomentum().x(),
                              trkMC.getMomentum().y(),
                              trkMC.getMomentum().z()),
            trkMC.getCharge());
        hMC.pivot(Origin);
        const double chgMC = trkMC.getCharge();
        const double ptMC = hMC.momentum().perp();
        const double phiMC = hMC.phi0();
        const double drMC = hMC.dr();

        TVectorD tvd(8);
        tvd[0] = chg;
        tvd[1] = pt;
        tvd[2] = phi;
        tvd[3] = dr;
        tvd[4] = chgMC;
        tvd[6] = ptMC;
        tvd[6] = phiMC;
        tvd[7] = drMC;

        new((* _tracks2D)[i]) TVectorD(tvd);

        //cout << "drMC = " << drMC << ", drmc2 = " << trkMC.getVertex().X()
        //     << endl;
    }

    //...MC track information...
    const unsigned localSwitch = 0;           // 0:for found MC trk only
    // 1:for all MC trk
    // mcStatus[0]: statusbit, mcStatus[1]: pdg, mcStatus[2]: charge
    TVectorD mcStatus(3);
    int iStoredPart = 0;
    if (localSwitch == 0) {

        // Loop over all the found mc tracks.
        for(unsigned i = 0; i < trackIDs.size(); i++) {
            mcStatus[0] = mcParticles[trackIDs[i]]->getStatus();
            mcStatus[1] = mcParticles[trackIDs[i]]->getPDG();
            mcStatus[2] = mcParticles[trackIDs[i]]->getCharge();
            new(mcTrackStatus[iStoredPart]) TVectorD(mcStatus);
            new(mcTrack4Vector[iStoredPart])
                TLorentzVector(mcParticles[trackIDs[i]]->get4Vector());
            new(mcTrackVertexVector[iStoredPart])
                TVector3(mcParticles[trackIDs[i]]->getVertex());
            // cout << "ID: " << mcParticles[i]->getIndex()
            // 	 << " Status: " << mcParticles[i]->getStatus()
            // 	 << " PDG: " << mcParticles[i]->getPDG()
            // 	 << " Charge: " << mcParticles[i]->getCharge()
            // 	 << " pT: " << mcParticles[i]->get4Vector().Pt()
            // 	 << endl;
            iStoredPart += 1;
        }
    }
    else if (localSwitch == 1) {

        //...Get MC contribution list...
        if (! _perfect2DFinder) {
            vector<TCTrack *> dummyList;
            _pFinder->doit(trackList);
        }
        const vector<int> & trackListMC = _pFinder->trackListMC();

        // cout << "size=" << trackListMC.size() << endl;
        // for (unsigned i = 0; i < trackListMC.size(); i++) {
        //     cout << "    " << i << " " << trackListMC[i] << endl;
        // }

        //...Loop over all MC tracks...
        for (signed i = 0; i < mcParticles.getEntries(); i++) {

            //...Loop over contribution list...
            bool found = false;
            for (unsigned j = 0; j < trackListMC.size(); j++) {
                if (mcParticles[i]->getIndex() == trackListMC[j]) {
                    found = true;
                    break;
                }
            }

            //...Check contribution...
            if (! found) continue;

            //...Check pT...
            if (mcParticles[i]->getMomentum().Mag() < 0.01) continue;

            mcStatus[0] = mcParticles[i]->getStatus();
            mcStatus[1] = mcParticles[i]->getPDG();
            mcStatus[2] = mcParticles[i]->getCharge();

            //...iw special...
            mcStatus[2] = 0;
            if (trackList.size())
                mcStatus[2] += 1;
            if (trackList3D.size())
                mcStatus[2] += 10;

            new(mcTrackStatus[iStoredPart]) TVectorD(mcStatus);
            new(mcTrack4Vector[iStoredPart])
                TLorentzVector(mcParticles[i]->get4Vector());
            new(mcTrackVertexVector[iStoredPart])
                TVector3(mcParticles[i]->getVertex());
            //  cout << "ID: " << mcParticles[i]->getIndex()
            // << " Status: " << mcParticles[i]->getStatus()
            // << " PDG: " << mcParticles[i]->getPDG()
            // << " Charge: " << mcParticles[i]->getCharge()
            // << " pT: " << mcParticles[i]->get4Vector().Pt()
            // << endl;
            iStoredPart += 1;
        }
    }

    //cout<<"Num Tracks: "<<iStoredPart<<endl;

    if (iFit != 0) {
        m_tree->Fill();
    }
    m_treeAllTracks->Fill();
    _tree2D->Fill();

    m_eventNum += 1;

#ifdef TRGCDC_DISPLAY
    //cdc.dump("hits");
    vector<const TCTrack*> tt;
    tt.assign(trackList.begin(), trackList.end());
    D->endOfEvent();
    string stg = "2D : Perfect Finding";
    string inf = "   ";
    D->clear();
    D->stage(stg);
    D->information(inf);
    D->area().append(hits());
    D->area().append(segmentHits());
    D->area().append(tt);
    D->show();
    D->run();
    //     unsigned iFront = 0;
    //     while (const TCFrontEnd * f = _cdc.frontEnd(iFront++)) {
    //         D->clear();
    //         D->beginEvent();
    //         D->area().append(* f);
    //         D->run();
    //     }
    //     unsigned iMerger = 0;
    //     while (const TCMerger * f = _cdc.merger(iMerger++)) {
    //         D->clear();
    //         D->beginEvent();
    //         D->area().append(* f);
    //         D->run();
    //     }
#endif

    TRGDebug::leaveStage("TRGCDC simulation");
    return;
        cout << "               -------------End of fastsimulate-------------"<< endl;
}

void
TRGCDC::firmwareSimulation(void) {

        cout << "               -------------A TRGCDC is firmwaresimulated -------------"<< endl;

    //...Wire level simulation is in update().
#ifdef TRGCDC_DISPLAY
    D->beginningOfEvent();
#endif

    //const bool trackSegmentSimulationOnly = _fastSimulationMode & 1;
    const bool trackSegmentClockSimulation = _fastSimulationMode & 2;
    //...Front ends...
    const unsigned nFronts = _fronts.size();
    for (unsigned i = 0; i < nFronts; i++) {

        //...Skip inner-most FE because they have no contribution to TRG
        if (i < 10)
            continue;

        //...Skip other layers to speed up simulation
    //    if ( !(i==254 || i==278) ) continue;
//	if ( i > 49 || i < 10 ) continue;

        //...FE simulation...

        _fronts[i]->simulate();
    }

    //// Get results
    //vector<TRGSignalBundle*> feSignalBundle(2);
    //feSignalBundle[0] = _fronts[254]->getOSB();
    //feSignalBundle[1] = _fronts[278]->getOSB();
    //// Print results
    //for(int iFE=0; iFE<feSignalBundle.size(); iFE++){
    //  cout<<"FE"<<iFE<<" output"<<endl;
    //  vector<int> changeHistory = feSignalBundle[iFE]->stateChanges();
    //  for(int iHistory=0; iHistory<changeHistory.size(); iHistory++){
    //    int t_time = changeHistory[iHistory];
    //    TRGState t_feState = feSignalBundle[iFE]->state(t_time);
    //    string t_feOutput = t_feState;
    //    cout<<"["<<t_time<<"]: "<<t_feOutput<<endl;
    //  }
    //}

    //// Get TRG output
    //TObjString rootTRGRawInformation;
    //for(int iWindow=0; iWindow<m_rootTRGRawInformation->GetEntriesFast(); iWindow++){
    //  rootTRGRawInformation = *(TObjString*)m_rootTRGRawInformation->At(iWindow);
    //  stringstream t_trgRawInformation; 
    //  t_trgRawInformation << rootTRGRawInformation.String();
    //  cout<<"["<<iWindow<<"]: "<<t_trgRawInformation.str().substr(0,64)<<endl;
    //}

    //// Compare results
    //for(int iWindow=0; iWindow<m_rootTRGRawInformation->GetEntriesFast(); iWindow++){
    //  // TRG Firmware
    //  rootTRGRawInformation = *(TObjString*)m_rootTRGRawInformation->At(iWindow);
    //  stringstream t_trgRawInformation; 
    //  t_trgRawInformation << rootTRGRawInformation.String();
    //  // TRG simulation. One clock is used as simulated calculation time.
    //  string t_feOutput = feSignalBundle[0]->state(iWindow+1);
    //  //if(t_trgRawInformation.str().substr(0,64) != t_feOutput)
    //  if(t_trgRawInformation.str().substr(5,56) != t_feOutput.substr(5,56)) {
    //    cout<<"[Error] Firmware and TSIM do not match"<<endl;
    //    //cout<<"Firmware ["<<iWindow<<"]: "<<t_trgRawInformation.str().substr(0,64)<<endl;
    //    //cout<<"TSIM     ["<<iWindow<<"]: "<<t_feOutput<<endl;
    //    cout<<"Firmware ["<<iWindow<<"]: "<<t_trgRawInformation.str().substr(8,56)<<endl;
    //    cout<<"TSIM     ["<<iWindow+1<<"]: "<<t_feOutput.substr(8,56)<<endl;
    //  }
    //}

    //// 2013,1003: physjg: add to simulate Mergers
    const unsigned nMergers = _mergers.size();
    ////    cout << "Merger no: " << nMergers << endl;
    for (unsigned i = 0; i < nMergers; i++) {

	//if ( i > 19 || i < 0 ) continue;

      _mergers[i]->simulate(); 

    }


    const unsigned nTSFBoards = _tsfboards.size();
    for (unsigned i=0;i<nTSFBoards; i++){
//	if ( i > 1 || i < 0 ) continue;
	_tsfboards[i]->simulateBoard();
}
    //...TSF...
    //_tsFinder->doit(_tss, trackSegmentClockSimulation, _segmentHits, _segmentHitsSL);

    //...Event Time... In ns scale. 
    // [FIXME] Not really firmware mode. Just for display.
    double t_eventTime = _eventTime.back()->getT0();
    // Update WireHit driftTime using eventTime
    // [FIXME] This method is a temorary method. Next time change everything inside EventTime class.
    //cout<<"Hit timing ";
    for(unsigned iHits=0; iHits<_hits.size(); iHits++){
        //cout<<"["<<_hits[iHits]->wire().layerId()<<"]["<<_hits[iHits]->wire().localId()<<"]: "<<(_hits[iHits]->drift(0))<<" ";
        _hits[iHits]->setDriftTime((_hits[iHits]->drift(0)-t_eventTime)/250, 0);
        _hits[iHits]->setDriftTime((_hits[iHits]->drift(1)-t_eventTime)/250, 1);
    }
    //cout<< endl;
    //cout<<"Event timing: "<<t_eventTime<<endl;

#ifdef TRGCDC_DISPLAY
    //cdc.dump("hits");
    D->endOfEvent();
    string stg = "2D : Perfect Finding";
    string inf = "   ";
    D->clear();
    D->stage(stg);
    D->information(inf);
    D->area().append(hits());
    D->area().append(segmentHits());
    D->show();
    D->run();
    //     unsigned iFront = 0;
    //     while (const TCFrontEnd * f = _cdc.frontEnd(iFront++)) {
    //         D->clear();
    //         D->beginEvent();
    //         D->area().append(* f);
    //         D->run();
    //     }
    //     unsigned iMerger = 0;
    //     while (const TCMerger * f = _cdc.merger(iMerger++)) {
    //         D->clear();
    //         D->beginEvent();
    //         D->area().append(* f);
    //         D->run();
    //     }
#endif

        cout << "               -------------End of firmwaresimulate-------------"<< endl;
}

void
TRGCDC::configure(void) {

        cout << "               -------------A TRGCDC is configured-------------"<< endl;

    //...Open configuration file...
    ifstream infile(_configFilename.c_str(), ios::in);
    if (infile.fail()) {
        cout << "TRGCDC !!! can not open file" << endl
             << "    " << _configFilename << endl;
        return;
    }

    //...Read configuration data...
    char b[800];
    unsigned lines = 0;
    string cdcVersion = "";
    string configVersion = "";


    /*
    // A temporary one to list the content of the configuration file
    unsigned fbase=0, mbase=0, mno=0;
    while (! infile.eof()) {
    infile.getline(b, 800);
    const string l(b);
    string cdr = l;

    bool skip = false;
    unsigned wid = 0;
    unsigned lid = 0;
    unsigned fid = 0;
    unsigned mid = 0;
    unsigned tid = 0;
    for (unsigned i = 0; i < 5; i++) {
    const string car = TRGUtil::carstring(cdr);
    cdr = TRGUtil::cdrstring(cdr);

    if (car == "#") {
    cout << l << endl;
    skip = true;
    break;
    } else if (car == "CDC") {
    cout << l << endl;
    skip = true;
    break;
    }

    if (i == 0) {
    wid = atoi(car.c_str());
    } else if (i == 1) {
    lid = atoi(car.c_str());
    } else if (i == 2) {
    fid = atoi(car.c_str());
    } else if (i == 3) {
    mid = atoi(car.c_str());
    } else if (i == 4) {
    tid = atoi(car.c_str());
    }
    }
    if (!skip) {
    int  delta_fid = 0, newmid=99999;
    if  (mid != 99999) {   // modify mid from original config file to have 2 FE to 1 merger unit
    switch (tid) {
    case 0: fbase = 10;      mbase = 0;       mno = 10; break;
    case 1: fbase = 30;      mbase = 10;      mno = 10; break;
    case 2: fbase = 50;      mbase = 20;      mno = 12; break;
    case 3: fbase = 74;      mbase = 32;      mno = 14; break;
    case 4: fbase = 102;     mbase = 46;      mno = 16; break;
    case 5: fbase = 134;     mbase = 62;      mno = 18; break;
    case 6: fbase = 170;     mbase = 80;      mno = 20; break;
    case 7: fbase = 210;     mbase = 100;     mno = 22; break;
    case 8: fbase = 254;     mbase = 122;     mno = 24; break;
    default: cout << "tid/mid = " << tid << " / " << mid << " ---> somthing wrong!!!!" << endl; break;
    }
    delta_fid = fid - fbase;
    if (delta_fid >= mno) delta_fid -= mno;
    if ( (delta_fid<0) || ( delta_fid>mno) ) cout <<  "Mid calculation wrong at layer/FE = " << lid << "/" << fid << endl;
    newmid = delta_fid + mbase;
    }
    //	  cout  << "     "  <<  wid << "        " << lid << "       " << fid << "       "  << mid << "       " << newmid << "       " << tid << endl;
    cout  << "      "  <<  wid << "          " << lid << "          " << fid << "       "  << newmid << "       " << tid << endl;
    mid = newmid;
    }
    }
    */

    while (! infile.eof()) {
        infile.getline(b, 800);
        const string l(b);
        string cdr = l;

        bool skip = false;
        unsigned wid = 0;
        unsigned lid = 0;
        unsigned fid = 0;
        unsigned mid = 0;
        unsigned tid = 0;
        if(lid!=0) mid = lid + tid; //jb
        for (unsigned i = 0; i < 5; i++) {
            const string car = TRGUtil::carstring(cdr);
            cdr = TRGUtil::cdrstring(cdr);

            if (car == "#") {
                skip = true;
                break;
            } else if (car == "CDC") {
                if (l.find("CDC Wire Config Version") != string::npos)
                    cdcVersion = l;
                else if (l.find("CDC TRG Config Version") != string::npos)
                    configVersion = l;
                skip = true;
                break;
            }

            if (i == 0) {
                wid = atoi(car.c_str());
            } else if (i == 1) {
                lid = atoi(car.c_str());
            } else if (i == 2) {
                fid = atoi(car.c_str());
            } else if (i == 3) {
                mid = atoi(car.c_str());
            } else if (i == 4) {
                tid = atoi(car.c_str());
            }
        }

        if (skip)
            continue;
        if (lines != wid)
            continue;

	//cout << " wireid " << wid << " layerid " << lid << " Front-end id" << fid << " Merger id " << mid << endl;

        //...Make a front-end board if necessary...
        bool newFrontEnd = false;
        TCFrontEnd * f = 0;
        if (fid < _fronts.size())
            f = _fronts[fid];
        if (! f) {
            newFrontEnd = true;
            const string name = "CDCFrontEnd_" + TRGUtil::itostring(fid);
            TCFrontEnd::boardType t = TCFrontEnd::unknown;
            if (_wires[wid]->superLayerId() == 0) {
                if (_wires[wid]->localLayerId() < 5)
                    t = TCFrontEnd::innerInside;
                else
                    t = TCFrontEnd::innerOutside;
            }
            else {
                if (_wires[wid]->localLayerId() < 3)
                    t = TCFrontEnd::outerInside;
                else
                    t = TCFrontEnd::outerOutside;
            }
            f = new TCFrontEnd(name, t, _clock, _clockD, _clockUser3125);
	
//	cout << "New FE name: " << name << endl;

            _fronts.push_back(f);
        }
        f->push_back(_wires[wid]);

//	cout << "Wire # " << wid << "is pushed back into FE " << f->name() << endl;
        //...Make a merger board if necessary...
        // 2013,0908: physjg: I think this should be done only when a new frontboard is created.
        //                    i.e., inlcuded in the if(!f) { .... } block.
        //
        // Or I can put this part inside the new FrontEnd creation part, well, seems not good in coding

	bool newMerger=false;
        TCMerger * m = 0;
        if (newFrontEnd) {
            if (mid != 99999) {
                if (mid < _mergers.size())
                    m = _mergers[mid];
                if (! m) {
		    newMerger = true;
                    const string name = "CDCMerger_" + TRGUtil::itostring(mid);
                    TCMerger::unitType mt = TCMerger::unknown;
                    if (_wires[wid]->superLayerId() == 0) 
                        mt = TCMerger::innerType;
                    else
                        mt = TCMerger::outerType;
                    m = new TCMerger(name,
                                     mt,
                                     _clock,
                                     _clockD,
                                     _clockUser3125,
                                     _clockUser6250);
                    _mergers.push_back(m);
                }
                m->push_back(f);
            }

            //...Make Aurora channel...
            if (f && m) {
                const string n = f->name() + string("-") + m->name();
                TRGChannel * ch = new TRGChannel(n, * f, * m);
                f->appendOutput(ch);
                m->appendInput(ch);
            }
        }
	
	if (newMerger){
//	  TCTSFBoard * t=0;
	  TSFinder * t=0;
	  if(tid !=99999){
	    if(tid<_tsfboards.size())
	      t=_tsfboards[tid];
	    if(!t){
	      const string name = "CDCTSFBoard_"+TRGUtil::itostring(tid);
	      TSFinder::boardType tt=TSFinder::unknown;
	      if(_wires[wid]->superLayerId()==0)
	        //tt=TCTSFBoard::innerType;
	        tt=TSFinder::innerType;
	      else
	        //tt=TCTSFBoard::outerType;
	        tt=TSFinder::outerType;
//	      t = new TCTSFBoard(name,
	      t = new TSFinder(*this,
				 name,
				 tt,
				 _clock,
				 _clockD,
				 _clockUser3125,
				 _clockUser6250,
				 _tsSL[tid]);
 //  t->initialize(_innerTSLUTFilename, _outerTSLUTFilename);
	      _tsfboards.push_back(t);
	    }
	    t->push_back(m);
	  }

	  if(m && t){
	    const string n = m->name() + string("-")+t->name();
	    TRGChannel * chmt = new TRGChannel (n, *m, *t);
	    m->appendOutput(chmt);
	    t->appendInput(chmt);
	  }
	}

        ++lines;
    }
    infile.close();

        cout << "               -------------End of configure-------------"<< endl;
}

const TRGCDCSegment&
TRGCDC::segment(unsigned lid, unsigned id) const {
    return * (const TRGCDCSegment*)(* _tsLayers[lid])[id];
}

//jb void
//jb TRGCDC::perfect3DFinder(vector<TCTrack*> trackList) const {
//jb 
//jb   TRGDebug::enterStage("Perfect 3D Finder");
//jb   if (TRGDebug::level())
//jb     cout << TRGDebug::tab() << "givenTrk#=" << trackList.size() << endl;
//jb 
//jb   //...Track loop....
//jb   for (unsigned j = 0; j < trackList.size(); j++) {
//jb 
//jb     //...G4 trackID...
//jb     TCTrack * trk = trackList[j];
//jb     unsigned id = trackList[j]->relation().contributor(0);
//jb     vector<const TCSHit*> tsList[9];
//jb 
//jb     //...Segment loop...
//jb     const vector<const TCSHit*> hits = segmentHits();
//jb     for (unsigned i = 0; i < hits.size(); i++) {
//jb       const TCSHit & ts = * hits[i];
//jb       if (ts.segment().axial()) continue;
//jb       if (! ts.signal().active()) continue;
//jb       const TCWHit * wh = ts.segment().center().hit();
//jb       if (! wh) continue;
//jb       const unsigned trackId = wh->iMCParticle();
//jb 
//jb       if (id == trackId)
//jb         tsList[wh->wire().superLayerId()].push_back(& ts);
//jb     }
//jb 
//jb     if (TRGDebug::level()) {
//jb       cout << TRGDebug::tab() << "trk#" << j << endl;
//jb       for (unsigned k = 0; k < 9; k++) {
//jb         if (k % 2) {
//jb           cout << TRGDebug::tab(4) << "superlayer " << k << ":";
//jb           for (unsigned l = 0; l < tsList[k].size(); l++) {
//jb             if (l)
//jb               cout << ",";
//jb             cout << tsList[k][l]->cell().name();
//jb           }
//jb           cout << endl;
//jb         }
//jb       }
//jb     }
//jb 
//jb     //...Select best one in each super layer...
//jb     for (unsigned i = 0; i < 9; i++) {
//jb       const TCSHit * best = 0;
//jb       if (tsList[i].size() == 0) {
//jb         continue;
//jb       } else if (tsList[i].size() == 1) {
//jb         best = tsList[i][0];
//jb       } else {
//jb         int timeMin = 99999;
//jb         for (unsigned k = 0; k < tsList[i].size(); k++) {
//jb           const TRGSignal & signal = tsList[i][k]->signal();
//jb           const TRGTime & t = * signal[0];
//jb           if (t.time() < timeMin) {
//jb             timeMin = t.time();
//jb             best = tsList[i][k];
//jb           }
//jb         }
//jb       }
//jb       //trk->append(new TCLink(0,
//jb       //                       best,
//jb       //                       best->cell().xyPosition()));
//jb     }
//jb 
//jb     if (TRGDebug::level())
//jb       trk->dump("", "> ");
//jb 
//jb   }
//jb 
//jb   TRGDebug::leaveStage("Perfect 3D Finder");
//jb }

double TRGCDC::getEventTime(void) const {
    return _eventTime.back()->getT0();
}

unsigned TRGCDC::getEventNumber(void) const {
    return m_eventNum;
}

void TRGCDC::saveCDCHitInformation(vector<vector<unsigned> >& hitWiresFromCDC) {

    TClonesArray& rootCDCHitInformation = *m_rootCDCHitInformation;
    rootCDCHitInformation.Clear();
    for(unsigned iHit=0; iHit<hitWiresFromCDC.size(); iHit++){
        TVectorD t_rootCDCHitInformation(5);
        t_rootCDCHitInformation[0] = hitWiresFromCDC[iHit][0];
        t_rootCDCHitInformation[1] = hitWiresFromCDC[iHit][1];
        t_rootCDCHitInformation[2] = hitWiresFromCDC[iHit][2];
        t_rootCDCHitInformation[3] = hitWiresFromCDC[iHit][3];
        t_rootCDCHitInformation[4] = hitWiresFromCDC[iHit][4];
        new(rootCDCHitInformation[iHit]) TVectorD(t_rootCDCHitInformation);
    } // End of hit loop

}

void TRGCDC::saveTRGHitInformation(vector<vector<int> > & hitWiresFromTRG) {

    TClonesArray& rootTRGHitInformation = *m_rootTRGHitInformation;
    rootTRGHitInformation.Clear();
    for(unsigned iHit=0; iHit<hitWiresFromTRG.size(); iHit++){
        TVectorD t_rootTRGHitInformation(4);
        t_rootTRGHitInformation[0] = hitWiresFromTRG[iHit][0];
        t_rootTRGHitInformation[1] = hitWiresFromTRG[iHit][1];
        t_rootTRGHitInformation[2] = hitWiresFromTRG[iHit][2];
        t_rootTRGHitInformation[3] = hitWiresFromTRG[iHit][3];
        new(rootTRGHitInformation[iHit]) TVectorD(t_rootTRGHitInformation);
    } // End of hit loop

}

void TRGCDC::saveTRGRawInformation(vector<string > & trgInformations) {

    TClonesArray& rootTRGRawInformation = *m_rootTRGRawInformation;
    rootTRGRawInformation.Clear();
    for(unsigned iWindow=0; iWindow<trgInformations.size(); iWindow++){
        TObjString t_rootTRGRawInformation;
        t_rootTRGRawInformation.SetString(trgInformations[iWindow].c_str());
        new(rootTRGRawInformation[iWindow]) TObjString(t_rootTRGRawInformation);
    } // End of hit loop

}

//void TRGCDC::saveCDCTRGTimeMatchInformation(vector<vector<map<int, int> > >& hitTimingComp) {
//
//  TClonesArray& CDCTRGTimeMatch = *m_CDCTRGTimeMatch;
//  CDCTRGTimeMatch.Clear();
//  int nTotalHits = 0;
//  for(unsigned iLayer=0; iLayer<5; iLayer++){
//    for(unsigned iWire=0; iWire<16; iWire++){
//      for(map<int, int>::iterator it = hitTimingComp[iLayer][iWire].begin(); it != hitTimingComp[iLayer][iWire].end(); it++){
//          TVectorD t_CDCTRGTimeMatch(4);
//          t_CDCTRGTimeMatch[0] = iLayer;
//          t_CDCTRGTimeMatch[1] = iWire;
//          t_CDCTRGTimeMatch[2] = (*it).first;
//          t_CDCTRGTimeMatch[3] = (*it).second;
//          new(CDCTRGTimeMatch[nTotalHits++]) TVectorD(t_CDCTRGTimeMatch);
//      }
//    } // End of wire loop
//  } // End of layer loop
//
//}

} // namespace Belle2
