/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DOSIDIGITIZERMODULE_H
#define DOSIDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <beast/dosi/dataobjects/DosiHit.h>

namespace Belle2 {
  namespace dosi {
    /**
     * Dosi tube digitizer
     *
     * Creates DosiHits from DosiSimHits
     *
     */
    class DosiDigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      DosiDigitizerModule();

      /**
       * Destructor
       */
      virtual ~DosiDigitizerModule();

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


    private:
      StoreArray<DosiHit> m_dosiHit; /**< array for DosiHit */

      /** reads data from DOSI.xml: threshold in MeV, range in MeV, and resolution in % */
      virtual void getXMLData();
      /*
      Double_t GetEnergyResolutionGeV(Double_t pEnergy) const
      {
      // Returns energy resolution in GeV when supplied Energy in GeV
      return (m_EnergyResolutionFactor * TMath::Sqrt(pEnergy) + m_EnergyResolutionConst * pEnergy);

      }
      */
      /** Fold energy resolution */
      Double_t GetEnergyResolutionGeV(Double_t, Int_t);

      /** number of detectors. Read from DOSI.xml*/
      int nDOSI = 8;
      /** Energy threshold */
      //double m_Threshold[8];
      /** Energy range */
      //double m_Range[8];
      /** Energy resolution factor */
      double m_EnergyResolutionFactor[8];
      /** Energy resolution constant */
      double m_EnergyResolutionConst[8];

    };

  }
}

#endif /* DOSIDIGITIZERMODULE_H */
