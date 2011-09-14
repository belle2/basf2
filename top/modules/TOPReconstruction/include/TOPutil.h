//*****************************************************************************
//-----------------------------------------------------------------------------
// Fast simulation and reconstruction package for TOP counter (F77 core)
// M. Staric, March-2009
//-----------------------------------------------------------------------------
//
// TOPutil.h
// C++ interface to F77 functions: data conversion utility (for debugging etc.)
// units: cm, ns
//-----------------------------------------------------------------------------
//*****************************************************************************

#ifndef _TOPutil_h
#define _TOPutil_h

#include "TOPf77fun.h"

// returns channel ID from position in local frame of QbarID
inline int chID(double X, double Y, double Z, int QbarID)
{
  float x = X; float y = Y; float z = Z;
  return ich_digiz_(&x, &y, &z, &QbarID);
}

// returns channel ID from position in local frame of QbarID
inline int chID(double X, double Y, int LR, int QbarID)
{
  float x = X; float y = Y;
  return ich_digi_(&x, &y, &LR, &QbarID);
}

// returns channel ID from PMTchannel, PMTid, window number, QbarID
inline int chID(int IX, int IY, int PMTidX, int PMTidY, int LR, int QbarID)
{
  return ichan_(&IX, &IY, &PMTidX, &PMTidY, &LR, &QbarID);
}

// converts channel ID of QbarID to PMTchannel, PMTid, window number
inline void chIDtoIII(int chID, int QbarID,
                      int & IX, int & IY, int & PMTidX, int & PMTidY, int & LR)
{
  ichiii_(&chID, &QbarID, &IX, &IY, &PMTidX, &PMTidY, &LR);
}

// converts channel ID to position in local frame of QbarID
inline void chIDtoXYZ(int chID, int QbarID,
                      double & X, double & Y, double & Z)
{
  float x, y, z;
  ichxyz_(&chID, &QbarID, &x, &y, &z);
  X = x; Y = y; Z = z;
}

// returns digitised time
inline int TDCdigi(double T)
{
  float t = T;
  return tdc_digi_(&t);
}

// returns time in nanoseconds from digitised value
inline double TIME(int TDC)
{
  return tdc_to_t_(&TDC);
}

// transformation of 3D point from Belle to Qbar frame
inline void PointToLocal(int QbarID, double X, double Y, double Z,
                         double & x, double & y, double & z)
{
  float r[3] = {X, Y, Z};
  float dir[3] = {0, 0, 0};
  to_local_(r, dir, &QbarID);
  x = r[0]; y = r[1]; z = r[2];
}

// transformation of 3D point from Qbar to Belle frame
inline void PointToGlobal(int QbarID, double X, double Y, double Z,
                          double & x, double & y, double & z)
{
  float r[3] = {X, Y, Z};
  float dir[3] = {0, 0, 0};
  to_global_(r, dir, &QbarID);
  x = r[0]; y = r[1]; z = r[2];
}

// transformation of momentum vector from Belle to Qbar frame
inline void MomentumToLocal(int QbarID, double Px, double Py, double Pz,
                            double & px, double & py, double & pz)
{
  float r[3] = {0, 0, 0};
  float dir[3] = {Px, Py, Pz};
  to_local_(r, dir, &QbarID);
  px = dir[0]; py = dir[1]; pz = dir[2];
}

// transformation of momentum vector from Qbar to Belle frame
inline void MomentumToGlobal(int QbarID, double Px, double Py, double Pz,
                             double & px, double & py, double & pz)
{
  float r[3] = {0, 0, 0};
  float dir[3] = {Px, Py, Pz};
  to_global_(r, dir, &QbarID);
  px = dir[0]; py = dir[1]; pz = dir[2];
}

// test if point is inside bar; point should be given in Qbar frame
inline bool InsideQbar(double X, double Y, double Z, int QbarID)
{
  float x = X; float y = Y; float z = Z;
  return inside_qbar_(&x, &y, &z, &QbarID) != 0;
}

// test if point is inside expansion volume; point should be given in Qbar frame
inline bool InsideExpansionVolume(double X, double Y, double Z, int QbarID)
{
  float x = X; float y = Y; float z = Z;
  return inside_extvol_(&x, &y, &z, &QbarID) != 0;
}

// returns phase refractive index at photon energy E [eV]
inline double PhaseIndex(double E)
{
  float e = E;
  return phase_index_(&e);
}

// returns mean phase refractive index
inline double PhaseIndex()
{
  float refind, gruind;
  get_top_par_(&refind, &gruind);
  return refind;
}

// returns group refractive index at photon energy E [eV]
inline double GroupIndex(double E)
{
  float e = E;
  return group_index_(&e);
}

// returns mean group refractive index
inline double GroupIndex()
{
  float refind, gruind;
  get_top_par_(&refind, &gruind);
  return gruind;
}

// returns atenuation length [cm] at photon energy E [eV]
inline double AtenuLength(double E)
{
  float e = E;
  return abspt_leng_(&e);
}

// returns mirror reflectivity at photon energy E [eV]
inline double MirrorRefl(double E)
{
  float e = E;
  return mirror_refl_(&e);
}

// returns quantum efficiency times collection efficiency at photon energy E [eV]
inline double QEffi(double E)
{
  float e = E;
  return qe_pmt_(&e);
}

// returns energy range of nonzero quantum efficiency
inline void QErange(double & Emin, double & Emax)
{
  float emin, emax;
  qe_range_(&emin, &emax);
  Emin = emin; Emax = emax;
}

// returns max of quantum efficiency times collection efficiency
inline double QEmax()
{
  return qe_max_();
}

// returns figure-of-merit N0
inline double Nzero()
{
  return qe_nzero_();
}

// returns Cerenkov angle
inline double CerAngle(double P, double Mass, double RefrIndex)
{
  float p = P; float mass = Mass; float refind = RefrIndex;
  return cher_angle_(&p, &mass, &refind);
}

// returns Qbar data: dimensions (A,B,C [cm]), exit window types (Lside,Rside),
//                    expansion volume ID's (LexpvID,RexpvID)
inline void getQbar(int QbarID, double & A, double & B, double & C,
                    int & Lside, int & Rside, int & LexpvID, int & RexpvID)
{
  float a, b, c;
  get_qbar_(&QbarID, &a, &b, &c, &Lside, &Rside, &LexpvID, &RexpvID);
  A = a; B = b; C = c;
}

// returns expansion volume data
inline void getExpansionVolume(int expvID, double & Dz, double & Yup, double & Ydown,
                               int & Shape, int & LR)
{
  float dz, yup, ydn;
  get_expvol_(&expvID, &dz, &yup, &ydn, &Shape, &LR);
  Dz = dz; Yup = yup; Ydown = ydn;
}

// returns mirror data: radius of curvature (R [cm]),
//                      center of curvature (Xc,Yc,Zc [cm]) given in Qbar frame
//                      focal length used within linear optics (F)
inline void getMirror(int QbarID, double & R, double & Xc, double & Yc, double & Zc,
                      double & F)
{
  float r, xc, yc, zc, f;
  get_mirror_(&QbarID, &r, &xc, &yc, &zc, &f);
  R = r; Xc = xc; Yc = yc; Zc = zc; F = f;
}

#endif

