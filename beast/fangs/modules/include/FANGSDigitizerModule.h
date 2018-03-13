/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FANGSDIGITIZERMODULE_H
#define FANGSDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <beast/fangs/dataobjects/FANGSHit.h>

#include <TRandom3.h>
#include <TF1.h>
#include <TVector3.h>
#include <TVector2.h>
#include <TLorentzVector.h>

#include <vector>
#include <map>
#include <tuple>

/** size of pixel hit */
constexpr int MAXtSIZE = 1000;
/** size of hit */
constexpr int MAXSIZE = 3000;

namespace Belle2 {
  namespace fangs {
    /**
     * FANGS ATLAS FE-I4B ASIC chip digitizer
     *
     * Creates FANGSHits from FANGSSimHits
     *
     */
    class FANGSDigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      FANGSDigitizerModule();

      /**  */
      virtual ~FANGSDigitizerModule();

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
      StoreArray<FANGSHit> m_fangsHit; /** array for FANGSHit */

      /** reads data from MICROFANGS.xml: tube location, drift data filename, sigma of impulse response function */
      void getXMLData();

      /** Produces the pixelization */
      void Pixelization();

      /** Define ToT calib 1 */
      TF1* fctToT_Calib1;
      /** Define ToT calib 2 */
      TF1* fctToT_Calib2;
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
      /** Work function */
      double m_Workfct;
      /** Fano factor */
      double m_Fanofac;
      /** sensor width */
      double m_sensor_width;
      /** Drift velocity in sensor */
      double m_v_sensor;
      /** chip store arrays */
      std::map<std::tuple<int, int, int>, int> m_dchip;
      /** chip map arrays */
      std::map<std::tuple<int, int>, int> m_dchip_map;
      /** chip pdg map arrays */
      std::map<std::tuple<int, int>, int> m_dchip_pdg_map;
      /** chip track ID map arrays */
      std::map<std::tuple<int, int>, int> m_dchip_trkID_map;
      /** chip Nb map arrays */
      std::map<std::tuple<int, int>, int> m_dchip_detNb_map;
      /** number of detectors. Read from MICROFANGS.xml*/
      int m_nFANGS = 15;

      /** Lower timing cut */
      double m_lowerTimingCut = 0;
      /** Upper timing cut */
      double m_upperTimingCut = 1000000;

      /** Old detector counter */
      int olddetNb = -1;

    };

  }
}

#endif /* FANGSDIGITIZERMODULE_H */
