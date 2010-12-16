//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Wire.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDCWire_INLINE_DEFINE_HERE
#define TRGCDC_SHORT_NAMES

#include "trg/trg/Clock.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"

#define P3D HepGeom::Point3D<double>

using namespace std;

namespace Belle2 {

TRGCDCWire::TRGCDCWire(unsigned id,
		   unsigned localId,
		   TRGCDCLayer * l,
		   const P3D & fp,
		   const P3D & bp)
    : _id(id),
      _localId(localId),
      _layer(l),
      _forwardPosition(fp),
      _backwardPosition(bp),
      _direction(fp - bp),
      _triggerOutput() {
    _state = 0;
    _hit = 0;
    _xyPosition = 0.5 * (_forwardPosition + _backwardPosition);
    _xyPosition.setZ(0.);
    _direction = _direction.unit();
    _triggerOutput.name(name() + string("to"));
}

TRGCDCWire::~TRGCDCWire() {
}

void
TRGCDCWire::dump(const string & msg, const string & pre) const {
    cout << pre;
    cout << "w " << _id;
    cout << ",local " << _localId;
    cout << ",layer " << layerId();
    cout << ",super layer " << superLayerId();
    cout << ",local layer " << localLayerId();
    cout << endl;
    if (msg.find("neighbor") != string::npos ||
	msg.find("detail") != string::npos) {
	for (unsigned i = 0; i < 7; i++)
	    if (neighbor(i))
		neighbor(i)->dump("", pre + TRGCDC::itostring(i) + "   ");
    }    
}
  
const TRGCDCWire * const
TRGCDCWire::neighbor(unsigned i) const {
    static bool first = false;
    if (first)
	cout << "TRGCDCWire::neighbor !!! "
		  << "this function is not tested yet"
		  << endl;

    const TRGCDC & cdc = * TRGCDC::getTRGCDC();
    const unsigned layerId = _layer->id();
    const unsigned superLayerId = _layer->superLayerId();
    const unsigned localLayerId = _layer->localLayerId();
    const unsigned nLayers = cdc.superLayer(superLayerId)->size();
    const int local = int(_localId);

    if (i == WireInnerLeft || i == WireInnerRight) {
	if (localLayerId == 0)
	    return 0;
	if (_layer->offset() != 0) {
	    if (i == WireInnerLeft)
		return cdc.wire(layerId - 1, local);
	    else
		return cdc.wire(layerId - 1, local + 1);
	}
	else {
	    if (i == WireInnerLeft)
		return cdc.wire(layerId - 1, local - 1);
	    else
		return cdc.wire(layerId - 1, local);
	}
    }
    else if (i == WireLeft || i == WireRight) {
	if (i == WireLeft)
	    return cdc.wire(layerId, local - 1);
	else
	    return cdc.wire(layerId, local + 1);
    }
    else if (i == WireOuterLeft || i == WireOuterRight) {
	if (localLayerId == (nLayers - 1))
	    return 0;
	if (_layer->offset() != 0) {
	    if (i == WireInnerLeft)
		return cdc.wire(layerId + 1, local);
	    else
		return cdc.wire(layerId + 1, local + 1);
	}
	else {
	    if (i == WireInnerLeft)
		return cdc.wire(layerId + 1, local - 1);
	    else
		return cdc.wire(layerId + 1, local);
	}
    }
    return 0;
}

// int
// TRGCDCWire::localIdForPlus(void) const {

//     const unsigned li = layerId();

//   //
//   // used in curl finder
//   //
//   // new version by NK
//   //
//   const TRGCDCLayer &l = *layer();
//   const int nw = l.nWires();
//   if(_localId+1==nw) return -1;
//   else return _localId;
// #if 0
//     if (ms_smallcell) {
//       if ((li == 1) || (li == 2)) {
// 	if (_localId == 127) return -1;
// 	else return _localId;
//       }
//       if(_localId == 63) return -1;
//       else return _localId;
//     }
//     else {
//       if (li <= 5) {
// 	if(_localId == 63)
// 	  return -1;
// 	else return _localId;
//       }
//     }

//   if(li >= 6 && li <= 8){
//     if(_localId == 79)
//       return -1;
//     else return _localId;
//   }

//   if(li >= 9 && li <= 14){
//     if(_localId == 95)
//       return -1;
//     else return _localId;
//   }
  
//   if(li >= 15 && li <= 17){
//     if(_localId == 127)
//       return -1;
//     else return _localId;
//   }

//   if(li >= 18 && li <= 22){
//     if(_localId == 143)
//       return -1;
//     else return _localId;
//   }

//   if(li >= 23 && li <= 26){
//     if(_localId == 159)
//       return -1;
//     else return _localId;
//   }
  
//   if(li >= 27 && li <= 31){
//     if(_localId == 191)
//       return -1;
//     else return _localId;
//   }
  
//   if(li >= 32 && li <= 35){
//     if(_localId == 207)
//       return -1;
//     else return _localId;
//   }

//   if(li >= 36 && li <= 40){
//     if(_localId == 239)
//       return -1;
//     else return _localId;
//   }

//   if(li >= 41 && li <= 44){
//     if(_localId == 255)
//       return -1;
//     else return _localId;
//   }

//   if(li >= 45){
//     if(_localId == 287)
//       return -1;
//     else return _localId;
//   }
// #endif
  
//   return -1;
// }

// int
// TRGCDCWire::localIdForMinus(void) const {

//     const unsigned li = layerId();

//   //
//   // used in curl finder
//   //
//   // new version ny NK
//   //
//   const TRGCDCLayer &l = *layer();
//   const int nw = l.nWires();
//   if(0==_localId) return nw;
//   else return _localId;
// #if 0
//     if (ms_smallcell) {
//       if ((li == 1) || (li == 2)) {
// 	if (_localId == 0) return 128;
// 	else return _localId;
//       }
//       if(_localId == 63) return -1;
//       else return _localId;
//     }
//     else {
//       if (li <= 5){
// 	if(_localId == 0)
// 	  return 64;
// 	else return _localId;
//       }
//     }

//   if(li >= 6 && li <= 8){
//     if(_localId == 0)
//       return 80;
//     else return _localId;
//   }

//   if(li >= 9 && li <= 14){
//     if(_localId == 0)
//       return 96;
//     else return _localId;
//   }
  
//   if(li >= 15 && li <= 17){
//     if(_localId == 0)
//       return 128;
//     else return _localId;
//   }

//   if(li >= 18 && li <= 22){
//     if(_localId == 0)
//       return 144;
//     else return _localId;
//   }

//   if(li >= 23 && li <= 26){
//     if(_localId == 0)
//       return 160;
//     else return _localId;
//   }
  
//   if(li >= 27 && li <= 31){
//     if(_localId == 0)
//       return 192;
//     else return _localId;
//   }
  
//   if(li >= 32 && li <= 35){
//     if(_localId == 0)
//       return 208;
//     else return _localId;
//   }

//   if(li >= 36 && li <= 40){
//     if(_localId == 0)
//       return 240;
//     else return _localId;
//   }

//   if(li >= 41 && li <= 44){
//     if(_localId == 0)
//       return 256;
//     else return _localId;
//   }

//   if(li >= 45){
//     if(_localId == 0)
//       return 288;
//     else return _localId;
//   }
// #endif
  
//   return -1;
// }

// void
// TRGCDCWire::wirePosition(float z,
// 		       HepGeom::Point3D<double> & xy,
// 		       HepGeom::Point3D<double> & back,
// 		       Vector3D & dir) const {
//     cout << "TRGCDCWire::wirePosition !!! this function is not test yet"
// 	      << endl;

//     back = _backwardPosition;

//     //...Check z position...
//     if (! (z > _backwardPosition.z() && z < _forwardPosition.z())) {
// 	xy = _xyPosition;
// 	dir = _direction;
//     }

//     //...Setup...
//     int wireID = id();
//     float wirePosition[3] = {0., 0., 0.};
//     float dydz = 0;
//     float ybSag = 0;
//     float yfSag = 0;

//     //...Get corrections...
//     if(ms_superb) {
//       double dz = _forwardPosition.z() - _backwardPosition.z();
//       double dzb = (z - _backwardPosition.z()) / dz;
//       double dzf = (_forwardPosition.z() - z) / dz;

//       xy = dzb * _forwardPosition + dzf * _backwardPosition;
//       cout << "f=" << _forwardPosition << endl;
//       cout << "b=" << _backwardPosition << endl;
//       cout << "p" << xy << endl;
//       cout << "z=" << z << " dz=" << dz << " dzb=" << dzb << " dzf=" << dzf << endl;
//       dir = _direction;
//       return;
//     }
//     calcdc_sag3_(& wireID, & z, wirePosition, & dydz, & ybSag, & yfSag);
    
//     //...Wire position...
//     xy.setX((double) wirePosition[0]);
//     xy.setY((double) wirePosition[1]);
//     xy.setZ((double) wirePosition[2]);
//     back.setY((double) ybSag);
//     Vector3D v_aux(_forwardPosition.x() - _backwardPosition.x(),
// 		   yfSag - ybSag,
// 		   _forwardPosition.z() - _backwardPosition.z());
//     dir = v_aux.unit();

//     return;
// }

int
TRGCDCWire::localIdDifference(const TRGCDCWire & a) const {

#ifdef CDCTRG_DEBUG_DETAIL
    if (superLayerId() != a.superLayerId()) {
	cout << "TRGCDCWire::localIdDifference !!!";
	cout << "super layer assumption violation" << endl;
    }
#endif

    int diff = int(a.localId()) - int(localId());
    unsigned nWires = layer().nWires();
    if (diff > 0) {
	int difR = nWires - diff;
	if (diff < difR) return diff;
	else return - difR;
    }
    else {
	int difR = nWires + diff;
	if (- diff < difR) return diff;
	else return difR;
    }
}

void
TRGCDCWire::clear(void) {
    _state = 0;
    _hit = 0;

    for (unsigned i = 0; i < _mcHits.size(); i++)
	delete _mcHits[i];
    _mcHits.clear();

    _triggerOutput.clear();
}

string
TRGCDCWire::name(void) const {
    if (axial())
	return string("w") +
	    TRGCDC::itostring(layerId()) +
	    string("-") +
	    TRGCDC::itostring(_localId);
    return string("w") + 
	TRGCDC::itostring(layerId()) +
	string("=") +
	TRGCDC::itostring(_localId);
}

const TRGSignal &
TRGCDCWire::triggerOutput(void) const {
    if (! _hit) {
	return _triggerOutput;
    }
    else {

	//...Clock...
	const TRGClock & clock = TRGCDC::getTRGCDC()->systemClock();

	//...Drift legnth(micron) to drift time(ns)...
	//   coefficient used here must be re-calculated.
	float driftTime = _hit->drift() * 10 * 1000 / 40;
	
//	cout << name() << " drift=" << _hit->drift() << endl;

 	TRGTime rise = TRGTime(driftTime, true, clock, name() + string("trg"));
	TRGTime fall = rise;
	fall.shift(1).reverse();
	_triggerOutput = TRGSignal(rise & fall);
//	_triggerOutput->name(name() + string("to"));
//	_triggerOutput->dump();
	
	return _triggerOutput;

    }

//     if (! _hit)
// 	return 0;

//     if (_triggerOutput) {
// 	return _triggerOutput;
//     }
//     else {

// 	//...Clock...
// 	const TRGClock & clock = TRGCDC::getTRGCDC()->systemClock();

// 	//...Drift legnth(micron) to drift time(ns)...
// 	//   coefficient used here must be re-calculated.
// 	float driftTime = _hit->drift() * 10 * 1000 / 40;
	
// //	cout << name() << " drift=" << _hit->drift() << endl;

// 	TRGTime rise = TRGTime(driftTime, true, clock, name() + string("trg"));
// 	TRGTime fall = rise;
// 	fall.shift(1).reverse();
// 	_triggerOutput = new TRGSignal(rise & fall);
// 	_triggerOutput->name(name() + string("to"));
// //	_triggerOutput->dump();
	
// 	return _triggerOutput;
//     }
}


} // namespace Belle2

