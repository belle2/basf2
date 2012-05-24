//-----------------------------------------------------------------------------
// $Id: TDebugUtilities.cc 10021 2007-03-03 05:43:02Z katayama $
//-----------------------------------------------------------------------------
// Filename : TUtilities.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : Definitions of utility functions
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.1  2005/11/24 01:33:13  yiwasaki
// addition of debug utilitiese
//
//-----------------------------------------------------------------------------

#define HEP_SHORT_NAMES


#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>
#include <map>
#include "tracking/modules/trasan/AList.h"
#include "tracking/modules/trasan/TDebugUtilities.h"


namespace Belle {

//...Trasan stage strings...
  AList<std::string> Stages;

//...Counter for each stage...
  std::map<std::string, stageInfo> Counters;



  std::string
  Stage(void)
  {
    std::string a;
    for (unsigned i = 0; i < (unsigned) Stages.length(); i++) {
      if (i)
        a += "/";
      a += * Stages[i];
    }
    return a;
  }

  std::string
  Tab(int shift)
  {
    std::string a;
    int totalShift = Stages.length() + shift;
    if (totalShift < 0) totalShift = 0;
    if (totalShift)
      for (unsigned i = 0; i < (unsigned) totalShift; i++)
        a += "    ";
    return a;
  }

  unsigned
  EnterStage(const std::string& a)
  {
    std::string* const now = new std::string(a);
    Stages.append(now);
    std::map<std::string, stageInfo>::iterator counter
      = Counters.find(Stage());
    struct rusage ru;
    getrusage(RUSAGE_SELF, & ru);
    if (counter == Counters.end()) {
      struct stageInfo c;
      c.counter = 1;
      c.usedUTime = 0;
      c.lastUTime = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec * 1e-6;
      c.usedSTime = 0;
      c.lastSTime = ru.ru_stime.tv_sec + ru.ru_stime.tv_usec * 1e-6;
      counter = Counters.insert(Counters.end(),
                                std::pair<std::string, stageInfo>
                                (Stage(), c));
    } else {
      ++counter->second.counter;
      counter->second.lastUTime =
        ru.ru_utime.tv_sec + ru.ru_utime.tv_usec * 1e-6;
      counter->second.lastSTime =
        ru.ru_stime.tv_sec + ru.ru_stime.tv_usec * 1e-6;
    }

    std::cout << Tab(-1) << "--> " << Stage() << "(" << NestLevel() << ")"
              << ",nCalls=" << counter->second.counter << std::endl;

    return (unsigned) Stages.length();
  }

  unsigned
  LeaveStage(const std::string& a)
  {
    std::cout << Tab(-1) << "<-- " << Stage() << "(" << NestLevel() << ")"
              << std::endl;

    const std::string& now = * Stages.last();
    if (now == a) {
      std::map<std::string, stageInfo>::iterator counter
        = Counters.find(Stage());
      if (counter != Counters.end()) {
        struct rusage ru;
        getrusage(RUSAGE_SELF, & ru);
        counter->second.usedUTime += ru.ru_utime.tv_sec
                                     + ru.ru_utime.tv_usec * 1e-6 - counter->second.lastUTime;
        counter->second.usedSTime += ru.ru_stime.tv_sec
                                     + ru.ru_stime.tv_usec * 1e-6 - counter->second.lastSTime;
      } else {
        std::cout << "!!! Stages are something wrong : counter not found"
                  << Stage() << std::endl;
      }
      delete Stages.last();
      Stages.remove(Stages.last());
    } else {
      std::cout << "!!! Stages are something wrong" << std::endl
                << "    Present stage=" << Stage() << std::endl
                << "    Leaving stage=" << a << std::endl;
    }
    return (unsigned) Stages.length();
  }

  unsigned
  NestLevel(void)
  {
    return (unsigned) Stages.length();
  }

  void
  DumpStageInformation(void)
  {
    std::cout << "Trasan Stage Information" << std::endl;
    std::cout << "    #stages=" << Counters.size() << std::endl;
// std::cout << "    #calls :      utime :      stime : stage" << std::endl;
    std::cout << "    #calls :   utime(s) : stage" << std::endl;
    std::map<std::string, stageInfo>::iterator p;
    for (p = Counters.begin(); p != Counters.end(); p++) {
      std::cout.width(10);
      std::cout << p->second.counter;
      std::cout << " : ";
      std::cout.setf(std::ios::fixed, std::ios::floatfield);
      std::cout.precision(3);
      std::cout.width(10);
      std::cout << (p->second.usedUTime + p->second.usedSTime);
//  std::cout << " : ";
//  std::cout.width(10);
//  std::cout << p->second.usedSTime;
      std::cout << " : " << p->first << std::endl;
    }
    std::cout.setf(std::ios::showpoint, std::ios::floatfield);
// std::cout << "    " << p->first << " : " << p->second << std::endl;
  }

} // namespace Belle

