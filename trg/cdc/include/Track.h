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

// #include <vector>
// #include "CLHEP/Vector/LorentzVector.h"
// #include "trg/trg/Utilities.h"
// #include "trg/cdc/TRGCDC.h"
#include "trg/cdc/TrackBase.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCTrack TRGCDCTrack
#endif

//...Definition of status...
#define TCTrack_2DTrack       1
#define TCTrack_2DTrackFitted 2
#define TCTrack_3DTrack       4
#define TCTrack_3DTrackFitted 8

namespace Belle2 {

/// A class to represent a reconstructed charged track in TRGCDC.
class TRGCDCTrack : public TRGCDCTrackBase {

  public:

    /// returns a list of TRGCDCTrack's.
    static std::vector<const TRGCDCTrack *> list(void);

    /// Constructor
    TRGCDCTrack();

    /// Destructor
    virtual ~TRGCDCTrack();

  private:// static members
    static std::vector<const TRGCDCTrack *> _list;
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

} // namespace Belle2

#endif /* TRGCDCTrack_FLAG_ */
