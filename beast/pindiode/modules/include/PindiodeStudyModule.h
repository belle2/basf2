/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PINDIODESTUDYMODULE_H
#define PINDIODESTUDYMODULE_H

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

      /**  */
      virtual ~PindiodeStudyModule();

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

      /** reads data from PINDIODE.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** number of detectors. Read from PINDIODE.xml*/
      //int nPIN = 0;
      /** PIN coordinate */
      //std::vector<TVector3> PINCenter;
      /** Event counter */
      //int Event = 0;
      /** Cremat gain */
      double m_CrematGain;
      /** Work function */
      double m_WorkFunction;
      /** Fano Factor */
      double m_FanoFactor;

      /** Rate */
      TH1F* h_pin_rate[10];
      /** histo dose */
      TH1F* h_dose1[100];
      /** histo dose */
      TH1F* h_dose2[100];
      /** histo dose */
      TH1F* h_dose1Weight[100];
      /** histo dose */
      TH1F* h_dose2Weight[100];
      /** histo volt */
      TH1F* h_volt[100];
      /** histo time */
      TH1F* h_time[100];
      /** histo time weighted by volt */
      TH1F* h_vtime[100];
      /** histo dose */
      TH1F* h_idose[100];
      /** histo dose */
      TH1F* h_idoseWeight[100];
      /** histo volt */
      TH1F* h_ivolt[100];
      /** histo time */
      TH1F* h_itime[100];
      /** histo time weighted by volt */
      TH1F* h_ivtime[100];
    };

  }
}

#endif /* PINDIODESTUDYMODULE_H */
