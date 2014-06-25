/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef PORTEDKARIMAKISMETHOD_H
#define PORTEDKARIMAKISMETHOD_H

#include "TMatrixDSym.h"
#include "TVectorD.h"


#include <tracking/cdcLocalTracking/fitting/CDCObservations2D.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class implementing the Riemann fit for two dimensional trajectory circle
    class PortedKarimakisMethod : public CDCLocalTracking::UsedTObject {

    public:
      ///Empty constructor
      PortedKarimakisMethod();

      ///Empty destructor
      ~PortedKarimakisMethod();

      /// Executes the fit and updates the trajectory parameters. This may render the information in the observation object.
      void update(CDCTrajectory2D& fit, CDCObservations2D& observations2D) const;


    private:
      int fit(CDCObservations2D& , double&) const;

    private:
      const bool _curved;
      const int _npar;
      mutable int _numPoints;


      mutable TVectorD _parameters;
      mutable TMatrixDSym _covariance;


    public:
      /** ROOT Macro to make PortedKarimakisMethod a ROOT class.*/
      ClassDefInCDCLocalTracking(PortedKarimakisMethod, 1);


    }; //class
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // PORTEDKARIMAKISMETHOD
