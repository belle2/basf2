//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWireHit.h
// Section  : CDC tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TWireHit_FLAG_
#define TWireHit_FLAG_

#include <vector>
#include "tracking/modules/trasan/TCellHit.h"

namespace Belle {

class TWire;
class TWireHitMC;

/// A class to represent a wire hit in CDC.
class TWireHit : public TCellHit {

  public:
    /// Constructor.
    TWireHit(const TWire &,
             unsigned indexCDCHit = 0,
             unsigned indexCDCSimHit = 0,
             unsigned indexMCParticle = 0,
             float driftLeft = 0,
             float driftLeftError = 0,
             float driftRight = 0,
             float driftRightError = 0,
             int mcLRflag=1,
             float fudgeFacgtor = 1);

    /// Destructor
    virtual ~TWireHit();

    /// destructs all TCellHit objects. (Called by TCDC)
    static void removeAll(void);

    /// new operator.
    static void * operator new(size_t);

    /// delete operator.
    static void operator delete(void *);

  public:// Selectors

    /// returns a pointer to a TWire.
    const TWire & wire(void) const;

    /// This will be removed.
    const TWireHitMC * mc(void) const;

    /// This will be removed.
    const TWireHitMC * mc(TWireHitMC *);

    ///  return index of CDCSimHit
    unsigned iCDCSimHit(void) const;

  public:// Static utility functions

    /// Sort function. This will be removed.
    static int sortByWireId(const TWireHit ** a,
			    const TWireHit ** b);

  private:

    /// Keeps all TWireHit created by new().
    static std::vector<TWireHit *> _all;

    /// This will be removed.
    const TWireHitMC * _mc;

    /// index of CDCSimHit
    unsigned _iCDCSimHit;

};

//-----------------------------------------------------------------------------

inline
const TWireHitMC *
TWireHit::mc(void) const {
    return _mc;
}

inline
const TWireHitMC *
TWireHit::mc(TWireHitMC * a) {
    return _mc = a;
}

inline 
unsigned 
TWireHit::iCDCSimHit(void) const{
    return _iCDCSimHit;
}

} // namespace Belle

#endif /* TWireHit_FLAG_ */
