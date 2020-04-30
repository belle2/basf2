//*****************************************************************************
//-----------------------------------------------------------------------------
// Fast simulation and reconstruction package for TOP counter (F77 core)
// M. Staric, April-2009
//-----------------------------------------------------------------------------
//
// TOPreco.h
// C++ interface to F77 functions: reconstruction
//-----------------------------------------------------------------------------
//*****************************************************************************

#pragma once

#include <top/reconstruction/TOPtrack.h>
#include <top/dbobjects/TOPCalChannelMask.h>
#include <top/dbobjects/TOPCalChannelT0.h>
#include <top/dbobjects/TOPCalTimebase.h>
#include <framework/database/DBObjPtr.h>
#include <top/dataobjects/TOPAsicMask.h>

extern "C" {
  void set_beta_rq_(float*);
  void set_time_window_(float*, float*);
  void get_time_window_(float*, float*);
  void set_pdf_opt_(int*, int*, int*);
  void set_store_opt_(int*);
  float get_logl_(float*, float*, float*, float*);
  void get_logl_ch_(float*, float*, float*, float*, float*);
  int data_getnum_();
  void set_channel_mask_(int*, int*, int*);
  void set_channel_off_(int*, int*);
  void print_channel_mask_();
  void set_channel_effi_(int*, int*, float*);
  void redo_pdf_(float*);
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

    /**
     * Local( = TOP module)/Global( = BelleII) frame
     */
    enum {c_Local = 0, c_Global = 1};

    /**
     * TOP reconstruction: this class provides interface to fortran code
     */
    class TOPreco {
    public:
      /**
       * Options for PDF:
       *   rough: no dependence on y
       *   fine:  y dependent PDF everywhere
       *   optimal: y dependent PDF only where necessary (default)
       */
      enum PDFoption {c_Rough = 0, c_Fine, c_Optimal};

      /**
       * Options for storing PDF parameters in Fortran common TOP_PIK
       *    reduced: only position, width, nphot and fic (default)
       *    full: also number of reflections etc.
       */
      enum StoreOption {c_Reduced, c_Full};

      /**
       * Constructor
       * @param NumHyp number of mass hypotheses
       * @param Masses masses
       * @param BkgPerModule estimation for minimal number of background hits
       * @param ScaleN0 scale factor to scale N0
       */
      TOPreco(int NumHyp, double Masses[], double BkgPerModule = 0, double ScaleN0 = 1);

      /**
       * Set channel mask
       * @param mask channel mask
       */
      static void setChannelMask(const DBObjPtr<TOPCalChannelMask>& mask,
                                 const TOPAsicMask& asicMask);

      /**
       * Set uncalibrated channels off
       * @param channelT0 channel T0 calibration
       */
      static void setUncalibratedChannelsOff(const DBObjPtr<TOPCalChannelT0>& channelT0);

      /**
       * Set uncalibrated channels off
       * @param timebase timebase calibration
       */
      static void setUncalibratedChannelsOff(const DBObjPtr<TOPCalTimebase>& timebase);

      /**
       * Print channel mask
       */
      static void printChannelMask() {print_channel_mask_();}

      /**
       * Set relative efficiencies of pixels
       */
      static void setChannelEffi();

      /**
       * Set hypothesis internal code: 1=e, 2=mu, 3=pi, 4=K, 5=p, 0=other
       * @param NumHyp number of mass hypotheses
       * @param HypID internal codes in the same order as Masses[]
       */
      void setHypID(int NumHyp, int HypID[]);

      /**
       * Set mass of the particle hypothesis (overrides settings in the constructor)
       * @param mass mass
       */
      void setMass(double mass);

      /**
       * Set time window for photons.
       * Allows to set window different than that defined by parameters of TOPNominalTDC.
       *
       * If Tmax <= Tmin the window is set to default from TOPNominalTDC.
       *
       * Window edges must not exceed those used during data taking or in simulation
       *
       * @param Tmin minimum time [ns]
       * @param Tmax maximum time [ns]
       */
      void setTimeWindow(double Tmin, double Tmax)
      {
        float tmin = (float) Tmin;
        float tmax = (float) Tmax;
        set_time_window_(&tmin, &tmax);
      }

      /**
       * Returns time window for photons.
       * @param Tmin minimum time [ns]
       * @param Tmax maximum time [ns]
       */
      void getTimeWindow(double& Tmin, double& Tmax)
      {
        float tmin = 0;
        float tmax = 0;
        get_time_window_(&tmin, &tmax);
        Tmin = tmin;
        Tmax = tmax;
      }

      /**
       * Sets PDF option
       * @param opt option - see definition of PDFoption
       * @param NP number of emission positions along track segment (equidistant)
       * @param NC number of Cerenkov angles (equdistant in photon energies)
       */
      void setPDFoption(PDFoption opt, int NP = 0, int NC = 0)
      {
        int iopt = opt;
        set_pdf_opt_(&iopt, &NP, &NC);
      }

      /**
       * Sets option for storing PDF parameters in Fortran common TOP_PIK
       * @param opt option - see definition of StoreOption
       */
      void setStoreOption(StoreOption opt)
      {
        int iopt = opt;
        set_store_opt_(&iopt);
      }

      /**
       * Clear data list
       */
      void clearData();

      /**
       * Add data
       * @param moduleID module ID
       * @param pixelID pixel ID (e.g. software channel, 1-based)
       * @param time TBC and t0-corrected time in [ns]
       * @param timeError time uncertainty in [ns]
       */
      int addData(int moduleID, int pixelID, double time, double timeError);

      /**
       * Return size of data list
       */
      int getDataSize()
      {
        return data_getnum_();
      }

      /**
       * Run reconstruction for a given track
       * @param trk track
       * @param pdg PDG code for which to compute pulls. If 0 use MC true PDG.
       */
      void reconstruct(TOPtrack& trk, int pdg = 0);

      /**
       * Return status
       * @return status: 1=OK, 0=out of acceptance, -1=inside gap
       */
      int getFlag();

      /**
       * Return expected number of photons (signal + background) for i-th mass hypothesis
       * @param i index of mass hypothesis
       * @return expected number of photons
       */
      double getExpectedPhotons(int i = 0);

      /**
       * Return expected number of background photons
       * @return expected number of background
       */
      double getExpectedBG();

      /**
       * Return number of measured photons
       * @return number of photons
       */
      int getNumOfPhotons();

      /**
       * Return log likelihood for i-th mass hypothesis
       * @param i index of mass hypothesis
       * @return log likelihood
       */
      double getLogL(int i = 0);

      /**
       * Get all reconstruction results at once
       * @param Size size of arrays
       * @param LogL log likelihoods for Masses[]
       * @param ExpNphot expected number of photons for Masses[]
       * @param Nphot measured number of photons
       */
      void getLogL(int Size, double LogL[], double ExpNphot[], int& Nphot);

      /**
       * Return log likelihood for the last mass hypothesis using time-shifted PDF
       * If timeMax <= timeMin use those set by setTimeWindow(double Tmin, double Tmax)
       * @param timeShift time shift of PDF
       * @param timeMin lower edge of time window within which the photons are accepted
       * @param timeMax upper edge of time window within which the photons are accepted
       * @param sigma additional time smearing sigma
       * @return log likelihood
       */
      double getLogL(double timeShift, double timeMin, double timeMax, double sigma = 0.0)
      {
        float t0 = (float) timeShift;
        float tmin = (float) timeMin;
        float tmax = (float) timeMax;
        float sigt = (float) sigma;
        return get_logl_(&t0, &tmin, &tmax, &sigt);
      }

      /**
       * Return pixel log likelihoods for the last mass hypothesis using time-shifted PDF
       * If timeMax <= timeMin use those set by setTimeWindow(double Tmin, double Tmax)
       * @param timeShift time shift of PDF
       * @param timeMin lower edge of time window within which the photons are accepted
       * @param timeMax upper edge of time window within which the photons are accepted
       * @param sigma additional time smearing sigma
       * @param logL return array of pixel log likelihood values (must be zeroed on input)
       */
      void getLogL(double timeShift, double timeMin, double timeMax, double sigma,
                   float* logL)
      {
        float t0 = (float) timeShift;
        float tmin = (float) timeMin;
        float tmax = (float) timeMax;
        float sigt = (float) sigma;
        get_logl_ch_(&t0, &tmin, &tmax, &sigt, logL);
      }

      /**
       * Return track hit at the bar in Local or Global frame
       * @param LocGlob select Local or Global frame
       * @param R track spatial position
       * @param Dir track direction (unit vector)
       * @param Len track length inside bar
       * @param Tlen track length from IP
       * @param Mom track momentum
       * @param moduleID module ID if hit, else -1
       */
      void getTrackHit(int LocGlob, double R[3], double Dir[3], double& Len,
                       double& Tlen, double& Mom, int& moduleID);

      /**
       * Print log likelihoods to std output
       * @param NumHyp number of hypotheses
       */
      void dumpLogL(int NumHyp);

      /**
       * Print track to std output
       * @param LocGlob in Local or Global frame
       */
      void dumpTrackHit(int LocGlob);

      /**
       * Get pulls: size
       * @return: size
       */
      int getPullSize();

      /**
       * Get pulls: K-th pull
       * @param K counter
       * @param pixelID pixel ID (e.g. software channel, 1-based)
       * @param T photon time
       * @param T0 PDF mean time
       * @param Wid PDF sigma
       * @param PhiCer azimuthal Cerenkov angle
       * @param Wt weight
       */
      void getPull(int K, int& pixelID, float& T, float& T0, float& Wid,
                   float& PhiCer, float& Wt);

      /**
       * Return PDF for pixel pixelID at time t for mass hypothesis mass
       * @param pixelID pixel ID (e.g. software channel, 1-based)
       * @param t time
       * @param jitter additional time jitter, like electronic jitter
       * @param mass mass
       */
      double getPDF(int pixelID, double t, double mass, double jitter = 0);

      /**
       * Set track beta (for beta resolution studies)
       * if beta>0 this value is used instead of beta from momentum and mass
       * @param beta beta value
       */
      void setBeta(double beta) {m_beta = beta; float bt = beta; set_beta_rq_(&bt);};

      /**
       * Return track beta
       * @return beta value
       */
      double getBeta() const {return m_beta;};

      /**
       * Re-calculate PDF for a given particle mass using option c_Fine
       * @param mass particle mass
       */
      void redoPDF(double mass)
      {
        float m = mass;
        redo_pdf_(&m);
      }

      /**
       * Returns number of peaks for given pixel describing signal PDF
       * @param pixelID pixel ID (1-based)
       * @return number of peaks
       */
      int getNumofPDFPeaks(int pixelID) const
      {
        pixelID--; // 0-based is used in fortran
        return get_num_peaks_(&pixelID);
      }

      /**
       * Returns k-th PDF peak for given pixel describing signal PDF
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       */
      void getPDFPeak(int pixelID, int k,
                      float& position, float& width, float& numPhotons) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        get_peak_(&pixelID, &k, &position, &width, &numPhotons);
      }

      /**
       * Returns estimated background level for given pixel
       * @param pixelID pixel ID (1-based)
       * @return number of background hits per nano second
       */
      float getBkgLevel(int pixelID) const
      {
        pixelID--; // 0-based is used in fortran
        return get_bgr_(&pixelID);
      }

      /**
       * Returns type of the k-th PDF peak for given pixel
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return 0 unknown, 1 direct, 2 reflected
       */
      int getPDFPeakType(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return get_pik_typ_(&pixelID, &k);
      }

      /**
       * Returns Cerenkov azimuthal angle of PDF peak
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return azimuthal angle
       */
      float getPDFPeakFic(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return get_pik_fic_(&pixelID, &k);
      }

      /**
       * Returns photon energy of PDF peak
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return photon energy [eV]
       */
      float getPDFPeakE(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return get_pik_e_(&pixelID, &k);
      }

      /**
       * Returns photon energy spread of PDF peak
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return photon energy sigma [eV]
       */
      float getPDFPeakSigE(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return get_pik_sige_(&pixelID, &k);
      }

      /**
       * Returns total number of reflections in x of PDF peak
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return total number of reflections
       */
      int getPDFPeakNx(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return abs(get_pik_nx_(&pixelID, &k));
      }

      /**
       * Returns total number of reflections in y of PDF peak
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return total number of reflections
       */
      int getPDFPeakNy(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return abs(get_pik_ny_(&pixelID, &k));
      }

      /**
       * Returns number of reflections in x before mirror
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return total number of reflections before mirror
       */
      int getPDFPeakNxm(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return abs(get_pik_nxm_(&pixelID, &k));
      }

      /**
       * Returns number of reflections in y before mirror
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return total number of reflections before mirror
       */
      int getPDFPeakNym(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return abs(get_pik_nym_(&pixelID, &k));
      }

      /**
       * Returns number of reflections in x in prism
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return total number of reflections before mirror
       */
      int getPDFPeakNxe(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return abs(get_pik_nxe_(&pixelID, &k));
      }

      /**
       * Returns number of reflections in y in prism
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return total number of reflections before mirror
       */
      int getPDFPeakNye(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return abs(get_pik_nye_(&pixelID, &k));
      }

      /**
       * Returns unfolded x position of pixel
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return unfolded x
       */
      float getPDFPeakXD(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return get_pik_xd_(&pixelID, &k);
      }

      /**
       * Returns unfolded y position of pixel
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return unfolded y
       */
      float getPDFPeakYD(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return get_pik_yd_(&pixelID, &k);
      }

      /**
       * Returns photon reconstructed direction in x at emission
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return direction in x
       */
      float getPDFPeakKxe(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return get_pik_kxe_(&pixelID, &k);
      }

      /**
       * Returns photon reconstructed direction in y at emission
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return direction in y
       */
      float getPDFPeakKye(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return get_pik_kye_(&pixelID, &k);
      }

      /**
       * Returns photon reconstructed direction in z at emission
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return direction in z
       */
      float getPDFPeakKze(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return get_pik_kze_(&pixelID, &k);
      }

      /**
       * Returns photon reconstructed direction in x at detection
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return direction in x
       */
      float getPDFPeakKxd(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return get_pik_kxd_(&pixelID, &k);
      }

      /**
       * Returns photon reconstructed direction in y at detection
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return direction in y
       */
      float getPDFPeakKyd(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return get_pik_kyd_(&pixelID, &k);
      }

      /**
       * Returns photon reconstructed direction in z at detection
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return direction in z
       */
      float getPDFPeakKzd(int pixelID, int k) const
      {
        pixelID--; // 0-based is used in fortran
        k++; // counter starts with 1 in fortran
        return get_pik_kzd_(&pixelID, &k);
      }


    private:

      /**
       * Run reconstruction
       * @param X track spatial position x
       * @param Y track spatial position y
       * @param Z track spatial position z
       * @param Tlen track length from IP
       * @param Px track momentum component x
       * @param Py track momentum component y
       * @param Pz track momentum component z
       * @param Q track charge
       * @param pdg PDG code for which to compute pulls
       * @param moduleID module ID (optional)
       */
      void reconstruct(double X, double Y, double Z, double Tlen,
                       double Px, double Py, double Pz, int Q,
                       int pdg = 0, int moduleID = 0);

      int m_hypID;    /**< true hypothesis ID */
      double m_beta;  /**< beta value, if set */

    };

  } // TOP namespace
} // Belle2 namespace

