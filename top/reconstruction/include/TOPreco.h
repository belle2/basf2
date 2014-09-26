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

#ifndef _TOPreco_h
#define _TOPreco_h

#include <top/reconstruction/TOPtrack.h>

extern "C" {
  void set_beta_rq_(float*);
  void set_tmax_(float*);
}

namespace Belle2 {
  namespace TOP {

    /**
     * Local(=bar)/Global(=BelleII) frame
     */
    enum {Local = 0, Global = 1};

    /**
     * TOP reconstruction: this class provides interface to fortran code
     */
    class TOPreco {
    public:
      /**
       * constructor
       * @param NumHyp number of mass hypotheses
       * @param Masses masses
       * @param BkgPerQbar estimation for minimal number of background hits
       * @param ScaleN0 scale factor to scale N0
       */
      TOPreco(int NumHyp, double Masses[], double BkgPerQbar = 0, double ScaleN0 = 1);

      /**
       * set hypothesis internal code: 1=e, 2=mu, 3=pi, 4=K, 5=p, 0=other
       * @param NumHyp number of mass hypotheses
       * @param HypID internal codes in the same order as Masses[]
       */
      void setHypID(int NumHyp, int HypID[]);

      /**
       * set maximum for photon times (allows to set it lower than TDC range)
       * @param Tmax maximum time [ns]
       */
      void setTmax(double Tmax) {
        float tmax = (float) Tmax;
        set_tmax_(&tmax);
      }

      /**
       * clear data list
       */
      void clearData();

      /**
       * add data
       * @param QbarID bar ID
       * @param chID channel ID
       * @param TDC digitized time
       */
      int addData(int QbarID, int chID, int TDC);

      /**
       * get size of data list
       */
      int getDataSize();

      /**
       * run reconstruction
       * @param X track spatial position x
       * @param Y track spatial position y
       * @param Z track spatial position z
       * @param Tlen track length from IP
       * @param Px track momentum component x
       * @param Py track momentum component y
       * @param Pz track momentum component z
       * @param Q track charge
       * @param HYP true hypothesis
       */
      void reconstruct(double X, double Y, double Z, double Tlen,
                       double Px, double Py, double Pz, int Q, int HYP = 0);

      /**
       * run reconstruction
       * @param trk track
       */
      void reconstruct(TOPtrack& trk);

      /**
       * get status
       * @return status: 1=OK, 0=out of acceptance, -1=inside gap
       */
      int getFlag();

      /**
       * get reconstruction results
       * @param Size size of arrays
       * @param LogL log likelihoods for Masses[]
       * @param ExpNphot expected number of photons for Masses[]
       * @param Nphot measured number of photons
       */
      void getLogL(int Size, double LogL[], double ExpNphot[], int& Nphot);

      /**
       * get track hit at the bar in Local or Global frame
       * @param LocGlob select Local or Global frame
       * @param R track spatial position
       * @param Dir track direction (unit vector)
       * @param Len track length inside bar
       * @param Tlen track length from IP
       * @param Mom track momentum
       * @param QbarID bar ID if hit, else -1
       */
      void getTrackHit(int LocGlob, double R[3], double Dir[3], double& Len,
                       double& Tlen, double& Mom, int& QbarID);

      /**
       * print log likelihoods to std output
       * @param NumHyp number of hypotheses
       */
      void dumpLogL(int NumHyp);

      /**
       * print track to std output
       * @param LocGlob in Local or Global frame
       */
      void dumpTrackHit(int LocGlob);

      /**
       * get pulls: size
       * @return: size
       */
      int getPullSize();

      /**
       * get pulls: K-th pull
       * @param K counter
       * @param T photon time
       * @param T0 PDF mean time
       * @param Wid PDF sigma
       * @param PhiCer azimuthal Cerenkov angle
       * @param Wt weight
       */
      void getPull(int K, double& T, double& T0, double& Wid, double& PhiCer,
                   double& Wt);

      /**
       * get PDF for channel chID at time t for mass hypothesis mass
       * @param chID channel ID
       * @param t time
       * @param mass mass
       */
      double getPDF(int chID, double t, double mass);

      /**
       * set track beta (for beta resolution studies)
       * if beta>0 this value is used instead of beta from momentum and mass
       * @param beta beta value
       */
      void setBeta(double beta) {m_beta = beta; float bt = beta; set_beta_rq_(&bt);};

      /**
       * return track beta
       * @return beta value
       */
      double getBeta() {return m_beta;};


    private:
      int m_HYP;      /**< true hypothesis*/
      double m_beta;  /**< beta value, if set */

    };

  } // top namespace
} // Belle2 namespace

#endif
