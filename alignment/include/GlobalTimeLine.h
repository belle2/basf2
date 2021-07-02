/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <alignment/GlobalLabel.h>
#include <alignment/GlobalParam.h>
#include <framework/dataobjects/EventMetaData.h>

#include <Python.h>

#include <memory>

namespace Belle2 {
  namespace alignment {
    namespace timeline {

      /// Pair to hold exp + run number
      typedef std::pair<int, int> ExpRun;
      /// "Header" with actuall starting events (EventMetaData) assigned to each time interval
      typedef std::vector<EventMetaData> EventHeader;
      /// "Header" with runs corresponding to events in EventHeader
      typedef std::vector<ExpRun> RunHeader;
      /// Map of keys (unique ID of payload - like 1 for BeamSpot) -> rows
      /// A row is vector of integers: either 0s and 1s (for initial table) or indices
      /// of payloads using continuous numbering in PayloadsTable
      typedef std::unordered_map<int, std::vector<int>> TableData;
      /// Event header + run header + map with rows at tuple
      typedef std::tuple<EventHeader, RunHeader, TableData> TimeTable;

      /// The smallest unit is a run with event dependencies. For no event dependency, a run or run
      /// range will contain vector of size 1 (the usuall case)
      /// The shared pointer host the generic accessor to DB objects exposing interface for Millepede
      typedef std::vector< std::pair<EventMetaData, std::shared_ptr<GlobalParamSetAccess>>> IntraIoVPayloads;
      /// The row is composed of IoVs and blocks with either single payload or and intra-run dependency
      typedef std::vector<std::pair<IntervalOfValidity, IntraIoVPayloads>> PayloadIovBlockRow;
      /// The of of unique ids to the rows with their payloads
      typedef std::unordered_map<int, PayloadIovBlockRow> PayloadsTable;

      /// Move in TimeTable until the next change in other run
      ///
      /// @param timeTable The finalized TimeTable with payload indices
      /// @param uid the unique id of the payload (aka the row)
      /// @param timeid reference to the index (column) from which to start
      /// WARNING: the timeid index is advanced to the last position when the payload
      /// at starting index is valid. Will not change if already at end of validity.
      /// @return the EventMetaData of the final column and row.
      EventMetaData gotoNextChangeRunWise(TimeTable& timeTable, int uid, int& timeid);

      /// Move in TimeTable until the next possible change for payload in same run
      ///
      /// @param timeTable The finalized TimeTable with payload indices
      /// @param uid the unique id of the payload (aka the row)
      /// @param timeid reference to the index (column) from which to start
      /// WARNING: the timeid index is advanced to the last position when the payload
      /// at starting index is valid.  Will not change if already at last intra-run dependency
      /// @return the EventMetaData of the final column and row.
      EventMetaData gotoNextChangeInRun(TimeTable& timeTable, int uid, int& timeid);

      /// Convert the finalized TimeTable to PayloadsTable
      /// @param timeTable finalized TimeTable
      /// @param vector GlobalParamVector initialized with DB object for which payloads should be generated
      PayloadsTable TimeIdsTable2PayloadsTable(TimeTable& timeTable, const GlobalParamVector& vector);

      /// Create the initial TimeTable fromm the mapping of timedep parameters
      /// in GlobalLabel
      /// @param events The vector of EventMetaData to interpret the indices of columns in TimeTable
      /// @param label This more for style - only static functions are called on this to read the mapping
      /// of parameters to time intervals
      /// @return the initial TimeTable with only timedep payload rows and only 0s or 1s in rows
      /// - 0 means the payload is the same as in previous column
      /// - 1 means the payload can change at this column
      TimeTable makeInitialTimeTable(std::vector<EventMetaData> events, GlobalLabel& label);

      /// Convert the initial TimeTable (with 0s and 1s) to final table of payload indices
      /// @param table the initial table
      /// @return the final table with indices
      void finalizeTimeTable(TimeTable& table);

      /// Get payload by the index from final TimeTable
      /// @param payloadsTable the payload table to look in
      /// @param uid of row to look in
      /// @param index the index from final TimeTable
      std::pair<EventMetaData, std::shared_ptr<GlobalParamSetAccess>> getPayloadByContinuousIndex(PayloadsTable& payloadsTable, int uid,
          long unsigned int index);;

      /// Get cell (continous index of payload) at given row and column
      /// @param uid of row
      /// @param timeid of column
      int getContinuousIndexByTimeID(const TimeTable& timeTable, int uid, int timeid);

      /// Setup the complete time dependence of parameters at once (ensures consistency) (Python version)
      ///
      /// (Calls GlobalLabel static functions internally to fill its timedep. map)
      ///
      /// @param config python list of tuples of size 2, first element is list of parameter numbers
      /// retrieved by GlobalLabel.construct(payload id (uid), element id, param id) (with empty timedep map)
      /// second is list of event metadata as tuple of size 3 (event, run, exp)
      /// For example:
      ///
      /// >>> config = [([1, 2], [(0, 0, 0), (111, 0, 0)]), ([3], (0, 3, 0))]
      /// >>> setupTimedepGlobalLabels(config)
      ///
      /// will define parameters 1 and 2 to timedep with values changing at event 0 and 111 of run 0 exp 0
      /// and (added automatically) at event 0 of run 1. Parameter 3 can change its value from run 2 to 3.
      ///
      /// @return vector of EventMetaData with the finest granularity inferred from the parameter configuration
      ///
      std::vector<EventMetaData> setupTimedepGlobalLabels(PyObject* config);

      /// Setup the complete time dependence of parameters at once (ensures consistency) (C++ version)
      ///
      /// (Calls GlobalLabel static functions internally to fill its timedep. map)
      ///
      /// @param config vector of tuples of size 2, first element is vector of parameter numbers
      /// retrieved by GlobalLabel.construct(payload id (uid), element id, param id) (with empty timedep map)
      /// second is vector of event metadata as tuple of size 3 (event, run, exp)
      /// For example:
      ///
      /// >>> config = [([1, 2], [(0, 0, 0), (111, 0, 0)]), ([3], (0, 3, 0))]
      /// >>> setupTimedepGlobalLabels(config)
      ///
      /// will define parameters 1 and 2 to be timedep with values changing at event 0 and 111 of run 0 exp 0
      /// and (added automatically) at event 0 of run 1. Parameter 3 can change its value from run 2 to 3.
      ///
      /// @return vector of EventMetaData with the finest granularity inferred from the parameter configuration
      ///
      /// WARNING: The function expects event metadata tuple in form (event, run, exp) while the implementation internally
      /// reverses this for proper sorting of event metadata in sets!
      std::vector<EventMetaData> setupTimedepGlobalLabels(
        std::vector< std::tuple< std::vector< int >, std::vector< std::tuple< int, int, int > > > >& config);

      /// Convenient class to automatically create payloads from allowed time
      /// depedence of parameter, load their value from database, update te constants one by one
      /// usually from Millepde result) and output the final payloads (including EventDependencies)
      /// such that one can store them (updated) in the database
      class GlobalParamTimeLine {

      private:
        /// The final TimeTable with payload indices
        TimeTable timeTable{};
        /// Table with payloads
        PayloadsTable payloadsTable{};

      public:

        /// Constructor
        /// @param events vector of events to interpret time ids from GlobalLabel
        /// @param label (only for style) - static functions called to read the mapping of parameters and time intervals
        /// @param vector the global vector initialized with DB objects for which payloads shoudl be generated
        /// WARNING: do not construct() or loadFromDB() the vector - use it "raw" - the internal object handlers
        /// are copied into the payloads table constructing the internal DB objects would result in copiyng them around, too
        GlobalParamTimeLine(const std::vector<EventMetaData>& events, GlobalLabel& label, const GlobalParamVector& vector);

        /// Load every single payload with the content in database at its corresponding
        /// event (when it should start to be valid)
        /// WARNING: This is potentionally a very expensive operation with lots of HTTP communication
        /// TODO: make the loading from DB slightly more optimized -> needs change in GlobalParamSet<...>
        void loadFromDB();

        /// Add a correction to any payload's parameter in the timeline
        /// @param label the label any global parameter constructed from integer after Millepede calibration
        /// The mapping of time intervals has to be loaded for the label to provide correct infortmation
        /// about its validity in time intervals -> all payloads are updated until the end of validity of this
        /// parameter
        /// @param correction the value to be added to the given constant
        /// @param resetParam if True, the parameters is not updated with correction, but set to 'correction' value
        /// This has special use-case when filling the objects with errors/corrections data instead of absolute
        /// parameter values
        void updateGlobalParam(GlobalLabel label, double correction, bool resetParam = false);

        /// Release all the objects (you become the owner!) for DB storage
        /// @return vector of pairs. First is the IoV to store this in DB. Second is the object (payload) itself.
        /// Note that this might be EventDependencies in case objects can change inside runs
        std::vector<std::pair<IntervalOfValidity, TObject*>> releaseObjects();

      };

    } // namespace timeline
  } // namespace alignment
} // namespace Belle2
