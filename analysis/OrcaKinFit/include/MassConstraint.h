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
      MassConstraint(double mass_ = 0.    ///< The mass difference between object sets 1 and 2
                    );
      /// Virtual destructor
      virtual ~MassConstraint();

      /// Returns the value of the constraint
      virtual double getValue() const;

      /// Get first order derivatives.
      /// Call this with a predefined array "der" with the necessary number of entries!
      virtual void getDerivatives(int idim,      ///< First dimension of the array
                                  double der[]   ///< Array of derivatives, at least idim x idim
                                 ) const;

      /// Get the actual invariant mass of the fit objects with a given flag
      virtual double getMass(int flag = 1        ///< The flag
                            );

      /// Sets the target mass of the constraint
      virtual void setMass(double mass_            ///< The new mass
                          );

      virtual int getVarBasis() const;

    protected:

      double mass;   ///< The mass difference between object sets 1 and 2

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


#endif // __MASSCONSTRAINT_H
