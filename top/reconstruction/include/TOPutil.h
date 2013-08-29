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

#include <top/reconstruction/TOPf77fun.h>

namespace Belle2 {
  namespace TOP {

    /**
     * returns channel ID from position in local frame of QbarID
     * @param X x position (local frame)
     * @param Y y position (local frame)
     * @param Z z position (local frame)
     * @param QbarID bar ID
     * @return channel ID
     */
    inline int chID(double X, double Y, double Z, int QbarID)
    {
      float x = (float) X; float y = (float) Y; float z = (float) Z;
      return ich_digiz_(&x, &y, &z, &QbarID);
    }

    /**
     * returns channel ID from position in local frame of QbarID
     * @param X x position (local frame)
     * @param Y y position (local frame)
     * @param LR Left or Right
     * @param QbarID bar ID
     * @return channel ID
     */
    inline int chID(double X, double Y, int LR, int QbarID)
    {
      float x = (float) X; float y = (float) Y;
      return ich_digi_(&x, &y, &LR, &QbarID);
    }

    /**
     * returns channel ID from PMTchannel, PMTid, window number, QbarID
     * @param IX PMT channel column
     * @param IY PMT channel row
     * @param PMTidX PMT column
     * @param PMTidY PMT row
     * @param LR Left or Right
     * @param QbarID bar ID
     * @return channel ID
     */
    inline int chID(int IX, int IY, int PMTidX, int PMTidY, int LR, int QbarID)
    {
      return ichan_(&IX, &IY, &PMTidX, &PMTidY, &LR, &QbarID);
    }

    /**
     * converts channel ID of QbarID to PMTchannel, PMTid, window number
     * @param chID channel ID
     * @param QbarID bar ID
     * @param IX PMT channel column
     * @param IY PMT channel row
     * @param PMTidX PMT column
     * @param PMTidY PMT row
     * @param LR Left or Right
     */
    inline void chIDtoIII(int chID, int QbarID,
                          int& IX, int& IY, int& PMTidX, int& PMTidY, int& LR)
    {
      ichiii_(&chID, &QbarID, &IX, &IY, &PMTidX, &PMTidY, &LR);
    }

    /**
     * converts channel ID to position in local frame of QbarID
     * @param chID channel ID
     * @param QbarID bar ID
     * @param X x position (local frame)
     * @param Y y position (local frame)
     * @param Z z position (local frame)
     */
    inline void chIDtoXYZ(int chID, int QbarID,
                          double& X, double& Y, double& Z)
    {
      float x, y, z;
      ichxyz_(&chID, &QbarID, &x, &y, &z);
      X = x; Y = y; Z = z;
    }

    /**
     * returns digitised time
     * @param T time
     * @return digitized time
     */
    inline int TDCdigi(double T)
    {
      float t = (float) T;
      return tdc_digi_(&t);
    }

    /**
     * returns time in nanoseconds from digitised value
     * @param TDC digitized time
     * @return time
     */
    inline double TIME(int TDC)
    {
      return tdc_to_t_(&TDC);
    }

    /**
     * transformation of 3D point from Belle to Qbar frame
     * @param QbarID bar ID
     * @param X x position (global frame)
     * @param Y y position (global frame)
     * @param Z z position (global frame)
     * @param x x position (local frame)
     * @param y y position (local frame)
     * @param z z position (local frame)
     */
    inline void PointToLocal(int QbarID, double X, double Y, double Z,
                             double& x, double& y, double& z)
    {
      float r[3] = {(float) X, (float) Y, (float) Z};
      float dir[3] = {0, 0, 0};
      to_local_(r, dir, &QbarID);
      x = r[0]; y = r[1]; z = r[2];
    }

    /**
     * transformation of 3D point from Qbar to Belle frame
     * @param
     * @param QbarID bar ID
     * @param X x position (local frame)
     * @param Y y position (local frame)
     * @param Z z position (local frame)
     * @param x x position (global frame)
     * @param y y position (global frame)
     * @param z z position (global frame)
     */
    inline void PointToGlobal(int QbarID, double X, double Y, double Z,
                              double& x, double& y, double& z)
    {
      float r[3] = {(float) X, (float) Y, (float) Z};
      float dir[3] = {0, 0, 0};
      to_global_(r, dir, &QbarID);
      x = r[0]; y = r[1]; z = r[2];
    }

    /**
     * transformation of momentum vector from Belle to Qbar frame
     * @param QbarID bar ID
     * @param Px momentum component x (global frame)
     * @param Py momentum component y (global frame)
     * @param Pz momentum component z (global frame)
     * @param px momentum component x (local frame)
     * @param py momentum component y (local frame)
     * @param pz momentum component z (local frame)
     */
    inline void MomentumToLocal(int QbarID, double Px, double Py, double Pz,
                                double& px, double& py, double& pz)
    {
      float r[3] = {0, 0, 0};
      float dir[3] = {(float) Px, (float) Py, (float) Pz};
      to_local_(r, dir, &QbarID);
      px = dir[0]; py = dir[1]; pz = dir[2];
    }

    /**
     * transformation of momentum vector from Qbar to Belle frame
     * @param QbarID bar ID
     * @param Px momentum component x (local frame)
     * @param Py momentum component y (local frame)
     * @param Pz momentum component z (local frame)
     * @param px momentum component x (global frame)
     * @param py momentum component y (global frame)
     * @param pz momentum component z (global frame)
     */
    inline void MomentumToGlobal(int QbarID, double Px, double Py, double Pz,
                                 double& px, double& py, double& pz)
    {
      float r[3] = {0, 0, 0};
      float dir[3] = {(float) Px, (float) Py, (float) Pz};
      to_global_(r, dir, &QbarID);
      px = dir[0]; py = dir[1]; pz = dir[2];
    }

    /**
     * test if spatial point is inside bar (point given in Qbar frame)
     * @param X x position (local frame)
     * @param Y y position (local frame)
     * @param Z z position (local frame)
     * @param QbarID bar ID
     * @return true, if inside
     */
    inline bool InsideQbar(double X, double Y, double Z, int QbarID)
    {
      float x = (float) X; float y = (float) Y; float z = (float) Z;
      return inside_qbar_(&x, &y, &z, &QbarID) != 0;
    }

    /**
     * test if spatial point is inside expansion volume (point given in Qbar frame)
     * @param X x position (local frame)
     * @param Y y position (local frame)
     * @param Z z position (local frame)
     * @param QbarID bar ID
     * @return true, if inside
     */
    inline bool InsideExpansionVolume(double X, double Y, double Z, int QbarID)
    {
      float x = (float) X; float y = (float) Y; float z = (float) Z;
      return inside_extvol_(&x, &y, &z, &QbarID) != 0;
    }

    /**
     * returns phase refractive index at photon energy E [eV]
     * @param E photon energy in eV
     * @return phase index
     */
    inline double PhaseIndex(double E)
    {
      float e = (float) E;
      return phase_index_(&e);
    }

    /**
     * returns mean phase refractive index
     * @return mean phase index
     */
    inline double PhaseIndex()
    {
      float refind, gruind;
      get_top_par_(&refind, &gruind);
      return refind;
    }

    /**
     * returns group refractive index at photon energy E [eV]
     * @param E photon energy in eV
     * @return group index
     */
    inline double GroupIndex(double E)
    {
      float e = (float) E;
      return group_index_(&e);
    }

    /**
     * returns mean group refractive index
     * @return mean group index
     */
    inline double GroupIndex()
    {
      float refind, gruind;
      get_top_par_(&refind, &gruind);
      return gruind;
    }

    /**
     * returns atenuation length [cm] at photon energy E [eV]
     * @param E photon energy in eV
     * @return attenuation length
     */
    inline double AtenuLength(double E)
    {
      float e = (float) E;
      return abspt_leng_(&e);
    }

    /**
     * returns mirror reflectivity at photon energy E [eV]
     * @param E photon energy in eV
     * @return mirror reflectivity
     */
    inline double MirrorRefl(double E)
    {
      float e = (float) E;
      return mirror_refl_(&e);
    }

    /**
     * returns quantum efficiency times collection efficiency at photon energy E [eV]
     * @param E photon energy in eV
     * @return quantum efficiency times collection efficiency
     */
    inline double QEffi(double E)
    {
      float e = (float) E;
      return qe_pmt_(&e);
    }

    /**
     * returns energy range of nonzero quantum efficiency
     * @param Emin lower bound in eV
     * @param Emax upper bound in eV
     */
    inline void QErange(double& Emin, double& Emax)
    {
      float emin, emax;
      qe_range_(&emin, &emax);
      Emin = emin; Emax = emax;
    }

    /**
     * returns max of quantum efficiency times collection efficiency
     * @return maximum quantum efficiency
     */
    inline double QEmax()
    {
      return qe_max_();
    }

    /**
     * returns figure-of-merit N0
     * @return N0
     */
    inline double Nzero()
    {
      return qe_nzero_();
    }

    /**
     * returns Cerenkov angle
     * @param P particle momentum
     * @param Mass particle mass
     * @param RefrIndex refractive index
     * @return Cerenkov angle
     */
    inline double CerAngle(double P, double Mass, double RefrIndex)
    {
      float p = (float) P; float mass = (float) Mass;
      float refind = (float) RefrIndex;
      return cher_angle_(&p, &mass, &refind);
    }

    /**
     * returns Qbar specs
     * @param QbarID bar ID
     * @param A width
     * @param B thickness
     * @param C length
     * @param Lside left-side window type
     * @param Rside right-side window type
     * @param LexpvID left-side expansion volume ID or 0
     * @param RexpvID right-side expansion volume ID or 0
     */
    inline void getQbar(int QbarID, double& A, double& B, double& C,
                        int& Lside, int& Rside, int& LexpvID, int& RexpvID)
    {
      float a, b, c;
      get_qbar_(&QbarID, &a, &b, &c, &Lside, &Rside, &LexpvID, &RexpvID);
      A = a; B = b; C = c;
    }

    /**
     * returns expansion volume specs
     * @param expvID expansion volume ID
     * @param Dz length
     * @param Yup uppermost y
     * @param Ydown lowermost y
     * @param YupIn uppermost y at bar
     * @param YdownIn lowermost y at bar
     * @param Width width
     * @param Shape shape
     * @param LR Left or Right
     */
    inline void getExpansionVolume(int expvID, double& Dz, double& Yup, double& Ydown,
                                   double& YupIn, double& YdownIn,
                                   double& Width, int& Shape, int& LR)
    {
      float dz, yup, ydn, a, yupi, ydni;
      get_expvol_(&expvID, &dz, &yup, &ydn, &yupi, &ydni, &a, &Shape, &LR);
      Dz = dz; Yup = yup; Ydown = ydn; YupIn = yupi; YdownIn = ydni; Width = a;
    }

    /**
     * returns mirror specs
     * @param QbarID bar ID
     * @param R radius
     * @param Xc center of curvature in x (local frame)
     * @param Yc center of curvature in y (local frame)
     * @param Zc center of curvature in z (local frame)
     * @param F effective focal length
     */
    inline void getMirror(int QbarID, double& R, double& Xc, double& Yc, double& Zc,
                          double& F)
    {
      float r, xc, yc, zc, f;
      get_mirror_(&QbarID, &r, &xc, &yc, &zc, &f);
      R = r; Xc = xc; Yc = yc; Zc = zc; F = f;
    }

    /**
     * returns thickness of bar box window
     * @param QbarID bar ID
     * @return thickness
     */
    inline double getBBoxWindow(int QbarID)
    {
      return get_bbwin_(&QbarID);
    }

  } // top namespace
} // Belle2 namespace


#endif

