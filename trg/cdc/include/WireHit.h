//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : WireHit.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCWireHit_FLAG_
#define TRGCDCWireHit_FLAG_

#include <string>
#include "CLHEP/Geometry/Point3D.h"
//#include "cdc/dataobjects/CDCHit.h"
//#include "cdc/hitcdc/CDCSimHit.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCWHit TRGCDCWireHit
#endif

#define WireHitLeft                0
#define WireHitRight               1

//...Masks for state()...

#define WireHitTimeValid      1048576
#define WireHitChargeValid    2097152
#define WireHitFindingValid   4194304
#define WireHitFittingValid   0x40000000
#define WireHitAxial                8
#define WireHitStereo              48

#define WireHitPatternLeft             256
#define WireHitPatternRight            512
#define WireHitIsolated               1024
#define WireHitContinuous             2048
#define WireHitNeighborHit              12
#define WireHitNeighborPatternMask 0x7f000
#define WireHitNeighborHit0           4096
#define WireHitNeighborHit1           8192
#define WireHitNeighborHit2          16384
#define WireHitNeighborHit3          32768
#define WireHitNeighborHit4          65536
#define WireHitNeighborHit5         131072
#define WireHitNeighborHit6         262144

#define WireHitLocked               128
#define WireHitUsed                  64
#define WireHitLeftMask               1
#define WireHitRightMask              2
#define WireHitMultiTrack       8388608
#define WireHitConformalFinder 16777216
#define WireHitCurlFinder      33554432
#define WireHitClustFinder     67108864
#define WireHitHoughFinder     67108864
#define WireHitTrackManager   134217728
#define WireHitInvalidForFit  268435456
#define WireHitFastFinder     536870912

namespace Belle2 {

class CDCHit;
class CDCSimHit;
class TRGCDCWire;

/// A class to represent a wire hit in CDC.
class TRGCDCWireHit {

  public:
    /// Constructor.
    TRGCDCWireHit(const TRGCDCWire &,
		  unsigned indexCDCHit = 0,
		  unsigned indexCDCSimHit = 0,
		  float driftLeft = 0,
		  float driftLeftError = 0,
		  float driftRight = 0,
		  float driftRightError = 0,
		  float fudgeFacgtor = 1);

    /// Destructor
    virtual ~TRGCDCWireHit();

  public:// Selectors

    /// dumps debug information.
    virtual void dump(const std::string & message = std::string(""),
		      const std::string & prefix = std::string("")) const;

    /// returns a pointer to a TRGCDCWire.
//  const TRGCDCWire * const wire(void) const;
    const TRGCDCWire & wire(void) const;

    /// returns state.
    unsigned state(void) const;

    /// returns drift distance.
    float drift(unsigned) const;

    /// returns drift distance error.
    float dDrift(unsigned) const;

    /// returns drift distance.
    float drift(void) const;

    /// returns drift distance error.
    float dDrift(void) const;

    /// returns position in the middle of wire. z is always zero, however.
    const HepGeom::Point3D<double>  & xyPosition(void) const;

    /// returns left position. z is always zero.
    HepGeom::Point3D<double>  position(unsigned) const;

    /// assigns a pointer to a TTrack.
    const void * const track(void) const;

    /// returns sequential Length in one segment : this parameter is used in TCurlFinder now.
    unsigned sequence(void) const;

    /// returns an index to CDCHit.
    unsigned iCDCHit(void) const;

    /// Access to CDCHit.
    const CDCHit * const hit(void) const;

    /// Access to CDCSimHit.
    const CDCSimHit * const simHit(void) const;

  public:// Modifiers
    /// sets state. Meaning of bits are written below.
    unsigned state(unsigned newState);

    /// sets state. Meaning of bits are written below. (tmp)
    unsigned state(unsigned newState) const;

    /// assigns a pointer to a TTrack.
    const void * const track(const void *);

    /// assigns a pointer to a TTrack. (tmp)
    const void * const track(const void *) const;

    /// sets sequential length in one segment : this parameter is used in TCurlFinder now.
    unsigned sequence(unsigned) const;

  public:// Static utility functions

    /// Sort function.
    static int sortByWireId(const TRGCDCWireHit ** a,
			    const TRGCDCWireHit ** b);

  private:
    mutable unsigned _state;
    float _drift[2];         // 0:left, 1:right
    float _driftError[2];
//  const TRGCDCWire * const _wire;
    const TRGCDCWire & _wire;
    const HepGeom::Point3D<double>  & _xyPosition;
    mutable const void * _track;
    mutable unsigned _sequentialLength;

    /// Index to CDCHit array
    unsigned _iCDCHit;

    /// Index to CDCSimHit array
    unsigned _iCDCSimHit;

   // _state bit definition

   // Pre-detemined
   //   20 : drift time valid
   //   21 : charge(dE/dx) valid
   //   22 : valid for finding
   //   30 : valid for fit
   //    3 : axial hit
   // 4, 5 : stereo hit

   // Hit pattern
   //    8 : hit pattern left
   //    9 : hit pattern right
   //   10 : isolated hit pattern
   //   11 : continuous hit pattern
   //   12 : neighbor hit 0
   //   13 : neighbor hit 1
   //   14 : neighbor hit 2
   //   15 : neighbor hit 3
   //   16 : neighbor hit 4
   //   17 : neighbor hit 5
   //   18 : neighbor hit 6

   // Tracking results
   //    7 : locked
   //    6 : used for tracking
   //    0 : left hit
   //    1 : right hit
   //   23 : shared by multi-track
   //   24 : found by conforaml finder
   //   25 : found by curl finder
   //   26 : found by clust finder
   //   27 : found by track manager
   //   28 : NOT valid for fitting

   // Fitting results
};

//-----------------------------------------------------------------------------

inline
// const TRGCDCWire * const
const TRGCDCWire &
TRGCDCWireHit::wire(void) const {
    return _wire;
}

inline
unsigned
TRGCDCWireHit::state(void) const {
    return _state;
}

inline
float
TRGCDCWireHit::drift(unsigned i) const {
    if (i) return _drift[1];
    return _drift[0];
}

inline
float
TRGCDCWireHit::dDrift(unsigned i) const {
    if (i) return _driftError[1];
    return _driftError[0];
}

inline
float
TRGCDCWireHit::drift(void) const {
    return (_drift[0] + _drift[1]) / 2.;
}

inline
float
TRGCDCWireHit::dDrift(void) const {
    return (_driftError[0] + _driftError[1]) / 2.;
}

inline
const HepGeom::Point3D<double>  &
TRGCDCWireHit::xyPosition(void) const {
    return _xyPosition;
}

inline
unsigned
TRGCDCWireHit::state(unsigned i) {
    return _state = i;
}

inline
unsigned
TRGCDCWireHit::state(unsigned i) const {
    return _state = i;
}

inline
const void * const
TRGCDCWireHit::track(void) const {
    return _track;
}

inline
const void * const
TRGCDCWireHit::track(const void * a) {
    return _track = a;
}

inline
const void * const
TRGCDCWireHit::track(const void * a) const {
    return _track = a;
}

inline
unsigned
TRGCDCWireHit::sequence(void) const {
    return _sequentialLength;
}

inline
unsigned
TRGCDCWireHit::sequence(unsigned a) const {
    return _sequentialLength = a;
}

inline
unsigned
TRGCDCWireHit::iCDCHit(void) const {
    return _iCDCHit;
}

} // namespace Belle2

#endif /* TRGCDCWireHit_FLAG_ */
