//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Track.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a reconstructed charged track in TRGCDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCTrack_FLAG_
#define TRGCDCTrack_FLAG_

#include "trg/cdc/TrackBase.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/Helix.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCTrack TRGCDCTrack
#endif

//...Definition of status...
#define TCTrack_2DTrack       1
#define TCTrack_2DTrackFitted 2
#define TCTrack_3DTrack       4
#define TCTrack_3DTrackFitted 8

namespace Belle2 {

class TRGCDCCircle;

/// A class to represent a reconstructed charged track in TRGCDC.
class TRGCDCTrack : public TRGCDCTrackBase {

  public:

    /// returns a list of TRGCDCTrack's.
    static std::vector<const TRGCDCTrack *> list(void);

    /// Constructor
    TRGCDCTrack();

    /// Constructor from a Circle.
    TRGCDCTrack(const TRGCDCCircle &);

    /// Destructor
    virtual ~TRGCDCTrack();

    /// returns helix parameter.
    const TRGCDCHelix & helix(void) const;

    /// calculates the closest approach to a wire in real space. Results are stored in TLink. Return value is negative if error happened.
    int approach(TRGCDCLink &, bool sagCorrection = false) const;
    
  private:// static members

    /// a vector to keep all TRGCDCTrack objects.
    static std::vector<const TRGCDCTrack *> _list;

  private:

    /// Helix parameter.
    TRGCDCHelix _helix;
};

//-----------------------------------------------------------------------------

// inline
// unsigned
// TRGCDCTrack::id(void) const{
//     if (_list.size()) {
//         for (unsigned i = 0; i < _list.size(); i++)
//             if (_list[i] == this)
//                 return unsigned(i);
//     }
//     return 999;
// }

inline
const TRGCDCHelix &
TRGCDCTrack::helix(void) const {
    return _helix;
}

} // namespace Belle2

#endif /* TRGCDCTrack_FLAG_ */
