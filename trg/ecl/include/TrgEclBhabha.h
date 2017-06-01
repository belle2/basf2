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
    //!  Belle 2D Bhabha veto method
    bool GetBhabha00(std::vector<double>);
    //!  Belle II 3D Bhabha veto method
    bool GetBhabha01();
    //! Output 2D Bhabha combination
    std::vector<double> GetBhabhaComb() {return BhabhaComb ;}
    //! Save
    void save(int);

  private:
    /** Object of TC Mapping */
    TrgEclMapping* _TCMap;
    /** Bhabha Combination*/
    std::vector<double> BhabhaComb;
    /** Max TC Id */
    std::vector<double> MaxTCId;
    /** Cluster Energy*/
    std::vector<double> ClusterEnergy;
    /** Cluster Timing*/
    std::vector<double> ClusterTiming;
    /** Cluster Timing*/
    std::vector<TVector3> ClusterPosition;


  };
//
//
//
} // namespace Belle2

#endif /* TRGECLBhabha_FLAG_ */
