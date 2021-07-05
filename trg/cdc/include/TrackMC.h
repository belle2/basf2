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
// Filename : TrackMC.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a GEN_HEPEVT particle in tracking.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCTrackMC_FLAG_
#define TRGCDCTrackMC_FLAG_

#include <vector>
#include "CLHEP/Vector/LorentzVector.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCTrackMC TRGCDCTrackMC
#endif

namespace Belle2 {

/// A class to represent a GEN_HEPEVT particle in tracking.
  class TRGCDCTrackMC {

  public:
    /// returns a list of TRGCDCTrackMC's.
    static std::vector<const TRGCDCTrackMC*> list(void);

  private:
    /// updates information.
    static void update(void);

  private:
    /// Constructor
    TRGCDCTrackMC();

  public:
    /// Destructor
    virtual ~TRGCDCTrackMC();

  public:
    /// returns name.
    std::string name(void) const;

    /// returns an id started from 0.
    unsigned id(void) const;

    /// returns status.
    int status(void) const;

    /// returns particle type.
    int pType(void) const;

    /// returns a pointer to a mother.
    const TRGCDCTrackMC* mother(void) const;

    /// returns a pointer to i'th child.
    const TRGCDCTrackMC* child(unsigned i) const;

    /// returns momentum vector.
    const CLHEP::HepLorentzVector& p(void) const;

    /// returns position vector.
    const CLHEP::Hep3Vector& v(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns a list of TRGCDCWireHitMC.
    std::vector<const TRGCDCWireHitMC*> hits(void) const;

    /// returns a pointer to gen_hepevt.
//cnv    const struct gen_hepevt * gen(void) const;

  public:

    /// Dummy particle.
    static TRGCDCTrackMC* _undefined;

  private:
    // const struct gen_hepevt * _hep;
    /// pointer to mother TRGCDCTrackMC
    TRGCDCTrackMC* _mother = nullptr;
    /// vector of pointers to children TRGCDCTrackMC
    std::vector<TRGCDCTrackMC*> _children;
    /// momentum
    const CLHEP::HepLorentzVector _p;
    /// vertex
    const CLHEP::Hep3Vector _v;
    /// vector of pointers to TRGCDCWireHitMC
    std::vector<TRGCDCWireHitMC*> _hits;

  private:// static members
    /// vector of pointers to TRGCDCTrackMC
    static std::vector<const TRGCDCTrackMC*> _list;

    friend class TRGCDC;
  };

//-----------------------------------------------------------------------------

  inline
  unsigned
  TRGCDCTrackMC::id(void) const
  {
//cnv    return _hep->m_ID - 1;
    return 0;
  }

  inline
  int
  TRGCDCTrackMC::status(void) const
  {
//cnv    return _hep->m_isthep;
    return 0;
  }

  inline
  int
  TRGCDCTrackMC::pType(void) const
  {
//    return _hep->m_idhep;
    return 0;
  }

  inline
  const TRGCDCTrackMC*
  TRGCDCTrackMC::mother(void) const
  {
    return _mother;
  }

  inline
  const TRGCDCTrackMC*
  TRGCDCTrackMC::child(unsigned i) const
  {
    return _children[i];
  }

  inline
  const CLHEP::HepLorentzVector&
  TRGCDCTrackMC::p(void) const
  {
    return _p;
  }

  inline
  const CLHEP::Hep3Vector&
  TRGCDCTrackMC::v(void) const
  {
    return _v;
  }

  inline
  std::string
  TRGCDCTrackMC::name(void) const
  {
    return "mc" + TRGUtilities::itostring(id());
  }

} // namespace Belle2

#endif /* TRGCDCTrackMC_FLAG_ */
