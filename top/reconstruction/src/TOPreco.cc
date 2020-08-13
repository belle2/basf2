//*****************************************************************************
//-----------------------------------------------------------------------------
// Fast simulation and reconstruction package for TOP counter (F77 core)
// M. Staric, April-2009, Sept-2011
//-----------------------------------------------------------------------------
//
// TOPreco.cc
// C++ interface to F77 functions: reconstruction
//-----------------------------------------------------------------------------
//*****************************************************************************

#include <iostream>
#include <iomanip>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPf77fun.h>
#include <top/geometry/TOPGeometryPar.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

extern "C" {
  void set_beta_rq_(float*);
  void set_time_window_(float*, float*);
  void get_time_window_(float*, float*);
  void set_pdf_opt_(int*, int*, int*);
  void set_store_opt_(int*);
  float get_logl_(float*, float*, float*, float*);
  void get_logl_ch_(float*, float*, float*, float*, float*);
  void get_logl_ch_sfot_(float*, float*, float*, float*, float*, float*);
  void get_sfot_(float*, float*, float*, float*);
  int data_getnum_();
  void set_channel_mask_(int*, int*, int*);
  void set_channel_off_(int*, int*);
  void print_channel_mask_();
  void set_channel_effi_(int*, int*, float*);
  void redo_pdf_(float*, int*);
  int get_num_peaks_(int*);
  void get_peak_(int*, int*, float*, float*, float*);
  float get_bgr_(int*);
  int get_pik_typ_(int*, int*);
  float get_pik_fic_(int*, int*);
  float get_pik_e_(int*, int*);
  float get_pik_sige_(int*, int*);
  int get_pik_nx_(int*, int*);
  int get_pik_ny_(int*, int*);
  int get_pik_nxm_(int*, int*);
  int get_pik_nym_(int*, int*);
  int get_pik_nxe_(int*, int*);
  int get_pik_nye_(int*, int*);
  float get_pik_xd_(int*, int*);
  float get_pik_yd_(int*, int*);
  float get_pik_kxe_(int*, int*);
  float get_pik_kye_(int*, int*);
  float get_pik_kze_(int*, int*);
  float get_pik_kxd_(int*, int*);
  float get_pik_kyd_(int*, int*);
  float get_pik_kzd_(int*, int*);
}

namespace Belle2 {
  namespace TOP {

    TOPreco::TOPreco(int Num, double Masses[], int pdgCodes[],
                     double BkgPerModule, double ScaleN0)
    {
      data_clear_();
      rtra_clear_();

      std::vector<float> masses;
      for (int i = 0; i < Num; i++) {
        masses.push_back((float) Masses[i]);
      }
      rtra_set_hypo_(&Num, masses.data());
      rtra_set_hypid_(&Num, pdgCodes);

      float b = (float) BkgPerModule;
      float s = (float) ScaleN0;
      set_top_par_(&b, &s);

      setPDFoption(c_Optimal); // default option
      setTimeWindow(0.0, 0.0); // use default (TDC range)
      setBeta(0); // use default: beta from momentum and mass
    }


    void TOPreco::setChannelMask(const DBObjPtr<TOPCalChannelMask>& mask,
                                 const TOPAsicMask& asicMask)
    {
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      const auto& mapper = TOPGeometryPar::Instance()->getChannelMapper();
      int numModules = geo->getNumModules();
      for (int moduleID = 1; moduleID <= numModules; moduleID++) {
        unsigned numPixels = geo->getModule(moduleID).getPMTArray().getNumPixels();
        for (unsigned channel = 0; channel < numPixels; channel++) {
          int mdn = moduleID - 1; // 0-based used in fortran
          int ich = mapper.getPixelID(channel) - 1; // 0-base used in fortran
          int flag = mask->isActive(moduleID, channel) and asicMask.isActive(moduleID, channel);
          set_channel_mask_(&mdn, &ich, &flag);
        }
      }
      B2INFO("TOPreco: new channel masks have been passed to reconstruction");
    }


    void TOPreco::setUncalibratedChannelsOff(const DBObjPtr<TOPCalChannelT0>& channelT0)
    {
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      const auto& mapper = TOPGeometryPar::Instance()->getChannelMapper();
      int numModules = geo->getNumModules();
      for (int moduleID = 1; moduleID <= numModules; moduleID++) {
        unsigned numPixels = geo->getModule(moduleID).getPMTArray().getNumPixels();
        for (unsigned channel = 0; channel < numPixels; channel++) {
          if (channelT0->isCalibrated(moduleID, channel)) continue;
          int mdn = moduleID - 1; // 0-based used in fortran
          int ich = mapper.getPixelID(channel) - 1; // 0-based used in fortran
          set_channel_off_(&mdn, &ich);
        }
      }
      B2INFO("TOPreco: channelT0-uncalibrated channels have been masked off");
    }


    void TOPreco::setUncalibratedChannelsOff(const DBObjPtr<TOPCalTimebase>& timebase)
    {
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      const auto& ch_mapper = TOPGeometryPar::Instance()->getChannelMapper();
      const auto& fe_mapper = TOPGeometryPar::Instance()->getFrontEndMapper();
      int numModules = geo->getNumModules();
      for (int moduleID = 1; moduleID <= numModules; moduleID++) {
        unsigned numPixels = geo->getModule(moduleID).getPMTArray().getNumPixels();
        for (unsigned channel = 0; channel < numPixels; channel++) {
          const auto* fe = fe_mapper.getMap(moduleID, channel / 128);
          if (not fe) {
            B2ERROR("TOPreco::setUncalibratedChannelsOff no front-end map found");
            continue;
          }
          auto scrodID = fe->getScrodID();
          const auto* sampleTimes = timebase->getSampleTimes(scrodID, channel);
          if (sampleTimes->isCalibrated()) continue;
          int mdn = moduleID - 1; // 0-based used in fortran
          int ich = ch_mapper.getPixelID(channel) - 1; // 0-based used in fortran
          set_channel_off_(&mdn, &ich);
        }
      }
      B2INFO("TOPreco: timebase-uncalibrated channels have been masked off");
    }

    void TOPreco::printChannelMask()
    {
      print_channel_mask_();
    }

    void TOPreco::setChannelEffi()
    {
      const auto* topgp = TOPGeometryPar::Instance();
      const auto* geo = topgp->getGeometry();
      int numModules = geo->getNumModules();
      for (int moduleID = 1; moduleID <= numModules; moduleID++) {
        int numPixels = geo->getModule(moduleID).getPMTArray().getNumPixels();
        for (int pixelID = 1; pixelID <= numPixels; pixelID++) {
          int mdn = moduleID - 1; // 0-based used in fortran
          int ich = pixelID - 1;  // 0-based used in fortran
          float effi = topgp->getRelativePixelEfficiency(moduleID, pixelID);
          set_channel_effi_(&mdn, &ich, &effi);
        }
      }
      B2INFO("TOPreco: new relative pixel efficiencies have been passed to reconstruction");
    }

    void TOPreco::setMass(double Mass, int pdg)
    {
      int Num = 1;
      float mass = (float) Mass;
      rtra_set_hypo_(&Num, &mass);
      rtra_set_hypid_(&Num, &pdg);
    }

    void TOPreco::setTimeWindow(double Tmin, double Tmax)
    {
      float tmin = (float) Tmin;
      float tmax = (float) Tmax;
      set_time_window_(&tmin, &tmax);
    }

    void TOPreco::getTimeWindow(double& Tmin, double& Tmax)
    {
      float tmin = 0;
      float tmax = 0;
      get_time_window_(&tmin, &tmax);
      Tmin = tmin;
      Tmax = tmax;
    }

    void TOPreco::setPDFoption(PDFoption opt, int NP, int NC)
    {
      int iopt = opt;
      set_pdf_opt_(&iopt, &NP, &NC);
    }

    void TOPreco::setStoreOption(StoreOption opt)
    {
      int iopt = opt;
      set_store_opt_(&iopt);
    }

    void TOPreco::clearData()
    {
      data_clear_();
      rtra_clear_();
    }

    int TOPreco::addData(int moduleID, int pixelID, double time, double timeError)
    {
      int status = 0;
      moduleID--; // 0-based ID used in fortran
      pixelID--;   // 0-based ID used in fortran
      float t = (float) time;
      float terr = (float) timeError;
      data_put_(&moduleID, &pixelID, &t, &terr, &status);
      if (status > 0) return status;
      switch (status) {
        case 0:
          B2WARNING("TOPReco::addData: no space available in /TOP_DATA/");
          return status;
        case -1:
          B2ERROR("TOPReco::addData: invalid module ID."
                  << LogVar("moduleID", moduleID + 1));
          return status;
        case -2:
          B2ERROR("TOPReco::addData: invalid pixel ID."
                  << LogVar("pixelID", pixelID + 1));
          return status;
        case -3:
          B2DEBUG(100, "TOPReco::addData: digit should already be masked-out (different masks used?)");
          return status;
        default:
          B2ERROR("TOPReco::addData: unknown return status."
                  << LogVar("status", status));
          return status;
      }
    }

    int TOPreco::getDataSize()
    {
      return data_getnum_();
    }

    void TOPreco::reconstruct(double X, double Y, double Z, double Tlen,
                              double Px, double Py, double Pz, int Q,
                              int HYP, int moduleID)
    {
      float x = (float) X;
      float y = (float) Y;
      float z = (float) Z;
      float t = float(Tlen / Const::speedOfLight);
      float px = (float) Px;
      float py = (float) Py;
      float pz = (float) Pz;
      int REF = 0;
      moduleID--; // 0-based ID used in fortran
      rtra_clear_();
      rtra_put_(&x, &y, &z, &t, &px, &py, &pz, &Q, &HYP, &REF, &moduleID);
      top_reco_();
    }

    void TOPreco::reconstruct(const TOPtrack& trk, int pdg)
    {
      m_hypID = abs(trk.getPDGcode());
      if (pdg == 0) pdg = m_hypID;
      int moduleID = trk.getModuleID();
      reconstruct(trk.getX(), trk.getY(), trk.getZ(), trk.getTrackLength(),
                  trk.getPx(), trk.getPy(), trk.getPz(), trk.getCharge(),
                  pdg, moduleID);
    }

    int TOPreco::getFlag()
    {
      int K = 1;
      return rtra_get_flag_(&K);
    }

    double TOPreco::getExpectedPhotons(int i)
    {
      int K = 1;
      i++;
      return rtra_get_sfot_(&K, &i);
    }

    double TOPreco::getExpectedBG()
    {
      int K = 1;
      return rtra_get_bfot_(&K);
    }

    int TOPreco::getNumOfPhotons()
    {
      int K = 1;
      return rtra_get_nfot_(&K);
    }

    double TOPreco::getLogL(int i)
    {
      int K = 1;
      i++;
      return rtra_get_plkh_(&K, &i);
    }

    void TOPreco::getLogL(int Size, double LogL[], double ExpNphot[], int& Nphot)
    {
      int K = 1;
      std::vector<float> logl(Size), sfot(Size);
      int Flag, MTRA, REF;
      rtra_get_(&K, logl.data(), sfot.data(), &Size, &Nphot, &Flag, &MTRA, &REF);
      for (int i = 0; i < Size; i++) {
        LogL[i] = logl[i];
        ExpNphot[i] = sfot[i];
      }
    }

    double TOPreco::getLogL(double timeShift, double timeMin, double timeMax,
                            double sigma)
    {
      float t0 = (float) timeShift;
      float tmin = (float) timeMin;
      float tmax = (float) timeMax;
      float sigt = (float) sigma;
      return get_logl_(&t0, &tmin, &tmax, &sigt);
    }

    void TOPreco::getLogL(double timeShift, double timeMin, double timeMax, double sigma,
                          float* logL)
    {
      float t0 = (float) timeShift;
      float tmin = (float) timeMin;
      float tmax = (float) timeMax;
      float sigt = (float) sigma;
      get_logl_ch_(&t0, &tmin, &tmax, &sigt, logL);
    }

    void TOPreco::getLogL(double timeShift, double timeMin, double timeMax, double sigma,
                          float* logL, float* sphot)
    {
      float t0 = (float) timeShift;
      float tmin = (float) timeMin;
      float tmax = (float) timeMax;
      float sigt = (float) sigma;
      get_logl_ch_sfot_(&t0, &tmin, &tmax, &sigt, logL, sphot);
    }

    void TOPreco::getSigPhot(double timeShift, double timeMin, double timeMax, float* sfot)
    {
      float t0 = (float) timeShift;
      float tmin = (float) timeMin;
      float tmax = (float) timeMax;
      get_sfot_(&t0, &tmin, &tmax, sfot);
    }

    void TOPreco::getTrackHit(int LocGlob, double R[3], double Dir[3], double& Len,
                              double& Tlen, double& Mom, int& moduleID)
    {
      int K = 1;
      float r[3], dir[3], len, tof, p;
      rtra_gethit_(&K, &LocGlob, r, dir, &len, &tof, &p, &moduleID);
      moduleID++;
      for (int i = 0; i < 3; i++) {
        R[i] = r[i];
        Dir[i] = dir[i];
      }
      Len = len; Tlen = tof * Const::speedOfLight; Mom = p;
    }

    void TOPreco::dumpLogL(int Size)
    {
      std::vector<double> logl(Size), sfot(Size);
      std::vector<int> hypid(Size);
      int Nphot;
      getLogL(Size, logl.data(), sfot.data(), Nphot);
      rtra_get_hypid_(&Size, hypid.data());

      int i_max = 0;
      double logl_max = logl[0];
      for (int i = 1; i < Size; i++) {
        if (logl[i] > logl_max) {logl_max = logl[i]; i_max = i;}
      }

      using namespace std;
      cout << "TOPreco::dumpLogL: Flag=" << getFlag();
      cout << "  Detected Photons=" << Nphot << endl;
      cout << " i         HypID   LogL   ExpPhot" << endl;
      cout << showpoint << fixed << right;
      for (int i = 0; i < Size; i++) {
        cout << setw(2) << i;
        cout << setw(12) << hypid[i];
        cout << setw(10) << setprecision(2) << logl[i];
        cout << setw(8) << setprecision(2) << sfot[i];
        if (i == i_max) cout << " <";
        if (hypid[i] == m_hypID) cout << " <-- truth";
        cout << endl;
      }
    }

    void TOPreco::dumpTrackHit(int LocGlob)
    {
      double r[3], dir[3], len, Tlen, p;
      int moduleID;
      getTrackHit(LocGlob, r, dir, len, Tlen, p, moduleID);

      using namespace std;
      cout << showpoint << fixed << right;
      cout << "TOPreco::dumpTrackHit: moduleID=" << moduleID;
      cout << "  Len=" << setprecision(2) << len;
      cout << "cm  Tlen=" << setprecision(1) << Tlen;
      cout << "cm  p=" << setprecision(2) << p << "GeV/c" << endl;
      cout << "position [cm]: ";
      for (int i = 0; i < 3; i++) {
        cout << setw(10) << setprecision(2) << r[i];
      }
      if (LocGlob == c_Local) {cout << " (local)" << endl;}
      else {cout << " (global)" << endl;}

      cout << "direction:     ";
      for (int i = 0; i < 3; i++) {
        cout << setw(10) << setprecision(4) << dir[i];
      }
      if (LocGlob == c_Local) {cout << " (local)" << endl;}
      else {cout << " (global)" << endl;}
    }

    int TOPreco::getPullSize()
    {
      int n;
      getnum_pulls_(&n);
      return n;
    }

    void TOPreco::getPull(int K, int& ich, float& t, float& t0, float& wid,
                          float& fic, float& wt)
    {
      K++;
      get_pulls_(&K, &t, &t0, &wid, &fic, &wt, &ich);
      ich++; // convert to 1-based
    }

    double TOPreco::getPDF(int pixelID, double T, double Mass, int PDG, double Terr)
    {
      pixelID--;  // 0-based ID used in fortran
      float t = (float) T;
      float terr = (float) Terr;
      float mass = (float) Mass;
      return get_pdf_(&pixelID, &t, &terr, &mass, &PDG);
    }

    void TOPreco::setBeta(double beta)
    {
      m_beta = beta;
      float bt = beta;
      set_beta_rq_(&bt);
    }

    void TOPreco::redoPDF(double mass, int PDG)
    {
      float m = mass;
      redo_pdf_(&m, &PDG);
    }

    int TOPreco::getNumofPDFPeaks(int pixelID) const
    {
      pixelID--; // 0-based is used in fortran
      return get_num_peaks_(&pixelID);
    }

    void TOPreco::getPDFPeak(int pixelID, int k,
                             float& position, float& width, float& numPhotons) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      get_peak_(&pixelID, &k, &position, &width, &numPhotons);
    }

    float TOPreco::getBkgLevel(int pixelID) const
    {
      pixelID--; // 0-based is used in fortran
      return get_bgr_(&pixelID);
    }

    int TOPreco::getPDFPeakType(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return get_pik_typ_(&pixelID, &k);
    }

    float TOPreco::getPDFPeakFic(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return get_pik_fic_(&pixelID, &k);
    }

    float TOPreco::getPDFPeakE(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return get_pik_e_(&pixelID, &k);
    }

    float TOPreco::getPDFPeakSigE(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return get_pik_sige_(&pixelID, &k);
    }

    int TOPreco::getPDFPeakNx(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return abs(get_pik_nx_(&pixelID, &k));
    }

    int TOPreco::getPDFPeakNy(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return abs(get_pik_ny_(&pixelID, &k));
    }

    int TOPreco::getPDFPeakNxm(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return abs(get_pik_nxm_(&pixelID, &k));
    }

    int TOPreco::getPDFPeakNym(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return abs(get_pik_nym_(&pixelID, &k));
    }

    int TOPreco::getPDFPeakNxe(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return abs(get_pik_nxe_(&pixelID, &k));
    }

    int TOPreco::getPDFPeakNye(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return abs(get_pik_nye_(&pixelID, &k));
    }

    float TOPreco::getPDFPeakXD(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return get_pik_xd_(&pixelID, &k);
    }

    float TOPreco::getPDFPeakYD(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return get_pik_yd_(&pixelID, &k);
    }

    float TOPreco::getPDFPeakKxe(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return get_pik_kxe_(&pixelID, &k);
    }

    float TOPreco::getPDFPeakKye(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return get_pik_kye_(&pixelID, &k);
    }

    float TOPreco::getPDFPeakKze(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return get_pik_kze_(&pixelID, &k);
    }

    float TOPreco::getPDFPeakKxd(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return get_pik_kxd_(&pixelID, &k);
    }

    float TOPreco::getPDFPeakKyd(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return get_pik_kyd_(&pixelID, &k);
    }

    float TOPreco::getPDFPeakKzd(int pixelID, int k) const
    {
      pixelID--; // 0-based is used in fortran
      k++; // counter starts with 1 in fortran
      return get_pik_kzd_(&pixelID, &k);
    }

  } // end top namespace
} // end Belle2 namespace

