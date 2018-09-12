/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * RecoilMassConstraint for OrcaKinFit. Allows constraints of the type    *
 * mass(P_beam - P) - RecoilMass = 0s                                      *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (torben.ferber@desy.de) (TF)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef __RECOILMASSCONSTRAINT_H
#define __RECOILMASSCONSTRAINT_H

#include "analysis/OrcaKinFit/ParticleConstraint.h"

namespace Belle2 {
  namespace OrcaKinFit {

    class ParticleFitObject;

    class RecoilMassConstraint : public ParticleConstraint {
    public:

      /// Constructor
      RecoilMassConstraint(double recoilmass = 0., double beampx = 0., double beampy = 0., double beampz = 0, double beampe = 0.);

      /// Virtual destructor
      virtual ~RecoilMassConstraint();

      /// Returns the value of the constraint
      virtual double getValue() const;

      /// Get first order derivatives.
      /// Call this with a predefined array "der" with the necessary number of entries!
      virtual void getDerivatives(int idim,      ///< First dimension of the array
                                  double der[]   ///< Array of derivatives, at least idim x idim
                                 ) const;

      /// Get the actual recoil mass of the fit objects
      virtual double getRecoilMass();

      /// Sets the target recoil mass of the constraint
      virtual void setRecoilMass(double recoilmass);

      virtual int getVarBasis() const;

    protected:

      double m_recoilMass;
      double m_beamPx;
      double m_beamPy;
      double m_beamPz;
      double m_beamE;

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

#endif // __RECOILMASSCONSTRAINT_H
