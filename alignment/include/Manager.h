/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <map>
#include <Geant4/G4Transform3D.hh>
#include <root/TMatrixD.h>
#include <genfit/StateOnPlane.h>
#include <root/TGeoMatrix.h>
#include <set>

#include <framework/logging/Logger.h>

#include <framework/dbobjects/BeamParameters.h>
#include <alignment/dbobjects/VXDAlignment.h>
#include <alignment/dbobjects/CDCCalibration.h>
#include <alignment/dbobjects/BKLMAlignment.h>
#include <eklm/dbobjects/EKLMAlignment.h>

#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/dbobjects/CDCTimeWalks.h>
#include <cdc/dbobjects/CDCAlignment.h>
#include <cdc/dbobjects/CDCXtRelations.h>

#include <alignment/GlobalParam.h>
#include <alignment/GlobalLabel.h>
#include <alignment/Hierarchy.h>

namespace Belle2 {
  namespace alignment {

    /*
    class GlobalCalibrationManager;
    class ManagerController;
    class ManagerStateBase;
    class ManagerTransitionBase;

    class IControlerStateMachine {
    public:
      void transitionRequested(ManagerTransitionBase&) {}
      void addStates(const std::vector<ManagerStateBase*>&) {}
      void addTransitions(const std::vector<ManagerTransitionBase*>&) {}
    };

    template<int number>
    struct StateNumber {
      enum {state = number};
    };

    template<int left, int right>
    struct CompareStatesLess {
      enum {boolean = static_cast<bool>(left < right)};
    };

    class ManagerStateBase {
    public:
      ManagerStateBase(int state) : m_state(state) {}
      virtual ~ManagerStateBase() {}
      int m_state {-1};
    };

    template<int State>
    class ManagerState : public ManagerStateBase {
    public:
      ManagerState() : ManagerStateBase(State) {}
      virtual ~ManagerState() {}
      enum {state = State};
    };

    class ManagerTransitionBase {
    public:
      ManagerTransitionBase(std::shared_ptr<IControlerStateMachine> ctrl) : m_ctrl(ctrl) {}
      virtual ~ManagerTransitionBase() {}
      virtual void operator()() {m_ctrl->transitionRequested(*this);}
      virtual bool operator()(int from, int to) = 0;
    private:
      std::shared_ptr<IControlerStateMachine> m_ctrl {};
    };

    template<int From, int To>
    class ManagerTransition : public ManagerTransitionBase {
      public:
        ManagerTransition(ManagerController& ctrl) : ManagerTransitionBase(ctrl) {}
        virtual ~ManagerTransition() {}
        virtual bool operator()(int from, int to) override {return (from == From) and (to == To);}

      private:
    };



    class ManagerInterfaceGlobalVect {
    public:
      //GlobalParamVector& getGlobalParamVector() {return GlobalCalibrationManager::getInstance().getGlobalParamVector();}
    };



    class ManagerConstructed   : public ManagerState<0> {};
    class ManagerPrepared      : public ManagerState<1>, public ManagerInterfaceGlobalVect {};
    class ManagerRunStartEnd   : public ManagerState<2> {};
    class ManagerCollecting    : public ManagerState<3> {};
    class ManagerTerminated    : public ManagerState<4> {};
    class ManagerInitialized   : public ManagerState<5> {};
    class ManagerCalibrated    : public ManagerState<6> {};
    // State transitions during data collection
    class prepareManager       : public ManagerTransition<0, 1> {};
    class beginRunManager      : public ManagerTransition<1, 2> {};
    class eventManager         : public ManagerTransition<2, 3> {}; // = first event
    class collectManager       : public ManagerTransition<3, 3> {}; // <- real event (collection) loop
    class endRunManager        : public ManagerTransition<3, 2> {};
    class terminateManager     : public ManagerTransition<3, 4> {};
    // State transitions during calibration
    class initializeManager    : public ManagerTransition<4, 5> {};
    class calibrateManager     : public ManagerTransition<5, 6> {};



    template <int N, class Enable = void>
    class ManagerInState {
    };

    class ManagerConstGetters {
    public:
      void getters() {}
    };


    template <int N>
    class ManagerInState<N, typename std::enable_if<N == 0>::type> : public ManagerConstGetters
    {
    public:
      void config() {}
      void init() {}
    };

    template <int N>
    class ManagerInState<N, typename std::enable_if<N == 1>::type> : public ManagerConstGetters
    {
    public:
      void collect() {} // -> disable 0
      void terminate() {}
    };

    template <int N>
    class ManagerInState<N, typename std::enable_if<N == 2>::type> : public ManagerConstGetters
    {
    public:
      void calibrate() {}
    };

    class ManagerController : public IControlerStateMachine {
      friend class ManagerTransitionBase;
    public:
      ManagerController() {}
      ~ManagerController() {}
      /// TODO: temp
      void receiveTransitionMsg(int from, int to) {
        std::cout << "Msg to change state from " << from << " to " << to << std::endl;
      }
    private:
      int m_currentState = -1;
      std::vector<int> m_entryStates {0, 4};
    };

    void test() {
      //GlobalCalibration<ManagerConstructed>().execute([](void) -> void { std::cout << " test " << std::endl; });
      //GlobalCalibration<ManagerConstructed>().make();
      ManagerInState<0> man0;
      man0.config();
      man0.getters();
      man0.init();

      ManagerInState<1> man1;
      man1.collect();
      man1.getters();
      man1.terminate();

      ManagerInState<2> man2;
      man2.getters();
      man2.calibrate();

    }
    */


    /// Class to hold hierarchy of whole Belle2
    class GlobalCalibrationManager {

    public:



      /// Comparison function for EventMetaData
      std::function<bool(const EventMetaData&, const EventMetaData&)> cmpEventMetaData = [](const EventMetaData& lhs,
      const EventMetaData& rhs) -> bool {
        if (lhs.getExperiment() < rhs.getExperiment()) return true;
        if (lhs.getRun() < rhs.getRun()) return true;
        if (lhs.getEvent() < rhs.getEvent()) return true;
        return false;
      };

      /// Destructor
      ~GlobalCalibrationManager() {}
      /// Get instance of the Manager
      ///     auto& gcm = GlobalCalibrationManager::getInstance();
      ///
      /// Note you actually initialize the Manager with your specific
      /// selection of DB objects to calibrate (or select all for global
      /// calibration -default) and other options.
      static GlobalCalibrationManager& getInstance();


      /// Initialize a given GlobalParamVector with all DB objects and interfaces
      ///
      ///       auto gpv = GlobalParamVector({"BeamParameters"});
      ///       GlobalCalibrationManager::initGlobalVector(gpv);
      ///
      /// This one central function should be used to prepare the global vectors even
      /// for local calibrations using Millepede. By setting the components argument
      /// of GlobalParamVector one can limit which addDBObj<>() calls are actually executed
      static void initGlobalVector(GlobalParamVector& vector);

      /// Sort vector of EventMetaData and make it unique to process
      /// user defined time splittings of the constants
      void sortUniqueVector(std::vector<EventMetaData>& vec);

      /// -------------------------------------------------------------------------
      /// To be used by MillepedeCollector

      /// Initialize the manager with given configuration (from MillepedeCollector)
      void initialize(const std::vector<std::string>& components = {}, const std::vector<EventMetaData>& timeSlices = {});

      /// Notice manager of a comming event (from MillepedeCollector)
      void preCollect(const EventMetaData& emd);

      /// -------------------------------------------------------------------------
    public:
      /// Get the rigid body alignment hierarchy
      RigidBodyHierarchy& getAlignmentHierarchy() const { return *m_alignment; }
      /// Get the Lorentz shift hierarchy
      LorentShiftHierarchy& getLorentzShiftHierarchy() const { return *m_lorentzShift; }
      /// Get the constraints collected so far
      Constraints& getConstraints() { return m_constraints; }

      /// Write-out complete hierarchy to a text file
      void writeConstraints(std::string txtFilename);

    private:
      /** Singleton class, hidden constructor */
      GlobalCalibrationManager() {};
      /** Singleton class, hidden copy constructor */
      GlobalCalibrationManager(const GlobalCalibrationManager&);
      /** Singleton class, hidden assignment operator */
      GlobalCalibrationManager& operator=(const GlobalCalibrationManager&);

      /// Update the current time ID in the GlobalLabel
      /// @param emd current EventMetaData - generated time ID
      /// will be the index of the smallest slice larger than (or. equal) to EventMetaData
      int updateTimeDepGlobalLabels(const EventMetaData& event);


      /// Finished initialization?
      bool m_initialized {false};

      /// The alignment hierarchy
      std::unique_ptr<RigidBodyHierarchy> m_alignment {new RigidBodyHierarchy()};
      /// Hierarchy for Lorentz shift corrections
      std::unique_ptr<LorentShiftHierarchy> m_lorentzShift {new LorentShiftHierarchy()};
      /// The global vector for unified access to DB constants
      std::unique_ptr<GlobalParamVector> m_globalVector {new GlobalParamVector()};
      /// Map of constraints {unique label, labels and coefficients}
      std::map<int, Constraint> m_constraints {};

      /// Vector of EventMetaData containing the time slicing of the calibration job
      std::vector<EventMetaData> m_dbTimeSlicing {};
    };

  }
}
