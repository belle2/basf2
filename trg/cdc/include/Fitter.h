//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Fitter.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCFITTER_FLAG_
#define TRGCDCFITTER_FLAG_

#include <string>

namespace Belle2 {

  class TRGCDCTrackBase;
  class TRGCDCTrack;

#ifdef TRGCDC_SHORT_NAMES
#define TCFitter TRGCDCFitter
#endif

#define TRGCDCFitAlreadyFitted 1;
#define TRGCDCFitErrorFewHits -1;
#define TRGCDCFitFailed       -2;
#define TRGCDCFitUnavailable  -3;

/// A class to fit a TRGCDCTrackBase object.
  class TRGCDCFitter {

  public:
    /// Constructor.
    TRGCDCFitter(const std::string& name);

    /// Destructor
    virtual ~TRGCDCFitter();

  public:// Selectors
    /// returns name.
    const std::string& name(void) const;
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:// Fit functions.
    virtual int fit(TRGCDCTrackBase&) const = 0;

  protected:
    /// sets the fitted flag. (Bad implementation)
    void fitDone(TRGCDCTrackBase&) const;

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
  TRGCDCFitter::name(void) const
  {
    return _name;
  }

#endif

#undef inline

} // namespace Belle2

#endif /* TRGCDCFITTER_FLAG_ */
