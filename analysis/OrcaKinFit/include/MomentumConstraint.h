/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * Adopted by: Torben Ferber (ferber@physics.ubc.ca) (TF)                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef __MOMENTUMCONSTRAINT_H
#define __MOMENTUMCONSTRAINT_H

#include "analysis/OrcaKinFit/ParticleConstraint.h"

namespace Belle2 {

  namespace OrcaKinFit {

    class ParticleFitObject;

//  Class PConstraint:
/// Implements a constraint of the form efact*sum(E)+pxfact*sum(px)+pyfact*sum(py)+pzfact*sum(pz)=value
    /**
     *
     * Author: Jenny List, Benno List
     * Last update: $Date: 2008/02/18 09:59:34 $
     *          by: $Author: blist $
     *
     */
    class MomentumConstraint : public ParticleConstraint {
    public:
      MomentumConstraint(double efact_ = 0,     ///< Factor for energy sum
                         double pxfact_ = 0,   ///< Factor for px sum
                         double pyfact_ = 0,   ///< Factor for py sum
                         double pzfact_ = 0,   ///< Factor for pz sum
                         double value_ = 0     ///< Target value of sum
                        );
      virtual ~MomentumConstraint();
      virtual double getValue() const;
      /// Get first order derivatives.
      /// Call this with a predefined array "der" with the necessary number of entries!
      virtual void getDerivatives(int idim,      ///< First dimension of the array
                                  double der[]   ///< Array of derivatives, at least idim x idim
                                 ) const;

      // not used at all
      //    virtual void addToGlobalDerMatrix (double lambda, int idim, double *M) const;

      virtual void invalidateCache() const;

      virtual int getVarBasis() const;

    protected:
      void updateCache() const;


      double efact;
      double pxfact;
      double pyfact;
      double pzfact;
      double value;

      mutable bool cachevalid;
      mutable int  nparams;

      /// Second derivatives with respect to the 4-vectors of Fit objects i and j; result false if all derivatives are zero
      virtual bool secondDerivatives(int i,                         ///< number of 1st FitObject
                                     int j,                        ///< number of 2nd FitObject
                                     double* derivatives           ///< The result 4x4 matrix
                                    ) const;
      /// First derivatives with respect to the 4-vector of Fit objects i; result false if all derivatives are zero
      virtual bool firstDerivatives(int i,                         ///< number of 1st FitObject
                                    double* derivatives           ///< The result 4-vector
                                   ) const;

      enum { VAR_BASIS = 0 }; // this means that the constraint knows about E,px,py,pz

    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // __MOMENTUMCONSTRAINT_H
