/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrajectory2D;
    class UncertainPerigeeCircle;
    class CDCObservations2D;

    /// Class implementing the Karimaki fit for two dimensional trajectory circle
    class KarimakisMethod {

    public:
      /// Constructor setting the default constraints.
      KarimakisMethod();

      /// Executes the fit and updates the trajectory parameters. This may render the information in the observation object.
      void update(CDCTrajectory2D& trajectory2D, CDCObservations2D& observations2D) const;

    private:
      /// Internal method doing the heavy work.
      UncertainPerigeeCircle fitInternal(CDCObservations2D& observations2D) const;

    public:
      /// Getter for the indictor that lines should be fitted by this fitter
      bool isLineConstrained() const
      {
        return m_lineConstrained;
      }

      /// Indicator if this fitter is setup to fit lines
      void setLineConstrained(bool constrained = true)
      {
        m_lineConstrained = constrained;
      }

    private:
      /// Memory for the flag indicating that lines should be fitted
      bool m_lineConstrained;
    };
  }
}
