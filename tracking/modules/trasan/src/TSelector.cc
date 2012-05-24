//-----------------------------------------------------------------------------
// $Id: TSelector.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TSelector.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to select a TTrackBase object.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.11  2004/03/26 06:07:04  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.10  2003/12/25 12:03:35  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.9  2001/04/11 01:09:11  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.8  2000/04/29 03:45:55  yiwasaki
// old conf. fixed, new conf. unchanged
//
// Revision 1.7  2000/04/13 02:53:39  yiwasaki
// Trasan 2.00rc29 : minor changes
//
// Revision 1.6  1999/10/30 10:12:23  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.5  1999/06/14 12:40:22  yiwasaki
// Trasan 1.42 release : bug in findCloseHits fixed, sakura 1.06
//
// Revision 1.4  1999/03/15 07:57:13  yiwasaki
// Trasan 1.27 release : curl finder update
//
// Revision 1.3  1999/01/11 03:03:14  yiwasaki
// Fitters added
//
// Revision 1.2  1998/06/15 09:58:16  yiwasaki
// Trasan 1 beta 3.2, bug fixed
//
// Revision 1.1  1998/06/14 11:09:51  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
//-----------------------------------------------------------------------------




#include "tracking/modules/trasan/TSelector.h"
#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TCircle.h"
#include "tracking/modules/trasan/TLink.h"

namespace Belle {

  TSelector::TSelector()
    : _nLinksDefined(false),
      _nLinksStereoDefined(false),
      _nSuperLayersDefined(false),
      _minPtDefined(false),

      _nLinks(0),
      _nLinksStereo(0),
      _nSuperLayers(0),
      _minPt(0.)
  {
  }

  TSelector::TSelector(const TSelector& a)
    : _nLinksDefined(a._nLinksDefined),
      _nLinksStereoDefined(a._nLinksStereoDefined),
      _nSuperLayersDefined(a._nSuperLayersDefined),
      _minPtDefined(a._minPtDefined),

      _nLinks(a._nLinks),
      _nLinksStereo(a._nLinksStereo),
      _nSuperLayers(a._nSuperLayers),
      _minPt(a._minPt)
  {
  }

  TSelector::~TSelector()
  {
  }

  bool
  TSelector::select(TTrackBase& b) const
  {

    if (_nLinksDefined) {
      const unsigned n = b.nLinks();
      if (n < _nLinks) return false;
    }

    if (_nLinksStereoDefined) {
      const unsigned n = TLink::nStereoHits(b.links());
      if (n < _nLinksStereo) return false;
    }

    if (_nSuperLayersDefined) {
      const unsigned n = TLink::nSuperLayers(b.links());
      if (n < _nSuperLayers) return false;
    }

    if (_minPtDefined) {
      const unsigned type = b.objectType();
      if (type == Track) {
        if (((TTrack&) b).pt() < _minPt) return false;
      }
      if (type == Circle) {
        if (((TCircle&) b).pt() < _minPt) return false;
      }
    }

    return true;
  }

} // namespace Belle

