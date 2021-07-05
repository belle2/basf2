/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : PerfectFinder.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks usning Hough algorithm
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCPerfectFinder_FLAG_
#define TRGCDCPerfectFinder_FLAG_

#include <string>

#ifdef TRGCDC_SHORT_NAMES
#define TCPFinder TRGCDCPerfectFinder
#endif

namespace Belle2 {

  class TRGCDC;
  class TRGCDCTrack;
  class TRGCDCLink;

/// A class to find 2D tracks using MC information
  class TRGCDCPerfectFinder {

  public:

    /// Contructor.
    TRGCDCPerfectFinder(const std::string& name,
                        const TRGCDC&);

    /// Destructor
    virtual ~TRGCDCPerfectFinder();

  public:

    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// do track finding.
    int doit(std::vector<TRGCDCTrack*>& trackListClone, std::vector<TRGCDCTrack*>& trackList);

    /// returns MC track list which contributes to CDC hits.
    const std::vector<int>& trackListMC(void) const;

  private:

    /// do perfect finding.
    int doitPerfectly(std::vector<TRGCDCTrack*>& trackList);

  private:

    /// Name
    const std::string _name;

    /// CDCTRG
    const TRGCDC& _cdc;

    /// MC track ID list
    std::vector<int> _mcList;
  };

//-----------------------------------------------------------------------------

  inline
  std::string
  TRGCDCPerfectFinder::name(void) const
  {
    return _name;
  }

  inline
  const std::vector<int>&
  TRGCDCPerfectFinder::trackListMC(void) const
  {
    return _mcList;
  }

} // namespace Belle2

#endif
