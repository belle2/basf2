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
// Filename : Relation.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent relations between TrackBase and MC track
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCRelation_FLAG_
#define TRGCDCRelation_FLAG_

#include <map>

#ifdef TRGCDC_SHORT_NAMES
#define TCRelation TRGCDCRelation
#endif

#include "trg/cdc/TrackBase.h"

namespace Belle2 {

//class G4Track;
  class MCParticle;
//class TRGCDCTrackBase;

/// A class to represent a wire in CDC.
  class TRGCDCRelation {

  public:

    /// Constructor.
    TRGCDCRelation(const TRGCDCTrackBase& track,
                   const std::map<unsigned, unsigned>& relation);

    /// Destructor
    virtual ~TRGCDCRelation();

  public:// Selectors

    /// returns a track.
    const TRGCDCTrackBase& track(void) const;

    /// returns /# of contributors.
    unsigned nContributors(void) const;

    /// returns i'th contributor of MCParticle.
//  G4Track contributor(unsigned i = 0) const;
    unsigned contributor(unsigned i = 0) const;

    /// returns i'th contributor.
    const MCParticle& mcParticle(unsigned i = 0) const;

    /// returns i'th purity.
    float purity(unsigned i = 0) const;

    /// returns purity for 3D for trkID particle which should be from 2D.
    float purity3D(unsigned trkID) const;

    /// returns efficiency of TS for 3D
    float efficiency3D(unsigned trkID, std::map<unsigned, unsigned>& numTSsParticle) const;

    /// Dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  private:

    /// Track
    const TRGCDCTrackBase& _track;

    /// Map
    const std::map<unsigned, unsigned> _relations;

    /// Pairs
    mutable const std::pair<unsigned, unsigned>** _pairs;
  };

//-----------------------------------------------------------------------------

  inline
  const TRGCDCTrackBase&
  TRGCDCRelation::track(void) const
  {
    return _track;
  }

  inline
  unsigned
  TRGCDCRelation::nContributors(void) const
  {
    return _relations.size();
  }

} // namespace Belle2
#endif /* TRGCDCRelation_FLAG_ */

