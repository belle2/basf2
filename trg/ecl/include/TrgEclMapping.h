//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TrgEclMapping.h
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent TRG ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGECLTCMAPPING_FLAG_
#define TRGECLTCMAPPING_FLAG_

#include <iostream>
#include <string>
#include <vector>
#include "TVector3.h"
//
//
//
namespace Belle2 {
//
//
//
  class TrgEclMapping;
//
  /*! A class of TC Mapping.*/
//
  class TrgEclMapping {

  public:

    /** Constructor */
    TrgEclMapping();

    /** Destructor */
    virtual ~TrgEclMapping() {};

  public:

    /** get [TC ID] from [Xtal ID] */
    int getTCIdFromXtalId(int);
    /** get [Xtal ID] from [TC ID] (Output as a vector)*/
    std::vector<int> getXtalIdFromTCId(int);
    /** get [TC sub ID] from [Xtal ID]*/
    int getTCSubIdFromXtalId(int);
    /** get [TC Theta ID] from [TC ID] */
    int getTCThetaIdFromTCId(int);
    /** get [TC Phi ID] from [TC ID] */
    int getTCPhiIdFromTCId(int);
    /**TC position (cm)*/
    TVector3 getTCPosition(int);

  private:


    /** TC Id */
    int _tcid;       // ID = 1-576
    /** TC Sub Id*/
    int _tcsubid;    // ID = 0-1
    /** TC Theta Id*/
    int _tcthetaid;  // ID = 1-17
    /** TC Phi Id*/
    int _tcphiid;    // ID = 1-36(1-32 for both endcap)

    /** Matrix of the relation of TC and Xtal*/
    std::vector<std::vector<int>> TC2Xtal;
  };
//
//
//
} // namespace Belle2

#endif /* TRGECLTCMAPPING_FLAG_ */
