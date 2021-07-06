/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object.
//-----------------------------------------------------------------------------

#ifndef TRGCDCFITTER_FLAG_
#define TRGCDCFITTER_FLAG_

#include <string>

namespace Belle2 {

  class TRGCDCTrackBase;

#ifdef TRGCDC_SHORT_NAMES
#define TCFitter TRGCDCFitter
#endif

/// parameter to identify fit result
#define TRGCDCFitAlreadyFitted 1;
/// parameter to identify fit result
#define TRGCDCFitErrorFewHits -1;
/// parameter to identify fit result
#define TRGCDCFitFailed       -2;
/// parameter to identify fit result
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
    /// dump debug info
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:
    /// Fit functions.
    virtual int fit(TRGCDCTrackBase&) const = 0;

  protected:
    /// sets the fitted flag. (Bad implementation)
    void fitDone(TRGCDCTrackBase&) const;

  private:
    /// name
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
