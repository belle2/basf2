/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <tracking/trackFindingCDC/utilities/EvalVariadic.h>

#include <framework/core/Module.h>
#include <framework/core/ModuleParamList.h>

#include <utility>
#include <vector>
#include <array>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Adapter of a findlet to a module that exposes the parameters of the findlet and manages the IO with the DataStore
    template<class AFindlet>
    class FindletModule : public Module {

    private:
      /// Type of the base class
      using Super = Module;

      /// Tuple of input / output types of the findlet
      using IOTypes = typename AFindlet::IOTypes;

      /// Accessor for the individual input / output types of the findlet
      template<std::size_t I>
      using IOType = typename std::tuple_element<I, IOTypes>::type;

      /// Type function to strip constant and reference qualification form a given type
      template<class T>
      using Stripped = typename std::remove_reference<typename std::remove_const<T>::type>::type;

      /// Accessor for the individual input / output types of the findlet stripped from modifiers
      template<std::size_t I>
      using StrippedIOType = Stripped<IOType<I>>;

      /// Accessor for the individual input / output vectors type registered to the DataStore
      template <std::size_t I>
      using StoreVector = StoreWrappedObjPtr<std::vector<StrippedIOType<I>>>;

      /**
       *  Test if the given io type designates an input store vector.
       *  Types that are marked as constant and / or reference are input.
       */
      template<std::size_t I>
      static constexpr bool isInputStoreVector()
      {
        return std::is_const<IOType<I>>::value or std::is_reference<IOType<I>>::value;
      }

      /// Number of typpes served to the findlet
      static const std::size_t c_nTypes = std::tuple_size<IOTypes>::value;

      /// Helper class to iterate over the individual types served to the findlet
      using Indices = std::make_index_sequence<c_nTypes>;

    public:
      /// Constructor of the module
      FindletModule(const std::array<std::string, c_nTypes>& storeVectorNames = {})
        : m_param_storeVectorNames(storeVectorNames)
      {
        this->setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);
        std::string description = "Findlet: ";
        if (std::tuple_size<IOTypes>() == 0) {
          // Drop Findlet prefix for full finders with no IOTypes
          description = "";
        }

        description += m_findlet.getDescription();
        this->setDescription(description);

        this->addStoreVectorParameters(Indices());

        ModuleParamList moduleParamList = this->getParamList();

        const std::string prefix = "";
        m_findlet.exposeParameters(&moduleParamList, prefix);

        this->setParamList(moduleParamList);
      }

      /// Make destructor of interface virtual
      virtual ~FindletModule() = default;

      /// Initialize the Module before event processing
      virtual void initialize() override
      {
        Super::initialize();
        this->requireOrRegisterStoreVectors(Indices());
        m_findlet.initialize();
      }

      /// Signal the beginning of a new run.
      virtual void beginRun() override
      {
        Super::beginRun();
        m_findlet.beginRun();
      }

      /// Start processing the current event
      virtual void event() override
      {
        m_findlet.beginEvent();
        this->createStoreVectors(Indices());
        applyFindlet(Indices());
      }

      /// Signal the end of the run.
      virtual void endRun() override
      {
        m_findlet.endRun();
        Super::endRun();
      }

      /// Signal to terminate the event processing
      virtual void terminate() override
      {
        m_findlet.terminate();
        Super::terminate();
      }

    private:
      /// Get the vectors from the DataStore and apply the findlet
      template <size_t... Is>
      void applyFindlet(std::index_sequence<Is...>)
      {
        m_findlet.apply(*(getStoreVector<Is>())...);
      }

      /// Create the vectors on the DataStore
      template <size_t... Is>
      void createStoreVectors(std::index_sequence<Is...>)
      {
        evalVariadic((createStoreVector<Is>(), std::ignore)...);
      }

      /// Require or register the vectors on the DataStore
      template <size_t... Is>
      void requireOrRegisterStoreVectors(std::index_sequence<Is...>)
      {
        evalVariadic((requireStoreVector<Is>(), std::ignore) ...);
        evalVariadic((registerStoreVector<Is>(), std::ignore) ...);
      }

      /** Require the vector with index I to be on the DataStore.
       *  Only types that are marked as constant or reference are required to be on
       *  the DataStore before this module. Others are generally
       *  output vectors and need to be registered only.
       */
      template <std::size_t I>
      void requireStoreVector()
      {
        if (isInputStoreVector<I>()) {
          getStoreVector<I>().isRequired();
        }
      }

      /** Register the vector with index I to the DataStore.*/
      template <std::size_t I>
      void registerStoreVector()
      {
        if (not isInputStoreVector<I>()) {
          getStoreVector<I>().registerInDataStore(DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
        }
      }

      /** Create the vector with index I on the DataStore.*/
      template<std::size_t I>
      void createStoreVector()
      {
        if (not getStoreVector<I>().isValid()) {
          getStoreVector<I>().construct();
        }
      }

      /** Get the vector with index I from the DataStore.*/
      template<std::size_t I>
      StoreVector<I> getStoreVector()
      {
        StoreVector<I> storeVector(m_param_storeVectorNames[I]);
        return storeVector;
      }

      /** Expose parameters to set the names of the vectors on the DataStore */
      template<size_t ... Is>
      void addStoreVectorParameters(std::index_sequence<Is...>)
      {
        evalVariadic((addStoreVectorParameter<Is>(), std::ignore)...);
      }

      /** Expose parameter to set the names of the vector with index I on the DataStore */
      template<std::size_t I>
      void addStoreVectorParameter()
      {
        std::string name = getStoreVectorParameterName<I>();
        std::string description = getStoreVectorParameterDescription<I>();
        if (m_param_storeVectorNames[I] == "") {
          // Make a forced parameter
          this->addParam(name,
                         m_param_storeVectorNames[I],
                         description);
        } else {
          // Make an unforced parameter
          this->addParam(name,
                         m_param_storeVectorNames[I],
                         description,
                         m_param_storeVectorNames[I]);
        }
      }

      /** Get parameter name for the StoreVector at index I */
      template <std::size_t I>
      std::string getStoreVectorParameterName() const
      {
        bool primary = I == GetIndexInTuple<IOType<I>, IOTypes>::value;
        int order = primary ? 1 : 2;
        bool input = isInputStoreVector<I>();

        // Just a little bit of ADL
        std::string classParameterName = getClassMnemomicParameterName(static_cast<StrippedIOType<I>*>(nullptr));
        return getStoreVectorParameterName(classParameterName, order, input);
      }

      /** Get parameter description for the StoreVector at index I */
      template<std::size_t I>
      std::string getStoreVectorParameterDescription() const
      {
        bool primary = I == GetIndexInTuple<IOType<I>, IOTypes>::value;
        int order = primary ? 1 : 2;
        bool input = isInputStoreVector<I>();

        // Just a little bit of ADL
        std::string classParameterDescription = getClassMnemomicParameterDescription(static_cast<StrippedIOType<I>*>(nullptr));
        return getStoreVectorParameterDescription(classParameterDescription, order, input);
      }

      /** Compose a parameter name for the name of the vector on the DataStore.
       *  @param classMnemomic Short name of the value type that is stored in the vector
       *  @param order In case of mulitple occurances of the same type which occurance is it. 1 and 2  are supported.
       *  @param input Should the parameter name state that this is an input.
       */
      std::string
      getStoreVectorParameterName(std::string classMnenomic, int order, bool input) const
      {
        std::string orderPrefix;
        if (order == 1) {
          orderPrefix = "";
        } else if (order == 2) {
          orderPrefix = "secondary";
        } else {
          B2ERROR("More than two inputs of the same type are not supported");
        }
        std::string inputPrefix = input ? "input" : "";

        if (input or order > 1) {
          classMnenomic[0] = ::toupper(classMnenomic.at(0));
        }
        if (input and order > 1) {
          inputPrefix[0] = ::toupper(inputPrefix.at(0));
        }

        return orderPrefix + inputPrefix + classMnenomic + "s";
      }

      /** Compose a parameter description for the name of the vector on the DataStore.
       *  @param classMnemomic Short name of the value type that is stored in the vector
       *  @param order In case of mulitple occurances of the same type which occurance is it. 1 and 2 are supported.
       *  @param input Should the parameter name state that this is an input.
       */
      std::string getStoreVectorParameterDescription(const std::string classMnenomic, int order, bool input) const
      {
        std::string orderPrefix;
        if (order == 1) {
          orderPrefix = "";
        } else if (order == 2) {
          orderPrefix = "secondary ";
        } else {
          B2ERROR("More than two inputs of the same type are not supported");
        }
        std::string inputPrefix = input ? "input " : "";
        return "Name of the " + orderPrefix + inputPrefix + classMnenomic + " vector.";
      }

    private:
      /// Parameters : Names of the vectors on the DataStore
      std::array<std::string, c_nTypes> m_param_storeVectorNames;

      /// Findlet that implements the algorithm to be executed.
      AFindlet m_findlet;
    };
  }
}
