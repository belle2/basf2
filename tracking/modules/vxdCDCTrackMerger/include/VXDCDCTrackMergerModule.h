/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Benjamin Oberhof, Thomas Hauth                          *
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

#include <tracking/dataobjects/RecoTrack.h>

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
    virtual ~VXDCDCTrackMergerModule() = default;

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, RelationArrays,
     *  StoreObjPtrs) here, see the respective class documentation for details.
     */
    virtual void initialize();

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event();

  private:

    /**
     * pair of matched Tracks, first enry is the index of the VXD Track, second of the CDC Track
     */
    typedef std::pair<int, int> MatchPairType;

    /**
     * list of matched track pairs
     */
    typedef std::vector<MatchPairType> MatchPairList;

    /**
     * Takes the list of matches pairs and adds all items which do not
     * occur in the matchedList to the list out of output tracks (a.k.a. m_mergedRecoTracks)
     * This is useful to keep VXD and CDC tracks, which have not found any reasonbale merge.
     * if useFirstIndex = true, the VXD index will be used for the check, otherwise the CDC track index
     * will be used.
     */
    size_t addUnmatchedTracks(StoreArray<RecoTrack>& singleRecoTracks,
                              MatchPairList const& matchedList, bool useFirstIndex = true);

    /** Radius of the inner CDC wall in centimeters */
    double m_CDC_wall_radius = 16.25;
    /** Maximum distance between extrapolated tracks on the CDC wall */
    double m_merge_radius;

    /** StoreArray name of the VXD Track collection */
    std::string m_VXDRecoTrackColName;
    /** StoreArray name of the CDC Track collection */
    std::string m_CDCRecoTrackColName;
    /** StoreArray name of the merged Track collection */
    std::string m_mergedRecoTrackColName;

    /** StoreArray of the VXD Track collection */
    StoreArray<RecoTrack>  m_VXDRecoTracks;
    /** StoreArray of the CDC Track collection */
    StoreArray<RecoTrack>  m_CDCRecoTracks;
    /** StoreArray of the merged Track collection */
    StoreArray<RecoTrack>  m_mergedRecoTracks;

  };
}
#endif
