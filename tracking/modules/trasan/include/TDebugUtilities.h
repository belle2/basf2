//-----------------------------------------------------------------------------
// $Id: TDebugUtilities.h 10019 2007-03-02 05:04:42Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TDebugUtilities.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : Definitions of utility functions
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.1  2005/11/24 01:33:15  yiwasaki
// addition of debug utilitiese
//
//-----------------------------------------------------------------------------

#ifndef TDebugUtilities_FLAG_
#define TDebugUtilities_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>

namespace Belle {

//...Trasan stage information...
  std::string Stage(void);
  std::string Tab(int shift = 0);
  unsigned EnterStage(const std::string&);
  unsigned LeaveStage(const std::string&);
  unsigned NestLevel(void);
  void DumpStageInformation(void);

//...Structure for stage analyses...
  struct stageInfo {
    unsigned counter;
    double usedUTime;
    double lastUTime;
    double usedSTime;
    double lastSTime;
  };

} // namespace Belle

#endif
