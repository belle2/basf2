/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef EXTENDEDRIEMANNSMETHOD_H
#define EXTENDEDRIEMANNSMETHOD_H

#include "TMatrixDSym.h"
#include "TVectorD.h"

#include <tracking/cdcLocalTracking/fitting/CDCObservations2D.h>
namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class implementing the Riemann fit for two dimensional trajectory circle
    class ExtendedRiemannsMethod : public CDCLocalTracking::SwitchableRootificationBase {

    public:
      ///Empty constructor
      ExtendedRiemannsMethod();

      ///Empty destructor
      ~ExtendedRiemannsMethod();

      /// Executes the fit and updates the trajectory parameters. This may render the information in the observation object.
      void update(CDCTrajectory2D& fit, CDCObservations2D& observations2D) const;


    private:
      /// Internal method doing the heavy work.
      UncertainPerigeeCircle fit(CDCObservations2D&) const;


    public:
      /// Getter for the indictor that lines should be fitted by this fitter
      bool isLineConstrained() const
      { return m_lineConstrained; }

      /// Getter for the indictor that curves through the origin should be fitted by this fitter
      bool isOriginConstrained() const
      { return m_originConstrained; }

      /// Indicator if this fitter is setup to fit lines
      void setLineConstrained(bool constrained = true)
      { m_lineConstrained = constrained; }

      /// Indicator if this fitter is setup to fit curves through the origin
      void setOriginConstrained(bool constrained = true)
      { m_originConstrained = constrained; }

    protected:
      bool m_lineConstrained; ///< Memory for the flag indicating that lines should be fitted
      bool m_originConstrained; ///< Memory for the flag indicating that curves through the origin shall be fitter

    public:
      /** ROOT Macro to make ExtendedRiemannsMethod a ROOT class.*/
      CDCLOCALTRACKING_SwitchableClassDef(ExtendedRiemannsMethod, 1);

    }; //class
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // EXTENDEDRIEMANNSMETHOD
