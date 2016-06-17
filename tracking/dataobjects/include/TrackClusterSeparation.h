/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <G4ThreeVector.hh>

namespace Belle2 {

  //! Store one Track-KLMCluster separation as a ROOT object.
  class TrackClusterSeparation : public RelationsObject {

  public:

    //! Empty constructor for default and ROOT IO (needed to make the class storable)
    TrackClusterSeparation();

    //! Destructor
    virtual ~TrackClusterSeparation() {}

    //! @return track index for track that is nearest the KLM cluster
    int getTrackIndex() const { return m_TrackIndex; }

    //! @return separation distance between track and KLM cluster
    double getDistance() const { return m_Distance; }

    //! @return separation direction between track and KLM cluster
    G4ThreeVector getDirection() const { return m_Direction; }

    //! @return angle between track-cluster direction and track momentum at the POCA
    double getTrackClusterAngle() const { return m_TrackClusterAngle; }

    //! @return angle between cluster direction and track's initial momentum
    double getTrackClusterInitialSeparationAngle() const { return m_TrackClusterInitialSeparationAngle; }

    //! @return angle between cluster direction and track's momentum at the POCA
    double getTrackClusterSeparationAngle() const { return m_TrackClusterSeparationAngle; }

    //! @return angle between track's momentum at the origin and at the POCA
    double getTrackRotationAngle() const { return m_TrackRotationAngle; }

    //! set track index for track that is nearest the KLM cluster
    void setTrackIndex(int t) { m_TrackIndex = t; }

    //! set separation distance between track and KLM cluster
    void setDistance(double d) { m_Distance = d; }

    //! set separation direction between track and KLM cluster
    void setDirection(G4ThreeVector dir) { m_Direction = dir; }

    //! set angle between track-cluster direction and track momentum at the POCA
    void setTrackClusterAngle(double angle) { m_TrackClusterAngle = angle; }

    //! set angle between cluster direction and track's initial momentum
    void setTrackClusterInitialSeparationAngle(double angle) { m_TrackClusterInitialSeparationAngle = angle; }

    //! set angle between cluster direction and track's momentum at the POCA
    void setTrackClusterSeparationAngle(double angle) { m_TrackClusterSeparationAngle = angle; }

    //! set angle between track's momentum at the origin and at the POCA
    void setTrackRotationAngle(double angle) { m_TrackRotationAngle = angle; }

  private:

    //! track index of the track nearest the KLM cluster
    int m_TrackIndex;

    //! distance between the track and KLM cluster
    double m_Distance;

    //! direction between the track's nearest position and KLM cluster
    G4ThreeVector m_Direction;

    //! angle between the track's momentum at the POCA and the track-to-cluster direction (nominally pi/2)
    double m_TrackClusterAngle;

    //! angle between the track's momentum at the origin and the cluster direction
    double m_TrackClusterInitialSeparationAngle;

    //! angle between the track's momentum at the POCA and the cluster direction
    double m_TrackClusterSeparationAngle;

    //! angle between the track's momentum at the origin and at the POCA
    double m_TrackRotationAngle;

    //! Needed to make the ROOT object storable
    ClassDef(TrackClusterSeparation, 2)

  };
}
