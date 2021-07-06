/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to represent a GEN_HEPEVT particle in tracking.
//-----------------------------------------------------------------------------

#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/TrackMC.h"

namespace Belle2 {

  TRGCDCTrackMC*
  TRGCDCTrackMC::_undefined = new TRGCDCTrackMC();

  std::vector<const TRGCDCTrackMC*>
  TRGCDCTrackMC::_list = std::vector<const TRGCDCTrackMC*>();

  void
  TRGCDCTrackMC::update(void)
  {
    if (_list.size()) {
      for (unsigned i = 0; i < _list.size(); i++)
        delete _list[i];
      _list.clear();
    }

//    unsigned n = 0;

//     for (unsigned i = 0; i < n; i++) {
//        struct gen_hepevt * h = 0;
//         if (! h) {
//             std::cout << "TRGCDCTrackMC::update !!! can not access to GEN_HEPEVT";
//             std::cout << std::endl;
//             break;
//         }
// //         if (h->m_P[3] != 0.0 && (h->m_P[0] * h->m_P[0] + h->m_P[1] * h->m_P[1]
// //                                  + h->m_P[2] * h->m_P[2]) != 0.0) {
//             _list.push_back(new TRGCDCTrackMC(h));
// //         } else {
// //             std::cout << "TRGCDCTrackMC::update !!! momentum/energy is zero";
// //             std::cout << std::endl;
// //         }
//     }
  }

// TRGCDCTrackMC::TRGCDCTrackMC(const struct gen_hepevt * h)
// : _hep(h),
//   _mother(0),
//   _p(h->m_P[0], h->m_P[1], h->m_P[2], h->m_P[3]),
//   _v(h->m_V[0], h->m_V[1], h->m_V[2]) {
//     if (_hep->m_mother != 0) {
//         _mother = _list[_hep->m_mother - 1];
//         _mother->_children.push_back(this);
//     }
// }

  TRGCDCTrackMC::TRGCDCTrackMC()
  {
  }

  TRGCDCTrackMC::~TRGCDCTrackMC()
  {
    if (_list.size()) {
      for (unsigned i = 0; i < _list.size(); i++)
        delete _list[i];
      _list.clear();
    }
  }

  void
  TRGCDCTrackMC::dump(const std::string&, const std::string& pre) const
  {
    std::cout << pre;
    std::cout << id() << ":";
    std::cout << pType() << ":";
    if (_mother) std::cout << _mother->id();
    else         std::cout << "-";
    std::cout << ":";
    std::cout << _p << ":" << _v;
    std::cout << std::endl;
  }

  std::vector<const TRGCDCTrackMC*>
  TRGCDCTrackMC::list(void)
  {
    std::vector<const TRGCDCTrackMC*> t;
    t.assign(_list.begin(), _list.end());
    return t;
  }

  std::vector<const TRGCDCWireHitMC*>
  TRGCDCTrackMC::hits(void) const
  {
    std::vector<const TRGCDCWireHitMC*> t;
    t.assign(_hits.begin(), _hits.end());
    return t;
  }

} // namespace Belle2
