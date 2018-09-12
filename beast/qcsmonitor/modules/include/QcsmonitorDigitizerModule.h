/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef QCSMONITORDIGITIZERMODULE_H
#define QCSMONITORDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <beast/qcsmonitor/dataobjects/QcsmonitorHit.h>
#include <string>
#include <vector>

//ROOT
#include <TRandom3.h>
#include <TF1.h>
#include <TVector3.h>

/** size of hit */
const Int_t MAXSIZE         = 10000;
/** size of pixel hit */
const Int_t MAXtSIZE        = 1000;

namespace Belle2 {
  namespace qcsmonitor {
    /**
     * Qcsmonitor tube digitizer
     *
     * Creates QcsmonitorHits from QcsmonitorSimHits
     *
     */
    class QcsmonitorDigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      QcsmonitorDigitizerModule();

      /**  */
      virtual ~QcsmonitorDigitizerModule();

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
      StoreArray<QcsmonitorHit> m_qcsmonitorHit; /** Array for QcsmonitorHit */

      /** reads data from QCSMONITOR.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** Time step */
      double m_TimeStep;
      /** Convertor factor keV to MIP */
      double m_C_keV_to_MIP;
      /** Convertor factor MIP to PE */
      double m_C_MIP_to_PE;
      /** Number of QCSMONITOR scintillator cell */
      int m_ScintCell;
      /** Lower time limit */
      double m_MinTime;
      /** Upper time limit */
      double m_MaxTime;
      /** Energy threshold */
      double m_MIPthres;
      /** Event MIP array*/
      double hitsarrayinMIP[1000][100];

    };

  }
}

#endif /* QCSMONITORDIGITIZERMODULE_H */
