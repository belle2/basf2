//-----------------------------------------------------------------------------
// $Id: TSelector.h 10021 2007-03-03 05:43:02Z katayama $
//-----------------------------------------------------------------------------
// Filename : TSelector.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to select a TTrackBase object.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.11  2004/03/26 06:07:27  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.10  2001/12/23 09:58:57  katayama
// removed Strings.h
//
// Revision 1.9  2001/12/19 02:59:56  katayama
// Uss find,istring
//
// Revision 1.8  2001/12/14 02:54:51  katayama
// For gcc-3.0
//
// Revision 1.7  2001/04/11 01:10:04  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.6  1999/10/30 10:12:52  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.5  1999/01/11 03:03:28  yiwasaki
// Fitters added
//
// Revision 1.4  1998/07/29 04:35:26  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.2  1998/06/15 09:58:19  yiwasaki
// Trasan 1 beta 3.2, bug fixed
//
// Revision 1.1  1998/06/14 11:09:59  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
//-----------------------------------------------------------------------------

#ifndef TSelector_FLAG_
#define TSelector_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif
#define HEP_SHORT_NAMES


#include <iostream>
#include <string>

namespace Belle {

  class TTrackBase;
  class TCircle;

/// A class to select a TTrackBase object.
  class TSelector {

  public:
    /// Constructor.
    TSelector();

    /// Copy constructor.
    TSelector(const TSelector&);

    /// Destructor
    virtual ~TSelector();

  public:// Selectors
    /// returns true if given track satisfys criteria after fitting.
    bool select(TTrackBase&) const;

  public:// Flags
    /// sets \# of hits(TLinks) requried.
    bool nLinks(bool);

    /// sets min. \# of stereo hits(TLinks) requried.
    bool nLinksStereo(bool);

    /// sets \# of super layers required.
    bool nSuperLayers(bool);

    /// sets min. pt required.
    bool minPt(bool);

  public:// Parameters
    /// min. \# of hits(TLinks) requried.
    unsigned nLinks(unsigned);
    unsigned nLinks(void) const;

    /// min. \# of stereo hits(TLinks) requried.
    unsigned nLinksStereo(unsigned);
    unsigned nLinksStereo(void) const;

    /// \# of super layers required.
    unsigned nSuperLayers(unsigned);
    unsigned nSuperLayers(void) const;

    /// min. pt required.
    double minPt(double);
    double minPt(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  private:
    bool _nLinksDefined;
    bool _nLinksStereoDefined;
    bool _nSuperLayersDefined;
    bool _minPtDefined;

    unsigned _nLinks;
    unsigned _nLinksStereo;
    unsigned _nSuperLayers;
    float _minPt;
  };

//-----------------------------------------------------------------------------

#ifdef TSelector_NO_INLINE
#define inline
#else
#undef inline
#define TSelector_INLINE_DEFINE_HERE
#endif

#ifdef TSelector_INLINE_DEFINE_HERE

  inline
  unsigned
  TSelector::nLinks(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _nLinksDefined)
      std::cout << "TSelector !!! min. nLinks is not defined" << std::endl;
#endif
    return _nLinks;
  }

  inline
  unsigned
  TSelector::nLinks(unsigned a)
  {
    _nLinksDefined = true;
    return _nLinks = a;
  }

  inline
  unsigned
  TSelector::nSuperLayers(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _nSuperLayers)
      std::cout << "TSelector !!! min. nSuperLayers is not defined" << std::endl;
#endif
    return _nSuperLayers;
  }

  inline
  unsigned
  TSelector::nSuperLayers(unsigned a)
  {
    _nSuperLayersDefined = true;
    return _nSuperLayers = a;
  }

  inline
  double
  TSelector::minPt(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _minPtDefined)
      std::cout << "TSelector !!! min. pt is not defined" << std::endl;
#endif
    return _minPt;
  }

  inline
  double
  TSelector::minPt(double a)
  {
    _minPtDefined = true;
    return _minPt = a;
  }

  inline
  unsigned
  TSelector::nLinksStereo(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _nLinksStereoDefined)
      std::cout << "TSelector !!! min. nLinksStereo is not defined" << std::endl;
#endif
    return _nLinksStereo;
  }

  inline
  unsigned
  TSelector::nLinksStereo(unsigned a)
  {
    _nLinksStereoDefined = true;
    return _nLinksStereo = a;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TSelector_FLAG_ */
