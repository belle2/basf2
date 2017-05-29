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
    //! SetUp
    void Setup(std::vector<int>, std::vector<double>, std::vector<double>);
    //! Get Evnet-timing
    double GetEventTiming(int);//Get eventtming
    //! Fastest TC Timing
    double GetEventTiming00();// Fastest TC timing
    //! The Most energetic TC Timing
    double GetEventTiming01();// The Most energetic TC Timing
    //! Energy weighted Timing of Top 3 energetic TC
    double GetEventTiming02();
    //! Set # of  considered TC  in Energy weighted timing method.
    void SetNofTopTC(int NtopTC) {NofTopTC = NtopTC;}

  private:
    //! TC Energy
    std::vector<double> TCEnergy;
    //! TC Timing
    std::vector<double> TCTiming;
    //! TC Id
    std::vector<int> TCId;

    //! # of  considered TC  in Energy weighted timing method.
    int NofTopTC;
    /** Object of TC Mapping */
    TrgEclMapping* _TCMap;
  };
//
//
//
} // namespace Belle2

#endif /* TRGECLTiming_FLAG_ */
