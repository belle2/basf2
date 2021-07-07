/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * Forked from https://github.com/iLCSoft/MarlinKinfit                    *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "analysis/OrcaKinFit/SoftGaussMomentumConstraint.h"
#include "analysis/OrcaKinFit/ParticleFitObject.h"

#include<iostream>
#include<cmath>
#undef NDEBUG
#include<cassert>

namespace Belle2 {
  namespace OrcaKinFit {

// constructor
    SoftGaussMomentumConstraint::SoftGaussMomentumConstraint(double sigma_, double efact_, double pxfact_,
                                                             double pyfact_, double pzfact_, double value_)
      : SoftGaussParticleConstraint(sigma_),
        efact(efact_),
        pxfact(pxfact_),
        pyfact(pyfact_),
        pzfact(pzfact_),
        value(value_)
    {}

// destructor
    SoftGaussMomentumConstraint::~SoftGaussMomentumConstraint() = default;

// calulate current value of constraint function
    double SoftGaussMomentumConstraint::getValue() const
    {
      double totpx = 0;
      double totpy = 0;
      double totpz = 0;
      double totE = 0;
      for (auto fitobject : fitobjects) {
        if (pxfact != 0) totpx += fitobject->getPx();
        if (pyfact != 0) totpy += fitobject->getPy();
        if (pzfact != 0) totpz += fitobject->getPz();
        if (efact  != 0) totE  += fitobject->getE();
      }
      return pxfact * totpx + pyfact * totpy + pzfact * totpz + efact * totE - value;
    }

// calculate vector/array of derivatives of this contraint
// w.r.t. to ALL parameters of all fitobjects
// here: d M /d par(j)
//          = d M /d p(i) * d p(i) /d par(j)
//          =  +-1/M * p(i) * d p(i) /d par(j)
    void SoftGaussMomentumConstraint::getDerivatives(int idim, double der[]) const
    {
      for (auto fitobject : fitobjects) {
        for (int ilocal = 0; ilocal < fitobject->getNPar(); ilocal++) {
          if (!fitobject->isParamFixed(ilocal)) {
            int iglobal = fitobject->getGlobalParNum(ilocal);
            assert(iglobal >= 0 && iglobal < idim);
            double d = 0;
            if (pxfact != 0) d += pxfact * fitobject->getDPx(ilocal);
            if (pyfact != 0) d += pyfact * fitobject->getDPy(ilocal);
            if (pzfact != 0) d += pzfact * fitobject->getDPz(ilocal);
            if (efact  != 0) d +=  efact * fitobject->getDE(ilocal);
            der[iglobal] = d;
          }
        }
      }
    }


    bool SoftGaussMomentumConstraint::firstDerivatives(int i, double* derivativesf) const
    {
      (void) i;
      derivativesf[0] = efact;
      derivativesf[1] = pxfact;
      derivativesf[2] = pyfact;
      derivativesf[3] = pzfact;
      return true;
    }

    bool SoftGaussMomentumConstraint::secondDerivatives(int i, int j, double* derivativess) const
    {
      (void) i;
      (void) j;
      (void) derivativess;
      return false;
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace
