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

#ifndef __BASEHARDCONSTRAINT_H
#define __BASEHARDCONSTRAINT_H

#include "analysis/OrcaKinFit/BaseDefs.h"
#include "analysis/OrcaKinFit/BaseConstraint.h"
#include "analysis/OrcaKinFit/BaseFitObject.h"

#include <vector>

namespace Belle2 {

  namespace OrcaKinFit {

    class BaseFitObject;

//  Class BasehardConstraint:
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

    class BaseHardConstraint: public BaseConstraint {
    public:

      inline BaseHardConstraint();

      /// Virtual destructor
      virtual ~BaseHardConstraint();

      /// Adds first order derivatives to global covariance matrix M
      virtual void add1stDerivativesToMatrix(double* M,       ///< Global covariance matrix, dimension at least idim x idim
                                             int idim        ///< First dimension of array der
                                            ) const;

      /// Adds second order derivatives to global covariance matrix M
      virtual void add2ndDerivativesToMatrix(double* M,       ///< Global covariance matrix, dimension at least idim x idim
                                             int idim,       ///< First dimension of array der
                                             double lambda   ///< Lagrange multiplier for this constraint
                                            ) const;
      /// Add lambda times derivatives of chi squared to global derivative vector
      virtual void addToGlobalChi2DerVector(double* y,    ///< Vector of chi2 derivatives
                                            int idim,    ///< Vector size
                                            double lambda //< The lambda value
                                           ) const;
      /// Calculate directional derivative
      virtual double dirDer(double* p,                    ///< Vector of direction
                            double* w,   ///< Work vector
                            int idim,    ///< Vector size
                            double mu = 1 ///< optional multiplier
                           );

      /// Calculate directional derivative for abs(c)
      virtual double dirDerAbs(double* p,                 ///< Vector of direction
                               double* w,   ///< Work vector
                               int idim,    ///< Vector size
                               double mu = 1 ///< optional multiplier
                              );

      /// Second derivatives with respect to the meta-variables of Fit objects i and j; result false if all derivatives are zero
      virtual bool secondDerivatives(int i,                         ///< number of 1st FitObject
                                     int j,                        ///< number of 2nd FitObject
                                     double* derivatives           ///< The result 4x4 matrix
                                    ) const = 0;
      /// First derivatives with respect to the meta-variables of Fit objects i; result false if all derivatives are zero
      virtual bool firstDerivatives(int i,                         ///< number of 1st FitObject
                                    double* derivatives           ///< The result 4-vector
                                   ) const = 0;

      virtual int getVarBasis() const = 0;

      /// Returns the value of the constraint
      virtual double getValue() const = 0;

      /// Returns the error on the value of the constraint
      virtual double getError() const;

      /// Get first order derivatives.
      /// Call this with a predefined array "der" with the necessary number of entries!
      virtual void getDerivatives(int idim,      ///< First dimension of the array
                                  double der[]   ///< Array of derivatives, at least idim x idim
                                 ) const = 0;

      /// Accesses position of constraint in global constraint list
      virtual int  getGlobalNum() const
      {return globalNum;}
      /// Sets position of constraint in global constraint list
      virtual void setGlobalNum(int iglobal                 ///< Global constraint number
                               )
      {globalNum = iglobal;}

      virtual void printFirstDerivatives() const;
      virtual void printSecondDerivatives() const;

      virtual void test1stDerivatives();
      virtual void test2ndDerivatives();

      /// Evaluates numerically the 1st derivative w.r.t. a parameter
      virtual double num1stDerivative(int ifo,      ///< Number of  FitObject
                                      int ilocal,  ///< Local parameter number
                                      double eps   ///< variation of  local parameter
                                     );
      /// Evaluates numerically the 2nd derivative w.r.t. 2 parameters
      virtual double num2ndDerivative(int ifo1,     ///< Number of 1st FitObject
                                      int ilocal1, ///< 1st local parameter number
                                      double eps1, ///< variation of 1st local parameter
                                      int ifo2,    ///< Number of 1st FitObject
                                      int ilocal2, ///< 1st local parameter number
                                      double eps2  ///< variation of 2nd local parameter
                                     );





    protected:

      /// Vector of pointers to ParticleFitObjects
      typedef std::vector <BaseFitObject*> FitObjectContainer;
      /// Iterator through vector of pointers to ParticleFitObjects
      typedef FitObjectContainer::iterator FitObjectIterator;
      /// Constant iterator through vector of pointers to ParticleFitObjects
      typedef FitObjectContainer::const_iterator ConstFitObjectIterator;
      ///  The FitObjectContainer
      FitObjectContainer fitobjects;
      ///  The derivatives
      std::vector <double> derivatives;
      ///  The flags can be used to divide the FitObjectContainer into several subsets
      ///  used for example to implement an equal mass constraint (see MassConstraint).
      std::vector <int> flags;

      /// Position of constraint in global constraint list
      int globalNum;

    };

    BaseHardConstraint::BaseHardConstraint()
      : fitobjects(FitObjectContainer()), derivatives(std::vector <double> ()), flags(std::vector <int> ()), globalNum(0)
    {
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // __BASEHARDCONSTRAINT_H
