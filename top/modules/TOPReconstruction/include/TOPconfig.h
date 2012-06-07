//*****************************************************************************
//-----------------------------------------------------------------------------
// Fast simulation and reconstruction package for TOP counter (F77 core)
// M. Staric, March-2009, Sept-2011
//-----------------------------------------------------------------------------
//
// TOPconfig.h
// C++ interface to F77 functions: TOP counter configuration
// units: cm, ns, T, nm
//-----------------------------------------------------------------------------
//*****************************************************************************

#ifndef _TOPconfig_h
#define _TOPconfig_h

#include "TOPf77fun.h"
#include <cstring>

namespace Belle2 {
  namespace TOP {

    /*! functions to configure TOP
     * provide interface to fortran code
     */

    /*! exit window types */
    enum {None = 0, PMT, PlaneM, CylindricM, SphericM};
    /*! argument LR */
    enum {Left = 0, Right = 1};
    /*! expansion volume shapes */
    enum {NoRefl = 0, Box, Prism};

    /*! define TOP counter volume (must be called first)
     * @param R1 inner radius of TOP volume
     * @param R2 outer radius of TOP volume
     * @param Z1 backward border of TOP volume
     * @param Z2 forward border of TOP volume
     */
    inline void TOPvolume(double R1, double R2, double Z1, double Z2)
    {
      float r1 = R1; float r2 = R2; float z1 = Z1; float z2 = Z2;
      set_topvol_(&r1, &r2, &z1, &z2);
    }

    /*! set magnetic field (use negative value to reverse polarity)
     * @param B magnetic field
     */
    inline void setBfield(double B)
    {
      float b = B;
      set_bfield_(&b);
    }

    /*! set bar edge roughness (radius)
     * @param R roughness
     */
    inline void setEdgeRoughness(double R)
    {
      float r = R;
      set_qbar_redg_(&r);
    }

    /*! define PMT dimensions (must be called prior to setQbar)
     * @param A size in x
     * @param B size in y
     * @param Asens sensitive area in x
     * @param Bsens sensitive area in y
     * @param Nx number of pads in x
     * @param Ny number of pads in y
     * @param TTS time resolution (rms)
     */
    inline void setPMT(double A, double B, double Asens, double Bsens,
                       int Nx, int Ny, double TTS = 50.e-3)
    {
      float a = A; float b = B; float aa = Asens; float bb = Bsens; float tts = TTS;
      set_pmt_(&a, &b, &aa, &bb, &Nx, &Ny, &tts);
    }

    /*! define TTS with multi-gaussian PDF
     * @param ng number of Gaussian terms
     * @param Frac fractions of Gaussian terms
     * @param Mean mean values of Gaussian terms
     * @param Sigma sigma values of Gaussian terms
     */
    inline void setTTS(int ng, double Frac[], double Mean[], double Sigma[])
    {
      float frac[ng], t0[ng], sig[ng];
      for (int i = 0; i < ng; i++) {
        frac[i] = Frac[i];
        t0[i] = Mean[i];
        sig[i] = Sigma[i];
      }
      set_tts_(&ng, frac, t0, sig);
    }

    /*! read quantum efficencies from file
     * @param file file name
     * @param CE electron collection efficiency
     */
    inline void setQE(const char* file, double CE)
    {
      int len = strlen(file); float ce = CE;
      read_qeffi_(file, &ce, len);
    }

    /*! set quantum efficiency
     * @param Wavelength wavelength values
     * @param QE quantum efficiency values
     * @param Size array size
     * @param CE electron collection efficiency
     */
    inline void setQE(double Wavelength[], double QE[], int Size, double CE)
    {
      float lam[Size], qef[Size];
      for (int i = 0; i < Size; i++) {
        lam[i] = Wavelength[i];
        qef[i] = QE[i];
      }
      float ce = CE;
      set_qeffi_(lam, qef, &Size, &ce);
    }

    /*! define bar geometry; returns QbarID
     * @param A bar width
     * @param B bar thickness
     * @param Z1 backward window position
     * @param Z2 forward window position
     * @param R inner radius
     * @param Dx shift along the bar frame x-axis
     * @param Phi angle in respect to Belle y-axis
     * @param Lside type of the left side exit window
     * @param Rside type of the right side exit window
     * @return bar ID
     */
    inline int setQbar(double A, double B, double Z1, double Z2, double R,
                       double Dx, double Phi, int Lside, int Rside)
    {
      float a = A; float b = B; float z1 = Z1; float z2 = Z2; float r = R;
      float dx = Dx; float phi = Phi;
      int id = set_qbar_(&a, &b, &z1, &z2, &r, &dx, &phi, &Lside, &Rside);
      return id;
    }

    /*! add i-TOP expansion volume
     * @param QbarID bar ID
     * @param LR Left or Right
     * @param Shape shape (Box, Prism, etc.)
     * @param Dz length
     * @param Yup uppermost y
     * @param Ydown lowermost y
     */
    inline void addExpansionVolume(int QbarID, int LR, int Shape, double Dz, double Yup,
                                   double Ydown)
    {
      float dz = Dz; float yup = Yup; float ydn = Ydown;
      set_extvol_(&QbarID, &LR, &Shape, &dz, &yup, &ydn);
    }

    /*! re-arrange PMT's at (both) exit window(s)
     * @param QbarID bar ID
     * @param sizX box size in x
     * @param sizY box size in y
     * @param Dx box offset in x
     * @param Dy box offset in y
     */
    inline void arrangePMT(int QbarID, double sizX, double sizY,
                           double Dx = 0, double Dy = 0)
    {
      float sizx = sizX; float sizy = sizY; float dx = Dx; float dy = Dy;
      int LR = 0;
      arrange_pmt_(&QbarID, &LR, &sizx, &sizy, &dx, &dy);
      LR = 1;
      arrange_pmt_(&QbarID, &LR, &sizx, &sizy, &dx, &dy);
    }

    /*! re-arrange PMT's at exit window LR
     * @param QbarID bar ID
     * @param LR Left or Right
     * @param sizX box size in x
     * @param sizY box size in y
     * @param Dx box offset in x
     * @param Dy box offset in y
     */
    inline void arrangePMT(int QbarID, int LR, double sizX, double sizY,
                           double Dx = 0, double Dy = 0)
    {
      float sizx = sizX; float sizy = sizY; float dx = Dx; float dy = Dy;
      arrange_pmt_(&QbarID, &LR, &sizx, &sizy, &dx, &dy);
    }

    /*! set mirror radius
     * @param QbarID bar ID
     * @param R radius
     */
    inline void setMirrorRadius(int QbarID, double R)
    {
      float r = R;
      set_rmi_(&QbarID, &r);
    }

    /*! set mirror center
     * @param QbarID bar ID
     * @param Xc center of curvature in x (local frame)
     * @param Yc center of curvature in y (local frame)
     */
    inline void setMirrorCenter(int QbarID, double Xc, double Yc)
    {
      float xc = Xc; float yc = Yc;
      set_xyc_(&QbarID, &xc, &yc);
    }

    /*! define TDC
     * @param NBIT   number of bits
     * @param ChWid  channel width
     * @param Offset offset
     */
    inline void setTDC(int NBIT, double ChWid, double Offset = 0)
    {
      float chwid = ChWid; float offset = Offset;
      set_tdc_(&NBIT, &chwid, &offset);
    }

    /*! finalize TOP configuration (must be called last)
     * @param Dump print configuration to std output
     */
    inline void TOPfinalize(int Dump = 1)
    {
      top_geo_finalize_(&Dump);
    }

  } // top namespace
} // Belle2 namespace

#endif

