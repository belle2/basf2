/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef CSIDIGITIZER_V2MODULE_H
#define CSIDIGITIZER_V2MODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <beast/csi/dataobjects/CsiHit_v2.h>

namespace Belle2 {
  namespace csi {
    /**
     * Csi tube digitizer_v2
     *
     * Creates CsiHits from CsiSimHits
     *
     */
    class CsiDigitizer_v2Module : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      CsiDigitizer_v2Module();

      /**
       * Destructor
       */
      virtual ~CsiDigitizer_v2Module();

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
      StoreArray<CsiHit_v2> m_csiHit_v2; /**< array for CsiHit_v2 */

      /** reads data from CSI.xml: threshold in MeV, range in MeV, and resolution in % */
      virtual void getXMLData();
      /*
      Double_t GetEnergyResolutionGeV(Double_t pEnergy) const
      {
      // Returns energy resolution in GeV when supplied Energy in GeV
      return (m_EnergyResolutionFactor * TMath::Sqrt(pEnergy) + m_EnergyResolutionConst * pEnergy);

      }
      */
      /** Fold energy resolution */
      Double_t GetEnergyResolutionGeV(Double_t, int);

      /** number of detectors. Read from CSI.xml*/
      int nCSI = 18;
      /** Energy threshold */
      double m_Threshold[18];
      /** Enegy range */
      double m_Range[18];
      /** Energy resolution factor */
      double m_EnergyResolutionFactor[18];
      /** Energy resolution constant */
      double m_EnergyResolutionConst[18];

    };

  }
}

#endif /* CSIDIGITIZER_V2MODULE_H */
