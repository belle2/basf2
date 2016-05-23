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

    //! @return angle between separation direction and track momentum
    double getTrackAngle() const { return m_TrackAngle; }

    //! set track index for track that is nearest the KLM cluster
    void setTrackIndex(int t) { m_TrackIndex = t; }

    //! set separation distance between track and KLM cluster
    void setDistance(double d) { m_Distance = d; }

    //! set separation direction between track and KLM cluster
    void setDirection(G4ThreeVector dir) { m_Direction = dir; }

    //! set angle between separation direction and track momentum
    void setTrackAngle(double angle) { m_TrackAngle = angle; }

  private:

    //! track index of the track nearest the KLM cluster
    int m_TrackIndex;

    //! distance between the track and KLM cluster
    double m_Distance;

    //! direction between the track's nearest position and KLM cluster
    G4ThreeVector m_Direction;

    //! angle between the track's momentum and the track-to-cluster direction
    double m_TrackAngle;

    //! Needed to make the ROOT object storable
    ClassDef(TrackClusterSeparation, 1)

  };
}
