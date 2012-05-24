//-----------------------------------------------------------------------------
// $Id: TFitter.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : TFitter.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.7  2001/12/23 09:58:56  katayama
// removed Strings.h
//
// Revision 1.6  2001/12/19 02:59:55  katayama
// Uss find,istring
//
// Revision 1.5  2001/12/14 02:54:49  katayama
// For gcc-3.0
//
// Revision 1.4  2001/04/11 01:10:03  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.3  1999/10/30 10:12:49  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.2  1999/06/16 08:29:55  yiwasaki
// Trasan 1.44 release : new THelixFitter
//
// Revision 1.1  1999/01/11 03:03:25  yiwasaki
// Fitters added
//
//-----------------------------------------------------------------------------

#ifndef TFITTER_FLAG_
#define TFITTER_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif
#define HEP_SHORT_NAMES


#include <string>

namespace Belle {

  class TTrackBase;
  class TTrack;

#define TFitAlreadyFitted 1;
#define TFitErrorFewHits -1;
#define TFitFailed       -2;
#define TFitUnavailable  -3;

/// A class to fit a TTrackBase object.
  class TFitter {

  public:
    /// Constructor.
    TFitter(const std::string& name);

    /// Destructor
    virtual ~TFitter();

  public:// Selectors
    /// returns name.
    const std::string& name(void) const;
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:// Fit functions.
    virtual int fit(TTrackBase&) const = 0;

  protected:
    /// sets the fitted flag. (Bad implementation)
    void fitDone(TTrackBase&) const;

  private:
    std::string _name;
  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define TFITTER_INLINE_DEFINE_HERE
#endif

#ifdef TFITTER_INLINE_DEFINE_HERE

  inline
  const std::string&
  TFitter::name(void) const
  {
    return _name;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TFITTER_FLAG_ */
