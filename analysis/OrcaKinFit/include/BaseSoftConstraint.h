/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Used with permission from ILC                                          *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * Adopted by: Torben Ferber (torben.ferber@desy.de) (TF)                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef __BaseSoftConstraint_H
#define __BaseSoftConstraint_H

#include "analysis/OrcaKinFit/BaseConstraint.h"

namespace Belle2 {

  namespace OrcaKinFit {

    class BaseFitObject;

//  Class BaseSoftConstraint:
/// Abstract base class for soft constraints of kinematic fits
    /**
     * This class defines the minimal functionality any constraint class must provide.
     * First of all a constraint should know on with particles (or FitObject) it is applied.
     * Where as for example a constraint on the total transvese momentum takes into
     * account all particles in the event, an invariant mass constraint usually applies only
     * to a subset of particles.
     *
     * The particle list is implemented as a vector containing pointers to objects derived
     * from BaseFitObject and can be either set a whole (setFOList) or enlarged by adding
     * a single BaseFitObject (addToFOList).
     *
     * From the four--momenta of all concerned fit objects the constraint has to be able
     * to calculate its current value (getValue). Constraints should be formulated such that
     * a value of zero corresponds to a perfectly fulfilled constraint.
     *
     * In order to find a solution to the constrained minimisation problem, fit algorithms
     * usually need the first order derivatives of the constraint with respect to the fit
     * parameters. Since many constraints can be most easily expressed in terms of E, px, py, pz,
     * the constraints supply their derivatives w.r.t. these parameters. If a FitObject uses
     * a different parametrisation, it is its own task to provide the additional derivatives
     * of  E, px, py, pz w.r.t. the parameters of the FitObject. Thus it is easily possible
     * to use FitObjects with different kinds of parametrisations under the same constraint.
     * Some fit algorithms also need the second derivatives of the constraint,
     * i.e. the NewtonFitter.
     *
     * First and second order derivatives of each constraint can be added directly to the
     * global covariance matrix containing the derivatives of all constraints w.r.t. to all
     * parameters (add1stDerivativesToMatrix, add2ndDerivativesToMatrix). This requires the
     * constraint to know its position in the overall list of constraints (globalNum).
     *
     *
     * Author: Jenny List, Benno List
     * Last update: $Date: 2011/03/03 15:03:02 $
     *          by: $Author: blist $
     *
     */

    class BaseSoftConstraint: public BaseConstraint {
    public:

      /// Virtual destructor
      virtual ~BaseSoftConstraint();

      /// Returns the chi2
      virtual double getChi2() const = 0;


      /// Adds second order derivatives to global covariance matrix M
      virtual void add2ndDerivativesToMatrix(double* M,       ///< Global covariance matrix, dimension at least idim x idim
                                             int idim        ///< First dimension of array der
                                            ) const = 0;
      /// Add derivatives of chi squared to global derivative matrix
      virtual void addToGlobalChi2DerVector(double* y,    ///< Vector of chi2 derivatives
                                            int idim     ///< Vector size
                                           ) const = 0;


//    protected:
//      char* name;
    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // __BASECONTRAINT_H
