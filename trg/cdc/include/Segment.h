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
  class CDCTriggerSegmentHit;

/// A class to represent a wire in CDC.
  class TRGCDCSegment : public TRGCDCCell {

  public:

    /// Constructor.
    TRGCDCSegment(unsigned id,
                  const TRGCDCLayer& layer,
                  const TRGCDCWire& w,
                  const TRGClock& clock,
                  const std::string& TSLUTFile,
                  const std::vector<const TRGCDCWire*>& wires);

    /// Destructor
    virtual ~TRGCDCSegment();

  public:// Selectors

    /// returns a vector containing pointers to a wire.
    const std::vector<const TRGCDCWire*>& wires(void) const;

    /// returns name.
    std::string name(void) const;

    /// returns a wire.
    const TRGCDCWire* operator[](unsigned id) const;

    /// returns a center wire.
    const TRGCDCWire& center(void) const;

    /// returns priority wire.
    const TRGCDCWire& priority(void) const;

    /// returns trigger output. Null will returned if no signal.
    const TRGSignal& signal(void) const;

    /// returns a pointer to a TRGCDCSegmentHit.
    const TRGCDCSegmentHit* hit(void) const;

    /// returns a pointer to a CDCTriggerSegmentHit.
    const std::vector<const CDCTriggerSegmentHit*> storeHits(void) const;

    /// returns hit pattern.
    unsigned hitPattern(void) const;
    /// returns hit pattern for hits in given time window.
    unsigned hitPattern(int clk0, int clk1) const;
    /// hit pattern containing bit for priority position
    unsigned lutPattern(void) const;
    /// hit pattern containing bit for priority position for hits in given time window.
    unsigned lutPattern(int clk0, int clk1) const;

    /// return fastest time in TSHit.
    float fastestTime(void) const;
    /// fastest time in TSHit that is larger of equal to clk0.
    float fastestTime(int clk0) const;

    /// return found time in TSHit.
    float foundTime(void)const;

    /// return priority time in TSHit.
    float priorityTime(void) const;

    /// return priority cell position in TSHit. 0: no hit, 3: 1st priority, 1: 2nd right, 2: 2nd left
    int priorityPosition(void)const;
    /// return priority cell position in TSHit for given time window.
    int priorityPosition(int clk0, int clk1)const;

    /// returns LUT
    const TRGCDCLUT* LUT(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;


    /// initilize variables.
    void initialize(void);
    void initialize(bool fevtTime);

  public:// Utility functions

    /// returns axial segments.
    /*     static const std::vector<const TRGCDCSegment *> */
    /*  axial(const std::vector<const TRGCDCSegment *> & list); */

    /// returns \# of stereo segments.
    static unsigned
    nStereo(const std::vector<const TRGCDCSegment*>& list);

    /// returns true this has member named a.
    virtual bool hasMember(const std::string& a) const;

  public:// Modifiers

    /// clears information.
    void clear(void);

    /// simulates TF hit using wire information.
    void simulate(bool clockSimulation, bool logicLUTFlag,
                  std::string cdcCollectionName = std::string(""),
                  std::string tsCollectionName = std::string(""));

    /// sets a pointer to a TRGCDCSegmentHit.
    const TRGCDCSegmentHit* hit(const TRGCDCSegmentHit*);

    /// sets a pointer to a CDCTriggerSegmentHit.
    void addStoreHit(const CDCTriggerSegmentHit*);

  public:

    /// simulates time-indegrated TF hit
    void simulateWithoutClock(bool logicLUTFlag);

    /// simulates TF hit time-dependently
    void simulateWithClock(std::string cdcCollectionName,
                           std::string tsCollectionName);

  private:

    /// LookUp Table. 0: no hit, 1: right, 2: left, 3: not determined.
    TRGCDCLUT* m_TSLUT;

    /// LookUp Table.
//    const TRGCDCLUT * const _lut;  //Will be Removed.

    /// Wires.
    std::vector<const TRGCDCWire*> _wires;

    /// Center wire.
    const TRGCDCWire* _center;

    /// Trigger signal.
    TRGSignal _signal;

    /// Wire hits.
    std::vector<const TRGCDCWireHit*> _hits;

    /// list of DataStore hits.
    std::vector<const CDCTriggerSegmentHit*> _storeHits;

    /// TS LUT file name.
    std::string m_TSLUTFileName;

    // Friends
    friend class TRGCDC;
  };

//-----------------------------------------------------------------------------

  inline
  const std::vector<const TRGCDCWire*>&
  TRGCDCSegment::wires(void) const
  {
    return _wires;
  }

  inline
  const TRGCDCWire*
  TRGCDCSegment::operator[](unsigned id) const
  {
    if (id < _wires.size())
      return _wires[id];
    return 0;
  }

  inline
  const TRGSignal&
  TRGCDCSegment::signal(void) const
  {
    return _signal;
  }

  inline
  const TRGCDCSegmentHit*
  TRGCDCSegment::hit(const TRGCDCSegmentHit* h)
  {
    return (const TRGCDCSegmentHit*)
           TRGCDCCell::hit((const TRGCDCCellHit*) h);
  }

  inline
  const TRGCDCSegmentHit*
  TRGCDCSegment::hit(void) const
  {
    return (const TRGCDCSegmentHit*) TRGCDCCell::hit();
  }

  inline
  void
  TRGCDCSegment::addStoreHit(const CDCTriggerSegmentHit* h)
  {
    _storeHits.push_back(h);
  }

  inline
  const std::vector<const CDCTriggerSegmentHit*>
  TRGCDCSegment::storeHits(void) const
  {
    return _storeHits;
  }

  inline
  const TRGCDCLUT*
  TRGCDCSegment::LUT(void)  const
  {
    return m_TSLUT;
  }

  inline
  const TRGCDCWire&
  TRGCDCSegment::center(void) const
  {
    if (_wires.size() == 15)
      return * _wires[0];
    return * _wires[5];
  }

} // namespace Belle2

#endif /* TRGCDCSegment_FLAG_ */
