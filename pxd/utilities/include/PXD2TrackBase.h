/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <mdst/dataobjects/Track.h>
#include <analysis/VertexFitting/RaveInterface/RaveVertexFitter.h>

#include <TVector3.h>
#include <vector>


namespace Belle2 {

  /** Class PXD2TrackBase: Event data container for performance and calibration studies.
   * This class holds all required variables for TTree creation and
   * the analysis, such that no relation lookup is needed.
   *
   * Caveat: this class is not placed in the dataobjects folder
   * to avoid dependency cycles.
   */

  template <class T>
  class PXD2TrackBase : public RelationsObject  {

  public:

    /** The base type for a track structure */
    typedef T baseType;

    /** default constructor for ROOT */
    PXD2TrackBase(): m_vx(0.0), m_vy(0.0), m_vz(0.0),
      m_track_p(),
      m_track_m()
    {
    }

    /** Standard constructor
     * @param vertex Vertex of the two tracks
     * @param track_p The track with positive charge
     * @param track_m The track with negative charge
     */
    PXD2TrackBase(const TVector3& vertex, const T& track_p, const T& track_m):
      m_vx(vertex.X()), m_vy(vertex.Y()), m_vz(vertex.Z()),
      m_track_p(track_p), m_track_m(track_m)
    {
    }

    /** Get the vertex.
     * @return Vertex of the 2 tracks.
     */
    TVector3 getVertex() const { return TVector3(m_vx, m_vy, m_vz); }

    /** Get the track with positive charge.
     * @return track struct with positive charge.
     */
    const T& getTrackP() const { return m_track_p; }

    /** Get the track with negative charge.
     * @return track struct with negative charge.
     */
    const T& getTrackM() const { return m_track_m; }

    /** Set values from RecoTrack collection.
     * @param recoTracks StoreArray of RecoTracks.
     * @param recoTracksName name of the related array of RecoTracks.
     * @param pxdInterceptsName name of the related array of PXDIntercepts.
     * @param pxdTrackClustersName name of the related array of PXDClustersFromTracks.
     * @return setting status true for success.
     */
    //bool setValues(const RelationVector<RecoTrack>& recoTracks);
    virtual bool setValues(const StoreArray<RecoTrack>& recoTracks,
                           const std::string& recoTracksName = "",
                           const std::string& pxdInterceptsName = "",
                           const std::string& pxdTrackClustersName = "PXDClustersFromTracks"
                          );

  protected:
    float m_vx;   /**< Position of the vertex in x. */
    float m_vy;   /**< Position of the vertex in y. */
    float m_vz;   /**< Position of the vertex in z. */
    T m_track_p; /**< Track with the positive charge. */
    T m_track_m; /**< Track with the negative charge. */

    ClassDef(PXD2TrackBase, 1)
  };

  template <class T>
  bool PXD2TrackBase<T>::setValues(const StoreArray<RecoTrack>& recoTracks,
                                   const std::string& recoTracksName,
                                   const std::string& pxdInterceptsName,
                                   const std::string& pxdTrackClustersName
                                  )
  {
    // Exactly 2 tracks
    //if(recoTracks.size() != 2) return false;
    if (!recoTracks.isValid() or recoTracks.getEntries() != 2) return false;

    // Valid Tracks
    auto track1Ptr = recoTracks[0]->getRelated<Track>("Tracks");
    auto track2Ptr = recoTracks[1]->getRelated<Track>("Tracks");
    if (!track1Ptr or !track2Ptr) return false;

    // Valid trackFitResults
    auto tfr1Ptr = track1Ptr->getTrackFitResultWithClosestMass(Const::pion);
    auto tfr2Ptr = track2Ptr->getTrackFitResultWithClosestMass(Const::pion);
    if (!tfr1Ptr or !tfr2Ptr) return false;

    // Valid P Values
    if (tfr1Ptr->getPValue() <= 0 or tfr2Ptr->getPValue() <= 0) return false;

    // Opposite charges
    if (tfr1Ptr->getChargeSign() * tfr2Ptr->getChargeSign() >= 0) return false;

    // Setup Rave vertex fitter
    auto bField = BFieldManager::getField(TVector3(0, 0, 0)).Z() / Unit::T;
    B2DEBUG(20, "B Field = " << bField << " T");
    analysis::RaveSetup::initialize(1, bField);
    analysis::RaveVertexFitter rvf;
    try {
      rvf.addTrack(tfr1Ptr);
      rvf.addTrack(tfr2Ptr);
    }  catch (...) {
      B2DEBUG(20, "Adding tracks to RaveVertexFitter failed.");
      return false;
    }
    if (rvf.fit() == 0) return false;
    // get vertex from fitting
    auto vertex = rvf.getPos();
    // Reset RAVE
    analysis::RaveSetup::getInstance()->reset();

    // Set vertex
    m_vx = vertex.X();
    m_vy = vertex.Y();
    m_vz = vertex.Z();
    B2DEBUG(20, "Vertex = (" << m_vx
            << ", " << m_vy
            << ", " << m_vz
            << ")");

    // Set track containers
    DBObjPtr<BeamSpot> beamSpotDB; // beam spot is required to correct d0/z0
    auto ip = beamSpotDB->getIPPosition();
    if (tfr1Ptr->getChargeSign() > 0) {
      m_track_p.setValues(*recoTracks[0], ip, recoTracksName, pxdInterceptsName, pxdTrackClustersName);
      m_track_m.setValues(*recoTracks[1], ip, recoTracksName, pxdInterceptsName, pxdTrackClustersName);
    } else {
      m_track_p.setValues(*recoTracks[1], ip, recoTracksName, pxdInterceptsName, pxdTrackClustersName);
      m_track_m.setValues(*recoTracks[0], ip, recoTracksName, pxdInterceptsName, pxdTrackClustersName);
    }
    return true;
  }
} // end namespace Belle2
