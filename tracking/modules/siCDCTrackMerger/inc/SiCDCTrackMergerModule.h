#ifndef SICDCTRACKMERGERMODULE_H
#define SICDCTRACKMERGERMODULE_H

#include <framework/core/Module.h>
#include "GFTrack.h"

//root stuff
#include <TTree.h>
#include <TFile.h>


namespace Belle2 {
  /** A module siCDCTrackMerger.
   *
   *  A detailed description of your module.
   */
  class SiCDCTrackMergerModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    SiCDCTrackMergerModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~SiCDCTrackMergerModule();

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, RelationArrays,
     *  StoreObjPtrs) here, see the respective class documentation for details.
     */
    virtual void initialize();

    /** Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun();

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event();

    /** Called once when a run ends.
     *
     *  Use this method to save run information, which you aggregated over the last run.
     */
    virtual void endRun();

    /** Clean up anything you created in initialize(). */
    virtual void terminate();
  private:
    void insertTrackInGrid(double track_theta, double track_phi, GFTrack* gftrack, std::vector<std::vector<std::vector<GFTrack*>*>*>* tracks_grid);
    void insertTrackAndPositionInGrid(TVector3 position, GFTrack* gftrack, std::vector<std::vector<std::vector<TVector3>*>*>* positions_grid,
                                      std::vector<std::vector<std::vector<GFTrack*>*>*>* tracks_grid);
    double m_CDC_wall_radius;
    //theta grid
    double m_theta_min;
    double m_theta_max;
    int m_N_theta_bin;
    double m_theta_bin_length;
    double m_theta_overlap_size;
    //phi grid
    double m_PI;
    double m_phi_min;
    double m_phi_max;
    int m_N_phi_bin;
    double m_phi_bin_length;
    double m_phi_overlap_size;

    std::string m_SiGFTracksColName;
    std::string m_CDCGFTracksColName;
    //position at which a cdc track crosses  the cdc wall
    std::vector<std::vector<std::vector<TVector3>*>*>* m_cdc_tracks_position;
    //the eta-phi grid silicon and cdc tracks grid
    std::vector<std::vector<std::vector<GFTrack*>*>*>* m_si_tracks_grid;
    std::vector<std::vector<std::vector<GFTrack*>*>*>* m_cdc_tracks_grid;

    //root stuff
    bool m_produce_root_file;
    std::string m_root_output_filename;
    TTree* m_ttree;
    TFile* m_root_file;

    //for global trk merging efficiency
    double m_total_pairs;
    double m_total_matched_pairs;
    //root tree variables
    int m_npair;
    int m_ncdc_trk;
    int m_nsi_trk;
    double m_trk_mrg_eff;
    std::vector<int>* m_match_vec;
    std::vector<float>* m_chi2_vec;
    std::vector<float>* m_dist_vec;
    std::vector<float>* m_dx_vec;
    std::vector<float>* m_dy_vec;
    std::vector<float>* m_dz_vec;
  };
}
#endif
