//-----------------------------------------------------------------------------
// $Id: TSelector0.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TSelector0.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to select a TTrackBase object.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.1  2004/03/26 06:17:36  yiwasaki
// Trasan 3.10 : new files
//
//-----------------------------------------------------------------------------




#include "tracking/modules/trasan/TSelector0.h"
#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TCircle.h"
#include "tracking/modules/trasan/TLink.h"

namespace Belle {

  TSelector0::TSelector0()
    : _nLinksDefined(false),
      _nSuperLayersDefined(false),
      _minPtDefined(false),
      _maxImpactDefined(false),
      _nLinks(0),
      _nSuperLayers(0),
      _minPt(0.),
      _maxImpact(0.),
      _nLinksStereoDefined(false),
      _maxDistanceDefined(false),
      _nLinksStereo(0),
      _maxDistance(0.)
  {
  }

  TSelector0::TSelector0(const TSelector0& a)
    : _nLinksDefined(a._nLinksDefined),
      _nSuperLayersDefined(a._nSuperLayersDefined),
      _minPtDefined(a._minPtDefined),
      _maxImpactDefined(a._maxImpactDefined),

      _nLinks(a._nLinks),
      _nSuperLayers(a._nSuperLayers),
      _minPt(a._minPt),
      _maxImpact(a._maxImpact),
      _nLinksStereoDefined(a._nLinksStereoDefined),
      _maxDistanceDefined(a._maxDistanceDefined),
      _nLinksStereo(a._nLinksStereo),
      _maxDistance(a._maxDistance)
  {
  }

  TSelector0::~TSelector0()
  {
  }

  bool
  TSelector0::select(TTrackBase& b) const
  {

#ifdef TRASAN_DEBUG_DETAIL
    if (! b.fitted()) {
      std::cout << "    TSelect::select !!! not fitted yet" << std::endl;
    }
#endif

    if (b.objectType() == Track) {
      TTrack& a = (TTrack&) b;

      if (_minPtDefined) {
        if (a.pt() < _minPt) {
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "    TSelect ... rejected by min. pt(";
          std::cout << a.pt() << ") < ";
          std::cout << _minPt << std::endl;
#endif
          return false;
        }
      }

      if (_maxImpactDefined) {
        if (fabs(a.impact()) > _maxImpact) {
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "    TSelect ... rejected by max. impact(";
          std::cout << a.impact() << ") < ";
          std::cout << _maxImpact << std::endl;
#endif
          return false;
        }
      }
    } else if (b.objectType() == Circle) {
      TCircle& a = (TCircle&) b;

      if (_minPtDefined) {
        if (a.pt() < _minPt) {
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "    TSelect ... rejected by min. pt(";
          std::cout << a.pt() << ") < ";
          std::cout << _minPt << std::endl;
#endif
          return false;
        }
      }

      if (_maxImpactDefined) {
        if (fabs(a.impact()) > _maxImpact) {
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "    TSelect ... rejected by max. impact(";
          std::cout << a.impact() << ") < ";
          std::cout << _maxImpact << std::endl;
#endif
          return false;
        }
      }
    } else {
      std::cout
          << "TSelector0 !!! Unknown object type" << std::endl;
      return false;
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    TSelect::select ... accepted" << std::endl;
#endif
    return true;
  }

  bool
  TSelector0::preSelect(const TTrackBase& a) const
  {
    if (_nLinksDefined) {
      if (a.nLinks() < _nLinks) {
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "    TSelect ... rejected by nLinks(";
        std::cout << a.nLinks() << ") < ";
        std::cout << _nLinks << std::endl;
#endif
        return false;
      }
    }

    if (_nSuperLayersDefined) {
      if (TLink::nSuperLayers(a.links()) < _nSuperLayers) {
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "    TSelect ... rejected by nSuperLayers(";
        std::cout << TLink::nSuperLayers(a.links()) << ") < ";
        std::cout << _nSuperLayers << std::endl;
#endif
        return false;
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    TSelect::preSelect ... accepted" << std::endl;
#endif
    return true;
  }

} // namespace Belle

