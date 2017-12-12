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

extern "C" {
  void data_clear_();
  void data_put_(int*, int*, float*, float*, int*);
  void set_top_par_(float*, float*);
  void rtra_clear_();
  void rtra_set_hypo_(int*, float*);
  void rtra_set_hypid_(int*, int*);
  void rtra_put_(float*, float*, float*, float*, float*, float*, float*,
                 int*, int*, int*, int*);
  void top_alignment_(int*, float*, float*, double*, float*, float*, int*);
}

using namespace std;

namespace Belle2 {
  namespace TOP {

    TOPalign::TOPalign(int moduleID,
                       double stepPosition,
                       double stepAngle,
                       double stepTime): m_moduleID(moduleID)
    {
      m_par.resize(c_numPar, 0);
      m_step.resize(3, stepPosition);
      m_step.resize(6, stepAngle);
      m_step.resize(c_numPar, stepTime);
      m_COV.resize(c_numPar * c_numPar, 0);
      m_U.resize(c_numPar * c_numPar, 0);
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
          B2ERROR("addData: invalid module ID " << moduleID + 1);
          return status;
        case -2:
          B2ERROR("addData: invalid pixel ID " << pixelID + 1);
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

      // run single iteration
      int ier = 0;
      std::vector<float> dpar(c_numPar, 0);
      int np = c_numPar;
      top_alignment_(&np, m_par.data(), m_step.data(), m_U.data(),
                     dpar.data(), m_COV.data(), &ier);
      if (ier < 0) return ier;

      m_numTracks++;
      if (ier != 0) return ier;

      for (unsigned i = 0; i < c_numPar; i++) {
        if (fabs(dpar[i]) > m_step[i]) return ier;
      }
      for (unsigned i = 0; i < c_numPar; i++) {
        m_par[i] += dpar[i];
      }
      m_valid = true;

      return ier;
    }


  }
}

