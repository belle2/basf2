/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/GlobalTimeLine.h>

#include <alignment/Manager.h>
#include <framework/core/PyObjConvUtils.h>
#include <framework/database/EventDependency.h>

namespace Belle2 {
  namespace alignment {
    namespace timeline {

      EventMetaData gotoNextChangeRunWise(TimeTable& timeTable, int uid, int& timeid)
      {
        auto& row = std::get<TableData>(timeTable).at(uid);

        auto lastIntervalStartEvent = std::get<EventHeader>(timeTable).at(row.size() - 1);

        if (timeid >= int(row.size())) {
          return lastIntervalStartEvent;
        }

        auto cell = row.at(timeid);
        auto cellRun = std::get<RunHeader>(timeTable).at(timeid);
        for (long unsigned int iCol = timeid + 1; iCol < row.size(); ++iCol) {
          if (row.at(iCol) != cell && std::get<RunHeader>(timeTable).at(iCol) != cellRun) {
            timeid = iCol - 1;
            return std::get<EventHeader>(timeTable).at(iCol);
            //return std::get<EventHeader>(timeTable).at(iCol - 1);
          }
        }
        timeid = row.size() - 1;

        return lastIntervalStartEvent;
      }

      EventMetaData gotoNextChangeInRun(TimeTable& timeTable, int uid, int& timeid)
      {
        auto& row = std::get<TableData>(timeTable).at(uid);
        auto cell = row.at(timeid);
        auto cellRun = std::get<RunHeader>(timeTable).at(timeid);
        for (long unsigned int iCol = timeid + 1; iCol < row.size(); ++iCol) {
          if (std::get<RunHeader>(timeTable).at(iCol) != cellRun) {
            timeid = iCol - 1;
            return std::get<EventHeader>(timeTable).at(iCol - 1);
          }
          if (row.at(iCol) != cell) {
            timeid = iCol;
            return std::get<EventHeader>(timeTable).at(iCol);
          }
        }
        return std::get<EventHeader>(timeTable).at(timeid);
      }

      PayloadsTable TimeIdsTable2PayloadsTable(TimeTable& timeTable, const GlobalParamVector& vector)
      {
        PayloadsTable payloadsTable;

        for (auto& uid_obj : vector.getGlobalParamSets()) {
          auto uid = uid_obj.first;
          auto& obj = uid_obj.second;

          payloadsTable[uid] = {};

          if (std::get<TableData>(timeTable).find(uid) == std::get<TableData>(timeTable).end()) {
            auto firstEvent = std::get<EventHeader>(timeTable).at(0);
//             auto lastEvent = std::get<EventHeader>(timeTable).at(std::get<EventHeader>(timeTable).size() - 1);

            auto iov = IntervalOfValidity(firstEvent.getExperiment(), firstEvent.getRun(), -1, -1);
            auto objCopy = std::shared_ptr<GlobalParamSetAccess>(obj->clone());
            payloadsTable[uid].push_back({ iov, {{firstEvent, objCopy}} });

            continue;
          }
          int iCol = 0;
          // Now add PayloadIovBlockRow with run-spanning IoVs
          for (; iCol < int(std::get<EventHeader>(timeTable).size()); ++iCol) {
            auto event = std::get<EventHeader>(timeTable).at(iCol);
            auto exprun = std::get<RunHeader>(timeTable).at(iCol);
            auto exp = exprun.first;
            auto run = exprun.second;

            // Prepare intra run objects
            // 1st is always there (even for non-intra-run)
            auto objCopy = std::shared_ptr<GlobalParamSetAccess>(obj->clone());
            IntraIoVPayloads intraRunEntries;
            intraRunEntries.push_back({event, objCopy});
            // At each change in run, add new entry
            auto lastEvent = event;
            for (; iCol < int(std::get<EventHeader>(timeTable).size());) {
              auto nextEvent = gotoNextChangeInRun(timeTable, uid, iCol);
              if (nextEvent != lastEvent) {
                auto objIntraRunCopy = std::shared_ptr<GlobalParamSetAccess>(obj->clone());
                intraRunEntries.push_back({nextEvent, objIntraRunCopy});
                lastEvent = nextEvent;
              } else {
                break;
              }
            }

            // Move to next IoV block (for intra-run deps in just processed block, next block is always the next run)
            auto endEvent = gotoNextChangeRunWise(timeTable, uid, iCol);
            int endExp = endEvent.getExperiment();
            //int endRun = endEvent.getRun();
            int endRun = std::max(0, endEvent.getRun() - 1);
            // Last IoV open:
            if (iCol == static_cast<int>(std::get<EventHeader>(timeTable).size()) - 1) {
              endRun = -1;
              endExp = -1;
            }
            // Store finished block
            payloadsTable[uid].push_back({IntervalOfValidity(exp, run, endExp, endRun), intraRunEntries});

          }

        }

        return payloadsTable;
      }

      TimeTable makeInitialTimeTable(std::vector< EventMetaData > events, GlobalLabel& label)
      {
        TimeTable table;
        std::vector<int> nullRow(events.size(), 0);

        // event header
        std::get<EventHeader>(table) = events;

        // run header
        RunHeader runs;
        for (auto event : events) {
          runs.push_back({event.getExperiment(), event.getRun()});
        }
        std::get<RunHeader>(table) = runs;

        for (auto& eidpid_intervals : label.getTimeIntervals()) {
          auto uid = GlobalLabel(eidpid_intervals.first).getUniqueId();
          if (std::get<TableData>(table).find(uid) == std::get<TableData>(table).end()) {
            std::get<TableData>(table)[uid] = nullRow;
          }
          unsigned int lastTime = 0;
          for (long unsigned int timeid = 0; timeid < events.size(); ++timeid) {
            if (lastTime != eidpid_intervals.second.get(timeid)) {
              std::get<TableData>(table)[uid][timeid] = 1;
              lastTime = timeid;
            }
          }
        }

        return table;
      }

      void finalizeTimeTable(TimeTable& table)
      {
        for (auto& row : std::get<TableData>(table)) {
          auto& cells = row.second;

          int currIndex = 0;
          for (long unsigned int iCell = 0; iCell < cells.size(); ++iCell) {
            auto cell = cells.at(iCell);
            if (iCell == 0) {
              if (cell != 0) {
                B2FATAL("First cell (index 0) has to be zero (time id for const objects or 1st instance of time-dep objects) for each row.");
              }
              continue;
            }
            if (cell != 0 && cell != 1) {
              B2FATAL("In initial time table, only cells with 0 (=no change of object at beginning of cell) or 1 (object can change at beginning of this cell) are allowed");
            }
            // Now cell is not first and is either 0 or 1 (-> increment index)
            if (cell == 1) {
              ++currIndex;
            }
            cells.at(iCell) = currIndex;
          }
        }
      }

      std::pair< EventMetaData, std::shared_ptr< GlobalParamSetAccess > > getPayloadByContinuousIndex(PayloadsTable& payloadsTable,
          int uid, long unsigned int index)
      {
        auto& row = payloadsTable.at(uid);

        long unsigned int currentIndex = 0;
        for (long unsigned int iIovBlock = 0; iIovBlock < row.size(); ++iIovBlock) {
          if (currentIndex + row.at(iIovBlock).second.size() > index) {
            return row.at(iIovBlock).second.at(index - currentIndex);
          }
          currentIndex += row.at(iIovBlock).second.size();
        }

        return {EventMetaData(), {}};
      }

      int getContinuousIndexByTimeID(const TimeTable& timeTable, int uid, int timeid)
      {
        if (timeid <= 0)
          return 0;
        if (std::get<TableData>(timeTable).find(uid) == std::get<TableData>(timeTable).end())
          return 0;
        if (timeid >= int(std::get<TableData>(timeTable).at(uid).size()))
          return std::get<TableData>(timeTable).at(uid).size() - 1;

        auto cIndex = std::get<TableData>(timeTable).at(uid)[timeid];
        return cIndex;
      }

      // GlobalParamTimeLine class -------------------------------------------------------------------------------------

      GlobalParamTimeLine::GlobalParamTimeLine(const std::vector< EventMetaData >& events, GlobalLabel& label,
                                               const GlobalParamVector& vector) : timeTable(makeInitialTimeTable(events, label))
      {
        finalizeTimeTable(timeTable);
        payloadsTable = TimeIdsTable2PayloadsTable(timeTable, vector);

      }

      void GlobalParamTimeLine::loadFromDB()
      {
        std::map<std::tuple<int, int, int>, std::vector<std::shared_ptr<GlobalParamSetAccess>>> eventPayloads{};
        for (auto& row : payloadsTable) {
          for (auto& iovBlock : row.second) {
            for (auto& payload : iovBlock.second) {
              auto eventTuple = std::make_tuple((int)payload.first.getExperiment(), (int)payload.first.getRun(), (int)payload.first.getEvent());
              auto iter_and_inserted = eventPayloads.insert(
              {eventTuple, std::vector<std::shared_ptr<GlobalParamSetAccess>>()}
              );
              iter_and_inserted.first->second.push_back(payload.second);
            }
          }
        }
        for (auto event_payloads : eventPayloads) {
          auto event = EventMetaData(std::get<2>(event_payloads.first), std::get<1>(event_payloads.first), std::get<0>(event_payloads.first));
          DBStore::Instance().update(event);
          DBStore::Instance().updateEvent(event.getEvent());
          for (auto& payload : event_payloads.second) {
            payload->loadFromDBObjPtr();
          }
        }
      }

      void GlobalParamTimeLine::updateGlobalParam(GlobalLabel label, double correction, bool resetParam)
      {
        auto timeid = label.getTimeId();
        auto eov = label.getEndOfValidity();
        auto uid = label.getUniqueId();

        std::set<int> payloadIndices;
        // this is probably dangerous if we do not impose additional invariant
        //TODO: better to always loop over whole event header?
        for (int i = timeid; i < std::min(eov + 1, int(std::get<EventHeader>(timeTable).size())); ++i) {
          payloadIndices.insert(getContinuousIndexByTimeID(timeTable, uid, i));
        }

        for (auto payloadIndex : payloadIndices) {
          auto payload = getPayloadByContinuousIndex(payloadsTable, label.getUniqueId(), payloadIndex).second;
          // If not found, we get an empty payload shared ptr
          if (payload) {
            if (resetParam) {
              payload->setGlobalParam(correction, label.getElementId(), label.getParameterId());
            } else {
              payload->updateGlobalParam(correction, label.getElementId(), label.getParameterId());
            }
          }
        }

      }

      std::vector< std::pair< IntervalOfValidity, TObject* > > GlobalParamTimeLine::releaseObjects()
      {
        std::vector<std::pair<IntervalOfValidity, TObject*>> result;

        for (auto& row : payloadsTable) {
          for (auto& iovBlock : row.second) {
            auto iov = iovBlock.first;
            auto obj = iovBlock.second.at(0).second->releaseObject();

            // non-intra-run
            if (iovBlock.second.size() == 1) {
              if (obj)
                result.push_back({iov, obj});

              continue;
            }

            // First obj in event dependency
            //TODO: how the lifetime of EventDependency is handled?
            // both work now -> have to check data storage in DB in real life scenario
            auto payloads = new EventDependency(obj);
            //auto payloads = EventDependency(obj);
            // Add others
            for (long unsigned int iObj = 1; iObj < iovBlock.second.size(); ++iObj) {
              auto nextEvent = iovBlock.second.at(iObj).first.getEvent();
              auto nextObj = iovBlock.second.at(iObj).second->releaseObject();

              if (nextObj)
                payloads->add(nextEvent, nextObj);
              //payloads.add(nextEvent, nextObj);
            }
            result.push_back({iov, payloads});
            //result.push_back({iov, &payloads});
          }
        }
        return result;
      }

      std::vector< EventMetaData > setupTimedepGlobalLabels(PyObject* config)
      {
        boost::python::handle<> handle(boost::python::borrowed(config));
        boost::python::list configList(handle);
        auto newConfig = PyObjConvUtils::convertPythonObject(configList,
                                                             std::vector< std::tuple< std::vector< int >, std::vector< std::tuple< int, int, int > > > >());
        return setupTimedepGlobalLabels(newConfig);

      }


      std::vector<EventMetaData> setupTimedepGlobalLabels(
        std::vector< std::tuple< std::vector< int >, std::vector< std::tuple< int, int, int > > > >& config)
      {
        std::vector< std::tuple< std::set< int >, std::set< std::tuple< int, int, int > > > > myConfig = {};
        std::set<std::tuple<int, int, int>> events;
        for (auto& params_events : config) {
          auto myRow = std::make_tuple(std::set<int>(), std::set<std::tuple< int, int, int>>());

          for (auto& param : std::get<0>(params_events))
            std::get<0>(myRow).insert(param);
          for (auto& event : std::get<1>(params_events)) {
            // WARNING: The function expect event metadata tuple in form (event, run, exp) while the implementation internally
            // reverses this for proper sorting of event metadata in sets!
            std::get<1>(myRow).insert(std::make_tuple(std::get<2>(event), std::get<1>(event), std::get<0>(event)));
          }

          for (auto& event : std::get<1>(params_events)) {
            int eventNum = std::get<0>(event);
            int runNum = std::get<1>(event);
            int expNum = std::get<2>(event);

            // WARNING: here we also need reversed order for the set to be sorted in ascending order
            auto emd = std::make_tuple(expNum, runNum, eventNum);
            events.insert(emd);


            if (eventNum != 0) {
              // Automatically add start of this run and start of next run as points where params can change
              //NOTE: this is the main invariant we need to keep - if something can change inside run, it is expected
              // it did change since last run and will change for next run, too... (i.e. if there is an event depencency,
              // the IoV of the payload has to span only single run)
              auto firstEventThisRun = std::make_tuple(expNum, runNum, 0);
              auto firstEventNextRun = std::make_tuple(expNum, runNum + 1, 0);

              events.insert(firstEventThisRun);
              events.insert(firstEventNextRun);

              std::get<1>(myRow).insert(firstEventThisRun);
              std::get<1>(myRow).insert(firstEventNextRun);
            }
          }
          myConfig.push_back(myRow);
        }

        std::vector<EventMetaData> eventsVect;
        std::map<std::tuple<int, int, int>, int> eventIndices;


        for (auto& event : events) {
          // WARNING: here we reverse order of exp,run,event back to "normal"
          eventIndices[event] = eventsVect.size();
          eventsVect.push_back(EventMetaData(std::get<2>(event), std::get<1>(event), std::get<0>(event)));
        }

        GlobalLabel::clearTimeDependentParamaters();

        for (auto& params_events : myConfig) {
          for (auto& param : std::get<0>(params_events)) {
            GlobalLabel label(param);

            for (auto& event : std::get<1>(params_events)) {
              auto eventIndex = eventIndices[event];
              //if (eventIndex > 0)
              label.registerTimeDependent(eventIndex);

            }
          }
        }

        return eventsVect;
      }
    }
  }
}
