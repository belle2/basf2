//-----------------------------------------------------------------------------
// $Id: TUpdater.cc 9944 2006-11-29 07:36:07Z katayama $
//-----------------------------------------------------------------------------
// Filename : TUpdater.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to handle update timing of static objects of tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.7  2003/12/25 12:03:36  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.6  2001/12/23 09:58:50  katayama
// removed Strings.h
//
// Revision 1.5  2001/12/19 02:59:48  katayama
// Uss find,istring
//
// Revision 1.4  2000/04/11 13:05:49  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.3  1999/06/09 15:09:54  yiwasaki
// Trasan 1.38 release : changes for lp
//
// Revision 1.2  1999/05/18 04:44:31  yiwasaki
// Trasan 1.33 release : bugs in salvage and masking are fixed, T0 calculation option is added
//
// Revision 1.1  1998/09/28 14:54:13  yiwasaki
// MC tables, TUpdater, oichan added
//
//
//-----------------------------------------------------------------------------

#include <iostream>

//#include "panther/panther.h"
#include "tracking/modules/trasan/TUpdater.h"

namespace Belle {

  TUpdater::TUpdater() : _expN(0), _runN(0), _evtN(0), _init(false)
  {
  }

  TUpdater::~TUpdater()
  {
  }

  void
  TUpdater::dump(const std::string&, const std::string& pre) const
  {
    std::cout << pre;

    std::cout << "updated:" << updated();
    std::cout << ",exp=" << _expN;
    std::cout << ",run=" << _runN;
    std::cout << ",evt=" << _evtN;
    std::cout << std::endl;
  }

  void
  TUpdater::update(void)
  {
//cnv     if (updated()) return;

//     struct belle_event * ev =
//  (struct belle_event *) BsGetEnt(BELLE_EVENT, 1, BBS_No_Index);

//     //...No BELLE_EVENT ???...
//     if (! ev) return;

//     _expN = ev->m_ExpNo;
//     _runN = ev->m_RunNo;
//     _evtN = ev->m_EvtNo;
  }

  bool
  TUpdater::updated(void) const
  {
//cnv
//     struct belle_event * ev =
//  (struct belle_event *) BsGetEnt(BELLE_EVENT, 1, BBS_No_Index);

//     //...No BELLE_EVENT ???...
//     if (! ev) return false;

//     //...Same event?...
//     if (_expN == ev->m_ExpNo &&
//  _runN == ev->m_RunNo &&
//  _evtN == ev->m_EvtNo) return true;

    return false;
  }

} // namespace Belle

