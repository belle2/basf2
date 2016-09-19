/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Store the Level 3 trigger information
  class L3Cluster : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    L3Cluster();

    //! Constructor with flag
    L3Cluster(const double energy, const TVector3& position);

    //! Destructor
    virtual ~L3Cluster() {}

    //! returns energy
    double getEnergy() const { return m_energy; }

    //! returns polar angle of the cluster position
    double getTheta() const { return m_theta; }

    //! return azimuthal angle of the cluster position
    double getPhi() const { return m_phi; }

    //! assigns energy
    void setEnergy(const double e) { m_energy = e; }

    //! assign polar angle of the cluster position
    void setTheta(const double theta) { m_theta = theta; }

    //! assign azimuthal angle of the cluster position
    void setPhi(const double phi) { m_phi = phi; }

  private:

    //! Energy of the cluster
    double m_energy;

    //! Polar angle of the cluster position
    double m_theta;

    //! Azimuthal angle of the cluster position
    double m_phi;

    //
    ClassDef(L3Cluster, 1)

  };
}
