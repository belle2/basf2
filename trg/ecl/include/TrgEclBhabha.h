/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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

    /** Copy constructor, deleted. */
    TrgEclBhabha(TrgEclBhabha&) = delete;

    /** Assignement operator, deleted. */
    TrgEclBhabha& operator=(TrgEclBhabha&) = delete;

  public:
    //!  Belle 2D Bhabha veto method
    bool GetBhabha00(std::vector<double>);
    //!  Belle II 3D Bhabha method for veto
    bool GetBhabha01();
    //!  Belle II 3D Bhabha method for selection
    bool GetBhabha02();
    //!  MuMu selection for calibration
    bool Getmumu();
    //!  Taub2b selection
    bool GetTaub2b(double);
    //! Output 2D Bhabha combination
    std::vector<double> GetBhabhaComb() {return BhabhaComb ;}
    //! Save
    void save(int);
    //! set 2D Bhabha Energy Threshold
    void set2DBhabhaThreshold(const std::vector<double>& i2DBhabhaThresholdFWD,
                              const std::vector<double>& i2DBhabhaThresholdBWD)
    {
      _2DBhabhaThresholdFWD = i2DBhabhaThresholdFWD;
      _2DBhabhaThresholdBWD = i2DBhabhaThresholdBWD;
    }
    //! set 3D selection Bhabha Energy Threshold
    void set3DBhabhaSelectionThreshold(const std::vector<double>& i3DBhabhaSelectionThreshold)
    {
      _3DBhabhaSelectionThreshold = i3DBhabhaSelectionThreshold;
    };
    //! set 3D veto Bhabha Energy Threshold
    void set3DBhabhaVetoThreshold(const std::vector<double>& i3DBhabhaVetoThreshold)
    {
      _3DBhabhaVetoThreshold = i3DBhabhaVetoThreshold;
    };
    //! set 3D selection Bhabha Energy Angle
    void set3DBhabhaSelectionAngle(const std::vector<double>& i3DBhabhaSelectionAngle)
    {
      _3DBhabhaSelectionAngle = i3DBhabhaSelectionAngle;
    };
    //! set 3D veto Bhabha Energy Angle
    void set3DBhabhaVetoAngle(const std::vector<double>& i3DBhabhaVetoAngle)
    {
      _3DBhabhaVetoAngle = i3DBhabhaVetoAngle;
    };
    //! set mumu bit Threshold
    void setmumuThreshold(int mumuThreshold) {_mumuThreshold = mumuThreshold; }
    //! set mumu bit Angle selection
    void setmumuAngle(const std::vector<double>&  imumuAngle) {_mumuAngle = imumuAngle; }
    // set ThetaID (low and high) for 3DBhabhaVetoInTrack
    void set3DBhabhaVetoInTrackThetaRegion(const std::vector<int>& i3DBhabhaVetoInTrackThetaRegion)
    {
      m_3DBhabhaVetoInTrackThetaRegion = i3DBhabhaVetoInTrackThetaRegion;
    }
    // set 2 Cluster angle selection for tau 1x1 decay
    // [0], [1] for low and high of dphi, [2], [3] for low and high of Theta Sum
    void setTaub2bAngleCut(const std::vector<int>& iTaub2bAngleCut)
    {
      m_Taub2bAngleCut = iTaub2bAngleCut;
    }
    //
    void setTaub2bEtotCut(const double iTaub2bEtotCut)
    {
      m_Taub2bEtotCut = iTaub2bEtotCut;
    }
    //
    void setTaub2bClusterECut(const double iTaub2bClusterECut1,
                              const double iTaub2bClusterECut2)
    {
      m_Taub2bClusterECut1 = iTaub2bClusterECut1;
      m_Taub2bClusterECut2 = iTaub2bClusterECut2;
    }
    // get trigger bit of flag(1bit) whether two clusters statisfy 3D Bhabha veto
    // are in CDCTRG region in theta (="InTrack") or not
    int get3DBhabhaVetoInTrackFlag(void)
    {
      return m_3DBhabhaVetoInTrackFlag;
    }
    // get each TCID(most energetic TC in a cluster) of two clusters of 3D Bhabha veto
    int get3DBhabhaVetoClusterTCId(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaVetoClusterTCIds.size() != 2) {
        return -10;
      }
      return m_3DBhabhaVetoClusterTCIds[cl_idx];
    }
    // get each TC theta ID(most energetic TC in a cluster) of
    // two clusters of 3D Bhabha veto
    int get3DBhabhaVetoClusterThetaId(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaVetoClusterThetaIds.size() != 2) {
        return -10;
      }
      return m_3DBhabhaVetoClusterThetaIds[cl_idx];
    }
    // get each cluster energy of two clusters of 3D Bhabha veto (GeV)
    double get3DBhabhaVetoClusterEnergy(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaVetoClusterEnergies.size() != 2) {
        return -10;
      }
      return m_3DBhabhaVetoClusterEnergies[cl_idx];
    }
    // get each cluster timing of two clusters of 3D Bhabha veto (ns)
    double get3DBhabhaVetoClusterTiming(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaVetoClusterTimings.size() != 2) {
        return -1000;
      }
      return m_3DBhabhaVetoClusterTimings[cl_idx];
    }
    // get trigger bit(2bits) of flag which shows theta position of clusters
    // of 3DBhabha Selection.
    // flag=0 : one of clusters goes to ThetaID=1
    // flag=1 : one of clusters goes to ThetaID=2
    // flag=2 : one of clusters goes to ThetaID=3
    // flag=3 : none of clusters fly to ThetaID=1-3
    // Based on this flag, pre-scale is applied on GDL to have flat entry of
    // Bhabha event in theta for calibration purpose
    int get3DBhabhaSelectionThetaFlag(void)
    {
      return m_3DBhabhaSelectionThetaFlag;
    }
    // get each TCID(most energetic TC in a cluster) of two clusters of 3D Bhabha selection
    int get3DBhabhaSelectionClusterTCId(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaSelectionClusterTCIds.size() != 2) {
        return -10;
      }
      return m_3DBhabhaSelectionClusterTCIds[cl_idx];
    }
    // get each TC theta ID(most energetic TC in a cluster) of
    // two clusters of 3D Bhabha selection
    int get3DBhabhaSelectionClusterThetaId(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaSelectionClusterThetaIds.size() != 2) {
        return -10;
      }
      return m_3DBhabhaSelectionClusterThetaIds[cl_idx];
    }
    // get each cluster energy of two clusters of 3D Bhabha selection (GeV)
    double get3DBhabhaSelectionClusterEnergy(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaSelectionClusterEnergies.size() != 2) {
        return -10;
      }
      return m_3DBhabhaSelectionClusterEnergies[cl_idx];
    }
    // get each cluster timing of two clusters of 3D Bhabha selection (ns)
    double get3DBhabhaSelectionClusterTiming(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaSelectionClusterTimings.size() != 2) {
        return -1000;
      }
      return m_3DBhabhaSelectionClusterTimings[cl_idx];
    }
    // get taub2b 2 cluster angle cut flag
    int getTaub2bAngleFlag(void) { return m_Taub2bAngleFlag; }
    // get total energy(TC theta id=1-17) flag for taub2b
    int getTaub2bEtotFlag(void) { return m_Taub2bEtotFlag; }
    // taub2b Cluster energy flag
    int getTaub2bClusterEFlag(void) { return m_Taub2bClusterEFlag; }

  private:
    /** Object of TC Mapping */
    TrgEclMapping* _TCMap = nullptr;
    /** Object of Trigger ECL DataBase */
    // cppcheck-suppress unsafeClassCanLeak
    TrgEclDataBase* _database = nullptr;

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
    // taub2b 2 Cluster angle cut
    std::vector<int> m_Taub2bAngleCut;
    // taub2b total energy(TC theta id=1-17) cut
    double m_Taub2bEtotCut{ -1};
    // taub2b Cluster energy cut
    double m_Taub2bClusterECut1{ -1};
    double m_Taub2bClusterECut2{ -1};
    // taub2b 2 cluster angle cut flag
    int m_Taub2bAngleFlag{0};
    // taub2b total energy(TC theta id=1-17) flag
    int m_Taub2bEtotFlag{0};
    // taub2b Cluster energy flag
    int m_Taub2bClusterEFlag{0};
    // trigger bit of flag(1bit) whether two clusters statisfy 3D Bhabha veto
    // are in CDCTRG region in theta (="InTrack") or not
    int                 m_3DBhabhaVetoInTrackFlag;
    // TCIDs of two clusters of 3D Bhabha veto
    std::vector<int>    m_3DBhabhaVetoClusterTCIds;
    // ThetaIds of two clusters of 3D Bhabha veto
    std::vector<int>    m_3DBhabhaVetoClusterThetaIds;
    // Energies of two clusters of 3D Bhabha veto (GeV)
    std::vector<double> m_3DBhabhaVetoClusterEnergies;
    // Timings of two clusters of 3D Bhabha veto (ns)
    std::vector<double> m_3DBhabhaVetoClusterTimings;
    // theta region(low and high) of 3D Bhbabha veto InTrack
    std::vector<int> m_3DBhabhaVetoInTrackThetaRegion;
    // flag which shows theta position of clusters of 3DBhabha Selection.
    int m_3DBhabhaSelectionThetaFlag = std::numeric_limits<int>::quiet_NaN();
    // TCIDs of two clusters of 3D Bhabha selection
    std::vector<int>    m_3DBhabhaSelectionClusterTCIds;
    // ThetaIDs of two clusters used for 3D Bhabha selection
    std::vector<int>    m_3DBhabhaSelectionClusterThetaIds;
    // Energies of two clusters used for 3D Bhabha selection (GeV)
    std::vector<double> m_3DBhabhaSelectionClusterEnergies;
    // Timings of two clusters used for 3D Bhabha selection (ns)
    std::vector<double> m_3DBhabhaSelectionClusterTimings;

  };
//
//
//
} // namespace Belle2
