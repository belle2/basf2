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

#include <cstdlib>
#include "framework/datastore/StoreArray.h"
#include "framework/datastore/RelationArray.h"
#include "generators/dataobjects/MCParticle.h"
#include "cdc/dataobjects/CDCHit.h"
#include "cdc/dataobjects/CDCSimHit.h"
#include "cdc/geometry/CDCGeometryPar.h"
#include "trg/trg/Debug.h"
#include "trg/trg/Time.h"
#include "trg/trg/Signal.h"
#include "trg/trg/Link.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/TrackMC.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/LUT.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"
#include "trg/cdc/HoughFinder.h"
#include "trg/cdc/Fitter3D.h"
#include "trg/cdc/Link.h"
#include "trg/cdc/Relation.h"

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
    return string("TRGCDC 5.14");
}

TRGCDC *
TRGCDC::_cdc = 0;

TRGCDC *
TRGCDC::getTRGCDC(const string & configFile,
		  unsigned simulationMode,
		  unsigned firmwareSimulationMode,
		  const string & innerTSLUTDataFile,
		  const string & outerTSLUTDataFile,
                  bool houghFinderPerfect,
                  unsigned houghFinderMeshX,
                  unsigned houghFinderMeshY) {
    if (_cdc){
        //delete _cdc;
        _cdc = 0;
    }

    if (configFile != "good-bye") {
        _cdc = new TRGCDC(configFile,
			  simulationMode,
			  firmwareSimulationMode,
			  innerTSLUTDataFile,
			  outerTSLUTDataFile,
                          houghFinderPerfect,
                          houghFinderMeshX,
                          houghFinderMeshY);
    }
    else {
        cout << "TRGCDC::getTRGCDC ... good-bye" << endl;
//        delete _cdc;
        _cdc = 0;
    }

    return _cdc;
}

TRGCDC *
TRGCDC::getTRGCDC(void) {
    if (! _cdc)
        cout << "TRGCDC::getTRGCDC !!! TRGCDC is not created yet" << endl;
    return _cdc;
}

TRGCDC::TRGCDC(const string & configFile,
	       unsigned simulationMode,
	       unsigned firmwareSimulationMode,
	       const string & innerTSLUTDataFile,
	       const string & outerTSLUTDataFile,
	       bool houghFinderPerfect,
	       unsigned houghFinderMeshX,
	       unsigned houghFinderMeshY) 
    : _debugLevel(0),
      _configFilename(configFile),
      _simulationMode(simulationMode),
      _firmwareSimulationMode(firmwareSimulationMode),
      _innerTSLUTDataFilename(innerTSLUTDataFile),
      _outerTSLUTDataFilename(outerTSLUTDataFile),
      _fudgeFactor(1.),
      _width(0),
      _r(0),
      _r2(0),
      _clock("CDCTrigge system clock", Belle2_GDL::GDLSystemClock, 1),
      _clockFE("CDCFETrigge system clock", Belle2_GDL::GDLSystemClock, 8),
      _offset(5.3),
      _hFinder(0),
      _fitter3D(0) {

#ifdef TRGCDC_DISPLAY
    int argc = 0;
    char ** argv = 0;
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

    initialize(houghFinderPerfect, houghFinderMeshX, houghFinderMeshY);

    if (TRGDebug::level()) {
        cout << "TRGCDC ... TRGCDC created with " << _configFilename << endl;
        Belle2_GDL::GDLSystemClock.dump();
        _clock.dump();
        _clockFE.dump();
    }
}

void
TRGCDC::initialize(bool houghFinderPerfect,
                   unsigned houghFinderMeshX,
                   unsigned houghFinderMeshY) {

    //...CDC...
    Belle2::CDCGeometryPar & cdc2 = * Belle2::CDCGeometryPar::Instance();
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
        }
        else {
            ++is;
            axialStereoLayerId = is;
        }

        //...Is this in a new super layer?...
        if ((lastNWires != nWiresInLayer) || (lastShifts != nShifts)) {
            ++superLayerId;
            superLayer = new vector<TRGCDCLayer *>;
            _superLayers.push_back(superLayer);
            if (axial) {
                ++ias;
                axialStereoSuperLayerId = ias;
                _axialSuperLayers.push_back(superLayer);
            }
            else {
                ++iss;
                axialStereoSuperLayerId = iss;
                _stereoSuperLayers.push_back(superLayer);
            }
            lastNWires = nWiresInLayer;
            lastShifts = nShifts;
        }

        //...Calculate radius...
        const float swr = cdc2.senseWireR(i);
        float fwr = cdc2.fieldWireR(i);
        if (i == nLayers - 2)
            fwrLast = fwr;
        else if (i == nLayers - 1)
            fwr = swr + (swr - fwrLast);
        const float innerRadius = swr - (fwr - swr);
        const float outerRadius = swr + (fwr - swr);

        if (TRGDebug::level() > 9)
            cout << "lyr " << i << ", in=" << innerRadius << ", out="
                 << outerRadius << ", swr=" << swr << ", fwr" << fwr << endl;

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
            TCWire * tw = new TCWire(nWires++, j, * layer, fp, bp);
            _wires.push_back(tw);
            layer->push_back(tw);
        }
    }

    //...LUT for LR decision : common to all TS...
    _luts.push_back(new TCLUT("LR LUT", * this));
    _luts.back()->initialize(_outerTSLUTDataFilename);
    
//  for (unsigned i = 0; _luts.size(); i++)
//	_luts[i]->doit();

    //...Make TSF's...
    const unsigned nWiresInTS[2] = {15, 11};
    const int shape[2][30] =
	{{-2,  0,  // relative layer id, relative wire id
	  -1, -1,  // assuming layer offset 0.0, not 0.5
	  -1,  0,
	  0,  -1,
	  0,   0,
	  0,   1,
	  1,  -2,
	  1,  -1,
	  1,   0,
	  1,   1,
	  2,  -2,
	  2,  -1,
	  2,   0,
	  2,   1,
	  2,   2},
	 {-2, -1,
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
	  0,   0}};
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
		* (TCWire *) (* (* _superLayers[i])[layerOffset[tsType]])[j];

            const unsigned localId = w.localId();
            const unsigned layerId = w.layerId();
            vector<const TCWire *> cells;

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
            TRGCDCSegment * ts = new TRGCDCSegment(idTS++,
						   * layer,
						   w,
						   _luts.back(),
						   cells);

            //...Store it...
            _tss.push_back(ts);
	    _tsSL[i].push_back(ts);
            layer->push_back(ts);
        }
    }

    //...Fill caches...
    if (_width) delete [] _width;
    if (_r) delete [] _r;
    if (_r2) delete [] _r2;
    _width = new float[nSuperLayers()];
    _r = new float[nSuperLayers() + 1];
    _r2 = new float[nSuperLayers() + 1];
    for (unsigned i = 0; i < nSuperLayers(); i++) {
        const vector<TRGCDCLayer *> & slayer = * _superLayers[i];
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
            cout << layerId << "," << cdc2.senseWireR(layerId) << ","
                 << cdc2.fieldWireR(layerId) << endl;
            cout << "    super layer " << i << " radius=" << _r[i] << "(r^2="
                 << _r2[i] << ")" << endl;
        }
    }

    //...Hough Finder...
    _hFinder = new TCHFinder("HoughFinder",
                             * this,
                             houghFinderMeshX,
                             houghFinderMeshY);
    _hFinder->perfect(houghFinderPerfect);

    //...3D fitter...
    _fitter3D = new TCFitter3D("Fitter3D", * this);
    _fitter3D->initialize();

    //...For module simulation (Front-end)...
    configure();

}

void
TRGCDC::terminate(void){
    _fitter3D->terminate();
}

void
TRGCDC::dump(const string & msg) const {
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
            if (w.timing().active())
                w.dump(dumpOption, TRGDebug::tab(4));
        }
    }
    if (msg.find("trgWireCentralHits") != string::npos) {
        const string dumpOption = "trigger detail";
        cout << "    wire hits" << endl;
        for (unsigned i = 0; i < nSegments(); i++) {
            const TCSegment & s = segment(i);
            if (s.wires()[5]->timing().active())
                s.wires()[5]->dump(dumpOption, TRGDebug::tab(4));
        }
    }
    if (msg.find("trgTSHits") != string::npos) {
        const string dumpOption = "trigger detail";
        cout << "    TS hits" << endl;
        for (unsigned i = 0; i < nSegments(); i++) {
            const TCSegment & s = segment(i);
            if (s.timing().active())
                s.dump(dumpOption, TRGDebug::tab(4));
        }
    }
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
        return (TCWire *) & _layers[layerId]->cell(localId);
    return 0;
}

// const TCWire *
// TRGCDC::wire(const HepGeom::Point3D<double> & p) const {
//     float r = p.mag();
//     float phi = p.phi();
//     return wire(r, phi);
// }

const TCWire *
TRGCDC::wire(float , float ) const {

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

    unsigned i = 0;
    while (TCWire * w = _wires[i++])
        w->clear();
    i = 0;
    while (TCSegment * s = _tss[i++])
        s->clear();
    _hitWires.clear();
    _hits.clear();
    _axialHits.clear();
    _stereoHits.clear();
    _badHits.clear();
    _hitsMC.clear();
    _segmentHits.clear();
    for (unsigned i = 0; i < 9; i++)
	_segmentHitsSL[i].clear();
}

void
TRGCDC::fastClear(void) {
}

void
TRGCDC::update(bool ) {

    TRGDebug::enterStage("TRGCDC update");

    //...Clear old information...
//  fastClear();
    clear();

    //...CDCSimHit...
    StoreArray<CDCSimHit> SimHits("CDCSimHits");
    if (! SimHits) {
	cout << "TRGCDC !!! can not access to CDCSimHits" << endl;
	TRGDebug::leaveStage("TRGCDC update");
	return;
    }
    const unsigned n = SimHits->GetEntries();

    //...CDCHit...
    StoreArray<CDCHit> CDCHits("CDCHits");
    if (! CDCHits) {
        cout << "TRGCDC !!! can not access to CDCHits" << endl;
        TRGDebug::leaveStage("TRGCDC update");
        return;
    }
    const unsigned nHits = CDCHits->GetEntries();

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
        if(h.getISuperLayer()==0) t_layerId = h.getILayer();
        else t_layerId = h.getILayer()+6*h.getISuperLayer()+2;
        const unsigned layerId = t_layerId;
        const unsigned wireId = h.getIWire();
	TCWire & w = * (TCWire *) wire(layerId, wireId);

        //...Drift legnth(micron) to drift time(ns)...
        //   coefficient used here must be re-calculated.
        const float driftTimeMC =
	    SimHits[iSimHit]->getDriftLength() * 10 * 1000 / 40;

	//cout << " -2 driftTimeMC=" << driftTimeMC << endl;

	//...Trigger timing...
        TRGTime rise = TRGTime(driftTimeMC, true, _clockFE, w.name());
        TRGTime fall = rise;
        fall.shift(1).reverse();
        w._timing = TRGSignal(rise & fall);
	w._timing.name(w.name());

	//...Simulated drift distance...
	const double driftLength =
	    _clockFE.absoluteTime(w._timing[0]->time()) * 40 / 10 / 1000;

	//w._timing.dump("detail", " -1 ");

        //...TCWireHit...
        TCWHit * hit = new TCWHit(w,
				  i,
				  iSimHit,
				  iMCPart,
				  driftLength,
				  0.15,
				  driftLength,
				  0.15,
				  1);
	hit->state(CellHitFindingValid | CellHitFittingValid );

        //...Store a hit...
        ((TCWire *) (* _layers[layerId])[wireId])->hit(hit);
        _hits.push_back(hit);
        if (w.axial()) _axialHits.push_back(hit);
        else           _stereoHits.push_back(hit);
    }

    //...Track segment... This part is moved to ::simulate().

    //...Hit classification...
//  _hits.sort(TCWHit::sortByWireId);
    classification();

    if (TRGDebug::level()) {
	StoreArray<CDCSimHit> simHits("CDCSimHits");
        cout << TRGDebug::tab() << "#CDCSimHit=" << n << ",#CDCHit=" << nHits
	     << endl;
    }

    TRGDebug::leaveStage("TRGCDC update");
}

void
TRGCDC::classification(void) {
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
        }
        else {
            const TCWHit * hr2 = neighbor[2]->neighbor(2)->hit();
            const TCWHit * hl2 = neighbor[3]->neighbor(3)->hit();
            if ((hr2 == 0) && (hr1 != 0) && (hl1 == 0) ||
                (hl2 == 0) && (hl1 != 0) && (hr1 == 0))
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

vector<const TCWHitMC *>
TRGCDC::hitsMC(void) const {
    vector<const TCWHitMC *> t;
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
    }
    else {
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
TRGCDC::layerId(unsigned , unsigned ) const {
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
        const vector<TRGCDCLayer *> & sl = * superLayer(iLayer);
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
        const vector<TRGCDCLayer *> & sl = * superLayer(iLayer);
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
TRGCDC::axialStereoSuperLayerId(unsigned ,
				unsigned ) const {
    cout << "TRGCDC::axialStereoSuperLayerId !!! "
              << "this function is not implemented yet"
              << endl;
    return TRGCDC_UNDEFINED;
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
    clear();

    delete [] _width;
    delete [] _r;
    delete [] _r2;

    if (_hFinder)
        delete _hFinder;
    if (_fitter3D)
        delete _fitter3D;

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

#ifdef TRGCDC_DISPLAY
    D->beginningOfEvent();
#endif

    TRGDebug::enterStage("TRGCDC simulation");

    //...Store TS hits...
    const unsigned n = _tss.size();
    for (unsigned i = 0; i < n; i++) {
        TCSegment & s = * _tss[i];
        s.simulate();
        if (s.timing().active()) {

	     //...Create TCShit...
	     unsigned j = 0;
	     const TCWire * w = s[j];
	     TCSHit * th = 0;
	     while (w) {
		 const TCWHit * h = w->hit();
		 if (h) {
		     if (! th) {
			 th = new TCSHit(s);
			 s.hit(th);
			 _segmentHits.push_back(th);
			 _segmentHitsSL[s.layerId()].push_back(th);
		     }
		     s._hits.push_back(h);
		 }
		 w = s[++j];
	     }
	}
    }

    if (TRGDebug::level() > 1) {
        cout << TRGDebug::tab() << "TS hit list" << endl;
	string dumpOption = "trigger";
        if (TRGDebug::level() > 2)
            dumpOption = "detail";
        for (unsigned i = 0; i < nSegments(); i++) {
            const TCSegment & s = segment(i);
            if (s.timing().active())
                s.dump(dumpOption, TRGDebug::tab(4));
        }

        cout << TRGDebug::tab() << "TS hit list (2)" << endl;
        if (TRGDebug::level() > 2)
            dumpOption = "detail";
        for (unsigned i = 0; i < _segmentHits.size(); i++) {
            const TCSHit & s = * _segmentHits[i];
	    s.dump(dumpOption, TRGDebug::tab(4));
        }

        cout << TRGDebug::tab() << "TS hit list (3)" << endl;
        if (TRGDebug::level() > 2)
            dumpOption = "detail";
	for (unsigned j = 0; j < _superLayers.size(); j++) {
	    for (unsigned i = 0; i < _segmentHitsSL[j].size(); i++) {
		const vector<TCSHit *> & s = _segmentHitsSL[j];
		for (unsigned k = 0; k < s.size(); k++)
		    s[k]->dump(dumpOption, TRGDebug::tab(4));
	    }
        }
    }

    if (_simulationMode == 1) {
	TRGDebug::leaveStage("TRGCDC simulation");
	return;
    }

    //...2D tracker : Hough finder...
    vector<TCTrack *> trackList;
    _hFinder->doit(trackList);

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


    //...Check relations...
    if (TRGDebug::level()) {
	for (unsigned i = 0; i < trackList.size(); i++) {
	    trackList[i]->relation().dump();
	}
    }

    //...Stereo finder...

    //...3D tracker...
    vector<TCTrack *> trackList3D;
    _fitter3D->doit(trackList, trackList3D);

    //...End of simulation...

#ifdef TRGCDC_DISPLAY
//  cdc.dump("hits");
    vector<const TCTrack *> tt;
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
}

void
TRGCDC::configure(void) {

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
                skip = true;
                break;
            }
            else if (car == "CDC") {
                if (l.find("CDC Wire Config Version") != string::npos)
                    cdcVersion = l;
                else if (l.find("CDC TRG Config Version") != string::npos)
                    configVersion = l;
                skip = true;
                break;
            }

            if (i == 0) {
                wid = atoi(car.c_str());
            }
            else if (i == 1) {
                lid = atoi(car.c_str());
            }
            else if (i == 2) {
                fid = atoi(car.c_str());
            }
            else if (i == 3) {
                mid = atoi(car.c_str());
            }
            else if (i == 4) {
                tid = atoi(car.c_str());
            }
        }

        if (skip)
            continue;
        if (lines != wid)
            continue;

        //...Make a front-end board if necessary...
        TCFrontEnd * f = 0;
        if (fid < _fronts.size())
            f = _fronts[fid];
        if (! f) {
            const string name = "CDCFrontEnd_" + TRGUtil::itostring(fid);
            f = new TCFrontEnd(name, _clock);
            _fronts.push_back(f);
        }
        f->push_back(_wires[wid]);

        //...Make a merger board if necessary...
        TCMerger * m = 0;
        if (mid != 99999) {
            if (mid < _mergers.size())
                m = _mergers[mid];
            if (! m) {
                const string name = "CDCMerger_" + TRGUtil::itostring(mid);
                m = new TCMerger(name, _clock);
                _mergers.push_back(m);
            }
            m->push_back(f);
//            cout << "  f added" << endl;
        }

        ++lines;
    }
    infile.close();

    //...Make a link in each front-end...
    const unsigned nFronts = _fronts.size();
    for (unsigned i = 0; i < nFronts; i++) {
        TCFrontEnd & f = * _fronts[i];
        TRGLink * fl = new TRGLink(f.name(), f.clock());
        f.append(fl);
	const unsigned nWires = f.size();
	for (unsigned j = 0; j < nWires; j++) {
            fl->append(& f[j]->timing());
        }
	if (TRGDebug::level() > 1)
	    f.dump("detail");
    }

    //...Make a link for mergers
//     TRGLink * fl = new TRGLink("CDCFrontEnd_0", _clock);
//     _links.push_back(fl);
//     for (unsigned j = 0; j < 3; j++)
//         for (unsigned i = 0; i < 16; i++)
//             fl->append(& (* _layers[j])[i]->timing());
//     TCFrontEnd f("CDCFrontEnd_0", _clock);
//     f.append(fl);
   
    infile.close();
}

const TRGCDCSegment &
TRGCDC::segment(unsigned lid, unsigned id) const {
    return * (const TRGCDCSegment *) (* _tsLayers[lid])[id];
}

} // namespace Belle2
