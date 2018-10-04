/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/reconstruction/TOPf77fun.h>
#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPCalModuleAlignment.h>

extern "C" {void top_geo_dump_();}

namespace Belle2 {
  namespace TOP {

    /**
     * Configure TOP geometry for reconstruction: provides interface to fortran code
     */
    class TOPconfigure {
    public:

      /**
       * Exit window types
       */
      enum {c_None = 0, c_PMT, c_PlaneM, c_CylindricM, c_SphericM};

      /**
       * Argument LR
       */
      enum {c_Left = 0, c_Right = 1};

      /**
       * Expansion volume shapes
       */
      enum {c_NoRefl = 0, c_Box, c_Prism};

      /**
       * Constructor (which does the stuff if reconstruction geometry not configured yet)
       */
      TOPconfigure();

      /**
       * Passes alignment constants to FORTRAN code
       * function is used in call-back
       */
      static void setAlignmentConstants();

      /**
       * Defines TOP counter volume (must be called first)
       * @param R1 inner radius of TOP volume
       * @param R2 outer radius of TOP volume
       * @param Z1 backward border of TOP volume
       * @param Z2 forward border of TOP volume
       */
      void setTOPvolume(double R1, double R2, double Z1, double Z2)
      {
        float r1 = (float) R1; float r2 = (float) R2;
        float z1 = (float) Z1; float z2 = (float) Z2;
        set_topvol_(&r1, &r2, &z1, &z2);
      }

      /**
       * Sets magnetic field (use negative value to reverse polarity)
       * @param B magnetic field
       */
      void setBfield(double B)
      {
        float b = (float) B;
        set_bfield_(&b);
      }

      /**
       * Sets bar edge roughness (radius)
       * @param R roughness
       */
      void setEdgeRoughness(double R)
      {
        float r = (float) R;
        set_qbar_redg_(&r);
      }

      /**
       * Defines PMT dimensions (must be called prior to setQbar)
       * @param A size in x
       * @param B size in y
       * @param Asens sensitive area in x
       * @param Bsens sensitive area in y
       * @param Nx number of pads in x
       * @param Ny number of pads in y
       * @param TTS time resolution (rms)
       */
      void setPMT(double A, double B, double Asens, double Bsens,
                  int Nx, int Ny, double TTS = 50.e-3)
      {
        float a = (float) A; float b = (float) B;
        float aa = (float) Asens; float bb = (float) Bsens;
        float tts = (float) TTS;
        set_pmt_(&a, &b, &aa, &bb, &Nx, &Ny, &tts);
      }

      /**
       * Defines TTS with multi-gaussian PDF
       * @param ng number of Gaussian terms
       * @param Frac fractions of Gaussian terms
       * @param Mean mean values of Gaussian terms
       * @param Sigma sigma values of Gaussian terms
       */
      void setTTS(int ng, float Frac[], float Mean[], float Sigma[])
      {
        set_tts_(&ng, Frac, Mean, Sigma);
      }

      /**
       * Sets quantum efficiency
       * @param Wavelength wavelength values
       * @param QE quantum efficiency values
       * @param Size array size
       * @param CE electron collection efficiency
       */
      void setQE(float Wavelength[], float QE[], int Size, double CE)
      {
        float ce = (float) CE;
        set_qeffi_(Wavelength, QE, &Size, &ce);
      }

      /**
       * Defines module geometry; returns module ID
       * @param A bar width
       * @param B bar thickness
       * @param Z1 backward window position
       * @param Z2 forward window position
       * @param R inner radius
       * @param Dx shift along the module frame x-axis
       * @param Phi angle in respect to Belle y-axis
       * @param Lside type of the left side exit window
       * @param Rside type of the right side exit window
       * @return module ID
       */
      int setQbar(double A, double B, double Z1, double Z2, double R,
                  double Dx, double Phi, int Lside, int Rside)
      {
        float a = (float) A; float b = (float) B;
        float z1 = (float) Z1; float z2 = (float) Z2; float r = (float) R;
        float dx = (float) Dx; float phi = (float) Phi;
        int id = set_qbar_(&a, &b, &z1, &z2, &r, &dx, &phi, &Lside, &Rside);
        return id;
      }

      /**
       * Adds QBB exit window
       * @param moduleID module ID
       * @param thickness window thickness
       */
      void setBBoxWindow(int moduleID, double thickness)
      {
        float d = thickness;
        set_bbwin_(&moduleID, &d);
      }

      /**
       * Adds expansion volume (wedge)
       * @param moduleID module ID
       * @param LR Left or Right
       * @param Shape shape (c_Box, c_Prism, etc.)
       * @param Dz length
       * @param Yup uppermost y
       * @param Ydown lowermost y
       * @param YupIn uppermost y at bar
       * @param YdownIn lowermost y at bar
       * @param Width width (0 means same width as bar)
       * @param DzRefl length of the reflective part at exit
       * @param Refl reflectivity of that part
       */
      void addExpansionVolume(int moduleID, int LR, int Shape, double Dz,
                              double Yup, double Ydown,
                              double YupIn = 0, double YdownIn = 0,
                              double Width = 0, double DzRefl = 0, double Refl = 0)
      {
        float dz = Dz; float yup = Yup; float ydn = Ydown; float a = Width;
        float yupi = YupIn; float ydni = YdownIn;
        float dzrefl = DzRefl; float refl = Refl;
        set_extvol_(&moduleID, &LR, &Shape, &dz, &yup, &ydn, &a, &yupi, &ydni,
                    &dzrefl, &refl);
      }

      /**
       * Re-arranges PMT's at (both) exit window(s)
       * @param moduleID module ID
       * @param sizX box size in x
       * @param sizY box size in y
       * @param Dx box offset in x
       * @param Dy box offset in y
       */
      void arrangePMT(int moduleID, double sizX, double sizY,
                      double Dx = 0, double Dy = 0)
      {
        float sizx = (float) sizX; float sizy = (float) sizY;
        float dx = (float) Dx; float dy = (float) Dy;
        int LR = 0;
        arrange_pmt_(&moduleID, &LR, &sizx, &sizy, &dx, &dy);
        LR = 1;
        arrange_pmt_(&moduleID, &LR, &sizx, &sizy, &dx, &dy);
      }

      /**
       * Re-arranges PMT's at exit window LR
       * @param moduleID module ID
       * @param LR Left or Right
       * @param sizX box size in x
       * @param sizY box size in y
       * @param Dx box offset in x
       * @param Dy box offset in y
       */
      void arrangePMT(int moduleID, int LR, double sizX, double sizY,
                      double Dx = 0, double Dy = 0)
      {
        float sizx = (float) sizX; float sizy = (float) sizY;
        float dx = (float) Dx; float dy = (float) Dy;
        arrange_pmt_(&moduleID, &LR, &sizx, &sizy, &dx, &dy);
      }

      /**
       * Sets mirror radius
       * @param moduleID module ID
       * @param R radius
       */
      void setMirrorRadius(int moduleID, double R)
      {
        float r = (float) R;
        set_rmi_(&moduleID, &r);
      }

      /**
       * Sets mirror center
       * @param moduleID module ID
       * @param Xc center of curvature in x (local frame)
       * @param Yc center of curvature in y (local frame)
       */
      void setMirrorCenter(int moduleID, double Xc, double Yc)
      {
        float xc = (float) Xc; float yc = (float) Yc;
        set_xyc_(&moduleID, &xc, &yc);
      }

      /**
       * Defines TDC
       * @param NBIT   number of bits
       * @param ChWid  channel width
       * @param Offset offset
       */
      void setTDC(int NBIT, double ChWid, double Offset = 0)
      {
        float chwid = (float) ChWid; float offset = (float) Offset;
        set_tdc_(&NBIT, &chwid, &offset);
      }

      /**
       * Defines CFD
       * @param delTpileup pileup time
       * @param delTdoubleHit double hit resolution
       */
      void setCFD(double delTpileup, double delTdoubleHit)
      {
        float dt = delTpileup; float delt = delTdoubleHit;
        set_cfd_(&dt, &delt);
      }

      /**
       * Finalizes TOP configuration (must be called last)
       * @param Dump print configuration to std output
       * @return true if successfull
       */
      bool TOPfinalize(int Dump = 1)
      {
        return top_geo_finalize_(&Dump);
      }

      /**
       * Prints configuration to std output (using fortran print)
       */
      void print() const {top_geo_dump_();}

      /**
       * Returns configuration status
       * @return status
       */
      bool isConfigured() const {return m_configured;}

      /**
       * Returns inner radius of space for TOP modules
       * @return inner radius
       */
      double getR1() const {return m_R1;}

      /**
       * Returns outer radius of space for TOP modules
       * @return outer radius
       */
      double getR2() const {return m_R2;}

      /**
       * Returns minimal z of space for TOP modules
       * @return minimal z
       */
      double getZ1() const {return m_Z1;}

      /**
       * Returns maximal z of space for TOP modules
       * @return maximal z
       */
      double getZ2() const {return m_Z2;}

      /**
       * Returns time range of TDC
       * @return time range
       */
      double getTDCTimeRange() const {return m_timeRange;}

      /**
       * Returns module specs
       * @param moduleID module ID
       * @param A width
       * @param B thickness
       * @param C length
       * @param Lside left-side window type
       * @param Rside right-side window type
       * @param LexpvID left-side expansion volume ID or 0
       * @param RexpvID right-side expansion volume ID or 0
       */
      void getQbar(int moduleID, double& A, double& B, double& C,
                   int& Lside, int& Rside, int& LexpvID, int& RexpvID)
      {
        moduleID--;
        float a, b, c;
        get_qbar_(&moduleID, &a, &b, &c, &Lside, &Rside, &LexpvID, &RexpvID);
        A = a; B = b; C = c;
      }

      /**
       * Returns expansion volume (wedge) specs
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
      void getExpansionVolume(int expvID, double& Dz, double& Yup, double& Ydown,
                              double& YupIn, double& YdownIn,
                              double& Width, int& Shape, int& LR)
      {
        float dz, yup, ydn, a, yupi, ydni;
        get_expvol_(&expvID, &dz, &yup, &ydn, &yupi, &ydni, &a, &Shape, &LR);
        Dz = dz; Yup = yup; Ydown = ydn; YupIn = yupi; YdownIn = ydni; Width = a;
      }

      /**
       * Returns mirror specs
       * @param moduleID module ID
       * @param R radius
       * @param Xc center of curvature in x (local frame)
       * @param Yc center of curvature in y (local frame)
       * @param Zc center of curvature in z (local frame)
       * @param F effective focal length
       */
      void getMirror(int moduleID, double& R, double& Xc, double& Yc, double& Zc,
                     double& F)
      {
        moduleID--;
        float r, xc, yc, zc, f;
        get_mirror_(&moduleID, &r, &xc, &yc, &zc, &f);
        R = r; Xc = xc; Yc = yc; Zc = zc; F = f;
      }

      /**
       * Returns thickness of the bar box window
       * @param moduleID module ID
       * @return thickness
       */
      double getBBoxWindow(int moduleID)
      {
        moduleID--;
        return get_bbwin_(&moduleID);
      }

    private:
      static bool m_configured; /**< set to true when done */
      static DBObjPtr<TOPCalModuleAlignment> m_alignment; /**< alignment constants */
      double m_R1 = 0;   /**< inner radius */
      double m_R2 = 0;   /**< outer radius */
      double m_Z1 = 0;   /**< backward z */
      double m_Z2 = 0;   /**< forward z */
      double m_timeRange = 0; /**< time range of TDC */

    };

  } // top namespace
} // Belle2 namespace

