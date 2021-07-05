/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/ECLCluster.h>

#include <TVector3.h>

namespace Belle2 {

  /** A bremsstrahlung hit that correlates an ECLCluster with a RecoTrack */
  class BremHit : public RelationsObject {
  public:
    /**
     * Empty constructor for root.
     */
    BremHit() = default;

    /**
     * Create a BremHit, with information about the energy and position of the radiation
     * Automatically adds relation to the RecoTrack and the bremsstrahlung ECLCluster
     * @param recoTrack The RecoTrack to which the bremsstrahlung was found
     * @param bremCluster The found bremsstrahlung cluster
     * @param position The Position where the radiation took place
     * @param bremEnergy The energy of the bremsstrahlung photon, which was radiated
     * @param clusterDistance The difference between the angle of the tracks direction
     * and the clusters position relative to the radiation position
     * @param acceptanceFactor  The acceptance factor which is needed to find the BremHit
     */
    BremHit(const RecoTrack* recoTrack, const ECLCluster* bremCluster, const TVector3& position, double bremEnergy,
            double clusterDistance, double acceptanceFactor) :
      m_position(position),
      m_positionRadius(position.Perp()),
      m_bremEnergy(bremEnergy),
      m_clusterDistance(clusterDistance),
      m_acceptanceFactor(acceptanceFactor)
    {
      addRelationTo(recoTrack);
      addRelationTo(bremCluster);
    }

    /** Set the radiation position. */
    void setPosition(const TVector3& position) {m_position = position;}

    /** Set the radiated energy. */
    void setBremEnergy(const double bremEnergy) {m_bremEnergy = bremEnergy;}

    /** Set the distance between the extrapolation and the bremCluster position. */
    void setClusterDistance(const double distance) {m_clusterDistance = distance;}

    /** Return the radiation position. */
    TVector3 getPosition() const
    {
      return m_position;
    }
    /** Return the radiated energy. */
    double getBremEnergy() const
    {
      return m_bremEnergy;
    }

    /** Return the angle difference between the extrapolation and the bremCluster position. */
    double getClusterDistance() const
    {
      return m_clusterDistance;
    }

    /** Return the effective acceptance factor to find the BremHit. */
    double getEffAcceptanceFactor() const
    {
      return m_acceptanceFactor;
    }

  private:
    /** The radiation position. */
    TVector3 m_position;

    /** The radiation position radius. */
    double m_positionRadius;

    /** The radiated energy. */
    double m_bremEnergy;

    /** Angle difference between the extrapolation and the bremCluster position. */
    double m_clusterDistance;

    /** The acceptance factor which is needed to find the BremHit. */
    double m_acceptanceFactor;

    /** Making this class a ROOT class.*/
    ClassDef(BremHit, 1);
  };
}
