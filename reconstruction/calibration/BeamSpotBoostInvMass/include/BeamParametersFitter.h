/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* MDST headers. */
#include <mdst/dbobjects/BeamSpot.h>
#include <mdst/dbobjects/CollisionBoostVector.h>
#include <mdst/dbobjects/CollisionInvariantMass.h>

/* Basf2 headers. */
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/dbobjects/BeamParameters.h>

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
     * Set invariant-mass error.
     */
    void setInvariantMassError(double invariantMassError)
    {
      m_InvariantMassError = invariantMassError;
    }

    /**
     * Set whether to be verbose (print Minuit output).
     */
    void setVerbose(bool verbose)
    {
      m_Verbose = verbose;
    }

    /**
     * Get beam parameters.
     */
    BeamParameters getBeamParameters() const
    {
      return m_BeamParameters;
    }

    /**
     * Set beam parameters.
     */
    void setBeamParameters(const BeamParameters* beamParameters)
    {
      m_BeamParameters = *beamParameters;
    }

    /**
     * Perform the fit.
     */
    void fit();

    /**
     * Fill beam spot (vertex) data. The covariance matrix is rescaled
     * to nominal XX and YY covariances. If covariances are negative,
     * then no rescaling is performed.
     * @param[in] covarianceXX Covariance-matrix XX element.
     * @param[in] covarianceYY Covariance-matrix YY element.
     */
    void fillVertexData(double covarianceXX, double covarianceYY);

    /**
     * Import beam parameters.
     */
    void importBeamParameters();

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

    /** Invariant-mass error (use only if error is 0). */
    double m_InvariantMassError = 0.0001;

    /** Whether to be verbose (print Minuit output). */
    bool m_Verbose = false;

    /** Beam parameters. */
    BeamParameters m_BeamParameters;

    /** Beam spot. */
    DBObjPtr<BeamSpot> m_BeamSpot;

    /** Collision boost vector. */
    DBObjPtr<CollisionBoostVector> m_CollisionBoostVector;

    /** Collision invariant mass. */
    DBObjPtr<CollisionInvariantMass> m_CollisionInvariantMass;

  };

}
