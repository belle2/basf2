/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

#include <mdst/dbobjects/BeamSpot.h>
#include <alignment/dbobjects/VXDAlignment.h>
#include <alignment/dbobjects/CDCCalibration.h>
#include <klm/dbobjects/bklm/BKLMAlignment.h>
#include <klm/dbobjects/eklm/EKLMAlignment.h>
#include <klm/dbobjects/eklm/EKLMSegmentAlignment.h>

#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/dbobjects/CDCTimeWalks.h>
#include <cdc/dbobjects/CDCAlignment.h>
#include <cdc/dbobjects/CDCXtRelations.h>

#include <alignment/GlobalParam.h>
#include <alignment/GlobalTimeLine.h>
#include <alignment/Hierarchy.h>

#include <map>
#include <string>
#include <vector>

namespace Belle2 {
  namespace alignment {

    /// Class to hold hierarchy of whole Belle2
    class GlobalCalibrationManager {

    public:



      /// Comparison function for EventMetaData
      std::function<bool(const EventMetaData&, const EventMetaData&)> cmpEventMetaData = [](const EventMetaData& lhs,
      const EventMetaData& rhs) -> bool {
        if (lhs.getExperiment() < rhs.getExperiment()) return true;
        if (lhs.getExperiment() > rhs.getExperiment()) return false;

        if (lhs.getRun() < rhs.getRun()) return true;
        if (lhs.getRun() > rhs.getRun()) return false;

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
      ///       auto gpv = GlobalParamVector({"BeamSpot"});
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
      /// @param event current event EventMetaData - generated time ID
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
      std::map<long, Constraint> m_constraints {};

      /// Vector of EventMetaData containing the time slicing of the calibration job
      std::vector<EventMetaData> m_dbTimeSlicing {};

      /// The initial time table generated from time intervals in GlobalLabel
      /// used to check if payload (labels) can change and update the hierarchy if so
      alignment::timeline::TimeTable m_iniTimeTable{};
    };

  }
}
