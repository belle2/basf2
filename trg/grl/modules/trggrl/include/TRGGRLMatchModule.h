/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI, Yun-Tsung Lai, Junhao Yin                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/ecl/dataobjects/TRGECLCluster.h>
#include <trg/klm/dataobjects/KLMTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/grl/dataobjects/TRGGRLShortTrack.h>
#include <trg/grl/dataobjects/TRGGRLInnerTrack.h>
#include <trg/grl/dataobjects/TRGGRLInfo.h>
#include <string>
#include <vector>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>


namespace Belle2 {
  /**
   * Match between CDC trigger track and ECL trigger cluster
   *
   *    *
   */
  class TRGGRLMatchModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TRGGRLMatchModule();

    /** Destructor */
    virtual ~TRGGRLMatchModule();

    /** Initialize the parameters */
    virtual void initialize() override;

    /** Called when entering a new run. */
    virtual void beginRun() override;

    /** Event processor. */
    virtual void event() override;

    /** End-of-run action. */
    virtual void endRun() override;

    /** Termination action. */
    virtual void terminate() override;

    /**calculate dr and dz between track and cluster*/
    void calculationdistance(CDCTriggerTrack* track, TRGECLCluster* cluster, double* ds, int _match3D);

    /**calculate dphi_d between track and cluster*/
    void calculationphiangle(CDCTriggerTrack* track, TRGECLCluster* cluster, int& dphi_d, std::vector<bool>& track_phimap,
                             std::vector<bool>& track_phimap_i);

    /**calculate dphi between 2D track and KLM track*/
    void sectormatching_klm(CDCTriggerTrack* track, KLMTriggerTrack* klmtrack, double& dphi);

    /**determine photon from isolated cluster*/
    bool photon_cluster(TRGECLCluster* cluster, std::vector<bool> track_phimap, double e_threshold);

    /** Force an int to be witnin 0 to 63*/
    int N64(int x);

    /** Force an int to be witnin 0 to 35*/
    int N36(int x);

    /** Fill the patterns in short tracking logic*/
    void fill_pattern_base2(std::vector< std::vector<int> >& patt);

    /** Make the full track phi veto map for short tracking*/
    void make_veto_map(StoreArray<CDCTriggerTrack> track2Dlist, std::vector<bool>&  map_veto);

    /** Make the ecl endcap phi map for inner/short track matching*/
    void make_eecl_map(StoreArray<TRGECLCluster> clusterlist, std::vector<bool>& ecl_phimap, std::vector<bool>& ecl_phimap_fwd,
                       std::vector<bool>& ecl_phimap_bwd);

    /** Short tracking logic*/
    void short_tracking(StoreArray<CDCTriggerSegmentHit> tslist, std::vector<bool>  map_veto, std::vector<bool>  phimap_i,
                        std::vector<bool>  ecl_phimap_fwd, std::vector<bool>  ecl_phimap_bwd,
                        std::vector<bool>  klm_sectormap_fwd, std::vector<bool>  klm_sectormap_bwd,
                        std::vector< std::vector<int> >& pattern_base0, std::vector< std::vector<int> >& pattern_base2,
                        StoreArray<TRGGRLShortTrack> grlst,
                        StoreObjPtr<TRGGRLInfo> trgInfo);

    void inner_tracking(StoreArray<CDCTriggerSegmentHit> tslist, std::vector<bool>  phimap_i,
                        std::vector<bool>  ecl_phimap, std::vector<bool>  klm_sectormap,
                        StoreArray<TRGGRLInnerTrack> grlit,
                        StoreObjPtr<TRGGRLInfo> trgInfo);

    /** Short track extrapolation (to endcap) function*/
    void extrapolation(int pattern, int& l, int& r, int& ec);

  private:
    StoreObjPtr<TRGGRLInfo> m_TRGGRLInfo; /**< output for TRGGRLInfo */

    /// Mode for TRGGRL simulation. 0th bit : fast simulation switch,
    /// 1st bit : firmware simulation switch.
    int m_simulationMode;

    /// Switch for the fast simulation. 0:do everything, 1:stop after
    /// the track segment simulation. Default is 0.
    int m_fastSimulationMode;

    /// Switch for the firmware simulation. 0:do nothing, 1:do everything
    int m_firmwareSimulationMode;

    /**max value of dr to be identified as match*/
    double m_dr_threshold;
    /**max value of dz to be identified as match */
    double m_dz_threshold;
    /**max value of dphi_d to be identified as match, 1 digit = 10 degrees */
    int m_dphi_d_threshold;
    /**min value of isolated cluster energy */
    double m_e_threshold;
    /**max value of dphi (CDC track to KLM sector) to be identified as match (in degrees) */
    double m_dphi_klm_threshold;
    /**36 bits phi map of all 2D tracks */
    std::vector<bool> track_phimap;
    /**36 bits phi map of all 2D tracks */
    std::vector<bool> track_phimap_i;
    /**36 bits phi map of ECL clusters at endcap */
    std::vector<bool> eecl_phimap;
    /**36 bits phi map of ECL clusters at forward endcap */
    std::vector<bool> eecl_phimap_fwd;
    /**36 bits phi map of ECL clusters at backward endcap */
    std::vector<bool> eecl_phimap_bwd;
    /**36 bits phi map of KLM clusters at endcap */
    std::vector<bool> eklm_sectormap;
    /**36 bits sector map of KLM clusters at forward endcap */
    std::vector<bool> eklm_sectormap_fwd;
    /**36 bits sector map of KLM clusters at backward endcap */
    std::vector<bool> eklm_sectormap_bwd;
    /**the 2D finder track list*/
    std::string m_2d_tracklist;
    /**the 3D NN track list*/
    std::string m_3d_tracklist;
    /**the ecl cluster list*/
    std::string m_clusterlist;
    /**the KLM track list*/
    std::string m_klmtracklist;
    /**the distance in phi direction between track and cluster*/
    //double m_dr;
    /**the distance in z direction between track and cluster*/
    //double m_dz;
    /**the matched 2d track list*/
    std::string m_2dmatch_tracklist;
    /**the matched 2d track list by phi matching*/
    std::string m_phimatch_tracklist;
    /**the matched 3d track list*/
    std::string m_3dmatch_tracklist;
    /**the matched 2d track list by KLM matching*/
    std::string m_klmmatch_tracklist;
    /**Non-matched cluster list at GRL*/
    std::string m_grlphotonlist;
    /**Track Segment list*/
    std::string m_hitCollectionName;
    /**GRL short track list*/
    std::string m_grlstCollectionName;
    /**GRL inner track list*/
    std::string m_grlitCollectionName;
    /** Name of the StoreArray holding projects information from grl */
    std::string m_TrgGrlInformationName;
    /** Short tracking patterns based on SL0*/
    std::vector< std::vector<int> > patterns_base0;
    /** Short tracking patterns based on SL2*/
    std::vector< std::vector<int> > patterns_base2;
  };
}
