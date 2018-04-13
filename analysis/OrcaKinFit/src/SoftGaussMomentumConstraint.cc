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
    SoftGaussMomentumConstraint::~SoftGaussMomentumConstraint()
    {
      // B2INFO("destroying SoftGaussMomentumConstraint");
    }

// calulate current value of constraint function
    double SoftGaussMomentumConstraint::getValue() const
    {
      double totpx = 0;
      double totpy = 0;
      double totpz = 0;
      double totE = 0;
      for (unsigned int i = 0; i < fitobjects.size(); i++) {
        if (pxfact != 0) totpx += fitobjects[i]->getPx();
        if (pyfact != 0) totpy += fitobjects[i]->getPy();
        if (pzfact != 0) totpz += fitobjects[i]->getPz();
        if (efact  != 0) totE  += fitobjects[i]->getE();
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
      for (unsigned int i = 0; i < fitobjects.size(); i++) {
        for (int ilocal = 0; ilocal < fitobjects[i]->getNPar(); ilocal++) {
          if (!fitobjects[i]->isParamFixed(ilocal)) {
            int iglobal = fitobjects[i]->getGlobalParNum(ilocal);
            assert(iglobal >= 0 && iglobal < idim);
            double d = 0;
            if (pxfact != 0) d += pxfact * fitobjects[i]->getDPx(ilocal);
            if (pyfact != 0) d += pyfact * fitobjects[i]->getDPy(ilocal);
            if (pzfact != 0) d += pzfact * fitobjects[i]->getDPz(ilocal);
            if (efact  != 0) d +=  efact * fitobjects[i]->getDE(ilocal);
            der[iglobal] = d;
          }
        }
      }
    }


    bool SoftGaussMomentumConstraint::firstDerivatives(int i, double* derivatives) const
    {
      (void) i;
      derivatives[0] = efact;
      derivatives[1] = pxfact;
      derivatives[2] = pyfact;
      derivatives[3] = pzfact;
      return true;
    }

    bool SoftGaussMomentumConstraint::secondDerivatives(int i, int j, double* derivatives) const
    {
      (void) i;
      (void) j;
      (void) derivatives;
      return false;
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace


