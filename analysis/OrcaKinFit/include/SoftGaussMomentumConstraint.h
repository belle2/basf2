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

#ifndef __SOFTGAUSSMOMENTUMCONSTRAINT_H
#define __SOFTGAUSSMOMENTUMCONSTRAINT_H

#include "analysis/OrcaKinFit/SoftGaussParticleConstraint.h"

namespace Belle2 {

  namespace OrcaKinFit {

    class ParticleFitObject;

//  Class SoftGaussMomentumConstraint:
/// Implements a soft constraint with chi^2=((efact*sum(E_i) + pxfact*sum(p_x,i)+pyfact*sum(p_y,i)+pzfact*sum(p_z,i)-value)/sigma)^2
    /**
     * This class implements soft constraints on the sum of energies, px, py, and pz.
     * It can be used to for constraints of the form
     * - chi^2 = (sum(E)-sqrt(s))^2/sigma^2
     * - chi^2 = (sum(px))^2/sigma^2
     * - chi^2 = (sum(py))^2/sigma^2
     * - chi^2 = (sum(pz))^2/sigma^2
     * - chi^2 = (sum(E)-sum(pz)-2 Ee)^2/sigma^2
     *
     * Author: Jenny List, Benno List
     * Last update: $Date: 2008/02/18 09:59:34 $
     *          by: $Author: blist $
     *
     */

    class SoftGaussMomentumConstraint : public SoftGaussParticleConstraint {
    public:

      /// Constructor
      SoftGaussMomentumConstraint(double sigma_ = 0,      ///< The sigma value
                                  double efact_  = 0,    ///< Factor for energy sum
                                  double pxfact_ = 0,    ///< Factor for px sum
                                  double pyfact_ = 0,    ///< Factor for py sum
                                  double pzfact_ = 0,    ///< Factor for pz sum
                                  double value_ = 0  ///< Target value of sum
                                 );
      /// Virtual destructor
      virtual ~SoftGaussMomentumConstraint();

      /// Returns the value of the constraint function
      virtual double getValue() const;

      /// Get first order derivatives.
      /// Call this with a predefined array "der" with the necessary number of entries!
      virtual void getDerivatives(int idim,      ///< First dimension of the array
                                  double der[]   ///< Array of derivatives, at least idim x idim
                                 ) const;

    protected:


      double efact;
      double pxfact;
      double pyfact;
      double pzfact;
      double value;


      /// Second derivatives with respect to the 4-vectors of Fit objects i and j; result false if all derivatives are zero
      virtual bool secondDerivatives(int i,                         ///< number of 1st FitObject
                                     int j,                        ///< number of 2nd FitObject
                                     double* derivatives           ///< The result 4x4 matrix
                                    ) const;
      /// First derivatives with respect to the 4-vector of Fit objects i; result false if all derivatives are zero
      virtual bool firstDerivatives(int i,                         ///< number of 1st FitObject
                                    double* derivatives           ///< The result 4-vector
                                   ) const;
    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // __SOFTGAUSSMOMENTUMCONSTRAINT_H
