/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Benjamin Oberhof                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDCDCTRACKMERGERMODULE_H
#define VXDCDCTRACKMERGERMODULE_H

#include <framework/core/Module.h>
#include "genfit/TrackCand.h"
#include "genfit/Track.h"
#include "genfit/RKTrackRep.h"

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreArray.h>

using namespace genfit;

namespace Belle2 {
  /** VXDCDCTrackMergerModule a module to merge VXD and CDC tracks
   *
   *  It merges VXD and CDC tracks looking at their intersections
   *  with a cylinder placed at the boundary of the CDC with the VXD.
   */
  class VXDCDCTrackMergerModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    VXDCDCTrackMergerModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~VXDCDCTrackMergerModule();

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
    //    void insertTrackInGrid(double track_theta, double track_phi, genfit::Track* gftrack, std::vector<std::vector<std::vector<genfit::Track*>*>*>* tracks_grid);
    //void insertTrackAndPositionInGrid(TVector3 position, genfit::Track* gftrack, std::vector<std::vector<std::vector<TVector3>*>*>* positions_grid,
    //                                   std::vector<std::vector<std::vector<genfit::Track*>*>*>* tracks_grid);
    double m_CDC_wall_radius;
    double m_chi2_max;
    double m_merge_radius;
    bool m_recover;

    std::string m_VXDGFTracksColName;
    std::string m_VXDGFTrackCandsColName;
    std::string m_CDCGFTracksColName;
    std::string m_CDCGFTrackCandsColName;
    std::string m_mergedGFTrackCandsColName;


    std::string m_relMatchedTracks;

    //for global trk merging efficiency
    double m_total_pairs;
    double m_total_matched_pairs;
    //root tree variables

    // VXD TrackCands, Tracks
    StoreArray<TrackCand>  m_VXDGFTrackCands;
    StoreArray<Track>      m_VXDGFTracks;


    // CDC TrackCands, Tracks
    StoreArray<TrackCand>  m_CDCGFTrackCands;
    StoreArray<Track>      m_CDCGFTracks;


    // create the TrackCand ouput list
    StoreArray<TrackCand>  m_TrackCands;

    void collectMergedTrackCands(void);
  };
}
#endif
