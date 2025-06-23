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

#include <Math/Vector3D.h>

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
    bool getBhabha00(std::vector<double>);
    //!  Belle II 3D Bhabha method for veto
    bool getBhabha01();
    //!  Belle II 3D Bhabha method for selection
    bool getBhabha02();
    //!  MuMu selection for calibration
    bool getmumu();
    //!  Taub2b selection
    bool getTaub2b(double);
    //!  Taub2b selection (tighter selection than Taub2b)
    bool getTaub2b2(double);
    //! Taub2b3
    bool getTaub2b3(double);
    //! Additional Bhabha veto
    int getBhabhaAddition(void);
    //! Output 2D Bhabha combination
    std::vector<double> getBhabhaComb() {return m_BhabhaComb ;}
    //! Save
    void save(int);
    //! set 2D Bhabha Energy Threshold
    void set2DBhabhaThreshold(const std::vector<double>& i2DBhabhaThresholdFWD,
                              const std::vector<double>& i2DBhabhaThresholdBWD)
    {
      m_2DBhabhaThresholdFWD = i2DBhabhaThresholdFWD;
      m_2DBhabhaThresholdBWD = i2DBhabhaThresholdBWD;
    }
    //! set 3D veto Bhabha Energy Angle
    void set3DBhabhaVetoAngle(const std::vector<int>& i3DBhabhaVetoAngle)
    {
      m_3DBhabhaVetoAngle = i3DBhabhaVetoAngle;
    };
    //! set 3D veto Bhabha Energy Threshold
    void set3DBhabhaVetoThreshold(const std::vector<double>& i3DBhabhaVetoThreshold)
    {
      m_3DBhabhaVetoThreshold = i3DBhabhaVetoThreshold;
    };
    //! set 3D selection Bhabha Energy Threshold
    void set3DBhabhaSelectionThreshold(const std::vector<double>& i3DBhabhaSelectionThreshold)
    {
      m_3DBhabhaSelectionThreshold = i3DBhabhaSelectionThreshold;
    };
    //! set 3D selection Bhabha Energy Angle
    void set3DBhabhaSelectionAngle(const std::vector<int>& i3DBhabhaSelectionAngle)
    {
      m_3DBhabhaSelectionAngle = i3DBhabhaSelectionAngle;
    };
    //! set 3D selection pre-scale
    void set3DBhabhaSelectionPreScale(const std::vector<int>& i3DBhabhaSelectionPreScale)
    {
      m_3DBhabhaSelectionPreScale = i3DBhabhaSelectionPreScale;
    };
    //! set mumu bit Threshold
    void setmumuThreshold(double mumuThreshold) { m_mumuThreshold = mumuThreshold; }
    //! set mumu bit Angle selection
    void setmumuAngle(const std::vector<int>& imumuAngle) { m_mumuAngle = imumuAngle; }
    //! set hie12 3D Bhabha addtion Angle selection
    void sethie12BhabhaVetoAngle(const std::vector<int>& ihie12BhabhaVetoAngle)
    {
      m_hie12BhabhaVetoAngle = ihie12BhabhaVetoAngle;
    }
    //! set ThetaID (low and high) for 3DBhabhaVetoInTrack
    void set3DBhabhaVetoInTrackThetaRegion(const std::vector<int>& i3DBhabhaVetoInTrackThetaRegion)
    {
      m_3DBhabhaVetoInTrackThetaRegion = i3DBhabhaVetoInTrackThetaRegion;
    }
    //! set 2 Cluster angle selection for tau 1x1 decay
    //! [0], [1] for low and high of dphi, [2], [3] for low and high of Theta Sum
    void setTaub2bAngleCut(const std::vector<int>& iTaub2bAngleCut)
    {
      m_taub2bAngleCut = iTaub2bAngleCut;
    }
    //! set total energy cut for taub2b
    void setTaub2bEtotCut(const double iTaub2bEtotCut)
    {
      m_taub2bEtotCut = iTaub2bEtotCut;
    }
    //! set cluster energy cut for taub2b
    void setTaub2bCLELabCut(const double iTaub2bCLELabCut)
    {
      m_taub2bCLELabCut = iTaub2bCLELabCut;
    }
    //! set taub2b2 cut
    void setTaub2b2Cut(const std::vector<int>& iTaub2b2AngleCut,
                       const double iTaub2b2EtotCut,
                       const std::vector<double>& iTaub2b2CLELabCut)
    {
      m_taub2b2AngleCut  = iTaub2b2AngleCut;
      m_taub2b2EtotCut   = iTaub2b2EtotCut;
      m_taub2b2CLELabCut = iTaub2b2CLELabCut;
    }
    //! set taub2b3 cut
    void setTaub2b3Cut(const std::vector<int>& iTaub2b3AngleCut,
                       const double iTaub2b3EtotCut,
                       const double iTaub2b3CLEb2bLabCut,
                       const std::vector<double>& iTaub2b3CLELabCut)
    {
      m_taub2b3AngleCut     = iTaub2b3AngleCut;
      m_taub2b3EtotCut      = iTaub2b3EtotCut;
      m_taub2b3CLEb2bLabCut = iTaub2b3CLEb2bLabCut;
      m_taub2b3CLELabCut    = iTaub2b3CLELabCut;
    }
    //! (hie4) CL E cut for miniimum energy cluster in Lab in GeV
    void sethie4LowCLELabCut(const double hie4LowCLELabCut)
    {
      m_hie4LowCLELabCut = hie4LowCLELabCut;
    }
    //! get trigger bit of flag(1bit) whether two clusters statisfy 3D Bhabha veto
    //! are in CDCTRG region in theta (="InTrack") or not
    int get3DBhabhaVetoInTrackFlag(void)
    {
      return m_3DBhabhaVetoInTrackFlag;
    }
    //! get each TCID(most energetic TC in a cluster) of two clusters of 3D Bhabha veto
    int get3DBhabhaVetoClusterTCId(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaVetoClusterTCIds.size() != 2) {
        return -10;
      }
      return m_3DBhabhaVetoClusterTCIds[cl_idx];
    }
    //! get each TC theta ID(most energetic TC in a cluster) of
    //! two clusters of 3D Bhabha veto
    int get3DBhabhaVetoClusterThetaId(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaVetoClusterThetaIds.size() != 2) {
        return -10;
      }
      return m_3DBhabhaVetoClusterThetaIds[cl_idx];
    }
    //! get each cluster energy of two clusters of 3D Bhabha veto (GeV)
    double get3DBhabhaVetoClusterEnergy(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaVetoClusterEnergies.size() != 2) {
        return -10;
      }
      return m_3DBhabhaVetoClusterEnergies[cl_idx];
    }
    //! get each cluster timing of two clusters of 3D Bhabha veto (ns)
    double get3DBhabhaVetoClusterTiming(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaVetoClusterTimings.size() != 2) {
        return -1000;
      }
      return m_3DBhabhaVetoClusterTimings[cl_idx];
    }
    //! get trigger bit(2bits) of flag which shows theta position of clusters
    //! of 3DBhabha Selection.
    //! flag=0 : one of clusters goes to ThetaID=1
    //! flag=1 : one of clusters goes to ThetaID=2
    //! flag=2 : one of clusters goes to ThetaID=3
    //! flag=3 : none of clusters fly to ThetaID=1-3
    //! Based on this flag, pre-scale is applied on GDL to have flat entry of
    //! Bhabha event in theta for calibration purpose
    int get3DBhabhaSelectionThetaFlag(void)
    {
      return m_3DBhabhaSelectionThetaFlag;
    }
    //! get each TCID(most energetic TC in a cluster) of two clusters of 3D Bhabha selection
    int get3DBhabhaSelectionClusterTCId(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaSelectionClusterTCIds.size() != 2) {
        return -10;
      }
      return m_3DBhabhaSelectionClusterTCIds[cl_idx];
    }
    //! get each TC theta ID(most energetic TC in a cluster) of
    //! two clusters of 3D Bhabha selection
    int get3DBhabhaSelectionClusterThetaId(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaSelectionClusterThetaIds.size() != 2) {
        return -10;
      }
      return m_3DBhabhaSelectionClusterThetaIds[cl_idx];
    }
    //! get each cluster energy of two clusters of 3D Bhabha selection (GeV)
    double get3DBhabhaSelectionClusterEnergy(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaSelectionClusterEnergies.size() != 2) {
        return -10;
      }
      return m_3DBhabhaSelectionClusterEnergies[cl_idx];
    }
    //! get each cluster timing of two clusters of 3D Bhabha selection (ns)
    double get3DBhabhaSelectionClusterTiming(int cl_idx)
    {
      if (cl_idx < 0 || cl_idx > 1 || m_3DBhabhaSelectionClusterTimings.size() != 2) {
        return -1000;
      }
      return m_3DBhabhaSelectionClusterTimings[cl_idx];
    }
    //! get taub2b 2 cluster angle cut flag
    int getTaub2bAngleFlag(void) { return m_taub2bAngleFlag; }
    //! get total energy(TC theta id=1-17) flag for taub2b
    int getTaub2bEtotFlag(void) { return m_taub2bEtotFlag; }
    //! taub2b Cluster energy flag
    int getTaub2bClusterEFlag(void) { return m_taub2bClusterEFlag; }

  private:
    /** Object of TC Mapping */
    TrgEclMapping* m_TCMap = nullptr;
    /** Object of Trigger ECL DataBase */
    //! cppcheck-suppress unsafeClassCanLeak
    TrgEclDataBase* m_database = nullptr;

    //! 2 cluster energies, phi difference and theta sum
    void get2CLETP(int, int, int&, int&, int&, int&);

    /** Bhabha Combination*/
    std::vector<double> m_BhabhaComb;
    /** Max TC Id */
    std::vector<double> m_MaxTCId;
    /** Cluster ThetaId */
    std::vector<double> m_MaxTCThetaId;
    /** Cluster Energy*/
    std::vector<double> m_ClusterEnergyLab;
    /** Cluster Timing*/
    std::vector<double> m_ClusterTiming;
    //! 2D Bhabha Energy Threshold
    std::vector<double> m_2DBhabhaThresholdFWD;
    //! 2D Bhabha Energy Threshold
    std::vector<double> m_2DBhabhaThresholdBWD;
    //! 3D Veto Bhabha Energy Threshold
    std::vector<double> m_3DBhabhaVetoThreshold;
    //! 3D Veto Bhabha Energy Angle
    std::vector<int> m_3DBhabhaVetoAngle;
    //! 3D Selection Bhabha Energy Threshold
    std::vector<double> m_3DBhabhaSelectionThreshold;
    //! 3D Selection Bhabha Energy Angle
    std::vector<int> m_3DBhabhaSelectionAngle;
    //! 3D Selection Bhabha pre-scale
    std::vector<int> m_3DBhabhaSelectionPreScale;
    //! mumu bit Energy Threshold
    double m_mumuThreshold;
    //! mumu bit  Angle
    std::vector<int> m_mumuAngle;
    //! hie12 bit, Angle selection of additional Bhabha veto in CMS in degree
    std::vector<int> m_hie12BhabhaVetoAngle;


    //! taub2b 2 Cluster angle cut (degree)
    std::vector<int> m_taub2bAngleCut;
    //! taub2b Cluster one of energy cut in b2b in lab (GeV)
    double m_taub2bCLELabCut;
    //! taub2b total energy(TC theta id=1-17) cut (GeV)
    double m_taub2bEtotCut;


    //! taub2b 2 cluster angle cut flag
    int m_taub2bAngleFlag;
    //! taub2b total energy(TC theta id=1-17) flag
    int m_taub2bEtotFlag;
    //! taub2b Cluster energy flag
    int m_taub2bClusterEFlag;
    //! taub2b2 total energy cut (GeV)
    double m_taub2b2EtotCut;
    //! taub2b2 two Cluster angle cut (degree)
    std::vector<int> m_taub2b2AngleCut;
    //! taub2b2 cluster energy cut(high, low) (GeV) in lab
    std::vector<double> m_taub2b2CLELabCut;
    //! taub2b3 selection cuts
    //! (3,2,1,0) = (dphi low, dphi high, theta_sum low, theta_sum high)
    std::vector<int> m_taub2b3AngleCut;
    //! taub2b3 total energy (TC theta ID =1-17) (GeV)
    double m_taub2b3EtotCut;
    //! taub2b3 cluster energy cut (GeV) with b2b cluster condition in lab
    double m_taub2b3CLEb2bLabCut;
    //! taub2b3 cluster energy cut (GeV) for all clusters in lab
    std::vector<double> m_taub2b3CLELabCut;
    //! (hie4) CL E cut for miniimum energy cluster in Lab in GeV
    double m_hie4LowCLELabCut;
    //! trigger bit of flag(1bit) whether two clusters statisfy 3D Bhabha veto
    //! are in CDCTRG region in theta (="InTrack") or not
    int                 m_3DBhabhaVetoInTrackFlag;
    //! TCIDs of two clusters of 3D Bhabha veto
    std::vector<int>    m_3DBhabhaVetoClusterTCIds;
    //! ThetaIds of two clusters of 3D Bhabha veto
    std::vector<int>    m_3DBhabhaVetoClusterThetaIds;
    //! Energies of two clusters of 3D Bhabha veto (GeV)
    std::vector<double> m_3DBhabhaVetoClusterEnergies;
    //! Timings of two clusters of 3D Bhabha veto (ns)
    std::vector<double> m_3DBhabhaVetoClusterTimings;
    //! theta region(low and high) of 3D Bhbabha veto InTrack
    std::vector<int>    m_3DBhabhaVetoInTrackThetaRegion;
    //! flag which shows theta position of clusters of 3DBhabha Selection.
    int m_3DBhabhaSelectionThetaFlag = std::numeric_limits<int>::quiet_NaN();
    //! TCIDs of two clusters of 3D Bhabha selection
    std::vector<int>    m_3DBhabhaSelectionClusterTCIds;
    //! ThetaIDs of two clusters used for 3D Bhabha selection
    std::vector<int>    m_3DBhabhaSelectionClusterThetaIds;
    //! Energies of two clusters used for 3D Bhabha selection (GeV)
    std::vector<double> m_3DBhabhaSelectionClusterEnergies;
    //! Timings of two clusters used for 3D Bhabha selection (ns)
    std::vector<double> m_3DBhabhaSelectionClusterTimings;
  };
//
//
//
} // namespace Belle2
