/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter, Thomas Hauth, Nils Braun                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/base/StoreArrayLoader.h>

#include <tracking/modules/trackTimeExtraction/TrackTimeExtraction.h>


#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/dataobjects/EventT0.h>
#include <framework/core/Module.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <tuple>
#include <type_traits>

namespace Belle2 {
  namespace TrackFindingCDC {

    //TWrappedFindlet wi
    template <class TWrappedFindlet>
    class FindletStoreArrayInput : public TrackFindingCDC::Findlet<> {

    private:
      /// Type of the base class
      using Super = TrackFindingCDC::Findlet<>;

    public:

      // this will be something like RecoTrack const * , so string the pointer
      using DataStoreInputTypePtrType = typename std::tuple_element<0, typename TWrappedFindlet::IOTypes>::type;
      using DataStoreInputTypeRefType = typename std::remove_pointer<DataStoreInputTypePtrType>::type;

      /// Create a new instance of the module.
      FindletStoreArrayInput()
      {
        addProcessingSignalListener(&m_storeArrayLoader);
        addProcessingSignalListener(&m_wrappedFindlet);
      }

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final
      {
        Super::exposeParameters(moduleParamList, prefix);

        m_storeArrayLoader.exposeParameters(moduleParamList, prefix);
        m_wrappedFindlet.exposeParameters(moduleParamList, prefix);
      }

      /// Short description of the findlet
      std::string getDescription() override final
      {
        return m_wrappedFindlet.getDescription();
      }

      void beginEvent() override final
      {
        m_storeArrayInput.clear();
      }

      /// timing extraction for this findlet
      void apply() override final
      {
        m_storeArrayLoader.apply(m_storeArrayInput);
        m_wrappedFindlet.apply(m_storeArrayInput);
      }

    private:

      TrackFindingCDC::StoreArrayLoader<DataStoreInputTypeRefType> m_storeArrayLoader;
      TWrappedFindlet m_wrappedFindlet;

      /// as member to keep vector memory allocated from event to event
      std::vector<DataStoreInputTypePtrType> m_storeArrayInput;
    };
  }
}
