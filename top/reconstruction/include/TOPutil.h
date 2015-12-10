//*****************************************************************************
//-----------------------------------------------------------------------------
// Fast simulation and reconstruction package for TOP counter (F77 core)
// M. Staric, March-2009
//-----------------------------------------------------------------------------
//
// TOPutil.h
// C++ interface to F77 functions: data conversion utility (for debugging etc.)
// units: cm, ns
// mostly opsolete now !
//-----------------------------------------------------------------------------
//*****************************************************************************

#ifndef _TOPutil_h
#define _TOPutil_h

#include <top/reconstruction/TOPf77fun.h>

namespace Belle2 {
  namespace TOP {

    /**
     * transformation of 3D point from Belle to TOP module frame
     * @param moduleID module ID
     * @param X x position (global frame)
     * @param Y y position (global frame)
     * @param Z z position (global frame)
     * @param x x position (local frame)
     * @param y y position (local frame)
     * @param z z position (local frame)
     */
    inline void PointToLocal(int moduleID, double X, double Y, double Z,
                             double& x, double& y, double& z)
    {
      moduleID--;
      float r[3] = {(float) X, (float) Y, (float) Z};
      float dir[3] = {0, 0, 0};
      to_local_(r, dir, &moduleID);
      x = r[0]; y = r[1]; z = r[2];
    }

    /**
     * transformation of 3D point from TOP module to Belle frame
     * @param
     * @param moduleID module ID
     * @param X x position (local frame)
     * @param Y y position (local frame)
     * @param Z z position (local frame)
     * @param x x position (global frame)
     * @param y y position (global frame)
     * @param z z position (global frame)
     */
    inline void PointToGlobal(int moduleID, double X, double Y, double Z,
                              double& x, double& y, double& z)
    {
      moduleID--;
      float r[3] = {(float) X, (float) Y, (float) Z};
      float dir[3] = {0, 0, 0};
      to_global_(r, dir, &moduleID);
      x = r[0]; y = r[1]; z = r[2];
    }

    /**
     * transformation of momentum vector from Belle to TOP module frame
     * @param moduleID module ID
     * @param Px momentum component x (global frame)
     * @param Py momentum component y (global frame)
     * @param Pz momentum component z (global frame)
     * @param px momentum component x (local frame)
     * @param py momentum component y (local frame)
     * @param pz momentum component z (local frame)
     */
    inline void MomentumToLocal(int moduleID, double Px, double Py, double Pz,
                                double& px, double& py, double& pz)
    {
      moduleID--;
      float r[3] = {0, 0, 0};
      float dir[3] = {(float) Px, (float) Py, (float) Pz};
      to_local_(r, dir, &moduleID);
      px = dir[0]; py = dir[1]; pz = dir[2];
    }

    /**
     * transformation of momentum vector from TOP module to Belle frame
     * @param moduleID module ID
     * @param Px momentum component x (local frame)
     * @param Py momentum component y (local frame)
     * @param Pz momentum component z (local frame)
     * @param px momentum component x (global frame)
     * @param py momentum component y (global frame)
     * @param pz momentum component z (global frame)
     */
    inline void MomentumToGlobal(int moduleID, double Px, double Py, double Pz,
                                 double& px, double& py, double& pz)
    {
      moduleID--;
      float r[3] = {0, 0, 0};
      float dir[3] = {(float) Px, (float) Py, (float) Pz};
      to_global_(r, dir, &moduleID);
      px = dir[0]; py = dir[1]; pz = dir[2];
    }

    /**
     * test if spatial point is inside quartz bar (point given in TOP module frame)
     * @param X x position (local frame)
     * @param Y y position (local frame)
     * @param Z z position (local frame)
     * @param moduleID module ID
     * @return true, if inside
     */
    inline bool InsideQbar(double X, double Y, double Z, int moduleID)
    {
      moduleID--;
      float x = (float) X; float y = (float) Y; float z = (float) Z;
      return inside_qbar_(&x, &y, &z, &moduleID) != 0;
    }

    /**
     * test if spatial point is inside expansion volume (point given in Qbar frame)
     * @param X x position (local frame)
     * @param Y y position (local frame)
     * @param Z z position (local frame)
     * @param moduleID module ID
     * @return true, if inside
     */
    inline bool InsideExpansionVolume(double X, double Y, double Z, int moduleID)
    {
      moduleID--;
      float x = (float) X; float y = (float) Y; float z = (float) Z;
      return inside_extvol_(&x, &y, &z, &moduleID) != 0;
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


  } // top namespace
} // Belle2 namespace


#endif

