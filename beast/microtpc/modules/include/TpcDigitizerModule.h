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
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <beast/microtpc/dataobjects/MicrotpcHit.h>

#include <TRandom3.h>
#include <TF1.h>
#include <TVector3.h>
#include <TVector2.h>
#include <TLorentzVector.h>

#include <vector>
#include <map>
#include <tuple>

/** size of hit */
constexpr int MAXSIZE = 26880;
/** size of pixel hit */
constexpr int MAXtSIZE = 10000;

namespace Belle2 {
  namespace microtpc {
    /**
     * Micro TPC digitizer
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

      /** Initialize the Module.
       * This method is called only once before the actual event processing starts.
       */
      virtual void initialize() override;

      /** Called when entering a new run.
       */
      virtual void beginRun() override;

      /** This method is the core of the module.
       * This method is called for each event. All processing of the event has to take place in this method.
       */
      virtual void event() override;

      /** This method is called if the current run ends.
       */
      virtual void endRun() override;

      /** This method is called at the end of the event processing.
       */
      virtual void terminate() override;


    private:

      StoreArray<MicrotpcHit> m_microtpcHit; /** Array for MicrotpcHit */

      /** reads data from MICROTPC.xml: tube location, drift data filename, sigma of impulse response function */
      void getXMLData();

      /** Produces the pixelization */
      //bool Pixelization(int);
      void Pixelization();

      /** Drift ionization
       * Make the ionization drifting from (x,y,z) to GEM1 top plane
       * */
      /*TLorentzVector Drift(
        double x1, double y1, double z1,
        double st, double sl, double vd
      );*/
      virtual void Drift(double, double, double, double&, double&, double&, double&, double, double, double);

      /** GEMazition of GEM1 */
      TVector2 GEMGeo1(double x1, double y1);

      /** GEMazition of GEM2 */
      TVector2 GEMGeo2(double x1, double y1);

      /** Phase */
      Int_t m_phase;
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
      /** Work function */
      double m_Workfct;
      /** Fano factor */
      double m_Fanofac;
      /** Absorption in gas */
      double m_GasAbs;
      /** chip store arrays */
      //int m_dchip[10][80][336][MAXtSIZE];
      std::map<std::tuple<int, int, int>, int> m_dchip;
      /** chip map arrays */
      std::map<std::tuple<int, int>, int> m_dchip_map;
      /** chip pdg map arrays */
      std::map<std::tuple<int, int>, int> m_dchip_pdg_map;
      /** chip track ID map arrays */
      std::map<std::tuple<int, int>, int> m_dchip_trkID_map;
      /** chip Nb map arrays */
      std::map<std::tuple<int, int>, int> m_dchip_detNb_map;
      /** Flag 0/1 only look at nuclear recoils*/
      int m_LookAtRec;
      /** number of detectors. Read from MICROTPC.xml*/
      int m_nTPC = 0;
      /** TPC coordinate */
      std::vector<TVector3> m_TPCCenter;
      /** TPC angle X */
      std::vector<float> m_TPCAngleX;
      /** TPC angle Z */
      std::vector<float> m_TPCAngleZ;

      /** Lower timing cut */
      double m_lowerTimingCut = 0;
      /** Upper timing cut */
      double m_upperTimingCut = 1000000;

      /** Old detector counter */
      int olddetNb = -1;

      /** Old track ID */
      int oldtrkID = -1;

    };

  }
}

#endif /* TPCDIGITIZERMODULE_H */
