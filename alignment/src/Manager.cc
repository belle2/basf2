#include <alignment/Manager.h>
#include <cdc/dbobjects/CDCLayerAlignment.h>

namespace Belle2 {
  namespace alignment {

    GlobalCalibrationManager& GlobalCalibrationManager::getInstance()
    {
      static std::unique_ptr<GlobalCalibrationManager> instance(new GlobalCalibrationManager());
      return *instance;
    }

    void GlobalCalibrationManager::initGlobalVector(GlobalParamVector& vector)
    {
      // Interfaces for sub-detectors
      auto cdcInterface = std::shared_ptr<IGlobalParamInterface>(new CDCGlobalParamInterface());
      auto vxdInterface = std::shared_ptr<IGlobalParamInterface>(new VXDGlobalParamInterface());

      // Try add all supported DB objects
      // - will be not added if not in selected components of the 'vector'
      vector.addDBObj<BeamParameters>();
      vector.addDBObj<VXDAlignment>(vxdInterface);
      vector.addDBObj<CDCAlignment>(cdcInterface);
      vector.addDBObj<CDCLayerAlignment>(cdcInterface);
      vector.addDBObj<CDCTimeZeros>(cdcInterface);
      vector.addDBObj<CDCTimeWalks>(cdcInterface);
      vector.addDBObj<CDCXtRelations>(cdcInterface);
      vector.addDBObj<BKLMAlignment>();
      vector.addDBObj<EKLMAlignment>();
    }

    void GlobalCalibrationManager::initialize(const std::vector< std::string >& components,
                                              const std::vector< EventMetaData >& timeSlices)
    {
      // Reset the config of global vector to restrict components if needed
      m_globalVector.reset(new GlobalParamVector(components));
      // Fill with DB objects and interfaces according to components
      initGlobalVector(*m_globalVector);
      // Set the components for global label system
      GlobalLabel::setComponents(m_globalVector->getComponentsIDs());

      // Reset time intervals and add pre-defined times where
      // constants can change (in addition of what is already in DB)
      m_dbTimeSlicing = timeSlices;
      sortUniqueVector(m_dbTimeSlicing);

      StoreObjPtr<EventMetaData> emd;
      if (emd.isValid()) updateTimeDepGlobalLabels(*emd);

      // already during initialization (due to geometry) we know the first event
      // to be processed, so we can actually load the consts here - and construct the vector
      //
      // m_globalVector->loadFromDB();
      // m_globalVector->listGlobalParams().size();

      // event data loaded
      // gearbox loaded
      // geometry loaded
      // extrapolation setup

      // Try to build hierarchy now by interfaces
      //TODO by restricting components you stop to observe some hierarchy changes!
      m_globalVector->postHierarchyChanged(getAlignmentHierarchy());
      m_globalVector->postHierarchyChanged(getLorentzShiftHierarchy());

      // Try to init as much as possible before event processing
      // geometry, reconstruction config -> recohits & derivatives

      m_initialized = true;

    }

    void GlobalCalibrationManager::preCollect(const EventMetaData& emd)
    {
      // TODO Really?? Move elsewhere
      // Add time slices when objects change automatically
      /*
      if (m_globalVector->hasBeenChangedInDB({}, false)) {
        // Add time slices
        m_dbTimeSlicing.push_back(EventMetaData(emd));
        sortUniqueVector(m_dbTimeSlicing);

        for (auto& handler : m_globalVector->getGlobalParamSets()) {
          if (not handler.second->hasBeenChangedInDB(false))
            continue;
          //TODO listGlobalParams not fully implemented by DB objects
          for (auto& params : handler.second->listGlobalParams()) {
            GlobalLabel label;
            label.construct(handler.second->getGlobalUniqueID(), params.first, params.second);
            label.registerTimeDependent(GlobalLabel::getCurrentTimeInterval());
          }
        }
      }
      */

      // Generate time id from current EventMetaData
      // range for time dependent calibration consts
      updateTimeDepGlobalLabels(emd);

      // Update hierarchy and constraints if DB objects changed
      if (m_globalVector->hasBeenChangedInDB(getAlignmentHierarchy().getUsedDBObjUniqueIDs(), false)) {
        m_globalVector->postHierarchyChanged(getAlignmentHierarchy());
        getAlignmentHierarchy().buildConstraints(m_constraints);
      }
      if (m_globalVector->hasBeenChangedInDB(getLorentzShiftHierarchy().getUsedDBObjUniqueIDs(), false)) {
        m_globalVector->postHierarchyChanged(getLorentzShiftHierarchy());
        getLorentzShiftHierarchy().buildConstraints(m_constraints);
      }

      // reset the hasBeenChanged state, ignore return value
      // TODO move to postCollect()?
      m_globalVector->hasBeenChangedInDB();
    }

    void GlobalCalibrationManager::writeConstraints(std::string txtFilename)
    {
      std::ofstream txt(txtFilename);
      // Last update before write-out
      m_alignment->buildConstraints(m_constraints);
      m_lorentzShift->buildConstraints(m_constraints);
      for (auto& name_elements : m_constraints) {
        auto name = name_elements.first;
        txt << "Constraint 0. ! Constraint for global label: " << name << std::endl;
        auto& elements = name_elements.second;
        for (auto& label_coefficient : elements) {
          auto label = label_coefficient.first;
          auto coeff = label_coefficient.second;
          txt << label << " " << coeff << std::endl;
        }
        txt << std::endl << std::endl;
      }
    }

    void GlobalCalibrationManager::sortUniqueVector(std::vector< EventMetaData >& vec)
    {
      std::sort(vec.begin(), vec.end(), cmpEventMetaData);
      vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }

    int GlobalCalibrationManager::updateTimeDepGlobalLabels(const EventMetaData& event)
    {
      //sortUniqueVector(m_dbTimeSlicing);

      if (m_dbTimeSlicing.empty()) {
        GlobalLabel::setCurrentTimeInterval(0);
        return 0;
      }

      /// These are intentionally two loops
      /// One goes to future events, the second backwards if ever needed
      /// This should deal with issues like randomized order of events in processing
      for (unsigned int index = GlobalLabel::getCurrentTimeInterval(); index < m_dbTimeSlicing.size(); ++index) {
        if (not cmpEventMetaData(event, m_dbTimeSlicing[index])) {
          // emd < slice <=> slice >= emd => first slice to contain this event
          GlobalLabel::setCurrentTimeInterval(index);
          return index;
        }
      }

      for (int index = GlobalLabel::getCurrentTimeInterval(); index >= 0; --index) {
        if (not cmpEventMetaData(event, m_dbTimeSlicing[index])) {
          GlobalLabel::setCurrentTimeInterval(index);
          return index;
        }
      }

      GlobalLabel::setCurrentTimeInterval(0);
      return 0;
    }
  }
}
