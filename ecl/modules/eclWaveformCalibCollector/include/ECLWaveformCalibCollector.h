/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

// ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dbobjects/ECLDigitWaveformParameters.h>
#include <ecl/digitization/shaperdsp.h>

#include <TTree.h>
#include <TFile.h>

class TMinuit;

namespace Belle2 {

  //class to store coveriance matrix related quantities
  class CrystalCovInfo {
  private:
  public:
  };

  /** Module performs offline fit for saved ecl waveforms.
   *    */
  class ECLWaveformCalibCollectorModule : public Module {

  public:

    /** Constructor.
     */
    ECLWaveformCalibCollectorModule();

    /** Destructor.
     */
    ~ECLWaveformCalibCollectorModule();

    /** Initialize variables. */
    virtual void initialize() override;

    /** begin run.*/
    virtual void beginRun() override;

    /** event per event.
     */
    virtual void event() override;

    /** end run. */
    virtual void endRun() override;

    /** terminate.*/
    virtual void terminate() override;

    /** ECLDigits Array Name.*/
    virtual const char* eclDigitArrayName() const
    { return "ECLDigits" ; }

    /** ECLDspsArray Name.*/
    virtual const char* eclDspArrayName() const
    { return "ECLDsps" ; }


  private:

    StoreArray<ECLDsp> m_eclDSPs;  /**< StoreArray ECLDsp */

    StoreArray<ECLDigit> m_eclDigits;   /**< StoreArray ECLDigit */

    std::vector<float> m_ADCtoEnergy;  /**< calibration vector form adc to energy*/

    std::string m_OutputFileName; /**< output filename >*/

    double m_LowEnergyThresholdGeV; /**< Low Energy Threshold in GeV. >*/

    double m_HighEnergyThresholdGeV; /**< High Energy Threshold in GeV. >*/

    int m_EventsProcessed; /**< number of events processed >*/

    TFile* m_rootFilePtr; /**< root file used for storing info >*/

    TTree* m_tree; /**< Root tree and file for saving the output >*/

    int m_CellID; /**< To read ntuple branch > */
    double m_OnlineE; /**< To read ntuple branch > */
    double m_OfflineE;  /**< To read ntuple branch > */
    double m_OfflineHadE; /**< To read ntuple branch > */
    double m_Baseline; /**< To read ntuple branch > */
    double m_BaselineRMS; /**< To read ntuple branch > */
    double m_calibConst; /**< To read ntuple branch > */
    double m_Chi2; /**< To read ntuple branch > */
    double m_Chi2Save0; /**< To read ntuple branch > */
    double m_Chi2Save1; /**< To read ntuple branch > */
    double m_Chi2Save2; /**< To read ntuple branch > */
    int m_FitType; /**< To read ntuple branch > */
    int m_ADC0; /**< To read ntuple branch > */
    int m_ADC1; /**< To read ntuple branch > */
    int m_ADC2; /**< To read ntuple branch > */
    int m_ADC3; /**< To read ntuple branch > */
    int m_ADC4; /**< To read ntuple branch > */
    int m_ADC5; /**< To read ntuple branch > */
    int m_ADC6; /**< To read ntuple branch > */
    int m_ADC7; /**< To read ntuple branch > */
    int m_ADC8; /**< To read ntuple branch > */
    int m_ADC9; /**< To read ntuple branch > */
    int m_ADC10; /**< To read ntuple branch > */
    int m_ADC11; /**< To read ntuple branch > */
    int m_ADC12; /**< To read ntuple branch > */
    int m_ADC13; /**< To read ntuple branch > */
    int m_ADC14; /**< To read ntuple branch > */
    int m_ADC15; /**< To read ntuple branch > */
    int m_ADC16; /**< To read ntuple branch > */
    int m_ADC17; /**< To read ntuple branch > */
    int m_ADC18; /**< To read ntuple branch > */
    int m_ADC19; /**< To read ntuple branch > */
    int m_ADC20; /**< To read ntuple branch > */
    int m_ADC21; /**< To read ntuple branch > */
    int m_ADC22; /**< To read ntuple branch > */
    int m_ADC23; /**< To read ntuple branch > */
    int m_ADC24; /**< To read ntuple branch > */
    int m_ADC25; /**< To read ntuple branch > */
    int m_ADC26; /**< To read ntuple branch > */
    int m_ADC27; /**< To read ntuple branch > */
    int m_ADC28; /**< To read ntuple branch > */
    int m_ADC29; /**< To read ntuple branch > */
    int m_ADC30; /**< To read ntuple branch > */

  };
} // end Belle2 namespace
