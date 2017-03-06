//---------------------------------------------------------
// $Id$
//---------------------------------------------------------
// Filename : TrgEclBhabha.h
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------
// Description : A class to represent TRG ECL.
//---------------------------------------------------------
// $Log$
//---------------------------------------------------------

#ifndef TRGECLBHABHA_FLAG_
#define TRGECLBHABHA_FLAG_

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
  class TrgEclBhabha {

  public:
    /** Constructor */
    TrgEclBhabha();    /// Constructor

    /** Destructor */
    virtual ~TrgEclBhabha();/// Destructor

  public:

    bool GetBhabha00(std::vector<double>);//  Belle 2D Bhabha veto method
    bool GetBhabha01(std::vector<double>, std::vector<double>); //  Belle II 3D Bhabha veto method
    std::vector<double> GetBhabhaComb() {return BhabhaComb ;} // Output 2D Bhabha combination

  private:
    //
    /** 4 region along r phi plane  */
    int Quadrant[3][4];
    /** Object of TC Mapping */
    TrgEclMapping* _TCMap;
    std::vector<double> BhabhaComb;
  };
//
//
//
} // namespace Belle2

#endif /* TRGECLBhabha_FLAG_ */
