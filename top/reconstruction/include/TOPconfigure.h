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

#include <framework/database/DBObjPtr.h>
#include <framework/database/DBArray.h>
#include <top/dbobjects/TOPCalModuleAlignment.h>
#include <top/dbobjects/TOPPmtInstallation.h>

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
       * Sets PMT types pixel table
       */
      static void setPMTTypes();

      /**
       * Defines TOP counter volume (must be called first)
       * @param R1 inner radius of TOP volume
       * @param R2 outer radius of TOP volume
       * @param Z1 backward border of TOP volume
       * @param Z2 forward border of TOP volume
       */
      void setTOPvolume(double R1, double R2, double Z1, double Z2);

      /**
       * Sets magnetic field (use negative value to reverse polarity)
       * @param B magnetic field
       */
      void setBfield(double B);

      /**
       * Sets bar edge roughness (radius)
       * @param R roughness
       */
      void setEdgeRoughness(double R);

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
                  int Nx, int Ny, double TTS = 50.e-3);

      /**
       * Defines TTS with multi-gaussian PDF
       * @param ng number of Gaussian terms
       * @param Frac fractions of Gaussian terms
       * @param Mean mean values of Gaussian terms
       * @param Sigma sigma values of Gaussian terms
       */
      void setTTS(int ng, float Frac[], float Mean[], float Sigma[]);

      /**
       * Sets PMT dependent TTS-es
       */
      void setPMTDependentTTSes();

      /**
       * Sets quantum efficiency
       * @param Wavelength wavelength values
       * @param QE quantum efficiency values
       * @param Size array size
       * @param CE electron collection efficiency
       */
      void setQE(float Wavelength[], float QE[], int Size, double CE);

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
                  double Dx, double Phi, int Lside, int Rside);

      /**
       * Adds QBB exit window
       * @param moduleID module ID (0 based, e.g. slot-1)
       * @param thickness window thickness
       */
      void setBBoxWindow(int moduleID, double thickness);

      /**
       * Adds expansion volume (wedge)
       * @param moduleID module ID (0 based, e.g. slot-1)
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
                              double Width = 0, double DzRefl = 0, double Refl = 0);

      /**
       * Re-arranges PMT's at (both) exit window(s)
       * @param moduleID module ID (0 based, e.g. slot-1)
       * @param sizX box size in x
       * @param sizY box size in y
       * @param Dx box offset in x
       * @param Dy box offset in y
       */
      void arrangePMT(int moduleID, double sizX, double sizY,
                      double Dx = 0, double Dy = 0);

      /**
       * Re-arranges PMT's at exit window LR
       * @param moduleID module ID (0 based, e.g. slot-1)
       * @param LR Left or Right
       * @param sizX box size in x
       * @param sizY box size in y
       * @param Dx box offset in x
       * @param Dy box offset in y
       */
      void arrangePMT(int moduleID, int LR, double sizX, double sizY,
                      double Dx = 0, double Dy = 0);

      /**
       * Sets mirror radius
       * @param moduleID module ID (0 based, e.g. slot-1)
       * @param R radius
       */
      void setMirrorRadius(int moduleID, double R);

      /**
       * Sets mirror center
       * @param moduleID module ID (0 based, e.g. slot-1)
       * @param Xc center of curvature in x (local frame)
       * @param Yc center of curvature in y (local frame)
       */
      void setMirrorCenter(int moduleID, double Xc, double Yc);

      /**
       * Sets mirror reflectivity
       * @param moduleID module ID (0 based, e.g. slot-1)
       * @param reflectivity reflectivity (absolute value)
       */
      void setMirrorReflectivity(int moduleID, double reflectivity);

      /**
       * Sets surface reflectivity
       * @param moduleID module ID (0 based, e.g. slot-1)
       * @param reflectivity reflectivity (absolute value)
       */
      void setSurfaceReflectivity(int moduleID, double reflectivity);

      /**
       * Defines TDC
       * @param NBIT   number of bits
       * @param ChWid  channel width
       * @param Offset offset
       */
      void setTDC(int NBIT, double ChWid, double Offset = 0);

      /**
       * Defines CFD
       * @param delTpileup pileup time
       * @param delTdoubleHit double hit resolution
       */
      void setCFD(double delTpileup, double delTdoubleHit);

      /**
       * Finalizes TOP configuration (must be called last)
       * @param Dump print configuration to std output
       * @return true if successfull
       */
      bool TOPfinalize(int Dump = 1);

      /**
       * Prints configuration to std output (using fortran print)
       */
      void print() const;

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
       * @param moduleID module ID (slot number, 1 based)
       * @param A width
       * @param B thickness
       * @param C length
       * @param Lside left-side window type
       * @param Rside right-side window type
       * @param LexpvID left-side expansion volume ID or 0
       * @param RexpvID right-side expansion volume ID or 0
       */
      void getQbar(int moduleID, double& A, double& B, double& C,
                   int& Lside, int& Rside, int& LexpvID, int& RexpvID);

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
                              double& Width, int& Shape, int& LR);

      /**
       * Returns mirror specs
       * @param moduleID module ID (slot number, 1 based)
       * @param R radius
       * @param Xc center of curvature in x (local frame)
       * @param Yc center of curvature in y (local frame)
       * @param Zc center of curvature in z (local frame)
       * @param F effective focal length
       */
      void getMirror(int moduleID, double& R, double& Xc, double& Yc, double& Zc,
                     double& F);

      /**
       * Returns thickness of the bar box window
       * @param moduleID module ID (slot number, 1 based)
       * @return thickness
       */
      double getBBoxWindow(int moduleID);

    private:

      static bool m_configured; /**< set to true when done */
      static DBObjPtr<TOPCalModuleAlignment> m_alignment; /**< alignment constants */
      static OptionalDBArray<TOPPmtInstallation> m_pmtInstalled; /**< PMT installation data */
      double m_R1 = 0;   /**< inner radius */
      double m_R2 = 0;   /**< outer radius */
      double m_Z1 = 0;   /**< backward z */
      double m_Z2 = 0;   /**< forward z */
      double m_timeRange = 0; /**< time range of TDC */

    };

  } // top namespace
} // Belle2 namespace

