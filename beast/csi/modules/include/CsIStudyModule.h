/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: beaulieu                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CSISTUDYMODULE_H
#define CSISTUDYMODULE_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <beast/csi/dataobjects/CsiSimHit.h>
#include <beast/csi/dataobjects/CsiHit.h>
#include <beast/csi/dataobjects/CsiDigiHit.h>

//C++/C standard lib elements.
#include <string>


//ROOT
#include <TH1F.h>

namespace Belle2 {

  /**
   * Analyze simulations of CsI readings in BEAST. Requires HistoManager
   *
   * Calculates the dose seen by each crystal. Todo: implement showers and digitization to differentiate between two types of channels.
   * Requires HistoManager to be added to path RIGHT AFTER the input module. See https://confluence.desy.de/display/BI/Software+HistogramManagement
   *
   */
  class CsIStudyModule : public HistoModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    CsIStudyModule();

    /** Empty destructor */
    virtual ~CsIStudyModule();

    /** Initialization: building histograms */
    virtual void initialize();

    /** To do at the beginning of each runs */
    virtual void beginRun();

    /** To do at the end of each runs. Write histos to file.*/
    virtual void endRun();


    /** Read each event, calculate doses and fill histograms */
    virtual void event();

    /** Clean everything up */
    virtual void terminate();


    //! function to define histograms
    /*
       Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
       to be placed in this function.
    */
    virtual void defineHisto();


    /** Reads  and unpacks the status bits of the digitizer
     * Unpacks in the appropriate booleans
     *
     * @param packedbits: a char containing the status bits in the following order [MSB-LSB] is [stop,holdoff,gate,trigger]
     *
     * @return : a pointer to the array of 8 bools;
     */

    bool* readDPPStatusBits(char packedbits);

  private:

    StoreArray<CsiSimHit> m_simhits; /**< Array of the simulated hits in the crystals (incl. each radiated photons..) */
    StoreArray<CsiHit> m_hits; /**< Array of the crystal hits (1 per event) */

    StoreArray<CsiDigiHit> m_digihits; /**< Array of the crystal digitized hits (1 per event per fired crystal) */

    // Parameters
    double m_paramTemplate;  /**< Template of an input parameter. Noop for now. */
    int m_nWFSamples
    ; /**< Number of samples in each of the waveforms (in the near future this should be imported from the digi module somehow */
    std::string  m_waveformFilename ; /**< Path where to save the waveforms (root file) */


    //Othe rmembers
    TFile* fWF; /**< ROOT file to save waveforms */

    // Histograms
    TH1F* h_CrystalEdep;        /**< Energy deposited in each crystal*/
    TH1F* h_CrystalSpectrum;    /**< Distribution of the energy of each simhit */
    TH1F* h_CrystalRadDose;     /**< Yearly radiation dose deposited in each crystal */
    TH1F* h_CrystalRadDoseSH;   /**< Yearly radiation dose deposited in each crystal (for hit-simhit check)*/
    TH1F* h_NhitCrystal;        /**< Number of hits in each crystal (to get hit rate)*/
    TH1F* h_LightYieldCrystal;  /**< Number of photons hits in each crystal (to validate light yield..) */
    TH1S* h_Waveform;           /**< Contains the digitized waveform (if enabled in the digitizer) */
    TH1C* h_Gate;               /**< Contains the integration gate (if waveforms are enabled in the digitizer) */
    TH1F* h_Charge ;            /**< Distribution of the integrated charge from the pulse processing algorithm*/
    TH1F* h_TrueEdep;           /**< Distribution of the true total deposited energy in the event*/
    TH1F* h_Height;           /**< Max height of the peak signa( */
    TH1F* h_Weight;           /**< Event weight assigned by the generator */



    // Constants
    const double GeVtoJ = 1.6e-10; /**< Conversion between GeV to Joule */
    const double Sampletime = 1000.0; /**< Sample time in us.. to fix: not truely a constant! */
    const double usInYr = 1.0e13; /**< Conversion between us to nominal year */
  };
}

#endif /* CSISTUDYMODULE_H */
