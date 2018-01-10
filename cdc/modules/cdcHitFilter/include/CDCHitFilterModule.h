/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * CDC hit filter module                                                  *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCHITFILTERMODULE_H
#define CDCHITFILTERMODULE_H

#include <framework/core/Module.h>
#include <cdc/dataobjects/CDCHit.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

#include <string>

namespace Belle2 {

  namespace CDC {

    /**
     * CDCHitFilter: Filters CDC hits according to given
     * configuration criteria. Does not copy the relations of the CDCHits.
     */

    class CDCHitFilterModule : public Module {

    public:
      /**
       * Constructor of the module.
       */
      CDCHitFilterModule();

      /**
       * Destructor of the module.
       */
      virtual ~CDCHitFilterModule();

      /**
       * Initializes the Module.
       */
      virtual void initialize();

      /**
       * Begin run action.
       */

      virtual void event();

    private:
      /**
       * Name of the CDCHit List to filter
       */
      std::string m_inputCDCHitListName;

      /**
       * Name of the CDCHit list, which will contain the hits passing all filter criteria
       */
      std::string m_outputCDCHitListName;


      int m_filterSuperLayer = -1; /**< Only perform digitization for a specific super layer or for all, if the value is -1 */
      int m_filterLayerMax = -1; /**< Only perform digitization up to a specific layer or for all, if the value is -1 */
      int m_filterWireMax = -1; /**< Only perform digitization for a specific wire or for all, if the value is -1 */

      StoreArray<CDCHit>  m_inputCDCHits; /**< Input CDCHit array */
      StoreArray<CDCHit>  m_outputCDCHits; /**< Output (filtered) CDCHit array */


    };//end class declaration


  } //end CDC namespace;
} // end namespace Belle2

#endif // CDCHitFilter_H

