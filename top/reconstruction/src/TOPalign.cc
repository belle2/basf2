/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/reconstruction/TOPalign.h>
#include <framework/logging/Logger.h>
#include <iostream>

extern "C" {
  void data_clear_();
  void data_put_(int*, int*, float*, float*, int*);
  void set_top_par_(float*, float*);
  void rtra_clear_();
  void rtra_set_hypo_(int*, float*);
  void rtra_set_hypid_(int*, int*);
  void rtra_put_(float*, float*, float*, float*, float*, float*, float*,
                 int*, int*, int*, int*);
  void set_pdf_opt_(int*, int*, int*);
  void top_alignment_(int*, float*, float*, double*, float*, float*, int*);
}

using namespace std;

namespace Belle2 {
  namespace TOP {

    TOPalign::TOPalign()
    {
      m_parNames.push_back("x");
      m_parNames.push_back("y");
      m_parNames.push_back("z");
      m_parNames.push_back("alpha");
      m_parNames.push_back("beta");
      m_parNames.push_back("gamma");
      m_parNames.push_back("t0");
      m_parNames.push_back("dn/n");
      unsigned numPar = m_parNames.size();
      m_parInit.resize(numPar, 0);
      m_par = m_parInit;
      m_steps.resize(numPar, 0);
      m_fixed.resize(numPar, false);
      m_COV.resize(numPar * numPar, 0);
      m_U.resize(numPar * numPar, 0);
      m_maxDpar.resize(numPar, 0);
    }


    void TOPalign::setSteps(double position, double angle, double time, double refind)
    {
      m_steps[0] = position;
      m_steps[1] = position;
      m_steps[2] = position;
      m_steps[3] = angle;
      m_steps[4] = angle;
      m_steps[5] = angle;
      m_steps[6] = time;
      m_steps[7] = refind;
      m_maxDpar = m_steps;
    }


    void TOPalign::clearData()
    {
      data_clear_();
    }

    int TOPalign::addData(int moduleID, int pixelID, double time, double timeError)
    {
      int status = 0;
      moduleID--; // 0-based ID used in fortran
      pixelID--;   // 0-based ID used in fortran
      float t = (float) time;
      float terr = (float) timeError;
      data_put_(&moduleID, &pixelID, &t, &terr, &status);
      switch (status) {
        case 0:
          B2WARNING("addData: no space available in /TOP_DATA/");
          return status;
        case -1:
          B2ERROR("addData: invalid module ID."
                  << LogVar("moduleID", moduleID + 1));
          return status;
        case -2:
          B2ERROR("addData: invalid pixel ID."
                  << LogVar("pixelID", pixelID + 1));
          return status;
        case -3:
          B2ERROR("addData: digit should already be masked-out (different masks used?)");
          return status;
        default:
          return status;
      }
    }

    void TOPalign::setPhotonYields(double bkgPerModule, double scaleN0)
    {
      float bkg = bkgPerModule;
      float sf = scaleN0;
      set_top_par_(&bkg, &sf);
    }


    int TOPalign::iterate(const TOPtrack& track, const Const::ChargedStable& hypothesis)
    {
      if (track.getModuleID() != m_moduleID) return -3;

      // pass track parameters to fortran code
      float x = track.getX();
      float y = track.getY();
      float z = track.getZ();
      float t = track.getTrackLength() / Const::speedOfLight;
      float px = track.getPx();
      float py = track.getPy();
      float pz = track.getPz();
      int Q = track.getCharge();
      int HYP = 0;
      int REF = 0;
      int moduleID = m_moduleID - 1;
      rtra_clear_();
      rtra_put_(&x, &y, &z, &t, &px, &py, &pz, &Q, &HYP, &REF, &moduleID);

      // set single mass hypothesis
      int Num = 1;
      float mass = hypothesis.getMass();
      rtra_set_hypo_(&Num, &mass);
      rtra_set_hypid_(&Num, &HYP);

      // set PDF option
      set_pdf_opt_(&m_opt, &m_NP, &m_NC);

      // run single iteration
      int ier = 0;
      int np = m_par.size();
      std::vector<float> dpar(np, 0);
      auto steps = m_steps;
      for (size_t i = 0; i < steps.size(); i++) {
        if (m_fixed[i]) steps[i] = 0;
      }
      top_alignment_(&np, m_par.data(), steps.data(), m_U.data(),
                     dpar.data(), m_COV.data(), &ier);
      if (ier < 0) return ier;

      m_numTracks++;
      if (ier != 0) return ier;

      for (size_t i = 0; i < dpar.size(); i++) {
        if (fabs(dpar[i]) > m_maxDpar[i]) return ier;
      }
      for (size_t i = 0; i < dpar.size(); i++) {
        m_par[i] += dpar[i];
      }
      m_numUsedTracks = m_numTracks;
      m_valid = true;

      return ier;
    }

    void TOPalign::reset()
    {
      m_par = m_parInit;
      for (auto& x : m_COV) x = 0;
      for (auto& x : m_U) x = 0;
      m_numTracks = 0;
      m_numUsedTracks = 0;
      m_valid = false;
    }

    std::vector<float> TOPalign::getErrors() const
    {
      std::vector<float> errors;
      int numPar = m_par.size();
      for (int i = 0; i < numPar; i++) {
        errors.push_back(sqrt(m_COV[i * (numPar + 1)]));
      }
      return errors;
    }


  } // namespace TOP
} // namespace Belle2

