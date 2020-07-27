/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TPCSTUDYMODULE_H
#define TPCSTUDYMODULE_H

#include <framework/core/HistoModule.h>
#include <vector>

#include <TVector3.h>
#include <TH1.h>
#include <TH2.h>

namespace Belle2 {
  namespace microtpc {

    /**
     * Study module for TPCs (BEAST)
     *
     * Produces histograms from BEAST data for the TPCs.   *
     */
    class TPCStudyModule : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      TPCStudyModule();

      /**
       * Destructor
       */
      virtual ~TPCStudyModule();

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

      /** reads data from MICROTPC.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** Chip column number */
      int m_ChipColumnNb;
      /** Chip row number */
      int m_ChipRowNb;
      /** Chip column x dimension */
      double m_ChipColumnX;
      /** Chip row y dimension */
      double m_ChipRowY;
      /** z drift gap */
      double m_z_DG;
      /** number of detectors. Read from MICROTPC.xml*/
      int nTPC = 0;
      /** TPC coordinate */
      std::vector<TVector3> TPCCenter;
      /** Event counter */
      //int Event = 0;
      /** Recoil kin  */
      TH1F* h_tpc_kin[100];
      /** Track XY */
      TH2F* h_tpc_xy[100];

    };

  }
}

#endif /* TPCSTUDYMODULE_H */
