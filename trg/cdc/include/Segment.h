//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Segment.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCSegment_FLAG_
#define TRGCDCSegment_FLAG_

#include <vector>
#include "trg/cdc/LUT.h"
#include "trg/cdc/Cell.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCSegment TRGCDCSegment
#endif

namespace Belle2 {

class TRGSignal;
class TRGCDCWire;
class TRGCDCLayer;
class TRGCDCLUT;
class TRGCDCWireHit;
class TRGCDCSegmentHit;
class TRGCDCEventTime;

/// A class to represent a wire in CDC.
class TRGCDCSegment : public TRGCDCCell {

  public:

    /// Constructor.
    TRGCDCSegment(unsigned id,
		  const TRGCDCLayer & layer,
		  const TRGCDCWire & w,
		  const TRGCDCLUT * lut,   // Will be removed.
		  const TRGClock & clock,
		  const TRGCDCEventTime * eventTime,
		  const std::string & TSLUTFile,
		  const std::vector<const TRGCDCWire *> & wires);

    /// Destructor
    virtual ~TRGCDCSegment();

  public:// Selectors

    /// returns a vector containing pointers to a wire.
    const std::vector<const TRGCDCWire *> & wires(void) const;

    /// returns name.
    std::string name(void) const;

    /// returns a wire.
    const TRGCDCWire * operator[](unsigned id) const;

    /// returns a center wire.
    const TRGCDCWire & center(void) const;

    /// returns trigger output. Null will returned if no signal.
    const TRGSignal & signal(void) const;

    /// returns a pointer to a TRGCDCSegmentHit.
    const TRGCDCSegmentHit * hit(void) const;

    /// returns hit pattern.
    unsigned hitPattern(void) const;

    /// returns LUT.
    const TRGCDCLUT * LUT(void) const;

    /// returns (new) LUT
    TRGCDCLUT * nLUT(void) ;

    /// dumps debug information.
    void dump(const std::string & message = std::string(""),
              const std::string & prefix = std::string("")) const;


    /// initilize variables.
    void initialize(void);
    void initialize(bool fevtTime);
    
    /// returns phi position.
    double phiPosition(void) const;

    /// returns event time.
    const TRGCDCEventTime * EvtTime(void) const;

  public:// Utility functions

    /// returns axial segments.
/*     static const std::vector<const TRGCDCSegment *> */
/* 	axial(const std::vector<const TRGCDCSegment *> & list); */

    /// returns \# of stereo segments.
    static unsigned
	nStereo(const std::vector<const TRGCDCSegment *> & list);

    /// returns true this has member named a.
    virtual bool hasMember(const std::string & a) const;

  public:// Modifiers

    /// clears information.
    void clear(void);

    /// simulates TF hit using wire information.
    void simulate(bool clockSimulation, bool logicLUTFlag);

    /// sets a pointer to a TRGCDCSegmentHit.
    const TRGCDCSegmentHit * hit(const TRGCDCSegmentHit *);

  public:

    /// simulates time-indegrated TF hit 
    void simulateWithoutClock(bool logicLUTFlag);

    /// simulates TF hit time-dependently
    void simulateWithClock(bool logicLUTFlag);

  private:

    TRGCDCLUT * m_TSLUT;

    /// LookUp Table.
    const TRGCDCLUT * const _lut;  //Will be Removed.

    /// Wires.
    std::vector<const TRGCDCWire *> _wires;

    /// Center wire.
    const TRGCDCWire * _center;

    /// Trigger signal.
    TRGSignal _signal;

    /// Wire hits.
    std::vector<const TRGCDCWireHit *> _hits;

    /// EventTime class.
    const TRGCDCEventTime * const _eventTime;

    /// TS LUT file name.
    std::string m_TSLUTFileName;
  // Friends
    friend class TRGCDC;
};

//-----------------------------------------------------------------------------

inline
const std::vector<const TRGCDCWire *> &
TRGCDCSegment::wires(void) const {
    return _wires;
}

inline
const TRGCDCWire *
TRGCDCSegment::operator[](unsigned id) const {
    if (id < _wires.size())
	return _wires[id];
    return 0;
}

inline
const TRGSignal &
TRGCDCSegment::signal(void) const {
    return _signal;
}

inline
const TRGCDCSegmentHit *
TRGCDCSegment::hit(const TRGCDCSegmentHit * h) {
    return (const TRGCDCSegmentHit *)
	TRGCDCCell::hit((const TRGCDCCellHit *) h);
}

inline
const TRGCDCSegmentHit *
TRGCDCSegment::hit(void) const {
    return  (const TRGCDCSegmentHit *) TRGCDCCell::hit();
}

// *** Will be removed. -KT
inline
const TRGCDCLUT *
TRGCDCSegment::LUT(void) const {
//    std::cout << "this function(LUT in Segment.h) will be replaced by new function -ktkim " << std::endl;
    return _lut;
}

inline
TRGCDCLUT *
TRGCDCSegment::nLUT(void)  {  //will be LUT()   -KT
    return m_TSLUT;
}

inline
const TRGCDCWire &
TRGCDCSegment::center(void) const {
    if (_wires.size() == 15)
	return * _wires[0];
    return * _wires[5];
}

inline
const TRGCDCEventTime *
TRGCDCSegment::EvtTime(void) const{
    return _eventTime;
}

} // namespace Belle2

#endif /* TRGCDCSegment_FLAG_ */
