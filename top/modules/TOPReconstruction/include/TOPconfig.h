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


enum {None = 0, PMT, PlaneM, CylindricM, SphericM}; // exit window types
enum {Left = 0, Right = 1}; // argument LR
enum {NoRefl = 0, Box, Prism};  // expansion volume shapes


// define TOP counter volume (must be called first)
// R1, R2: inner/outer radius [cm]; Z1, Z2: min/max z [cm]

inline void TOPvolume(double R1, double R2, double Z1, double Z2)
{
  float r1 = R1; float r2 = R2; float z1 = Z1; float z2 = Z2;
  set_topvol_(&r1, &r2, &z1, &z2);
}

// set magnetic field [T]
// use negative value to reverse the polarity
// B=0 is also possible now

inline void setBfield(double B)
{
  float b = B;
  set_bfield_(&b);
}

// set Q-bar edge roughness (radius)

inline void setEdgeRoughness(double R)
{
  float r = R;
  set_qbar_redg_(&r);
}

// define PMT dimensions (must be called prior to setQbar)
// A, B: dimensions in x and y [cm]
// Asens, Bsens: sensitive area [cm]
// Nx, Ny: number of channels in x and y
// TTS: time resolution (sigma) [ns] for (one) gaussian PDF

inline void setPMT(double A, double B, double Asens, double Bsens,
                   int Nx, int Ny, double TTS = 50.e-3)
{
  float a = A; float b = B; float aa = Asens; float bb = Bsens; float tts = TTS;
  set_pmt_(&a, &b, &aa, &bb, &Nx, &Ny, &tts);
}

// define TTS with multi-gaussian PDF
// ng: num. of Gaussains; Frac: fractions; Mean: means [ns]; Sigma: sigmas [ns]

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

// read quantum efficency from file
// CE: electron collection efficiency

inline void setQE(const char *file, double CE)
{
  int len = strlen(file); float ce = CE;
  read_qeffi_(file, &ce, len);
}

// set quantum efficiency
// CE: electron collection efficiency

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

// define Qbar geometry; returns QbarID
// A, B: width and thickness [cm]
// Z1, Z2: exit window positions in Belle frame (Z1<Z2) [cm]
// R: radius [cm]
// Dx: shift along the Q-bar frame x-axis
// Phi: angle in respect to Belle y-axis [radians]
// Lside, Rside: types of the left/right exit windows, see enum{} above

inline int setQbar(double A, double B, double Z1, double Z2, double R,
                   double Dx, double Phi, int Lside, int Rside)
{
  float a = A; float b = B; float z1 = Z1; float z2 = Z2; float r = R;
  float dx = Dx; float phi = Phi;
  int id = set_qbar_(&a, &b, &z1, &z2, &r, &dx, &phi, &Lside, &Rside);
  return id;
}

// add i-TOP expansion volume

inline void addExpansionVolume(int QbarID, int LR, int Shape, double Dz, double Yup,
                               double Ydown)
{
  float dz = Dz; float yup = Yup; float ydn = Ydown;
  set_extvol_(&QbarID, &LR, &Shape, &dz, &yup, &ydn);
}

// re-arrange PMT's at (both) exit window(s)

inline void arrangePMT(int QbarID, double sizX, double sizY, double Dx = 0, double Dy = 0)
{
  float sizx = sizX; float sizy = sizY; float dx = Dx; float dy = Dy;
  int LR = 0;
  arrange_pmt_(&QbarID, &LR, &sizx, &sizy, &dx, &dy);
  LR = 1;
  arrange_pmt_(&QbarID, &LR, &sizx, &sizy, &dx, &dy);
}

// re-arrange PMT's at exit window LR

inline void arrangePMT(int QbarID, int LR, double sizX, double sizY,
                       double Dx = 0, double Dy = 0)
{
  float sizx = sizX; float sizy = sizY; float dx = Dx; float dy = Dy;
  arrange_pmt_(&QbarID, &LR, &sizx, &sizy, &dx, &dy);
}

// set mirror radius [cm]

inline void setMirrorRadius(int QbarID, double R)
{
  float r = R;
  set_rmi_(&QbarID, &r);
}

// set mirror center to Xc, Yc [cm] (Qbar frame)

inline void setMirrorCenter(int QbarID, double Xc, double Yc)
{
  float xc = Xc; float yc = Yc;
  set_xyc_(&QbarID, &xc, &yc);
}

// define TDC
// NBIT: number of bits; ChWid: channel width [ns]; Offset: offset [ns]

inline void setTDC(int NBIT, double ChWid, double Offset = 0)
{
  float chwid = ChWid; float offset = Offset;
  set_tdc_(&NBIT, &chwid, &offset);
}

// finalize TOP configuration (must be called last)

inline void TOPfinalize(int Dump = 1)
{
  top_geo_finalize_(&Dump);
}

#endif

