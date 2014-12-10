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

//C++/C standard lib elements.
#include <string>


//ROOT
#include <TH1F.h>

namespace Belle2 {

  /**
   * Analyze simulations of CsI readings in BEAST. Requires HistoManager
   *
   * Calculates the dose seen by each crystal. Todo: implement showers and digitization to differentiate between two types of channels.
   * Requires HistoManager to be added to path RIGHT AFTER the input module. See https://belle2.cc.kek.jp/~twiki/bin/view/Software/HistogramManagement
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

  private:

    StoreArray<CsiSimHit> m_hits; /**< Array of the simulated hits in the crystals */

    // Parameters
    double m_paramTemplate;  /**< Template of an input parameter. Noop for now. */

    // Histograms
    TH1F* h_EdistCrystal;   /**< Energy deposited in each crystal per hit */
    TH1F* h_EdistTotal;     /**< Energy deposited in all crystals per hit */
    TH1F* h_CrystalRadDose; /**< Yearly radiation dose deposited in each crystal */
    TH1F* h_NhitCrystal;    /**< Number of hits in each crystals (to validate light yield..) */

    // Constants
    const double GeVtoJ = 1.6e-10; /**< Conversion between GeV to Joule */
    const double Sampletime = 1000.0; /**< Sample time in us.. to fix: not truely a constant! */
    const double usInYr = 1.0e13; /**< Conversion between us to nominal year */


  };
}

#endif /* CSISTUDYMODULE_H */
