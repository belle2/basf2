//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TrackBase.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track object in TRGCDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCTrackBase_FLAG_
#define TRGCDCTrackBase_FLAG_

#include <vector>
#include "CLHEP/Vector/LorentzVector.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"

#define TRGCDCTrackBase_Max_Layers 9

#ifdef TRGCDC_SHORT_NAMES
#define TCTBase TRGCDCTrackBase
#endif

namespace Belle2 {

class TRGCDCTrackSegment;

/// A class to represent a track object in TRGCDC.
class TRGCDCTrackBase {

  public:

    /// Constructor
    TRGCDCTrackBase(const std::string & name, float charge);

    /// Destructor
    virtual ~TRGCDCTrackBase();

  public:

    /// returns name.
    std::string name(void) const;

    /// returns status.
    int status(void) const;

    /// returns charge.
    float charge(void) const;

    /// returns momentum vector.
    const CLHEP::HepLorentzVector & p(void) const;

    /// returns Pt.
    virtual float pt(void) const;

    /// returns position vector.
    const CLHEP::Hep3Vector & v(void) const;

    /// dumps debug information.
    virtual void dump(const std::string & message = std::string(""),
                      const std::string & prefix = std::string("")) const;

    /// returns a vector to track segments.
    const std::vector<const TRGCDCTrackSegment *> &
        trackSegments(unsigned layerId) const;

  public: // Modifiers

    /// appends a track segment.
    void append(const TRGCDCTrackSegment *);

  private:

    /// Name.
    std::string _name;

    /// Status.
    int _status;

    /// Charge.
    float _charge;

    /// Momentum.
    CLHEP::HepLorentzVector _p;

    /// Position.
    CLHEP::Hep3Vector _v;

    /// Track Segments.
    std::vector<const TRGCDCTrackSegment *> _ts[TRGCDCTrackBase_Max_Layers];
};

//-----------------------------------------------------------------------------

inline
std::string
TRGCDCTrackBase::name(void) const {
    return _name;
}

inline
int
TRGCDCTrackBase::status(void) const {
    return _status;
}

inline
const CLHEP::HepLorentzVector &
TRGCDCTrackBase::p(void) const {
    return _p;
}

inline
const CLHEP::Hep3Vector &
TRGCDCTrackBase::v(void) const {
    return _v;
}

inline
float
TRGCDCTrackBase::pt(void) const {
    return _p.perp();
}

} // namespace Belle2

#endif /* TRGCDCTrackBase_FLAG_ */
