/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  //! Store one Track-KLMCluster separation as a ROOT object.
  class TrackClusterSeparation : public RelationsObject {

  public:

    //! Empty constructor for default and ROOT IO (needed to make the class storable)
    TrackClusterSeparation();

    //! Destructor
    virtual ~TrackClusterSeparation() {}

    //! @return separation distance between track and KLM cluster
    double getDistance() const { return m_Distance; }

    //! @return angle between track-cluster direction and track momentum at the POCA
    double getTrackClusterAngle() const { return m_TrackClusterAngle; }

    //! @return angle between cluster direction and track's initial momentum
    double getTrackClusterInitialSeparationAngle() const { return m_TrackClusterInitialSeparationAngle; }

    //! @return angle between cluster direction and track's momentum at the POCA
    double getTrackClusterSeparationAngle() const { return m_TrackClusterSeparationAngle; }

    //! @return angle between track's momentum at the origin and at the POCA
    double getTrackRotationAngle() const { return m_TrackRotationAngle; }

    //! set separation distance between track and KLM cluster
    void setDistance(double d) { m_Distance = d; }

    //! set angle between track-cluster direction and track momentum at the POCA
    void setTrackClusterAngle(double angle) { m_TrackClusterAngle = angle; }

    //! set angle between cluster direction and track's initial momentum
    void setTrackClusterInitialSeparationAngle(double angle) { m_TrackClusterInitialSeparationAngle = angle; }

    //! set angle between cluster direction and track's momentum at the POCA
    void setTrackClusterSeparationAngle(double angle) { m_TrackClusterSeparationAngle = angle; }

    //! set angle between track's momentum at the origin and at the POCA
    void setTrackRotationAngle(double angle) { m_TrackRotationAngle = angle; }

  private:

    //! distance between the track and KLM cluster
    float m_Distance;

    //! angle between the track's momentum at the POCA and the track-to-cluster direction (nominally pi/2)
    float m_TrackClusterAngle;

    //! angle between the track's momentum at the origin and the cluster direction
    float m_TrackClusterInitialSeparationAngle;

    //! angle between the track's momentum at the POCA and the cluster direction
    float m_TrackClusterSeparationAngle;

    //! angle between the track's momentum at the origin and at the POCA
    float m_TrackRotationAngle;

    //! Needed to make the ROOT object storable
    ClassDef(TrackClusterSeparation, 3)

  };
}
