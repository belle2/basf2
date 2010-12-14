//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerTrackMC.h
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a GEN_HEPEVT particle in tracking.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef CDCTriggerTrackMC_FLAG_
#define CDCTriggerTrackMC_FLAG_

#include <vector>
#include "CLHEP/Vector/LorentzVector.h"
#include "trigger/cdc/CDCTrigger.h"

#ifdef CDCTRIGGER_SHORT_NAMES
#define CTTrackMC CDCTriggerTrackMC
#endif

namespace Belle2 {

/// A class to represent a GEN_HEPEVT particle in tracking.
class CDCTriggerTrackMC {

  public:
    /// returns a list of CDCTriggerTrackMC's.
    static std::vector<const CDCTriggerTrackMC *> list(void);

  private:
    /// updates information.
    static void update(void);

  private:
    /// Constructor
    CDCTriggerTrackMC();

  public:
    /// Destructor
    virtual ~CDCTriggerTrackMC();

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
    const CDCTriggerTrackMC * mother(void) const;

    /// returns a pointer to i'th child.
    const CDCTriggerTrackMC * child(unsigned i) const;

    /// returns momentum vector.
    const CLHEP::HepLorentzVector & p(void) const;

    /// returns position vector.
    const CLHEP::Hep3Vector & v(void) const;

    /// dumps debug information.
    void dump(const std::string & message = std::string(""),
	      const std::string & prefix = std::string("")) const;

    /// returns a list of CDCTriggerWireHitMC.
    std::vector<const CDCTriggerWireHitMC *> hits(void) const;

    /// returns a pointer to gen_hepevt.
//cnv    const struct gen_hepevt * gen(void) const;

  private:
//    const struct gen_hepevt * _hep;
    CDCTriggerTrackMC * _mother;
    std::vector<CDCTriggerTrackMC *> _children;
    const CLHEP::HepLorentzVector _p;
    const CLHEP::Hep3Vector _v;
    std::vector<CDCTriggerWireHitMC *> _hits;

  private:// static members
    static std::vector<const CDCTriggerTrackMC *> _list;

    friend class CDCTrigger;
};

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define CDCTriggerTrackMC_INLINE_DEFINE_HERE
#endif

#ifdef CDCTriggerTrackMC_INLINE_DEFINE_HERE

inline
unsigned
CDCTriggerTrackMC::id(void) const {
//cnv    return _hep->m_ID - 1;
    return 0;
}

inline
int
CDCTriggerTrackMC::status(void) const {
//cnv    return _hep->m_isthep;
    return 0;
}

inline
int
CDCTriggerTrackMC::pType(void) const {
//    return _hep->m_idhep;
    return 0;
}

inline
const CDCTriggerTrackMC *
CDCTriggerTrackMC::mother(void) const {
    return _mother;
}

inline
const CDCTriggerTrackMC *
CDCTriggerTrackMC::child(unsigned i) const {
    return _children[i];
}

inline
const CLHEP::HepLorentzVector &
CDCTriggerTrackMC::p(void) const {
    return _p;
}

inline
const CLHEP::Hep3Vector &
CDCTriggerTrackMC::v(void) const {
    return _v;
}

inline
std::string
CDCTriggerTrackMC::name(void) const {
    return "mc" + CDCTrigger::itostring(id());
}

#endif

#undef inline

} // namespace Belle2

#endif /* CDCTriggerTrackMC_FLAG_ */
