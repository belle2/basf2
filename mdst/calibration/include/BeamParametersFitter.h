/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* MDST headers. */
#include <mdst/dbobjects/CollisionBoostVector.h>
#include <mdst/dbobjects/CollisionInvariantMass.h>

/* Belle 2 headers. */
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

namespace Belle2 {

  /**
   * Fitter calculating BeamParameters from CollisionBoostVector and
   * CollisionInvariantMass.
   */
  class BeamParametersFitter {

  public:

    /**
     * Constructor.
     */
    BeamParametersFitter()
    {
    }

    /**
     * Destructor.
     */
    ~BeamParametersFitter()
    {
    }

    /**
     * Set interval of validity.
     */
    void setIntervalOfValidity(const IntervalOfValidity& iov)
    {
      m_IntervalOfValidity = iov;
    }

    /**
     * Perform the fit.
     */
    void fit();

  protected:

    /**
     * Setup database.
     */
    void setupDatabase();

    /** Interval of validity. */
    IntervalOfValidity m_IntervalOfValidity;

    /** Collision boost vector. */
    DBObjPtr<CollisionBoostVector> m_CollisionBoostVector;

    /** Collision invariant mass. */
    DBObjPtr<CollisionInvariantMass> m_CollisionInvariantMass;

  };

}
