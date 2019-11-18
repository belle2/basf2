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
    //!  Belle II 3D Bhabha method for veto
    bool GetBhabha01();
    //!  Belle II 3D Bhabha method for selection
    bool GetBhabha02();
    //!  MuMu selection for calibration
    bool Getmumu();
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
    //! set 3D selection Bhabha Energy Threshold
    void set3DBhabhaSelectionThreshold(std::vector<double> i3DBhabhaSelectionThreshold) { _3DBhabhaSelectionThreshold = i3DBhabhaSelectionThreshold; };
    //! set 3D veto Bhabha Energy Threshold
    void set3DBhabhaVetoThreshold(std::vector<double> i3DBhabhaVetoThreshold) { _3DBhabhaVetoThreshold = i3DBhabhaVetoThreshold; };

    //! set 3D selection Bhabha Energy Angle
    void set3DBhabhaSelectionAngle(std::vector<double> i3DBhabhaSelectionAngle) { _3DBhabhaSelectionAngle = i3DBhabhaSelectionAngle; };
    //! set 3D veto Bhabha Energy Angle
    void set3DBhabhaVetoAngle(std::vector<double> i3DBhabhaVetoAngle) { _3DBhabhaVetoAngle = i3DBhabhaVetoAngle; };
    //! set mumu bit Threshold
    void setmumuThreshold(int mumuThreshold) {_mumuThreshold = mumuThreshold; }
    //! set mumu bit Angle selection
    void setmumuAngle(std::vector<double>  imumuAngle) {_mumuAngle = imumuAngle; }

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
    //! 3D Selection Bhabha Energy Threshold
    std::vector<double> _3DBhabhaSelectionThreshold;
    //! 3D Veto Bhabha Energy Threshold
    std::vector<double> _3DBhabhaVetoThreshold;
    //! 3D Selection Bhabha Energy Angle
    std::vector<double> _3DBhabhaSelectionAngle;
    //! 3D Veto Bhabha Energy Angle
    std::vector<double> _3DBhabhaVetoAngle;
    //! mumu bit Energy Threshold
    double _mumuThreshold;
    //! mumu bit  Angle
    std::vector<double> _mumuAngle;


  };
//
//
//
} // namespace Belle2

#endif /* TRGECLBhabha_FLAG_ */
