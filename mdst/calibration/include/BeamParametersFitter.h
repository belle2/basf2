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
     * Set HER angle.
     */
    void setAngleHER(double angleHER)
    {
      m_AngleHER = angleHER;
    }

    /**
     * Set LER angle.
     */
    void setAngleLER(double angleLER)
    {
      m_AngleLER = angleLER;
    }

    /**
     * Set angle error.
     */
    void setAngleError(double angleError)
    {
      m_AngleError = angleError;
    }

    /**
     * Set boost error.
     */
    void setBoostError(double boostError)
    {
      m_BoostError = boostError;
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

    /** HER angle. */
    double m_AngleHER = 0;

    /** LER angle. */
    double m_AngleLER = 0;

    /** Angle error. */
    double m_AngleError = 0;

    /** Boost error (use only if inverse error matrix is not available). */
    double m_BoostError = 0.0001;

    /** Collision boost vector. */
    DBObjPtr<CollisionBoostVector> m_CollisionBoostVector;

    /** Collision invariant mass. */
    DBObjPtr<CollisionInvariantMass> m_CollisionInvariantMass;

  };

}
