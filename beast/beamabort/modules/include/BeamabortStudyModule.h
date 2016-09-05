/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BEAMABORTSTUDYMODULE_H
#define BEAMABORTSTUDYMODULE_H

#include <framework/core/HistoModule.h>
#include <string>
#include <vector>


#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>


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

      /**  */
      virtual ~BeamabortStudyModule();

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

      /** Defines the histograms*/
      virtual void defineHisto();


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
