//---------------------------------------------------------
// $Id$
//---------------------------------------------------------
// Filename : TrgEclTiming.h
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------
// Description : A class to represent TRG ECL.
//---------------------------------------------------------
// $Log$
//---------------------------------------------------------

#ifndef TRGECLTIMING_FLAG_
#define TRGECLTIMING_FLAG_

#include <iostream>
#include <string>
#include <vector>

#include "trg/ecl/TrgEclMapping.h"
#include "TVector3.h"
//
//
//
namespace Belle2 {
//
//
//
  //
  /*! A Class of  ECL Trigger clustering  */
  //
  class TrgEclTiming {

  public:
    /** Constructor */
    TrgEclTiming();    /// Constructor

    /** Destructor */
    virtual ~TrgEclTiming();/// Destructor

  public:
    void Setup(std::vector<int>, std::vector<double>, std::vector<double>); //setting
    double GetEventTiming(int);//Get eventtming
    double GetEventTiming00();// Fastest TC timing
    double GetEventTiming01();// The Most energetic TC Timing
    double GetEventTiming02();// Energy weighted Timing of Top 3 energetic TC
    void SetNofTopTC(int NtopTC) {NofTopTC = NtopTC;}

  private:
    std::vector<double> TCEnergy;
    std::vector<double> TCTiming;
    std::vector<int> TCId;

    int NofTopTC;
    /** Object of TC Mapping */
    TrgEclMapping* _TCMap;
  };
//
//
//
} // namespace Belle2

#endif /* TRGECLTiming_FLAG_ */
