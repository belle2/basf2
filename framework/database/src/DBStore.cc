/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/DBStore.h>
#include <framework/database/Database.h>
#include <framework/logging/Logger.h>

#include <TClass.h>

namespace Belle2 {

  DBStore::~DBStore()
  {
    reset();
  }

  DBStore& DBStore::Instance()
  {
    static DBStore instance;
    return instance;
  }

  DBStoreEntry* DBStore::getEntry(DBStoreEntry::EPayloadType type, const std::string& name,
                                  const TClass* objClass, bool array, bool required)
  {
    // Check whether the map entry already exists
    const auto& entry = m_dbEntries.find(name);
    if (entry != m_dbEntries.end()) {
      DBStoreEntry& dbEntry = entry->second;

      // Check whether the types match
      if (!dbEntry.checkType(type, objClass, array)) return nullptr;

      B2DEBUG(39, "DBEntry " << name << " was requested once more.");
      if (required && !dbEntry.isRequired()) {
        dbEntry.require();
        B2DEBUG(34, "DBEntry " << name << " is now required");
      }
      return &dbEntry;
    }

    // Add the DBStore entry
    const auto iter = m_dbEntries.emplace(name, DBStoreEntry{type, name, objClass, array, required});
    DBStoreEntry& dbEntry = iter.first->second;
    B2DEBUG(34, "DBEntry " << name << " was created (" << (required ? "required" : "optional") << ")");

    // If no object was obtained from the database yet, but it should be available, get it

    // Check if we have a non-DataStore event to use
    if (m_manualEvent) {
      Database::DBQuery query(name, required);
      if (Database::Instance().getData(*m_manualEvent, query)) {
        dbEntry.updatePayload(query.revision, query.iov, query.filename, query.checksum, *m_manualEvent);
      }
      if (dbEntry.isIntraRunDependent()) m_intraRunDependencies.insert(&dbEntry);
    }
    // Otherwise use the DataStore if it is valid
    else if (m_storeEvent.isValid()) {
      Database::DBQuery query(name, required);
      if (Database::Instance().getData(*m_storeEvent, query)) {
        dbEntry.updatePayload(query.revision, query.iov, query.filename, query.checksum, *m_storeEvent);
      }
      if (dbEntry.isIntraRunDependent()) m_intraRunDependencies.insert(&dbEntry);
    }
    return &dbEntry;
  }

  void DBStore::update()
  {
    if (m_dbEntries.empty()) {
      // make sure we at least fix the list of globaltags on the first time
      Database::Instance().initialize(Database::c_InitGlobaltagList);
      return;
    }

    // Make sure our EventMetaData pointer is reconnected as it could get
    // disconnected if the DataStore is reset.
    // TODO: This can be removed once BII-1262 is fixed.
    StoreObjPtr<EventMetaData> event;
    m_storeEvent = event;
    // Clear the m_manualEvent to indicate that we now want to use the DataStore event numbers
    m_manualEvent = boost::none;
    performUpdate(*m_storeEvent);
  }

  void DBStore::update(const EventMetaData& event)
  {
    if (m_dbEntries.empty()) {
      // make sure we at least fix the list of globaltags on the first time
      Database::Instance().initialize(Database::c_InitGlobaltagList);
      return;
    }
    m_manualEvent = event;
    performUpdate(*m_manualEvent);
  }

  void DBStore::performUpdate(const EventMetaData& event)
  {
    // For the time being we will request updates for all payloads just to make
    // sure we never miss an update. This is done once per run so it should be
    // fine from performance.
    // TODO: once we are sure somehow to not have duplicate iovs we can relax
    // this requirement.
    std::vector<Database::DBQuery> entries;
    entries.reserve(m_dbEntries.size());
    for (auto& entry : m_dbEntries) {
      bool expired = !entry.second.getIoV().contains(event);
      if (expired) {
        B2DEBUG(34, "DBEntry " << entry.first << " out of date, will need update");
      }
      if (!entry.second.keepUntilExpired() || expired)
        entries.emplace_back(entry.first, entry.second.isRequired());
      // remove from intra run handling, will be added again after update if needed.
      m_intraRunDependencies.erase(&entry.second);
    }

    // nothing to update
    if (entries.empty()) return;

    // Request new objects and IoVs from database
    Database::Instance().getData(event, entries);

    // Update DBStore entries
    for (auto& query : entries) {
      auto& dbEntry = m_dbEntries.find(query.name)->second;
      dbEntry.updatePayload(query.revision, query.iov, query.filename, query.checksum, event);
      if (dbEntry.isIntraRunDependent()) m_intraRunDependencies.insert(&dbEntry);
    }
  }

  void DBStore::updateEvent()
  {
    if (!m_manualEvent) {
      performUpdateEvent(*m_storeEvent);
    } else {
      B2WARNING("DBStore is currently using manual event information. But you are asking for the DataStore's EventMetaData "
                "to be used to update the Intra-run dependencies. Update could not proceed. "
                "Did you forget to call DBStore::Instance().update() before calling this function?");
    }
  }

  void DBStore::updateEvent(const unsigned int eventNumber)
  {
    // As m_manualEvent doesn't use the DataStore, it isn't automatically updated and we must manually update the event
    // number prior to updating the intra-run objects.
    // This also updates it ready for getData if a DBObject gets constructed later.
    if (m_manualEvent) {
      m_manualEvent->setEvent(eventNumber);
      performUpdateEvent(*m_manualEvent);
    } else {
      B2WARNING("DBStore is not currently using manual event information. But you are asking for the event number to be set "
                " to a custom value. Update of the Intra-run dependencies could not proceed. "
                "Did you forget to call DBStore::Instance().update(event), or accidentally call DBStore::Instance().update() "
                "prior to to this?");
    }
  }

  void DBStore::performUpdateEvent(const EventMetaData& event)
  {
    // loop over intra-run dependent conditions and update the objects if needed
    for (auto& dbEntry : m_intraRunDependencies) {
      dbEntry->updateObject(event);
    }
  }

  void DBStore::reset(bool keepEntries)
  {
    if (!m_dbEntries.empty())
      B2DEBUG(31, "DBStore::reset(): Cleaning all database information");
    m_intraRunDependencies.clear();
    if (!keepEntries) {
      m_dbEntries.clear();
    } else {
      for (auto& entry : m_dbEntries) {
        entry.second.resetPayload();
      }
    }
    // Make sure our EventMetaData pointer is reconnected on next access ...
    // because probably this is after resetting the DataStore (BII-1262)
    StoreObjPtr<EventMetaData> event;
    m_storeEvent = event;
    m_manualEvent = boost::none;
  }

  void DBStore::addConstantOverride(const std::string& name, TObject* obj, bool oneRun)
  {
    IntervalOfValidity iov = IntervalOfValidity::always();
    if (oneRun) {
      int exp, run;
      if (m_manualEvent) {
        exp = m_manualEvent->getExperiment();
        run = m_manualEvent->getRun();
      } else {
        exp = m_storeEvent->getExperiment();
        run = m_storeEvent->getRun();
      }
      iov = IntervalOfValidity(exp, run, exp, run);
    }
    // Add the DBStore entry
    auto iter = m_dbEntries.find(name);
    if (iter == end(m_dbEntries)) {
      iter = m_dbEntries.emplace(name, DBStoreEntry::fromObject(name, obj, true)).first;
    }
    DBStoreEntry& dbEntry = iter->second;
    dbEntry.overrideObject(obj, iov);
    // we need to remove this entry from the intraRunDependencies list now.
    // Otherwise it will reset the object on the next event call
    m_intraRunDependencies.erase(&dbEntry);
    B2WARNING("An override for DBEntry " << name << " was created.");
  }

} // namespace Belle2
