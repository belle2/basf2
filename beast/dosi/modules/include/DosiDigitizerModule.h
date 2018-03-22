/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DOSIDIGITIZERMODULE_H
#define DOSIDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <beast/dosi/dataobjects/DosiHit.h>
#include <string>
#include <vector>

//ROOT
#include <TRandom3.h>
#include <TMath.h>
#include <TF1.h>
#include <TVector3.h>


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

      /**  */
      virtual ~DosiDigitizerModule();

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


    private:
      StoreArray<DosiHit> m_dosiHit; /** array for DosiHit */

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
