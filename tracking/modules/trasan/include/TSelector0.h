//-----------------------------------------------------------------------------
// $Id: TSelector0.h 10021 2007-03-03 05:43:02Z katayama $
//-----------------------------------------------------------------------------
// Filename : TSelector0.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to select a TTrackBase object.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.1  2004/03/26 06:17:38  yiwasaki
// Trasan 3.10 : new files
//
//-----------------------------------------------------------------------------

#ifndef TSelector0_FLAG_
#define TSelector0_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>
#include <iostream>

namespace Belle {

#define HEP_SHORT_NAMES


  class TTrackBase;
  class TCircle;

/// A class to select a TTrackBase object.
  class TSelector0 {

  public:
    /// Constructor.
    TSelector0();

    /// Copy constructor.
    TSelector0(const TSelector0&);

    /// Destructor
    virtual ~TSelector0();

  public:// Selectors
    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns min. \# of hits(TLinks) requried.
    unsigned nLinks(void) const;

    /// returns min. \# of super layers required.
    unsigned nSuperLayers(void) const;

    /// returns min. pt required.
    double minPt(void) const;

    /// returns max. impact(2D) required.
    double maxImpact(void) const;

    /// returns min. \# of stereo hits(TLinks) requried.
    unsigned nLinksStereo(void) const;

    /// returns max. distance required for stereo hits.
    double maxDistance(void) const;

    /// returns max. sigma for each TLink.
    double maxSigma(void) const;

    /// returns true if given track satisfys criteria after fitting.
    bool select(TTrackBase&) const;

    /// returns true if given track satisfys criteria before fitting.
    bool preSelect(const TTrackBase&) const;

  public:// Modifiers
    /// sets \# of hits(TLinks) requried.
    unsigned nLinks(unsigned);

    /// sets \# of super layers required.
    unsigned nSuperLayers(unsigned);

    /// sets min. pt required.
    double minPt(double);

    /// sets max. impact(2D) required.
    double maxImpact(double);

    /// sets min. \# of stereo hits(TLinks) requried.
    unsigned nLinksStereo(unsigned);

    /// sets max. distance required for stereo hits.
    double maxDistance(double);

    /// sets max. sigma for each TLink.
    double maxSigma(double);

  private:

  private:
    bool _nLinksDefined;
    bool _nSuperLayersDefined;
    bool _minPtDefined;
    bool _maxImpactDefined;
    bool _maxSigmaDefined;

    unsigned _nLinks;
    unsigned _nSuperLayers;
    double _minPt;
    double _maxImpact;
    double _maxSigma;

    bool _nLinksStereoDefined;
    bool _maxDistanceDefined;

    unsigned _nLinksStereo;
    double _maxDistance;
  };

//-----------------------------------------------------------------------------

#ifdef TSelector0_NO_INLINE
#define inline
#else
#undef inline
#define TSelector0_INLINE_DEFINE_HERE
#endif

#ifdef TSelector0_INLINE_DEFINE_HERE

  inline
  unsigned
  TSelector0::nLinks(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _nLinksDefined)
      std::cout << "TSelector0 !!! min. nLinks is not defined" << std::endl;
#endif
    return _nLinks;
  }

  inline
  unsigned
  TSelector0::nLinks(unsigned a)
  {
    _nLinksDefined = true;
    return _nLinks = a;
  }

  inline
  unsigned
  TSelector0::nSuperLayers(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _nSuperLayers)
      std::cout << "TSelector0 !!! min. nSuperLayers is not defined" << std::endl;
#endif
    return _nSuperLayers;
  }

  inline
  unsigned
  TSelector0::nSuperLayers(unsigned a)
  {
    _nSuperLayersDefined = true;
    return _nSuperLayers = a;
  }

  inline
  double
  TSelector0::minPt(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _minPtDefined)
      std::cout << "TSelector0 !!! min. pt is not defined" << std::endl;
#endif
    return _minPt;
  }

  inline
  double
  TSelector0::minPt(double a)
  {
    _minPtDefined = true;
    return _minPt = a;
  }

  inline
  double
  TSelector0::maxImpact(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _maxImpactDefined)
      std::cout << "TSelector0 !!! max. impact is not defined" << std::endl;
#endif
    return _maxImpact;
  }

  inline
  double
  TSelector0::maxImpact(double a)
  {
    _maxImpactDefined = true;
    return _maxImpact = a;
  }

  inline
  double
  TSelector0::maxSigma(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _maxSigmaDefined)
      std::cout << "TSelector0 !!! max. sigma is not defined" << std::endl;
#endif
    return _maxSigma;
  }

  inline
  double
  TSelector0::maxSigma(double a)
  {
    _maxSigmaDefined = true;
    return _maxSigma = a;
  }

  inline
  unsigned
  TSelector0::nLinksStereo(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _nLinksStereoDefined)
      std::cout << "TSelector0 !!! min. nLinksStereo is not defined" << std::endl;
#endif
    return _nLinksStereo;
  }

  inline
  unsigned
  TSelector0::nLinksStereo(unsigned a)
  {
    _nLinksStereoDefined = true;
    return _nLinksStereo = a;
  }

  inline
  double
  TSelector0::maxDistance(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _maxDistanceDefined)
      std::cout << "TSelector0 !!! max. distance is not defined" << std::endl;
#endif
    return _maxDistance;
  }

  inline
  double
  TSelector0::maxDistance(double a)
  {
    _maxDistanceDefined = true;
    return _maxDistance = a;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TSelector0_FLAG_ */
