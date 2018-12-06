/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * See https://github.com/tferber/OrcaKinfit, forked from                 *
 * https://github.com/iLCSoft/MarlinKinfit                                *
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

#include "analysis/OrcaKinFit/MomentumConstraint.h"
#include "analysis/OrcaKinFit/ParticleFitObject.h"
#include <framework/logging/Logger.h>

#include<iostream>

#undef NDEBUG
#include<cassert>


namespace Belle2 {

  namespace OrcaKinFit {

    MomentumConstraint::MomentumConstraint(double efact_, double pxfact_, double pyfact_,
                                           double pzfact_, double value_)
      : efact(efact_),
        pxfact(pxfact_),
        pyfact(pyfact_),
        pzfact(pzfact_),
        value(value_),
        cachevalid(false),
        nparams(0)
    {}

// destructor
    MomentumConstraint::~MomentumConstraint()
    {}

// calculate current value of constraint function
    double MomentumConstraint::getValue() const
    {
      double totpx = 0;
      double totpy = 0;
      double totpz = 0;
      double totE = 0;
      for (unsigned int i = 0; i < fitobjects.size(); i++) {

        const ParticleFitObject* foi =  dynamic_cast < ParticleFitObject* >(fitobjects[i]);
        assert(foi);

        if (pxfact != 0) totpx += foi->getPx();
        if (pyfact != 0) totpy += foi->getPy();
        if (pzfact != 0) totpz += foi->getPz();
        if (efact  != 0) totE  += foi->getE();
      }
      return pxfact * totpx + pyfact * totpy + pzfact * totpz + efact * totE - value;
    }

// calculate vector/array of derivatives of this contraint
// w.r.t. to ALL parameters of all fitobjects
// here: d sum(px) /d par(i,j)
//                      = d sum(px) /d px(i) * d px(i) /d par(i, j)
//                                      =  1 * d px(i) /d par(i, j)
    void MomentumConstraint::getDerivatives(int idim, double der[]) const
    {
      for (unsigned int i = 0; i < fitobjects.size(); i++) {
        for (int ilocal = 0; ilocal < fitobjects[i]->getNPar(); ilocal++) {
          if (!fitobjects[i]->isParamFixed(ilocal)) {
            int iglobal = fitobjects[i]->getGlobalParNum(ilocal);
            assert(iglobal >= 0 && iglobal < idim);
            double d = 0;
            const ParticleFitObject* foi =  dynamic_cast < ParticleFitObject* >(fitobjects[i]);
            assert(foi);
            if (pxfact != 0) d += pxfact * foi->getDPx(ilocal);
            if (pyfact != 0) d += pyfact * foi->getDPy(ilocal);
            if (pzfact != 0) d += pzfact * foi->getDPz(ilocal);
            if (efact  != 0) d +=  efact * foi->getDE(ilocal);
            der[iglobal] = d;
          }
        }
      }
    }

    void MomentumConstraint::invalidateCache() const
    {
      cachevalid = false;
    }

    void MomentumConstraint::updateCache() const
    {
      nparams = 0;
      for (unsigned int i = 0; i < fitobjects.size(); i++) {
        for (int ilocal = 0; ilocal < fitobjects[i]->getNPar(); ilocal++) {
          int iglobal = fitobjects[i]->getGlobalParNum(ilocal);
          if (!fitobjects[i]->isParamFixed(ilocal)) {
            assert(iglobal >= 0);
            nparams++;
          }
        }
      }
      cachevalid = true;
    }

    bool MomentumConstraint::secondDerivatives(int i, int j, double* dderivatives) const
    {
      (void) i;
      (void) j;
      (void) dderivatives;
      return false;
    }     //fix the warning

    bool MomentumConstraint::firstDerivatives(int i, double* dderivatives) const
    {
      (void) i;
      dderivatives[0] = efact;
      dderivatives[1] = pxfact;
      dderivatives[2] = pyfact;
      dderivatives[3] = pzfact;
      return true;
    }

    int MomentumConstraint::getVarBasis() const
    {
      return VAR_BASIS;
    }

  }// end OrcaKinFit namespace

} // end Belle2 namespace

