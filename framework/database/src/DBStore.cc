/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Marko Staric, Martin Ritter                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/DBStore.h>
#include <framework/database/IntraRunDependency.h>
#include <framework/database/Database.h>
#include <framework/logging/Logger.h>

#include <TClonesArray.h>
#include <TClass.h>
#include <TFile.h>

#include <list>

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

  DBStoreEntry* DBStore::getEntry(DBStoreEntry::EPayloadType type, const std::string& name, const TClass* objClass, bool array,
                                  bool required)
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
    if (m_event.isValid()) {
      Database::DBQuery query(name, required);
      if (Database::Instance().getData(*m_event, query)) {
        dbEntry.updatePayload(query.revision, query.iov, query.filename, query.checksum, *m_event);
      }
      if (dbEntry.isIntraRunDependent()) m_intraRunDependencies.insert(&dbEntry);
    }

    return &dbEntry;
  }


  void DBStore::update()
  {
    // For the time being we will request updates for all payloads just to make
    // sure we never miss an update. This is done once per run so it should be
    // fine from performance.
    // TODO: once we are sure somehow to not have duplicate iovs we can relax
    // this requirement.
    std::list<Database::DBQuery> entries;
    for (auto& entry : m_dbEntries) {
      bool expired = !entry.second.getIoV().contains(*m_event);
      if (expired) {
        B2DEBUG(34, "DBEntry " << entry.first << " out of date, will need update");
      }
      if (!entry.second.keepUntilExpired() || expired)
        entries.emplace_back(entry.first, entry.second.isRequired());
      // remove from intra run handling, will be added again after update if needed.
      m_intraRunDependencies.erase(&entry.second);
    }

    // Request new objects and IoVs from database
    Database::Instance().getData(*m_event, entries);

    // Update DBStore entries
    for (auto& query : entries) {
      auto& dbEntry = m_dbEntries.find(query.name)->second;
      dbEntry.updatePayload(query.revision, query.iov, query.filename, query.checksum, *m_event);
      if (dbEntry.isIntraRunDependent()) m_intraRunDependencies.insert(&dbEntry);
    }
  }


  void DBStore::updateEvent()
  {
    // loop over intra-run dependent conditions and update the objects if needed
    for (auto& dbEntry : m_intraRunDependencies) {
      dbEntry->updateObject(*m_event);
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
    StoreObjPtr<EventMetaData> event;
    m_event = event;
  }

  void DBStore::addConstantOverride(const std::string& name, TObject* obj, bool oneRun)
  {
    IntervalOfValidity iov = IntervalOfValidity::always();
    if (oneRun) {
      const int exp = m_event->getExperiment();
      const int run = m_event->getRun();
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
