/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>

#include <framework/core/Module.h>

#include "trg/ecl/TrgEclCluster.h"
#include "trg/ecl/TrgEclTiming.h"
#include "trg/ecl/TrgEclBhabha.h"
#include "trg/ecl/TrgEclBeamBKG.h"
#include "trg/ecl/TrgEclMapping.h"
//
//
//
namespace Belle2 {
//
//
//
  /** ETM class */
  class TrgEclMaster {

  public:

    //!  get pointer of TrgEclMaster object
    static TrgEclMaster* getTrgEclMaster(void);

    //! TrgEclMaster Constructor
    TrgEclMaster(void);

    //! TrgEclMaster Destructor
    virtual ~TrgEclMaster();

    //! Copy constructor, deleted
    TrgEclMaster(TrgEclMaster&) = delete;

    //! Assignment operator, deleted
    TrgEclMaster& operator=(TrgEclMaster&) = delete;

  public:

    /** initialize */
    void initialize(void);
    /**simulates ECL trigger for Global Cosmic data  */
    void simulate01(int);
    /**simulates ECL trigger for Data Analysis */
    void simulate02(int);

  public:

    /** returns name. */
    std::string name(void) const;

    /** returns version. */
    std::string version(void) const;
    /** ECL bit information for GDL */
    //  void simulate(int);
    /** ECL bit information for GDL */
    //int getECLtoGDL(void) { return bitECLtoGDL; }
    /** Set Phi Ring Sum  */
    void setRS(std::vector<int>, std::vector<double>,
               std::vector<double>&, std::vector<std::vector<double>>&);
    /** Get Event timing */
    //    void getEventTiming(int option);
    /** Set Cluster*/
    void setClusterMethod(int cluster) {m_Clustering = cluster;}
    /** Set the limit # of Cluster*/
    void setClusterLimit(int limit) {m_ClusterLimit = limit;}
    /** Set Cluster*/
    void setEventTimingMethod(int EventTiming) {m_EventTiming = EventTiming;}
    /** Set Trigger Decision window size*/
    void setTimeWindow(int timewindow) {m_TimeWindow = timewindow;}
    /** Set Trigger Decision overlap window size*/
    void setOverlapWindow(int overlapwindow) {m_OverlapWindow = overlapwindow;}
    /** set # of considered TC in energy weighted Timing method */
    void setNofTopTC(int noftoptc) {m_NofTopTC = noftoptc;}
    /** make LowMultiTriggerBit **/
    void makeLowMultiTriggerBit(std::vector<int>, std::vector<double>, int);

    /** make Trigger bit except for Low Multiplicity related bit **/
    /*
    void makeTriggerBit(int, int, int, int, double, int, int,
                        std::vector<int>, int, int, int, int,
                        int, int, int, int, int, int, int,
                        int, int, int, int);
    */
    void makeTriggerBit(int, int, int, int, double,
                        int, int, int, int, int,
                        std::vector<int>, int, int, int, int,
                        int, int, int, int, int, int, int,
                        int, int, int, int, int);//one int parameter was added for taub2b3

    /** Set Total Energy*/
    double setTotalEnergy(std::vector<double>);
    //! Get ECL Trigger bit
    int getTriggerbit(int i) {return m_Triggerbit[i];}
    //! Get Low Multiplicity Trigger Bit
    int getLowmultibit() {return m_Lowmultibit;}

    //! set conversion factor of ADC to Energy in Lab in GeV
    void setADCtoEnergy(const double ADCtoEnergy)
    {
      m_ADCtoEnergy = ADCtoEnergy;
    }
    //! set Total Energy Threshold (low, high, lum)
    void setTotalEnergyThreshold(const std::vector<double>& iTotalEnergy)
    {
      m_TotalEnergy = iTotalEnergy;
    }
    //! set 2D Bhabha Energy Threshold
    void set2DBhabhaThreshold(const std::vector<double>& i2DBhabhaThresholdFWD,
                              const std::vector<double>& i2DBhabhaThresholdBWD)
    {
      m_2DBhabhaThresholdFWD = i2DBhabhaThresholdFWD;
      m_2DBhabhaThresholdBWD = i2DBhabhaThresholdBWD;
    }
    //! set 3D veto Bhabha Energy Threshold
    void set3DBhabhaVetoThreshold(const std::vector<double>& i3DBhabhaVetoThreshold)
    {
      m_3DBhabhaVetoThreshold = i3DBhabhaVetoThreshold;
    };
    //! set 3D veto Bhabha Energy Angle
    void set3DBhabhaVetoAngle(const std::vector<int>& i3DBhabhaVetoAngle)
    {
      m_3DBhabhaVetoAngle = i3DBhabhaVetoAngle;
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
    //! set 3D selection Bhabha Energy PreScale
    void set3DBhabhaSelectionPreScale(const std::vector<int>& i3DBhabhaSelectionPreScale)
    {
      m_3DBhabhaSelectionPreScale = i3DBhabhaSelectionPreScale;
    };
    //! set mumu bit Threshold
    void setmumuThreshold(double mumuThreshold) {m_mumuThreshold = mumuThreshold; }
    //! set mumu bit Angle
    void setmumuAngle(const std::vector<int>& imumuAngle)
    {
      m_mumuAngle = imumuAngle;
    }
    //! set hie12 3D Bhabha addition Angle
    void sethie12BhabhaVetoAngle(const std::vector<int>&  ihie12BhabhaVetoAngle)
    {
      m_hie12BhabhaVetoAngle = ihie12BhabhaVetoAngle;
    }
    //! set tau b2b 2 cluster angle cut
    void setTaub2bAngleCut(const std::vector<int>& itaub2bAngleCut)
    {
      m_taub2bAngleCut = itaub2bAngleCut;
    }
    //! set tau b2b total energy cut
    void setTaub2bEtotCut(double itaub2bEtotCut)
    {
      m_taub2bEtotCut = itaub2bEtotCut;
    }
    //! set tau b2b  1Cluster energy cut
    void setTaub2bCLELabCut(double itaub2bCLELabCut)
    {
      m_taub2bCLELabCut = itaub2bCLELabCut;
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
    //! set mumu bit Threshold
    void setECLBurstThreshold(int ECLBurstThreshold)
    {
      m_ECLBurstThreshold = (double) ECLBurstThreshold;
    }
    //! set Low Multiplicity cluster E cut in Lab in GeV
    void setlmlCLELabCut(const std::vector<double>& ilmlCLELabCut)
    {
      m_lmlCLELabCut = ilmlCLELabCut;
    }
    //! set Low Multiplicity cluster E cut in CMS in GeV
    void setlmlCLECMSCut(const std::vector<double>& ilmlCLECMSCut)
    {
      m_lmlCLECMSCut = ilmlCLECMSCut;
    }
    //! set the number of cluster for lml00
    void setlml00NCLforMinE(const int ilml00NCLforMinE)
    {
      m_lml00NCLforMinE = ilml00NCLforMinE;
    }
    //! set the number of cluster for lml12
    void setlml12NCLforMinE(const int ilml12NCLforMinE)
    {
      m_lml12NCLforMinE = ilml12NCLforMinE;
    }
    //! set ThetaID for lml13
    void setlml13ThetaIdSelection(const int ilml13ThetaIdSelection)
    {
      m_lml13ThetaIdSelection = ilml13ThetaIdSelection;
    }
    //! set theta ID region (low and high) of 3DBhabhaVeto InTrack for gg selection
    void set3DBhabhaVetoInTrackThetaRegion(const std::vector<int>& i3DBhabhaVetoInTrackThetaRegion)
    {
      m_3DBhabhaVetoInTrackThetaRegion = i3DBhabhaVetoInTrackThetaRegion;
    }
    //! set energy threshold(low and high) of event timing quality flag (GeV)
    void setEventTimingQualityThreshold(const std::vector<double>& iEventTimingQualityThreshold)
    {
      m_EventTimingQualityThreshold = iEventTimingQualityThreshold;
    }
    //! (hie4) CL E cut for miniimum energy cluster in Lab in GeV
    void sethie4LowCLELabCut(const double hie4LowCLELabCut)
    {
      m_hie4LowCLELabCut = hie4LowCLELabCut;
    }
    //! set parameters for TrgEclBhabha
    void setBhabhaParameter(void);

  private:

    /** Hit TC Energy */
    std::vector<std::vector<double>>  m_TCEnergy;
    /** Hit TC Timing */
    std::vector<std::vector<double>>  m_TCTiming;
    /** Hit TC Beam Background tag */
    std::vector<std::vector<int>>  m_TCBeamBkgTag;

    /** Hit TC Energy in time window */
    std::vector<int>  m_HitTCId;
    /** Hit TC Energy in time window */
    std::vector<double>  m_TCHitEnergy;
    /** Hit TC Timing in time window*/
    std::vector<double>  m_TCHitTiming;
    /** Hit TC Beam Background tag in time window */
    std::vector<int>  m_TCHitBeamBkgTag;

    /**  Phi ring sum */
    std::vector< std::vector<double>>  m_PhiRingSum;
    /**  Theta ring sum */
    std::vector<std::vector<std::vector<double>>> m_ThetaRingSum;

    /** Hit TC Energy in time window */
    //   std::vector<double> ClusterEnergy;
    /** Hit TC Timing in time window*/
    //    std::vector<double>  ClusterTiming;

    /** TRG Decision Time window */
    double m_TimeWindow;
    /** TRG Decision overlap window */
    double m_OverlapWindow;

    /** clustering option*/
    int m_Clustering;
    /** EventTiming option*/
    int m_EventTiming;
    /** # of considered TC in energy weighted Timing method */
    int m_NofTopTC;
    /** The limit number of Cluster */
    int m_ClusterLimit;
    //! ECL Trigger  bit
    int m_Triggerbit[4];
    //! Low Multiplicity bit
    int m_Lowmultibit;
    //! Bhabha Prescale Factor
    int m_PrescaleFactor;
    //! Bhabha Prescale Counter
    int m_PrescaleCounter;

    //! conversion factor of ADC to Energy in Lab in GeV
    double m_ADCtoEnergy;
    //! Total Energy Threshold (low, high, lum) in Lab in GeV
    std::vector<double> m_TotalEnergy;
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
    //! 3D Selection Bhabha Energy PreScale
    std::vector<int> m_3DBhabhaSelectionPreScale;
    //! mumu bit Energy Threshold
    double m_mumuThreshold;
    //! mumu bit  Angle
    std::vector<int> m_mumuAngle;
    //! hie12 angle selection of additional Bhabha addition in CM frame in degree
    std::vector<int> m_hie12BhabhaVetoAngle;
    //! tau b2b 2 cluster angle cut (degree)
    //! (dphi low, dphi high, theta_sum low, theta_sum high)
    std::vector<int> m_taub2bAngleCut;
    //! tau b2b total energy (TC theta ID =1-17) (GeV)
    double m_taub2bEtotCut;
    //! taub2b one Cluster energy selection in Lab (GeV)
    double m_taub2bCLELabCut;
    //! taub2b2 angle selection(degree)
    //! (3,2,1,0) = (dphi low, dphi high, theta_sum low, theta_sum high)
    std::vector<int> m_taub2b2AngleCut;
    //! taub2b2 total energy (TC theta ID =1-17) (GeV)
    double m_taub2b2EtotCut;
    //! taub2b2 cluster energy cut(high, low) (GeV) in lab
    std::vector<double> m_taub2b2CLELabCut;
    //! taub2b3 selection cuts
    //! (3,2,1,0) = (dphi low, dphi high, theta_sum low, theta_sum high)
    std::vector<int> m_taub2b3AngleCut;
    //! taub2b3 total energy (TC theta ID =1-17) (GeV)
    double m_taub2b3EtotCut;
    //! taub2b3 cluster energy cut in lab for one of b2b clusters (GeV)
    double m_taub2b3CLEb2bLabCut;
    //! taub2b3 cluster energy cut(low and high) in lab for all clusters (GeV)
    std::vector<double> m_taub2b3CLELabCut;
    //!ECL Burst Bit Threshold
    double m_ECLBurstThreshold;
    //! Low Multiplicity cluster E cut in Lab in GeV
    std::vector<double> m_lmlCLELabCut;
    //! Low Multiplicity cluster E cut in CMS in GeV
    std::vector<double> m_lmlCLECMSCut;
    //! the number of cluster for lml00
    int m_lml00NCLforMinE;
    //! the number of cluster for lml12
    int m_lml12NCLforMinE;
    //! ThetaID for lml13
    int m_lml13ThetaIdSelection;
    //! Theta region (low, high) of 3D Bhabha Veto InTrack
    std::vector<int> m_3DBhabhaVetoInTrackThetaRegion;
    //! energy threshold(low, high) for quality flag (GeV)
    std::vector<double> m_EventTimingQualityThreshold;
    //! (hie4) CL E cut for minimum energy cluster in Lab in GeV
    double m_hie4LowCLELabCut;

    /** Mapping object */
    TrgEclMapping* m_obj_map;
    /**  Cluster object */
    TrgEclCluster* m_obj_cluster;
    /**  EventTiming object */
    TrgEclTiming* m_obj_timing;
    /**  Bhabha object */
    TrgEclBhabha* m_obj_bhabha;
    /**  Beam Background veto object */
    TrgEclBeamBKG* m_obj_beambkg;
    /**  Beam Background veto object */
    TrgEclDataBase* m_obj_database;

  };
//
//
//
} // namespace Belle2
