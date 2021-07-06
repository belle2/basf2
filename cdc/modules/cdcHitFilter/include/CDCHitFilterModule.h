/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <cdc/dataobjects/CDCHit.h>
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
      void initialize()  override;

      /**
       * Begin run action.
       */

      void event()  override;

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

