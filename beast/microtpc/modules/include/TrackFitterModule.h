/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRACKFITTERMODULE_H
#define TRACKFITTERMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <vector>

//ROOT
#include "TMath.h"
#include <TRandom3.h>
#include <TF1.h>
#include <TVector3.h>
#include "TGraph2DErrors.h"
#include <TGraph2DErrors.h>
#include <TVirtualFitter.h>
#include <Math/Vector3D.h>
#include <TPolyLine3D.h>
#include <assert.h>
#include <ctime>
#include <TMinuit.h>

using namespace ROOT::Math;
static void SumDistance2_angles(int&, double*, double&, double*, int);

namespace Belle2 {
  namespace microtpc {
    /**
     *
     *
     * Creates TrackFitter from TpcHits
     *
     */
    class TrackFitterModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      TrackFitterModule();

      /**  */
      virtual ~TrackFitterModule();

      /**  */
      virtual void initialize();

      /**  */
      virtual void beginRun();

      /**  */
      virtual void event();

      /**  */
      virtual void endRun();
      /**  */
      virtual void terminate();


    private:

      /** reads data from MICROTPC.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** Calculate distance squared */
      double distance2_angles(double , double , double , double*);

      /** Define random variable */
      //TRandom3* fRandom;
      /** Define Q calib 1 */
      TF1* fctQ_Calib1;
      /** Define Q calib 2 */
      TF1* fctQ_Calib2;
      /** GEM 1 gain */
      double m_GEMGain1;
      /** GEM 1 RMS */
      double m_GEMGainRMS1;
      /** Scale GEM 1 gain */
      double m_ScaleGain1;
      /** GEM 2 gain */
      double m_GEMGain2;
      /** GEM 2 RMS */
      double m_GEMGainRMS2;
      /** Scale GEM 2 gain */
      double m_ScaleGain2;
      /** GEM pitch */
      double m_GEMpitch;
      /** Pixel threshold */
      int m_PixelThreshold;
      /** Pixel threshold RMS */
      int m_PixelThresholdRMS;
      /** Chip column number */
      int m_ChipColumnNb;
      /** Chip row number */
      int m_ChipRowNb;
      /** Chip column x dimension */
      double m_ChipColumnX;
      /** Chip row y dimension */
      double m_ChipRowY;
      /** Pixel time number of bin */
      int m_PixelTimeBinNb;
      /** Pixel time bin */
      double m_PixelTimeBin;
      /** TOT factor A1 */
      double m_TOTA1;
      /** TOT factor B1 */
      double m_TOTB1;
      /** TOT factor C1 */
      double m_TOTC1;
      /** TOT factor Q1 */
      double m_TOTQ1;
      /** TOT factor A2 */
      double m_TOTA2;
      /** TOT factor B2 */
      double m_TOTB2;
      /** TOT factor C2 */
      double m_TOTC2;
      /** TOT factor Q2 */
      double m_TOTQ2;
      /** z drift gap */
      double m_z_DG;
      /** z transfer gap */
      double m_z_TG;
      /** z collection gap */
      double m_z_CG;
      /** Transverse diffusion in drift gap */
      double m_Dt_DG;
      /** Transverse diffusion in transfer gap */
      double m_Dt_TG;
      /** Transverse diffusion in collection gap */
      double m_Dt_CG;
      /** Longitudinal diffusion in drift gap */
      double m_Dl_DG;
      /** Longitudinal diffusion in transfer gap */
      double m_Dl_TG;
      /** Longitudinal diffusion in collection gap */
      double m_Dl_CG;
      /** Drift velocity in drift gap */
      double m_v_DG;
      /** Drift velocity in transfer gap */
      double m_v_TG;
      /** Drift velocity in collection gap */
      double m_v_CG;
      /** Pressure in vessel */
      //double m_P_vessel;
      /** Work function */
      double m_Workfct;
      /** Fano factor */
      double m_Fanofac;
      /** Absorption in gas */
      double m_GasAbs;
      /** chip store arrays */
      int dchip[10][80][336][MAXtSIZE];
      /** number of detectors. Read from MICROTPC.xml*/
      int nTPC = 0;
      /** TPC coordinate */
      std::vector<TVector3> TPCCenter;
      /** Event counter */
      int Event = 0;
      /** col vector */
      std::vector<int> colArray;
      /** row vector */
      std::vector<int> rowArray;
      /** tot vector */
      std::vector<int> totArray;
      /** bci vector */
      std::vector<int> bciArray;
      /** detector number */
      int m_detNb;
      /** chi^2 of the fit */
      float m_chi2;
      /** Polar angle theta in degrees */
      float m_theta;
      /** Azimuthal angle phi in degrees */
      float m_phi;
      /** total ionization energy */
      float m_esum;
      /** TOT sum */
      int m_totsum;
      /** track length */
      float m_trl;
      /** Trigger/time length */
      int m_time_range;
      /** Fit parameters */
      float m_parFit[5];
      /** Fit paramteter errors */
      float m_parFit_err[5];
      /** Covariant errors */
      float m_cov[5][5];
      /** Impact parameter x */
      float m_impact_x[4];
      /** Impact parameter y */
      float m_impact_y[4];
      /** Which side was/were hit */
      int m_side[5][4];
      /** x point of the track */
      float x[MAXSIZE];
      /** y point of the track */
      float y[MAXSIZE];
      /** z point of the track */
      float z[MAXSIZE];
      /** e point of the track */
      float e[MAXSIZE];
      /** TGraphErrors track */
      TGraph2DErrors* Track;
      /** track length */
      float* L; //!
      /** index ix */
      int* ix; //!
      /** index iy */
      int* iy; //!
      /** index iz */
      int* iz; //!
      /** index L */
      int* iL; //!

    };

  }
}

#endif /* TRACKFITTERMODULE_H */
