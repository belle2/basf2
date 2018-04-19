/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth, Nils Braun                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/base/StoreArrayLoader.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/core/Module.h>

#include <tuple>
#include <type_traits>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Findlet which can wrap another Findlet and forward the contents of a StoreArray to the
     * wrapped findlet. This simplifies development of findlets who can either be used stand-alone
     * (with input from a StoreArray) or within a more complex findlet chain with input from, for example,
     * an std::vector.
     * For an example, see the TrackTimeExtractionModule
     *
     * Internally, this class uses the StoreArrayLoader to transfer the pointers from a StoreArray to
     * an std::vector for each event.
     */
    template <class TWrappedFindlet>
    class FindletStoreArrayInput : public TrackFindingCDC::Findlet<> {

    private:
      /// Type of the base class
      using Super = TrackFindingCDC::Findlet<>;

    public:

      // this will be something like RecoTrack const * , so string the pointer
      using DataStoreInputTypePtrType = typename std::tuple_element<0, typename TWrappedFindlet::IOTypes>::type;
      // same as reference
      using DataStoreInputTypeRefType = typename std::remove_pointer<DataStoreInputTypePtrType>::type;

      /// Create a new instance of the module.
      FindletStoreArrayInput() : Super()
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

      /// Clear the local cache of the store array input for each new event
      void beginEvent() override final
      {
        Super::beginEvent();
        m_storeArrayInput.clear();
      }

      /// timing extraction for this findlet
      void apply() override final
      {
        m_storeArrayLoader.apply(m_storeArrayInput);
        m_wrappedFindlet.apply(m_storeArrayInput);
      }

    private:

      /// This class will transfer the content of the StoreArrey into a std::vector for each event
      /// The std::vector is used as input for the wrapped findlet
      TrackFindingCDC::StoreArrayLoader<DataStoreInputTypeRefType> m_storeArrayLoader;

      /// Instance of the wrapped findlet
      TWrappedFindlet m_wrappedFindlet;

      /// as member to keep vector memory allocated from event to event
      std::vector<DataStoreInputTypePtrType> m_storeArrayInput;
    };
  }
}
