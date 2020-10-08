/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Dey, Abi Soffer                                   *
 * Past Contributors: Omer Benami                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once
#include <framework/core/Module.h>
#include <string>
#include <TMatrixFSym.h>

#include <Eigen/Core>

#include <analysis/dataobjects/Btube.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  /**
   * Calculates distance between two vertices, distance of closest approach between a vertex and a track, distance of closest approach between a vertex and Btube. The calculation ignores track curvature, it's negligible for small distances.
   */
  class DistanceCalculatorModule : public Module {

  public:
    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    DistanceCalculatorModule();

    /** Destructor */
    virtual ~DistanceCalculatorModule();

    /** declare data store elements */
    virtual void initialize() override;

    /** process event */
    virtual void event() override;

  private:

    /** returns the distance between tracks or vertex objects (depending on the m_mode) */
    void getDistance(const Particle* p1, const Particle* p2);
    /** returns the error on the distance between tracks or vertex objects (depending on the m_mode) */
    void getDistanceErrors(const Particle* p1, const Particle* p2);
    /** returns the distance between the btube and the object (depending on m_mode) */
    void getBtubeDistance(const Particle* p, const Btube* t);
    /** returns the error on the distance between the btube and the object (depending on m_mode) */
    void getBtubeDistanceErrors(const Particle* p, const Btube* t);

    std::string m_listName; /**< name of particle list */
    std::string m_decayString;  /**< decay string */
    DecayDescriptor m_decayDescriptor;  /**< decay descriptor which specifies which particles are used to calculate the distance */
    std::string m_mode; /**< option string */
    Eigen::Vector3d m_distance; /**< distance between two objects (track/vertex/Btube and vertex/track) */
    TMatrixFSym m_distanceCovMatrix; /**< covariance matrix of distance */
    StoreObjPtr<ParticleList> m_plist; /**< input particle list */
  };
}


