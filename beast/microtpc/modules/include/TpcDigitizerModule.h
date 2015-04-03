/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TPCDIGITIZERMODULE_H
#define TPCDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <vector>

//ROOT
#include <TRandom3.h>
#include <TF1.h>
#include <TVector3.h>

/** size of hit */
const Int_t MAXSIZE         = 10000;
/** size of pixel hit */
const Int_t MAXtSIZE        = 1000;

namespace Belle2 {
  namespace microtpc {
    /**
     * Tpc tube digitizer
     *
     * Creates TpcHits from TpcSimHits
     *
     */
    class TpcDigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      TpcDigitizerModule();

      /**  */
      virtual ~TpcDigitizerModule();

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

      /** Reads drift data from file */
      virtual void getDriftData();

      /** interpolates the drift time for a given distance from the wire */
      double getDriftTime(double R);

      /** Produces the impulse response function */
      virtual void impulseResponse();

      /** Convolves the raw waveform with the impulse response function and returns the peak of the convolved waveform */
      double convolveWaveform(double* waveform);

      /** print the convolved wafeform to file. Useful for debugging */
      virtual void printConvWaveform(int eventNum, int detNB, double* convolvedWaveform);

      /** reads data from MICROTPC.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** Produces the pixelization */
      virtual bool Pixelization(int);

      /** Drift ionization */
      virtual void Drift(double, double, double, double&, double&, double&, double&, double, double, double);

      /** GEMazition of GEM1 */
      virtual void GEMGeo1(double, double, double&, double&);

      /** GEMazition of GEM2 */
      virtual void GEMGeo2(double, double, double&, double&);

      /** Define random variable */
      TRandom3* fRandom;
      /** Define ToT calib 1 */
      TF1* fctToT_Calib1;
      /** Define ToT calib 2 */
      TF1* fctToT_Calib2;
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
      /** Chip row number */
      int m_ChipRowNb;
      /** Chip column number */
      int m_ChipColumnNb;
      /** Chip row y dimension */
      int m_ChipRowY;
      /** Chip column x dimension */
      int m_ChipColumnX;
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
      double m_P_vessel;
      /** Work function */
      double m_Workfct;
      /** Fano factor */
      double m_Fanofac;
      /** Absorption in gas */
      double m_GasAbs;
      /** chip store arrays */
      int dchip[10][80][336][MAXtSIZE];

      /** filename of drift datafile */
      std::string m_driftDataFile;
      /** sigma of impulse response */
      double m_impulseSigma;
      /** distance from center of tube */
      std::vector<double> radius_drift;
      /** drift time for each distance from center of tube */
      std::vector<double> time_drift;
      /** size of waveforms */
      static const int waveformSize = 15000;
      /** number of detectors. Read from MICROTPC.xml*/
      int nTPC = 0;
      /** TPC coordinate */
      std::vector<TVector3> TPCCenter;
      /** Impulse response function */
      double iResponse[2400] = {0};
      /** Event counter */
      int Event = 0;

    };

  }
}

#endif /* TPCDIGITIZERMODULE_H */
