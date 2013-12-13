//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWireHit.cc
// Section  : CDC tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include "framework/datastore/StoreArray.h"
#include "tracking/modules/trasan/TWire.h"
#include "tracking/modules/trasan/TWireHit.h"

using namespace std;

namespace Belle {

vector<TWireHit *> TWireHit::_all;

TWireHit::TWireHit(const TWire & w,
		   unsigned indexCDCHit,
		   unsigned indexCDCSimHit,
		   unsigned indexMCParticle,
		   float driftLeft,
		   float driftLeftError,
		   float driftRight,
		   float driftRightError,
		   int mcLRflag,
		   float fudgeFactor)
    : TCellHit((TCell &) w,
	       indexCDCHit,
	       indexCDCSimHit,
	       indexMCParticle,
	       driftLeft,
	       driftLeftError,
	       driftRight,
	       driftRightError,
	       mcLRflag,
	       fudgeFactor), 
      _iCDCSimHit(indexCDCSimHit) {
}

TWireHit::~TWireHit() {
}

const TWire &
TWireHit::wire(void) const {
    return dynamic_cast<const TWire &>(cell());
}

int
TWireHit::sortByWireId(const TWireHit ** a,
                       const TWireHit ** b) {
    if ((* a)->cell().id() > (* b)->cell().id())
        return 1;
    else if ((* a)->cell().id() == (* b)->cell().id())
        return 0;
    else
        return -1;
}

void
TWireHit::removeAll(void) {
    while (_all.size())
	delete _all.back();
}

void *
TWireHit::operator new(size_t size) {
    void * p = malloc(size);
    _all.push_back((TWireHit *) p);

//     cout << ">---------------------" << endl;
//     for (unsigned i = 0; i < _all.size(); i++)
// 	cout << "> " << i << " " << _all[i] << endl;

    return p;
}

void
TWireHit::operator delete(void * t) {
    for (vector<TWireHit *>::iterator it = _all.begin();
	 it != _all.end();
	 it++) {
	if ((* it) == (TWireHit *) t) {
	    _all.erase(it);
	    break;
	}
    }
    free(t);

//     cout << "<---------------------" << endl;
//     cout << "==> " << t << " erased" << endl;
//     for (unsigned i = 0; i < _all.size(); i++)
// 	cout << "< " << i << " " << _all[i] << endl;
}

} // namespace Belle2
