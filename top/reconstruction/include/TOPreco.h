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
       * @param pdgCodes PDG codes
       * @param BkgPerModule estimation for minimal number of background hits
       * @param ScaleN0 scale factor to scale N0
       */
      TOPreco(int NumHyp, double Masses[], int pdgCodes[],
              double BkgPerModule = 0, double ScaleN0 = 1);

      /**
       * Set channel mask
       * @param mask channel mask
       * @param asicMask masked asics
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
      static void printChannelMask();

      /**
       * Set relative efficiencies of pixels
       */
      static void setChannelEffi();

      /**
       * Set mass of the particle hypothesis (overrides settings in the constructor)
       * @param mass mass
       * @param pdg PDG code
       */
      void setMass(double mass, int pdg);

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
      void setTimeWindow(double Tmin, double Tmax);

      /**
       * Returns time window for photons.
       * @param Tmin minimum time [ns]
       * @param Tmax maximum time [ns]
       */
      void getTimeWindow(double& Tmin, double& Tmax);

      /**
       * Sets PDF option
       * @param opt option - see definition of PDFoption
       * @param NP number of emission positions along track segment (equidistant)
       * @param NC number of Cerenkov angles (equdistant in photon energies)
       */
      void setPDFoption(PDFoption opt, int NP = 0, int NC = 0);

      /**
       * Sets option for storing PDF parameters in Fortran common TOP_PIK
       * @param opt option - see definition of StoreOption
       */
      void setStoreOption(StoreOption opt);

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
      int getDataSize();

      /**
       * Run reconstruction for a given track
       * @param trk track
       * @param pdg PDG code for which to compute pulls. If 0 use MC true PDG.
       */
      void reconstruct(const TOPtrack& trk, int pdg = 0);

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
      double getLogL(double timeShift, double timeMin, double timeMax, double sigma = 0.0);

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
                   float* logL);

      /**
       * Return pixel log likelihoods and expected pixel signal photon counts for the
       * last mass hypothesis using time-shifted PDF
       * If timeMax <= timeMin use those set by setTimeWindow(double Tmin, double Tmax)
       * @param timeShift time shift of PDF
       * @param timeMin lower edge of time window within which the photons are accepted
       * @param timeMax upper edge of time window within which the photons are accepted
       * @param sigma additional time smearing sigma
       * @param logL return array of pixel log likelihood values (must be zeroed on input)
       * @param sphot return array of pixel signal photon counts (must be zeroed on input)
       */
      void getLogL(double timeShift, double timeMin, double timeMax, double sigma,
                   float* logL, float* sphot);

      /**
       * Return pixel signal photons for the last mass hypothesis using time-shifted PDF
       * If timeMax <= timeMin use those set by setTimeWindow(double Tmin, double Tmax)
       * @param timeShift time shift of PDF
       * @param timeMin lower edge of time window within which the photons are accepted
       * @param timeMax upper edge of time window within which the photons are accepted
       * @param sfot return array of pixel signal photon counts (must be zeroed on input)
       */
      void getSigPhot(double timeShift, double timeMin, double timeMax, float* sfot);

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
       * @param mass particle mass
       * @param PDG particle code
       * @param jitter additional time jitter, like electronic jitter
       */
      double getPDF(int pixelID, double t, double mass, int PDG, double jitter = 0);

      /**
       * Set track beta (for beta resolution studies)
       * if beta>0 this value is used instead of beta from momentum and mass
       * @param beta beta value
       */
      void setBeta(double beta);

      /**
       * Return track beta
       * @return beta value
       */
      double getBeta() const {return m_beta;};

      /**
       * Re-calculate PDF for a given particle mass using option c_Fine
       * @param mass particle mass
       * @param PDG particle code
       */
      void redoPDF(double mass, int PDG);

      /**
       * Returns number of peaks for given pixel describing signal PDF
       * @param pixelID pixel ID (1-based)
       * @return number of peaks
       */
      int getNumofPDFPeaks(int pixelID) const;

      /**
       * Returns k-th PDF peak for given pixel describing signal PDF
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @param position pisition
       * @param width width
       * @param numPhotons number of photons
       */
      void getPDFPeak(int pixelID, int k,
                      float& position, float& width, float& numPhotons) const;

      /**
       * Returns estimated background level for given pixel
       * @param pixelID pixel ID (1-based)
       * @return number of background hits per nano second
       */
      float getBkgLevel(int pixelID) const;

      /**
       * Returns type of the k-th PDF peak for given pixel
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return 0 unknown, 1 direct, 2 reflected
       */
      int getPDFPeakType(int pixelID, int k) const;

      /**
       * Returns Cerenkov azimuthal angle of PDF peak
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return azimuthal angle
       */
      float getPDFPeakFic(int pixelID, int k) const;

      /**
       * Returns photon energy of PDF peak
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return photon energy [eV]
       */
      float getPDFPeakE(int pixelID, int k) const;

      /**
       * Returns photon energy spread of PDF peak
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return photon energy sigma [eV]
       */
      float getPDFPeakSigE(int pixelID, int k) const;

      /**
       * Returns total number of reflections in x of PDF peak
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return total number of reflections
       */
      int getPDFPeakNx(int pixelID, int k) const;

      /**
       * Returns total number of reflections in y of PDF peak
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return total number of reflections
       */
      int getPDFPeakNy(int pixelID, int k) const;

      /**
       * Returns number of reflections in x before mirror
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return total number of reflections before mirror
       */
      int getPDFPeakNxm(int pixelID, int k) const;

      /**
       * Returns number of reflections in y before mirror
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return total number of reflections before mirror
       */
      int getPDFPeakNym(int pixelID, int k) const;

      /**
       * Returns number of reflections in x in prism
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return total number of reflections before mirror
       */
      int getPDFPeakNxe(int pixelID, int k) const;

      /**
       * Returns number of reflections in y in prism
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return total number of reflections before mirror
       */
      int getPDFPeakNye(int pixelID, int k) const;

      /**
       * Returns unfolded x position of pixel
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return unfolded x
       */
      float getPDFPeakXD(int pixelID, int k) const;

      /**
       * Returns unfolded y position of pixel
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return unfolded y
       */
      float getPDFPeakYD(int pixelID, int k) const;

      /**
       * Returns photon reconstructed direction in x at emission
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return direction in x
       */
      float getPDFPeakKxe(int pixelID, int k) const;

      /**
       * Returns photon reconstructed direction in y at emission
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return direction in y
       */
      float getPDFPeakKye(int pixelID, int k) const;

      /**
       * Returns photon reconstructed direction in z at emission
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return direction in z
       */
      float getPDFPeakKze(int pixelID, int k) const;

      /**
       * Returns photon reconstructed direction in x at detection
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return direction in x
       */
      float getPDFPeakKxd(int pixelID, int k) const;

      /**
       * Returns photon reconstructed direction in y at detection
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return direction in y
       */
      float getPDFPeakKyd(int pixelID, int k) const;

      /**
       * Returns photon reconstructed direction in z at detection
       * @param pixelID pixel ID (1-based)
       * @param k peak counter (in C++ sense - starts with 0)
       * @return direction in z
       */
      float getPDFPeakKzd(int pixelID, int k) const;

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

      int m_hypID = 0;    /**< true hypothesis ID */
      double m_beta = 0;  /**< beta value, if set */

    };

  } // TOP namespace
} // Belle2 namespace

