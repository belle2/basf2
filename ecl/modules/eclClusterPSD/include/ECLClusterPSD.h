/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Savino Longo (longos@uvic.ca)                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCLUSTERPSDMODULE_H_
#define ECLCLUSTERPSDMODULE_H_

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>

// OTHER
#include <vector>

namespace Belle2 {
  namespace ECL {

    class ECLClusterPSDModule: public Module {

    public:

      /** Constructor.
       */
      ECLClusterPSDModule();

      /** Destructor.
       */
      ~ECLClusterPSDModule();

      /** Initialize variables. */
      virtual void initialize();

      /** begin run.*/
      virtual void beginRun();

      /** event per event.
       */
      virtual void event();

      /** end run. */
      virtual void endRun();

      /** terminate.*/
      virtual void terminate();

      virtual const char* eclCalDigitArrayName() const
      { return "ECLCalDigits" ; }

      /** Name of the ECLEventInformation.*/
      virtual const char* eclEventInformationName() const
      { return "ECLEventInformation" ; }

    protected:

    private:
      double m_Chi2Threshold;   /** Two component fit chi square threshold .*/
      double m_CrystalHadronEnergyThreshold; /** hadron component energy threshold to classify as hadron.*/
    };
  } // end ECL namespace
} // end Belle2 namespace

#endif
