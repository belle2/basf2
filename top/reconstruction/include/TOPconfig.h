//*****************************************************************************
//-----------------------------------------------------------------------------
// Fast simulation and reconstruction package for TOP counter (F77 core)
// M. Staric, March-2009, Sept-2011, Avg-2013
//-----------------------------------------------------------------------------
//
// TOPconfig.h
// C++ interface to F77 functions: TOP counter configuration
// units: cm, ns, T, nm
//-----------------------------------------------------------------------------
//*****************************************************************************

#ifndef _TOPconfig_h
#define _TOPconfig_h

#include <top/reconstruction/TOPf77fun.h>
#include <cstring>

namespace Belle2 {
  namespace TOP {

    /**
     * exit window types
     */
    enum {None = 0, PMT, PlaneM, CylindricM, SphericM};

    /**
     * argument LR
     */
    enum {Left = 0, Right = 1};

    /**
     * expansion volume shapes
     */
    enum {NoRefl = 0, Box, Prism};

    /**
     * define TOP counter volume (must be called first)
     * @param R1 inner radius of TOP volume
     * @param R2 outer radius of TOP volume
     * @param Z1 backward border of TOP volume
     * @param Z2 forward border of TOP volume
     */
    inline void setTOPvolume(double R1, double R2, double Z1, double Z2)
    {
      float r1 = (float) R1; float r2 = (float) R2;
      float z1 = (float) Z1; float z2 = (float) Z2;
      set_topvol_(&r1, &r2, &z1, &z2);
    }

    /**
     * set magnetic field (use negative value to reverse polarity)
     * @param B magnetic field
     */
    inline void setBfield(double B)
    {
      float b = (float) B;
      set_bfield_(&b);
    }

    /**
     * set bar edge roughness (radius)
     * @param R roughness
     */
    inline void setEdgeRoughness(double R)
    {
      float r = (float) R;
      set_qbar_redg_(&r);
    }

    /**
     * define PMT dimensions (must be called prior to setQbar)
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
      float a = (float) A; float b = (float) B;
      float aa = (float) Asens; float bb = (float) Bsens;
      float tts = (float) TTS;
      set_pmt_(&a, &b, &aa, &bb, &Nx, &Ny, &tts);
    }

    /**
     * define TTS with multi-gaussian PDF
     * @param ng number of Gaussian terms
     * @param Frac fractions of Gaussian terms
     * @param Mean mean values of Gaussian terms
     * @param Sigma sigma values of Gaussian terms
     */
    inline void setTTS(int ng, double Frac[], double Mean[], double Sigma[])
    {
      float frac[ng], t0[ng], sig[ng];
      for (int i = 0; i < ng; i++) {
        frac[i] = (float) Frac[i];
        t0[i] = (float) Mean[i];
        sig[i] = (float) Sigma[i];
      }
      set_tts_(&ng, frac, t0, sig);
    }

    /**
     * read quantum efficencies from file
     * @param file file name
     * @param CE electron collection efficiency
     */
    inline void setQE(const char* file, double CE)
    {
      int len = strlen(file); float ce = (float) CE;
      read_qeffi_(file, &ce, len);
    }

    /**
     * set quantum efficiency
     * @param Wavelength wavelength values
     * @param QE quantum efficiency values
     * @param Size array size
     * @param CE electron collection efficiency
     */
    inline void setQE(double Wavelength[], double QE[], int Size, double CE)
    {
      float lam[Size], qef[Size];
      for (int i = 0; i < Size; i++) {
        lam[i] = (float) Wavelength[i];
        qef[i] = (float) QE[i];
      }
      float ce = (float) CE;
      set_qeffi_(lam, qef, &Size, &ce);
    }

    /**
     * define bar geometry; returns QbarID
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
      float a = (float) A; float b = (float) B;
      float z1 = (float) Z1; float z2 = (float) Z2; float r = (float) R;
      float dx = (float) Dx; float phi = (float) Phi;
      int id = set_qbar_(&a, &b, &z1, &z2, &r, &dx, &phi, &Lside, &Rside);
      return id;
    }

    /**
     * add QBB exit window
     * @param QbarID bar ID
     * @param thickness window thickness
     */
    inline void setBBoxWindow(int QbarID, double thickness)
    {
      float d = thickness;
      set_bbwin_(&QbarID, &d);
    }

    /**
     * add i-TOP expansion volume
     * @param QbarID bar ID
     * @param LR Left or Right
     * @param Shape shape (Box, Prism, etc.)
     * @param Dz length
     * @param Yup uppermost y
     * @param Ydown lowermost y
     * @param YupIn uppermost y at bar
     * @param YdownIn lowermost y at bar
     * @param Width width (0 means same width as bar)
     * @param DzRefl length of the reflective part at exit
     * @param Refl reflectivity of that part
     */
    inline void addExpansionVolume(int QbarID, int LR, int Shape, double Dz,
                                   double Yup, double Ydown,
                                   double YupIn = 0, double YdownIn = 0,
                                   double Width = 0, double DzRefl = 0, double Refl = 0)
    {
      float dz = Dz; float yup = Yup; float ydn = Ydown; float a = Width;
      float yupi = YupIn; float ydni = YdownIn;
      float dzrefl = DzRefl; float refl = Refl;
      set_extvol_(&QbarID, &LR, &Shape, &dz, &yup, &ydn, &a, &yupi, &ydni,
                  &dzrefl, &refl);
    }

    /**
     * re-arrange PMT's at (both) exit window(s)
     * @param QbarID bar ID
     * @param sizX box size in x
     * @param sizY box size in y
     * @param Dx box offset in x
     * @param Dy box offset in y
     */
    inline void arrangePMT(int QbarID, double sizX, double sizY,
                           double Dx = 0, double Dy = 0)
    {
      float sizx = (float) sizX; float sizy = (float) sizY;
      float dx = (float) Dx; float dy = (float) Dy;
      int LR = 0;
      arrange_pmt_(&QbarID, &LR, &sizx, &sizy, &dx, &dy);
      LR = 1;
      arrange_pmt_(&QbarID, &LR, &sizx, &sizy, &dx, &dy);
    }

    /**
     * re-arrange PMT's at exit window LR
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
      float sizx = (float) sizX; float sizy = (float) sizY;
      float dx = (float) Dx; float dy = (float) Dy;
      arrange_pmt_(&QbarID, &LR, &sizx, &sizy, &dx, &dy);
    }

    /**
     * set mirror radius
     * @param QbarID bar ID
     * @param R radius
     */
    inline void setMirrorRadius(int QbarID, double R)
    {
      float r = (float) R;
      set_rmi_(&QbarID, &r);
    }

    /**
     * set mirror center
     * @param QbarID bar ID
     * @param Xc center of curvature in x (local frame)
     * @param Yc center of curvature in y (local frame)
     */
    inline void setMirrorCenter(int QbarID, double Xc, double Yc)
    {
      float xc = (float) Xc; float yc = (float) Yc;
      set_xyc_(&QbarID, &xc, &yc);
    }

    /**
     * define TDC
     * @param NBIT   number of bits
     * @param ChWid  channel width
     * @param Offset offset
     */
    inline void setTDC(int NBIT, double ChWid, double Offset = 0)
    {
      float chwid = (float) ChWid; float offset = (float) Offset;
      set_tdc_(&NBIT, &chwid, &offset);
    }

    /**
     * define CFD
     * @param delTpileup pileup time
     * @param delTdoubleHit double hit resolution
     */
    inline void setCFD(double delTpileup, double delTdoubleHit)
    {
      float dt = delTpileup; float delt = delTdoubleHit;
      set_cfd_(&dt, &delt);
    }

    /**
     * finalize TOP configuration (must be called last)
     * @param Dump print configuration to std output
     * @return true if successfull
     */
    inline bool TOPfinalize(int Dump = 1)
    {
      return top_geo_finalize_(&Dump);
    }

  } // top namespace
} // Belle2 namespace

#endif

