/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Qingyuan Liu                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/utilities/PXDTuple.h>
#include <TVector3.h>
#include <vector>

namespace Belle2 {

  /** Using PXD::Tuple namespace for struct Track_t and the related helper functions */
  using namespace Belle2::PXD::Tuple;

  /** Class PXD2TrackEvent: Event data container for performance and calibration studies.
   * This class holds all required variables for TTree creation and
   * the analysis, such that no relation lookup is needed.
   *
   * Caveat: this class is not placed in the dataobjects folder
   * to avoid dependency cycles.
   */
  class PXD2TrackEvent : public RelationsObject  {

  public:

    /** default constructor for ROOT */
    PXD2TrackEvent(): m_vx(0.0), m_vy(0.0), m_vz(0.0),
      m_track_p(Track_t<TrackCluster_t>()),
      m_track_m(Track_t<TrackCluster_t>())
    {
    }

    /** Standard constructor
     * @param vertex Vertex of the two tracks
     * @param track_p The track with positive charge
     * @param track_m The track with negative charge
     */
    PXD2TrackEvent(const TVector3& vertex, const Track_t<TrackCluster_t>& track_p, const Track_t<TrackCluster_t>& track_m):
      m_vx(vertex.X()), m_vy(vertex.Y()), m_vz(vertex.Z()),
      m_track_p(track_p), m_track_m(track_m)
    {
    }

    /** Set values from RecoTrack collection */
    //bool setValues(const RelationVector<RecoTrack>& recoTracks);
    bool setValues(const StoreArray<RecoTrack>& recoTracks);

  protected:
    float m_vx;   /**< Position of the vertex in x. */
    float m_vy;   /**< Position of the vertex in y. */
    float m_vz;   /**< Position of the vertex in z. */
    Track_t<TrackCluster_t> m_track_p; /**< Track with the positive charge. */
    Track_t<TrackCluster_t> m_track_m; /**< Track with the negative charge. */

    ClassDef(PXD2TrackEvent, 1)
  };
} // end namespace Belle2
