/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <vtx/dataobjects/VTXDigit.h>
#include <string>

namespace Belle2 {

  namespace VTX {
    /**
     * Plot each event with ROI and Pixels
     */
    class VTXEventPlotModule : public Module {

    public:

      /** Constructor defining the parameters */
      VTXEventPlotModule();

    private:
      void initialize() override final;

      void event() override final;

      void beginRun() override final;

    private:
      std::string m_storeDigitsName;             /**< VTXDigits StoreArray name */

      StoreObjPtr<EventMetaData> m_eventMetaData;/**< Event Metadata StorePtr */

      /** Storearray for digits   */
      StoreArray<VTXDigit> m_storeDigits;

      /** Maximum VCell */
      int m_maxVCell{0};

      /** Maximum UCell */
      int m_maxUCell{0};

    };//end class declaration


  } //end VTX namespace;
} // end namespace Belle2

