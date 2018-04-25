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

#ifndef __BASECONSTRAINT_H
#define __BASECONSTRAINT_H

#include <iostream>

namespace Belle2 {

  namespace OrcaKinFit {

    class BaseFitObject;

//  Class BaseConstraint:
/// Abstract base class for constraints of kinematic fits
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

    class BaseConstraint {
    public:
      /// Creates an empty BaseConstraint object
      BaseConstraint();

      /// Copy constructor
      BaseConstraint(const BaseConstraint& rhs               ///< right hand side
                    );
      /// Assignment
      BaseConstraint& operator= (const BaseConstraint& rhs   ///< right hand side
                                );

      /// Virtual destructor
      virtual ~BaseConstraint();

      /// Returns the value of the constraint function
      virtual double getValue() const = 0;

      /// Returns the error on the value of the constraint
      virtual double getError() const;

      /// Returns the name of the constraint
      virtual const char* getName() const;
      /// Set object's name
      virtual void setName(const char* name_);

      /// Get first order derivatives of the constraint function
      /// Call this with a predefined array "der" with the necessary number of entries!
      virtual void getDerivatives(int idim,                   ///< First dimension of array der
                                  double der[]               ///< Array of derivatives, dimension at least idim x idim
                                 ) const = 0;

      /// print object to ostream
      virtual std::ostream&  print(std::ostream& os        ///< The output stream
                                  ) const;

    protected:
      char* name;
    };

    /** \relates BaseConstraint
     *  \brief Prints out a BaseConstraint, using its print method
     */
    inline std::ostream& operator<< (std::ostream& os,         ///< The output stream
                                     const BaseConstraint& bc  ///< The object to print
                                    )
    {
      return bc.print(os);
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace


#endif // __BASECONSTRAINT_H
