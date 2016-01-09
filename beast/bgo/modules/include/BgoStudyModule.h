/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BGOSTUDYMODULE_H
#define BGOSTUDYMODULE_H

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
  namespace bgo {

    /**
     * Study module for Bgos (BEAST)
     *
     * Produces histograms from BEAST data for the Bgos.   *
     */
    class BgoStudyModule : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      BgoStudyModule();

      /**  */
      virtual ~BgoStudyModule();

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

      /** reads data from BGO.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** number of detectors. Read from BGO.xml*/
      int nDIA = 0;
      /** PIN coordinate */
      std::vector<TVector3> PINCenter;
      /** Event counter */
      int Event = 0;


      /** histo dose */
      TH1F* h_dose[100];
      /** histo energy deposited */
      TH1F* h_edep[100];
      /** histo time */
      TH1F* h_time[100];
      /** histo time weighted by volt */
      TH1F* h_vtime[100];
    };

  }
}

#endif /* BGOSTUDYMODULE_H */
