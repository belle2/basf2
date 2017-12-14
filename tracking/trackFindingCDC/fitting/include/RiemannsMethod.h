/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrajectory2D;
    class CDCObservations2D;

    /// Class implementing the Riemann fit for two dimensional trajectory circle
    class RiemannsMethod  {

    public:
      /// Default constructor
      RiemannsMethod();

      /// Executes the fit and updates the trajectory parameters. This may render the information in the observation object.
      void update(CDCTrajectory2D& trajectory2D, CDCObservations2D& observations2D) const;

    private:
      /**
       *  Executes the fit without using the drift length information.
       *
       *  This method is used if there is drift length information is available from the observations.
       */
      void updateWithoutDriftLength(CDCTrajectory2D& trajectory2D,
                                    CDCObservations2D& observations2D) const;

      /**
       *  Executes the fit using the drift length information.
       *
       *  This method is used if there is no  drift length information is available from the observations.
       */
      void updateWithDriftLength(CDCTrajectory2D& trajectory2D,
                                 CDCObservations2D& observations2D) const;

    public:
      /// Getter for the indictor that lines should be fitted by this fitter
      bool isLineConstrained() const
      {
        return m_lineConstrained;
      }

      /// Getter for the indictor that curves through the origin should be fitted by this fitter
      bool isOriginConstrained() const
      {
        return m_originConstrained;
      }

      /// Indicator if this fitter is setup to fit lines
      void setLineConstrained(bool constrained = true)
      {
        m_lineConstrained = constrained;
      }

      /// Indicator if this fitter is setup to fit curves through the origin
      void setOriginConstrained(bool constrained = true)
      {
        m_originConstrained = constrained;
      }

    private:
      /// Memory for the flag indicating that lines should be fitted.
      bool m_lineConstrained;

      /// Memory for the flag indicating that curves through the origin shall be fitter.
      bool m_originConstrained;
    };
  }
}
