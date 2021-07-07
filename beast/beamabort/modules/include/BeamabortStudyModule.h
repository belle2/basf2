/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef BEAMABORTSTUDYMODULE_H
#define BEAMABORTSTUDYMODULE_H

#include <framework/core/HistoModule.h>

#include <TH1.h>
#include <TH2.h>

namespace Belle2 {
  namespace beamabort {

    /**
     * Study module for Beamaborts (BEAST)
     *
     * Produces histograms from BEAST data for the Beamaborts.   *
     */
    class BeamabortStudyModule : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      BeamabortStudyModule();

      /**
       * Destructor
       */
      virtual ~BeamabortStudyModule();

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

      /**
       * Defines the histograms
       */
      virtual void defineHisto() override;


    private:

      /** reads data from BEAMABORT.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** number of detectors. Read from BEAMABORT.xml*/
      //int nDIA = 0;
      /** PIN coordinate */
      //std::vector<TVector3> PINCenter;
      /** Event counter */
      //int Event = 0;
      /** Work function */
      double m_WorkFunction;
      /** Fano Factor */
      double m_FanoFactor;

      /** Rate */
      TH1F* h_dia_rate[10];
      /** Rate */
      TH2F* h_dia_rs_rate[10];
      /** histo dose */
      TH1F* h_dia_dose[100];
      /** histo dose */
      TH1F* h_dia_doseWeight[100];
      /** histo amp */
      TH1F* h_dia_amp[100];
      /** histo dose */
      TH1F* h_dia_idose[100];
      /** histo dose */
      TH1F* h_dia_idoseWeight[100];
      /** histo dose */
      TH2F* h_dia_rs_idose[100];
      /** histo dose */
      TH2F* h_dia_rs_idoseWeight[100];
      /** histo amp */
      TH1F* h_dia_iamp[100];
      /** histo time */
      TH1F* h_dia_itime[100];
      /** histo time weighted by volt */
      TH1F* h_dia_ivtime[100];
      /** histo amp sum */
      TH1F* h_dia_Amp[100];
      /** histo edep sum */
      TH1F* h_dia_edep[100];
      /** histo time */
      TH1F* h_dia_time[100];
      /** histo time weighted by volt */
      TH1F* h_dia_vtime[100];
    };

  }
}

#endif /* BEAMABORTSTUDYMODULE_H */
