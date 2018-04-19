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

#ifndef __SOFTGAUSSPARTICLECONSTRAINT_H
#define __SOFTGAUSSPARTICLECONSTRAINT_H

#include<vector>
#include<cassert>
#include "analysis/OrcaKinFit/BaseSoftConstraint.h"
#include "analysis/OrcaKinFit/BaseFitObject.h"

namespace Belle2 {

  namespace OrcaKinFit {

    class ParticleFitObject;

//  Class SoftGaussParticleConstraint:
/// Abstract base class for constraints of kinematic fits
    /**
     * This class defines the minimal functionality any constraint class must provide.
     * First of all a constraint should know on with particles (or FitObject) it is applied.
     * Where as for example a constraint on the total transvese momentum takes into
     * account all particles in the event, an invariant mass constraint usually applies only
     * to a subset of particles.
     *
     * The particle list is implemented as a vector containing pointers to objects derived
     * from ParticleFitObject and can be either set a whole (setFOList) or enlarged by adding
     * a single ParticleFitObject (addToFOList).
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
     * $Date: 2008/02/13 12:37:38 $
     * $Author: blist $
     *
     */

    class SoftGaussParticleConstraint: public BaseSoftConstraint {
    public:
      /// Creates an empty SoftGaussParticleConstraint object
      SoftGaussParticleConstraint(double sigma_     ///< The sigma value
                                 );
      /// Virtual destructor
      virtual ~SoftGaussParticleConstraint() {};

      /// Adds several ParticleFitObject objects to the list
      virtual void setFOList(std::vector <ParticleFitObject*>* fitobjects_ ///< A list of BaseFitObject objects
                            )
      {
        for (int i = 0; i < (int) fitobjects_->size(); i++) {
          fitobjects.push_back((*fitobjects_)[i]);
          flags.push_back(1);
        }
      };
      /// Adds one ParticleFitObject objects to the list
      virtual void addToFOList(ParticleFitObject& fitobject, int flag = 1
                              )
      {
        fitobjects.push_back(&fitobject);
        flags.push_back(flag);
      };
      /// Resests ParticleFitObject list
      virtual void resetFOList()
      {
        fitobjects.resize(0);
        flags.resize(0);
      };

      /// Returns the value of the constraint function
      virtual double getValue() const = 0;

      /// Returns the chi2
      virtual double getChi2() const;

      /// Returns the error on the value of the constraint
      virtual double getError() const;

      /// Returns the sigma
      virtual double getSigma() const;

      /// Sets the sigma
      virtual double setSigma(double sigma_     ///< The new sigma value
                             );

      /// Get first order derivatives.
      /// Call this with a predefined array "der" with the necessary number of entries!
      virtual void getDerivatives(int idim,      ///< First dimension of the array
                                  double der[]   ///< Array of derivatives, at least idim x idim
                                 ) const = 0;
      /// Adds second order derivatives to global covariance matrix M
      virtual void add2ndDerivativesToMatrix(double* M,     ///< Covariance matrix, at least idim x idim
                                             int idim       ///< First dimension of the array
                                            ) const;

      /// Add derivatives of chi squared to global derivative matrix
      virtual void addToGlobalChi2DerVector(double* y,    ///< Vector of chi2 derivatives
                                            int idim     ///< Vector size
                                           ) const;


      /// Invalidates any cached values for the next event
      virtual void invalidateCache() const
      {}

      void test1stDerivatives();
      void test2ndDerivatives();

      /// Evaluates numerically the 1st derivative w.r.t. a parameter
      double num1stDerivative(int ifo,      ///< Number of  FitObject
                              int ilocal,  ///< Local parameter number
                              double eps   ///< variation of  local parameter
                             );
      /// Evaluates numerically the 2nd derivative w.r.t. 2 parameters
      double num2ndDerivative(int ifo1,     ///< Number of 1st FitObject
                              int ilocal1, ///< 1st local parameter number
                              double eps1, ///< variation of 1st local parameter
                              int ifo2,    ///< Number of 1st FitObject
                              int ilocal2, ///< 1st local parameter number
                              double eps2  ///< variation of 2nd local parameter
                             );

      int getVarBasis() const;

    protected:

      /// Second derivatives with respect to the 4-vectors of Fit objects i and j; result false if all derivatives are zero
      virtual bool secondDerivatives(int i,                         ///< number of 1st FitObject
                                     int j,                        ///< number of 2nd FitObject
                                     double* derivatives           ///< The result 4x4 matrix
                                    ) const = 0;
      /// First derivatives with respect to the 4-vector of Fit objects i; result false if all derivatives are zero
      virtual bool firstDerivatives(int i,                         ///< number of 1st FitObject
                                    double* derivatives           ///< The result 4-vector
                                   ) const = 0;


      /// Vector of pointers to ParticleFitObjects
      typedef std::vector <ParticleFitObject*> FitObjectContainer;
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

      /// The sigma of the Gaussian
      double sigma;

      enum { VAR_BASIS = BaseDefs::VARBASIS_EPXYZ }; // this means that the constraint knows about E,px,py,pz

    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // __SOFTGAUSSPARTICLECONSTRAINT_H
