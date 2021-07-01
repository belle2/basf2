/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef __MASSCONSTRAINT_H
#define __MASSCONSTRAINT_H

#include "analysis/OrcaKinFit/ParticleConstraint.h"

namespace Belle2 {

  namespace OrcaKinFit {

    class ParticleFitObject;

//  Class MassConstraint:
/// Implements constraint 0 = mass1 - mass2 - m
    /**
     * This class implements different mass constraints:
     * - the invariant mass of several objects should be m
     * - the difference of the invariant masses between two
     *   sets of objects should be m (normally m=0 in this case).
     *
     * Author: Jenny List, Benno List
     * Last update: $Date: 2008/02/12 11:03:32 $
     *          by: $Author: blist $
     *
     */

    class MassConstraint : public ParticleConstraint {
    public:

      /// Constructor
      explicit MassConstraint(double mass_ = 0.    ///< The mass difference between object sets 1 and 2
                             );
      /// Virtual destructor
      virtual ~MassConstraint();

      /// Returns the value of the constraint
      virtual double getValue() const override;

      /// Get first order derivatives.
      /// Call this with a predefined array "der" with the necessary number of entries!
      virtual void getDerivatives(int idim,      ///< First dimension of the array
                                  double der[]   ///< Array of derivatives, at least idim x idim
                                 ) const override;

      /// Get the actual invariant mass of the fit objects with a given flag
      virtual double getMass(int flag = 1        ///< The flag
                            );

      /// Sets the target mass of the constraint
      virtual void setMass(double mass_            ///< The new mass
                          );

      virtual int getVarBasis() const override;

    protected:

      double mass;   ///< The mass difference between object sets 1 and 2

      /// Second derivatives with respect to the 4-vectors of Fit objects i and j; result false if all derivatives are zero
      virtual bool secondDerivatives(int i,                         ///< number of 1st FitObject
                                     int j,                        ///< number of 2nd FitObject
                                     double* derivatives           ///< The result 4x4 matrix
                                    ) const override;

      /// First derivatives with respect to the 4-vector of Fit objects i; result false if all derivatives are zero
      virtual bool firstDerivatives(int i,                         ///< number of 1st FitObject
                                    double* derivatives           ///< The result 4-vector
                                   ) const override;

      enum { VAR_BASIS = 0 }; // this means that the constraint knows about E,px,py,pz

    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace


#endif // __MASSCONSTRAINT_H
