/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/reconstruction_cpp/ModuleAlignment.h>
#include <TVector3.h>
#include <TRotation.h>
#include <TDecompChol.h>

namespace Belle2 {
  namespace TOP {

    ModuleAlignment::ModuleAlignment(PDFConstructor::EPDFOption opt): m_opt(opt)
    {
      m_parNames.push_back("x");
      m_parNames.push_back("y");
      m_parNames.push_back("z");
      m_parNames.push_back("alpha");
      m_parNames.push_back("beta");
      m_parNames.push_back("gamma");
      m_parNames.push_back("t0");
      unsigned numPar = m_parNames.size();
      m_parInit.resize(numPar, 0);
      m_par = m_parInit;
      m_steps.resize(numPar, 0);
      m_fixed.resize(numPar, false);
      m_COV.ResizeTo(numPar, numPar);
      m_U.ResizeTo(numPar, numPar);
      setSteps(1.0, 0.01, 0.05);
    }

    void ModuleAlignment::setSteps(double position, double angle, double time)
    {
      m_steps[0] = position;
      m_steps[1] = position;
      m_steps[2] = position;
      m_steps[3] = angle;
      m_steps[4] = angle;
      m_steps[5] = angle;
      m_steps[6] = time;
      m_maxDpar = m_steps;
    }

    int ModuleAlignment::iterate(TOPTrack& track, const Const::ChargedStable& hypothesis)
    {
      m_numPhotons = 0;
      if (track.getModuleID() != m_moduleID) return -2;

      m_track = &track;
      m_hypothesis = hypothesis;

      std::vector<double> first(m_par.size(), 0);
      TMatrixDSym second(m_par.size());
      if (not derivatives(first, second)) return -1;

      m_U -= second;
      m_numTracks++;

      int ier = invertMatrixU();
      if (ier != 0) return ier;

      std::vector<double> dpar(m_par.size(), 0);
      for (size_t i = 0; i < dpar.size(); i++) {
        for (size_t k = 0; k < dpar.size(); k++) {
          dpar[i] += m_COV[i][k] * first[k];
        }
        if (fabs(dpar[i]) > m_maxDpar[i]) return ier;
      }

      for (size_t i = 0; i < dpar.size(); i++) {
        m_par[i] += dpar[i];
      }
      m_numUsedTracks = m_numTracks;
      m_valid = true;

      return 0;
    }

    void ModuleAlignment::reset()
    {
      m_par = m_parInit;
      m_COV.Zero();
      m_U.Zero();
      m_numTracks = 0;
      m_numUsedTracks = 0;
      m_valid = false;
      m_numPhotons = 0;
    }

    std::vector<float> ModuleAlignment::getParameters() const
    {
      std::vector<float> pars;
      for (auto par : m_par) pars.push_back(par);
      return pars;
    }

    std::vector<float> ModuleAlignment::getErrors() const
    {
      std::vector<float> errors;
      int numPar = m_par.size();
      for (int i = 0; i < numPar; i++) {
        errors.push_back(sqrt(m_COV[i][i]));
      }
      return errors;
    }

    double ModuleAlignment::getLogL(const std::vector<double>& par, bool& ok)
    {
      TVector3 translation(par[0], par[1], par[2]);
      TRotation rotation;
      rotation.RotateX(par[3]).RotateY(par[4]).RotateZ(par[5]);
      ok = m_track->overrideTransformation(rotation, translation);
      if (not ok) return 0;

      PDFConstructor pdfConstructor(*m_track, m_hypothesis, m_opt);
      if (not pdfConstructor.isValid()) {
        ok = false;
        return 0;
      }

      auto LL = pdfConstructor.getLogL(par[6]);
      m_numPhotons = LL.numPhotons;

      return LL.logL;
    }

    bool ModuleAlignment::derivatives(std::vector<double>& first, TMatrixDSym& second)
    {
      bool ok = false;
      double f0 = getLogL(m_par, ok);
      if (not ok) return false;

      auto par = m_par;
      for (size_t k = 0; k < par.size(); k++) {
        if (m_fixed[k]) continue;

        par[k] = m_par[k] + m_steps[k];
        double fp = getLogL(par, ok);
        if (not ok) return false;

        par[k] = m_par[k] - m_steps[k];
        double fm = getLogL(par, ok);
        if (not ok) return false;

        first[k] = (fp - fm) / 2 / m_steps[k];
        second[k][k] = (fp - 2 * f0 + fm) / pow(m_steps[k], 2);

        par[k] = m_par[k];
      }

      for (size_t k = 0; k < par.size(); k++) {
        if (m_fixed[k]) continue;
        for (size_t j = k + 1; j < par.size(); j++) {
          if (m_fixed[j]) continue;

          par[k] = m_par[k] + m_steps[k];
          par[j] = m_par[j] + m_steps[j];
          double fpp = getLogL(par, ok);
          if (not ok) return false;

          par[j] = m_par[j] - m_steps[j];
          double fpm = getLogL(par, ok);
          if (not ok) return false;

          par[k] = m_par[k] - m_steps[k];
          par[j] = m_par[j] + m_steps[j];
          double fmp = getLogL(par, ok);
          if (not ok) return false;

          par[j] = m_par[j] - m_steps[j];
          double fmm = getLogL(par, ok);
          if (not ok) return false;

          second[j][k] = second[k][j] = (fpp - fmp - fpm + fmm) / 4 / m_steps[k] / m_steps[j];

          par[j] = m_par[j];
        }
        par[k] = m_par[k];
      }

      return true;
    }

    int ModuleAlignment::invertMatrixU()
    {
      int n = 0;
      for (auto fixed : m_fixed) {
        if (not fixed) n++;
      }

      TMatrixDSym A(n);
      int ii = 0;
      for (int i = 0; i < m_U.GetNrows(); i++) {
        if (m_fixed[i]) continue;
        int kk = 0;
        for (int k = 0; k < m_U.GetNrows(); k++) {
          if (m_fixed[k]) continue;
          A[ii][kk] = m_U[i][k];
          kk++;
        }
        ii++;
      }

      TDecompChol chol(A);
      if (not chol.Decompose()) return 1;
      chol.Invert(A);

      ii = 0;
      for (int i = 0; i < m_U.GetNrows(); i++) {
        if (m_fixed[i]) continue;
        int kk = 0;
        for (int k = 0; k < m_U.GetNrows(); k++) {
          if (m_fixed[k]) continue;
          m_COV[i][k] = A[ii][kk];
          kk++;
        }
        ii++;
      }

      return 0;
    }

  } // end top namespace
} // end Belle2 namespace


