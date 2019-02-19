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
#include "trg/ecl/TrgEclDataBase.h"

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
    //! set 2D Bhabha Energy Threshold
    void set2DBhabhaThreshold(std::vector<double> i2DBhabhaThresholdFWD, std::vector<double> i2DBhabhaThresholdBWD)
    {
      _2DBhabhaThresholdFWD = i2DBhabhaThresholdFWD;
      _2DBhabhaThresholdBWD = i2DBhabhaThresholdBWD;
    }
    //! set 2D Bhabha Energy Threshold
    void set3DBhabhaThreshold(std::vector<double> i3DBhabhaThreshold) { _3DBhabhaThreshold = i3DBhabhaThreshold; };

  private:
    /** Object of TC Mapping */
    TrgEclMapping* _TCMap;
    /** Object of Trigger ECL DataBase */
    TrgEclDataBase* _database;

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
    //! 2D Bhabha Energy Threshold
    std::vector<double> _2DBhabhaThresholdFWD;
    //! 2D Bhabha Energy Threshold
    std::vector<double> _2DBhabhaThresholdBWD;
    //! 3D Bhabha Energy Threshold
    std::vector<double> _3DBhabhaThreshold;


  };
//
//
//
} // namespace Belle2

#endif /* TRGECLBhabha_FLAG_ */
