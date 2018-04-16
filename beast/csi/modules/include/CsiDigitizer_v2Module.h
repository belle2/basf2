/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CSIDIGITIZER_V2MODULE_H
#define CSIDIGITIZER_V2MODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <beast/csi/dataobjects/CsiHit_v2.h>
#include <string>
#include <vector>

//ROOT
#include <TRandom3.h>
#include <TMath.h>
#include <TF1.h>
#include <TVector3.h>


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

      /**  */
      virtual ~CsiDigitizer_v2Module();

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
      StoreArray<CsiHit_v2> m_csiHit_v2; /** array for CsiHit_v2 */

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
