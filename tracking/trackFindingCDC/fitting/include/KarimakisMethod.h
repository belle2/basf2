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

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/geometry/UncertainPerigeeCircle.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Class implementing the Karimaki fit for two dimensional trajectory circle
    class KarimakisMethod {

    public:
      /// Constructor setting the default constraints.
      KarimakisMethod();

      /// Executes the fit and updates the trajectory parameters. This may render the information in the observation object.
      void update(CDCTrajectory2D& fit, CDCObservations2D& observations2D) const;

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
