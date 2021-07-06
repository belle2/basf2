/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef PINDIODESTUDYMODULE_H
#define PINDIODESTUDYMODULE_H

#include <framework/core/HistoModule.h>

#include <TH1.h>
#include <TH2.h>

namespace Belle2 {
  namespace pindiode {

    /**
     * Study module for Pindiodes (BEAST)
     *
     * Produces histograms from BEAST data for the Pindiodes.   *
     */
    class PindiodeStudyModule : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      PindiodeStudyModule();

      /**
       * Destructor
       */
      virtual ~PindiodeStudyModule();

      /**
       * Initialize the Module.
       * This method is called at the beginning of data processing.
       */
      virtual void initialize() override;

      /**
       * Called when entering a new run.
       * Set run dependent things like run header parameters, alignment, etc.
       */
      virtual void beginRun() override;

      /**
       * Event processor.
       */
      virtual void event() override;

      /**
       * End-of-run action.
       * Save run-related stuff, such as statistics.
       */
      virtual void endRun() override;

      /**
       * Termination action.
       * Clean-up, close files, summarize statistics, etc.
       */
      virtual void terminate() override;

      /** Defines the histograms*/
      virtual void defineHisto() override;


    private:

      /** reads data from PINDIODE.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** number of detectors. Read from PINDIODE.xml*/
      //int nPIN = 0;
      /** PIN coordinate */
      //std::vector<TVector3> PINCenter;
      /** Cremat gain */
      double m_CrematGain;
      /** Work function */
      double m_WorkFunction;
      /** Fano Factor */
      double m_FanoFactor;

      /** Rate */
      TH1F* h_pin_rate[10];
      /** Rate */
      TH2F* h_pin_rs_rate[10];
      /** histo dose */
      TH1F* h_pin_dose1[100];
      /** histo dose */
      TH1F* h_pin_dose2[100];
      /** histo dose */
      TH1F* h_pin_dose1Weight[100];
      /** histo dose */
      TH1F* h_pin_dose2Weight[100];
      /** histo volt */
      TH1F* h_pin_volt[100];
      /** histo time */
      TH1F* h_pin_time[100];
      /** histo time weighted by volt */
      TH1F* h_pin_vtime[100];
      /** histo dose */
      TH1F* h_pin_idose[100];
      /** histo dose */
      TH1F* h_pin_idoseWeight[100];
      /** histo dose */
      TH2F* h_pin_rs_idose[100];
      /** histo dose */
      TH2F* h_pin_rs_idoseWeight[100];
      /** histo volt */
      TH1F* h_pin_ivolt[100];
      /** histo time */
      TH1F* h_pin_itime[100];
      /** histo time weighted by volt */
      TH1F* h_pin_ivtime[100];
    };

  }
}

#endif /* PINDIODESTUDYMODULE_H */
