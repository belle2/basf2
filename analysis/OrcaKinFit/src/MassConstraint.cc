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

#include "analysis/OrcaKinFit/MassConstraint.h"
#include "analysis/OrcaKinFit/ParticleFitObject.h"
#include <framework/logging/Logger.h>

#include<iostream>
#include<cmath>

#undef NDEBUG
#include<cassert>

namespace Belle2 {

  namespace OrcaKinFit {


// constructor
    MassConstraint::MassConstraint(double mass_)
      : mass(mass_)
    {}

// destructor
    MassConstraint::~MassConstraint()
    {}

// calulate current value of constraint function
    double MassConstraint::getValue() const
    {
      double totE[2] = {0, 0};
      double totpx[2] = {0, 0};
      double totpy[2] = {0, 0};
      double totpz[2] = {0, 0};
      for (unsigned int i = 0; i < fitobjects.size(); i++) {
        int index = (flags[i] == 1) ? 0 : 1; // default is 1, but 2 may indicate fitobjects for a second W -> equal mass constraint!
        ParticleFitObject* pfo = dynamic_cast < ParticleFitObject* >(fitobjects[i]);
        assert(pfo);
        totE[index]  += pfo->getE();
        totpx[index] += pfo->getPx();
        totpy[index] += pfo->getPy();
        totpz[index] += pfo->getPz();
      }
      double result = -mass;
      result += std::sqrt(std::abs(totE[0] * totE[0] - totpx[0] * totpx[0] - totpy[0] * totpy[0] - totpz[0] * totpz[0]));
      result -= std::sqrt(std::abs(totE[1] * totE[1] - totpx[1] * totpx[1] - totpy[1] * totpy[1] - totpz[1] * totpz[1]));
      return result;
    }

// calculate vector/array of derivatives of this contraint
// w.r.t. to ALL parameters of all fitobjects
// here: d M /d par(j)
//          = d M /d p(i) * d p(i) /d par(j)
//          =  +-1/M * p(i) * d p(i) /d par(j)
    void MassConstraint::getDerivatives(int idim, double der[]) const
    {
      double totE[2] = {0, 0};
      double totpx[2] = {0, 0};
      double totpy[2] = {0, 0};
      double totpz[2] = {0, 0};
      bool valid[2] = {false, false};
      for (unsigned int i = 0; i < fitobjects.size(); i++) {
        int index = (flags[i] == 1) ? 0 : 1; // default is 1, but 2 may indicate fitobjects for a second W -> equal mass constraint!
        valid[index] = true;
        ParticleFitObject* pfo = dynamic_cast < ParticleFitObject* >(fitobjects[i]);
        assert(pfo);
        totE[index]  += pfo->getE();
        totpx[index] += pfo->getPx();
        totpy[index] += pfo->getPy();
        totpz[index] += pfo->getPz();
      }
      double m2[2];
      double m_inv[2] = {0, 0};
      for (int index = 0; index < 2; ++index) {
        m2[index] = totE[index] * totE[index] - totpx[index] * totpx[index]
                    - totpy[index] * totpy[index] - totpz[index] * totpz[index];
        if (m2[index] < 0 && m2[index] > -1E-9) m2[index] = 0;
        if (m2[index] < 0 && valid[index]) {
          B2ERROR("MassConstraint::getDerivatives: m2<0!");
          for (unsigned int j = 0; j < fitobjects.size(); j++) {
            int jndex = (flags[j] == 1) ? 0 : 1;
            if (jndex == index) {

              ParticleFitObject* pfo = dynamic_cast < ParticleFitObject* >(fitobjects[j]);
              assert(pfo);
              B2ERROR(pfo->getName() <<
                      ": E =" << pfo->getE() <<
                      ", px=" << pfo->getPx() <<
                      ", py=" << pfo->getPy() <<
                      ", pz=" << pfo->getPz());
            }
          }
          B2ERROR("sum: E=" << totE[index] << ", px=" << totpx[index]
                  << ", py=" << totpy[index] << ", pz=" << totpz[index] << ", m2=" << m2[index]);
        }
        if (m2[index] != 0) m_inv[index] = 1 / std::sqrt(std::abs(m2[index]));
      }

      for (unsigned int i = 0; i < fitobjects.size(); i++) {
        int index = (flags[i] == 1) ? 0 : 1; // default is 1, but 2 may indicate fitobjects for a second W -> equal mass constraint!
        for (int ilocal = 0; ilocal < fitobjects[i]->getNPar(); ilocal++) {
          if (!fitobjects[i]->isParamFixed(ilocal)) {
            int iglobal = fitobjects[i]->getGlobalParNum(ilocal);
            assert(iglobal >= 0 && iglobal < idim);
            if (m2[index] != 0) {

              ParticleFitObject* pfo = dynamic_cast < ParticleFitObject* >(fitobjects[i]);
              assert(pfo);

              der[iglobal] =   totE[index]  * pfo->getDE(ilocal)
                               - totpx[index] * pfo->getDPx(ilocal)
                               - totpy[index] * pfo->getDPy(ilocal)
                               - totpz[index] * pfo->getDPz(ilocal);
              der[iglobal] *= m_inv[index];
            } else der[iglobal] = 1;
            if (index == 1) der[iglobal] *= -1.;
          }
        }
      }
    }

    double MassConstraint::getMass(int flag)
    {
      double totE = 0;
      double totpx = 0;
      double totpy = 0;
      double totpz = 0;
      for (unsigned int i = 0; i < fitobjects.size(); i++) {
        if (flags[i] == flag) {

          const ParticleFitObject* fok = dynamic_cast < ParticleFitObject* >(fitobjects[i]);
          assert(fok);
          totE  += fok->getE();
          totpx += fok->getPx();
          totpy += fok->getPy();
          totpz += fok->getPz();
        }
      }
      return std::sqrt(std::abs(totE * totE - totpx * totpx - totpy * totpy - totpz * totpz));
    }

    void MassConstraint::setMass(double mass_)
    {
      mass = mass_;
    }

    bool MassConstraint::secondDerivatives(int i, int j, double* dderivatives) const
    {
      B2DEBUG(14, "MassConstraint::secondDerivatives: i=" << i << ", j=" << j);
      int index = (flags[i] == 1) ? 0 : 1; // default is 1, but 2 may indicate fitobjects for a second W -> equal mass constraint!
      int jndex = (flags[j] == 1) ? 0 : 1; // default is 1, but 2 may indicate fitobjects for a second W -> equal mass constraint!
      if (index != jndex) return false;
      double totE = 0;
      double totpx = 0;
      double totpy = 0;
      double totpz = 0;
      for (unsigned int k = 0; k < fitobjects.size(); ++k) {
        int kndex = (flags[k] == 1) ? 0 : 1; // default is 1, but 2 may indicate fitobjects for a second W -> equal mass constraint!
        const ParticleFitObject* fok = dynamic_cast < ParticleFitObject* >(fitobjects[k]);
        assert(fok);
        if (index == kndex) {
          totE  += fok->getE();
          totpx += fok->getPx();
          totpy += fok->getPy();
          totpz += fok->getPz();
        }
      }

      if (totE <= 0) {
        B2ERROR("MassConstraint::secondDerivatives: totE = " << totE);
      }

      double m2 = std::abs(totE * totE - totpx * totpx - totpy * totpy - totpz * totpz);
      double m = std::sqrt(m2);
      if (index) m = -m;
      double minv3 = 1 / (m * m * m);

      assert(dderivatives);
      for (int k = 0; k < 16; ++k) dderivatives[k] = 0;
      dderivatives[4 * 0 + 0] = (m2 - totE * totE) * minv3;
      dderivatives[4 * 0 + 1] = dderivatives[4 * 1 + 0] =     totE * totpx * minv3;
      dderivatives[4 * 0 + 2] = dderivatives[4 * 2 + 0] =     totE * totpy * minv3;
      dderivatives[4 * 0 + 3] = dderivatives[4 * 3 + 0] =     totE * totpz * minv3;
      dderivatives[4 * 1 + 1] =                     -(m2 + totpx * totpx) * minv3;
      dderivatives[4 * 1 + 2] = dderivatives[4 * 2 + 1] =    -totpx * totpy * minv3;
      dderivatives[4 * 1 + 3] = dderivatives[4 * 3 + 1] =    -totpx * totpz * minv3;
      dderivatives[4 * 2 + 2] =                     -(m2 + totpy * totpy) * minv3;
      dderivatives[4 * 2 + 3] = dderivatives[4 * 3 + 2] =    -totpy * totpz * minv3;
      dderivatives[4 * 3 + 3] =                     -(m2 + totpz * totpz) * minv3;
      return true;
    }

    bool MassConstraint::firstDerivatives(int i, double* dderivatives) const
    {
      double totE = 0;
      double totpx = 0;
      double totpy = 0;
      double totpz = 0;
      int index = (flags[i] == 1) ? 0 : 1; // default is 1, but 2 may indicate fitobjects for a second W -> equal mass constraint!
      for (unsigned int j = 0; j < fitobjects.size(); ++j) {
        int jndex = (flags[j] == 1) ? 0 : 1; // default is 1, but 2 may indicate fitobjects for a second W -> equal mass constraint!
        const ParticleFitObject* foj =  dynamic_cast < ParticleFitObject* >(fitobjects[j]);
        assert(foj);
        if (index == jndex) {
          totE  += foj->getE();
          totpx += foj->getPx();
          totpy += foj->getPy();
          totpz += foj->getPz();
        }
      }

      if (totE <= 0) {
        B2ERROR("MassConstraint::firstDerivatives: totE = " << totE);
      }

      double m = std::sqrt(std::abs(totE * totE - totpx * totpx - totpy * totpy - totpz * totpz));
      if (index) m = -m;

      dderivatives[0] = totE / m;
      dderivatives[1] = -totpx / m;
      dderivatives[2] = -totpy / m;
      dderivatives[3] = -totpz / m;
      return true;
    }

    int MassConstraint::getVarBasis() const
    {
      return VAR_BASIS;
    }
  }// end OrcaKinFit namespace
} // end Belle2 namespace

