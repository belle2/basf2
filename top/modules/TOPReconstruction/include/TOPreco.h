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

#include "TOPtrack.h"

namespace Belle2 {
  namespace TOP {

    /*! TOP reconstruction: this class provides interface to fortran code
     */

    enum {Local = 0, Global = 1};

    class TOPreco {
    public:
      /*! constructor
       */
      TOPreco(int NumHyp, double Masses[], double BkgPerQbar = 0, double ScaleN0 = 1);

      /*! modifiers
       */
      void SetHypID(int NumHyp, int HypID[]);

      /*! clear data list
       */
      void Clear();

      /*! add data
       */
      int AddData(int QbarID, int chID, int TDC);

      /*! get size of data list
       */
      int DataSize();

      /*! run reconstruction
       */
      void Reconstruct(double X, double Y, double Z, double Tlen,
                       double Px, double Py, double Pz, int Q, int HYP = 0);

      /*! run reconstruction
       */
      void Reconstruct(TOPtrack& trk);

      /*! get status (1=OK, 0=out of acceptance, -1=inside gap)
       */
      int Flag();

      /*! get log likelihoods, expected photons, measured photons
       */
      void GetLogL(int Size, double LogL[], double ExpNphot[], int& Nphot);

      /*! get track hit at Q-bar in Local or Global frame
       */
      void GetHit(int LocGlob, double R[3], double Dir[3], double& Len,
                  double& Tlen, double& Mom, int& QbarID);

      /*! print log likelihoods to std output
       */
      void DumpLogL(int NumHyp);

      /*! print track at TOP to std output
       */
      void DumpHit(int LocGlob);

      /*! get pulls: size
       */
      int PullSize();

      /*! get pulls: K-th pull
       */
      void GetPull(int K, double& T, double& T0, double& Wid, double& PhiCer,
                   double& Wt);

      /*! get PDF for channel chID at time t for mass hypothesis mass
       */
      double PDF(int chID, double t, double mass);

    private:
      int m_HYP;

    };

  } // top namespace
} // Belle2 namespace

#endif
