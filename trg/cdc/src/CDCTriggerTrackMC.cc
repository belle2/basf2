//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerTrackMC.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a GEN_HEPEVT particle in tracking.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include "trigger/cdc/CDCTrigger.h"
#include "trigger/cdc/CDCTriggerTrackMC.h"

namespace Belle2 {

std::vector<const CDCTriggerTrackMC *>
CDCTriggerTrackMC::_list = std::vector<const CDCTriggerTrackMC *>();

void
CDCTriggerTrackMC::update(void) {
    if (_list.size()) {
	for (unsigned i = 0; i < _list.size(); i++)
	    delete _list[i];
	_list.clear();
    }

//    unsigned n = 0;

//     for (unsigned i = 0; i < n; i++) {
//	struct gen_hepevt * h = 0;
// 	if (! h) {
// 	    std::cout << "CDCTriggerTrackMC::update !!! can not access to GEN_HEPEVT";
// 	    std::cout << std::endl;
// 	    break;
// 	}
// // 	if (h->m_P[3] != 0.0 && (h->m_P[0] * h->m_P[0] + h->m_P[1] * h->m_P[1]
// // 				 + h->m_P[2] * h->m_P[2]) != 0.0) {
// 	    _list.push_back(new CDCTriggerTrackMC(h));
// // 	} else {
// // 	    std::cout << "CDCTriggerTrackMC::update !!! momentum/energy is zero";
// // 	    std::cout << std::endl;
// // 	}
//     }
}

// CDCTriggerTrackMC::CDCTriggerTrackMC(const struct gen_hepevt * h)
// : _hep(h),
//   _mother(0),
//   _p(h->m_P[0], h->m_P[1], h->m_P[2], h->m_P[3]),
//   _v(h->m_V[0], h->m_V[1], h->m_V[2]) {
//     if (_hep->m_mother != 0) {
// 	_mother = _list[_hep->m_mother - 1];
// 	_mother->_children.push_back(this);
//     }
// }

CDCTriggerTrackMC::CDCTriggerTrackMC() {
}

CDCTriggerTrackMC::~CDCTriggerTrackMC() {
    if (_list.size()) {
	for (unsigned i = 0; i < _list.size(); i++)
	    delete _list[i];
	_list.clear();
    }
}

void
CDCTriggerTrackMC::dump(const std::string &, const std::string & pre) const {
    std::cout << pre;
    std::cout << id() << ":";
    std::cout << pType() << ":";
    if (_mother) std::cout << _mother->id();
    else         std::cout << "-";
    std::cout << ":";
    std::cout << _p << ":" << _v;
    std::cout << std::endl;
}

std::vector<const CDCTriggerTrackMC *>
CDCTriggerTrackMC::list(void) {
    std::vector<const CDCTriggerTrackMC *> t;
    t.assign(_list.begin(), _list.end());
    return t;
}

std::vector<const CDCTriggerWireHitMC *>
CDCTriggerTrackMC::hits(void) const {
    std::vector<const CDCTriggerWireHitMC *> t;
    t.assign(_hits.begin(), _hits.end());
    return t;
}

} // namespace Belle2
