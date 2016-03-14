#ifndef VXDCDCTRACKMERGERANALYSISMODULE_H
#define VXDCDCTRACKMERGERANALYSISMODULE_H

#include <framework/core/Module.h>
#include "genfit/Track.h"
#include "genfit/RKTrackRep.h"
//#include "genfit/AbsTrackRep.h"

//root stuff
#include <TTree.h>
#include <TFile.h>


namespace Belle2 {
  class VXDCDCTrackMergerAnalysisModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    VXDCDCTrackMergerAnalysisModule();
    /** Use to clean up anything you created in the constructor. */
    virtual ~VXDCDCTrackMergerAnalysisModule();
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();

    /** Clean up anything you created in initialize(). */
    virtual void terminate();
  private:
    //    void insertTrackInGrid(double track_theta, double track_phi, genfit::Track* gftrack, std::vector<std::vector<std::vector<genfit::Track*>*>*>* tracks_grid);
    //void insertTrackAndPositionInGrid(TVector3 position, genfit::Track* gftrack, std::vector<std::vector<std::vector<TVector3>*>*>* positions_grid,
    //                                   std::vector<std::vector<std::vector<genfit::Track*>*>*>* tracks_grid);
    double m_CDC_wall_radius;
    double m_chi2_max;
    double m_merge_radius;
    std::string m_VXDGFTracksColName;
    std::string m_CDCGFTracksColName;
    std::string m_GFTracksColName;
    std::string m_TrackCandColName;
    std::string m_UnMergedCands;
    //std::string m_mcParticlesColName;
    //position at which a cdc track crosses  the cdc wall
    //std::vector<std::vector<std::vector<TVector3>*>*>* m_cdc_tracks_position;
    //the eta-phi grid silicon and cdc tracks grid
    //std::vector<std::vector<std::vector<genfit::Track*>*>*>* m_si_tracks_grid;
    //std::vector<std::vector<std::vector<genfit::Track*>*>*>* m_cdc_tracks_grid;

    //root stuff
    //bool m_produce_root_file;
    std::string m_root_output_filename;
    TTree* m_ttree;
    TFile* m_root_file;

    //for global trk merging efficiency
    double m_total_pairs;
    double m_total_matched_pairs;
    //root tree variables
    //int nEv;
    //int m_nevent;
    int m_npair;
    int m_ntruepair;
    int m_ncdc_trk;
    int m_nvxd_trk;
    //double m_trk_mrg_eff;
    std::vector<int>* m_match_vec;
    std::vector<int>* m_true_match_vec;
    std::vector<int>* m_true_match_mc;
    std::vector<int>* m_right_match_vec;
    //std::vector<int>* m_loop_match_vec;
    std::vector<float>* m_chi2_vec;
    std::vector<float>* m_dist_vec;
    std::vector<float>* m_dx_vec;
    std::vector<float>* m_dy_vec;
    std::vector<float>* m_dz_vec;
    std::vector<float>* m_x_vec;
    std::vector<float>* m_y_vec;
    std::vector<float>* m_z_vec;
    std::vector<float>* m_pos_vec;
    std::vector<float>* m_dmom_vec;
    std::vector<float>* m_dmomx_vec;
    std::vector<float>* m_dmomy_vec;
    std::vector<float>* m_dmomz_vec;
    std::vector<float>* m_momx_vec;
    std::vector<float>* m_momy_vec;
    std::vector<float>* m_momz_vec;
    std::vector<float>* m_mom_vec;
    std::vector<float>* m_vxdmomx_vec;
    std::vector<float>* m_vxdmomy_vec;
    std::vector<float>* m_vxdmomz_vec;
    std::vector<float>* m_vxdmom_vec;
  };
}
#endif
