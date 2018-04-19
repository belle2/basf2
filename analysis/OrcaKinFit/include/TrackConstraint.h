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

#ifndef __TrackConstraint_H
#define __TrackConstraint_H

#include<vector>
#include<cassert>
#include "analysis/OrcaKinFit/BaseConstraint.h"

namespace Belle2 {

  namespace OrcaKinFit {

    class TrackFitObject;

//  Class TrackConstraint:
/// Abstract base class for constraints of kinematic fits
    /**
     * This class defines the minimal functionality any constraint class must provide.
     * First of all a constraint should know on with particles (or FitObject) it is applied.
     * Where as for example a constraint on the total transvese momentum takes into
     * account all particles in the event, an invariant mass constraint usually applies only
     * to a subset of particles.
     *
     * The particle list is implemented as a vector containing pointers to objects derived
     * from TrackFitObject and can be either set a whole (setFOList) or enlarged by adding
     * a single TrackFitObject (addToFOList).
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
     * Author: Benno List, Jenny List
     * $Date: 2008/01/30 09:14:54 $
     * $Author: blist $
     *
     */

    class TrackConstraint: public BaseConstraint {
    public:
      /// Creates an empty TrackConstraint object
      inline TrackConstraint();
      /// Virtual destructor
      virtual ~TrackConstraint() {};

      /// Adds several TrackFitObject objects to the list
      virtual void setFOList(std::vector <TrackFitObject*>* fitobjects_ ///< A list of BaseFitObject objects
                            )
      {
        for (int i = 0; i < (int) fitobjects_->size(); i++) {
          fitobjects.push_back((*fitobjects_)[i]);
          flags.push_back(1);
          sign.push_back(i == 0 ? 1 : -1);
        }
      };
      /// Adds one TrackFitObject objects to the list
      virtual void addToFOList(TrackFitObject& fitobject, int flag = 1
                              )
      {
        fitobjects.push_back(&fitobject);
        flags.push_back(flag);
        sign.push_back(sign.size() == 0 ? 1 : -1);
      };
      /// Returns the value of the constraint
      virtual double getValue() const = 0;

      /// Get first order derivatives.
      /// Call this with a predefined array "der" with the necessary number of entries!
      virtual void getDerivatives(int idim, double der[]) const = 0;
      /// Adds first order derivatives to global covariance matrix M
      virtual void add1stDerivativesToMatrix(double* M,      ///< Covariance matrix, at least idim x idim
                                             int idim        ///< First dimension of the array
                                            ) const
      {assert(false);}
      /// Adds second order derivatives to global covariance matrix M
      virtual void add2ndDerivativesToMatrix(double* M,       ///< Global covariance matrix, dimension at least idim x idim
                                             int idim,       ///< First dimension of array der
                                             double lambda   ///< Lagrange multiplier for this constraint
                                            ) const
      {assert(false);}

      /// Add lambda times derivatives of chi squared to global derivative matrix
      virtual void addToGlobalChi2DerVector(double* y,    ///< Vector of chi2 derivatives
                                            int idim,    ///< Vector size
                                            double lambda //< The lambda value
                                           ) const
      {assert(false);};

      /// Accesses position of constraint in global constraint list
      virtual int  getGlobalNum() const {return globalNum;}
      /// Sets position of constraint in global constraint list
      virtual void setGlobalNum(int iglobal) {globalNum = iglobal;}

      /// Invalidates any cached values for the next event
      virtual void invalidateCache() const {};

    protected:
      /// Vector of pointers to TrackFitObjects
      typedef std::vector <TrackFitObject*> FitObjectContainer;
      /// Iterator through vector of pointers to TrackFitObjects
      typedef FitObjectContainer::iterator FitObjectIterator;
      /// Constant iterator through vector of pointers to TrackFitObjects
      typedef FitObjectContainer::const_iterator ConstFitObjectIterator;
      ///  The FitObjectContainer
      FitObjectContainer fitobjects;
      ///  The derivatives
      std::vector <double> derivatives;
      ///  The flags can be used to divide the FitObjectContainer into several subsets
      ///  used for example to implement an equal mass constraint (see MassConstraint).
      std::vector <int> flags;
      std::vector <double> sign;

      /// Position of constraint in global constraint list
      int globalNum;

    };

    TrackConstraint::TrackConstraint()
    {
      invalidateCache();
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // __TrackConstraint_H
