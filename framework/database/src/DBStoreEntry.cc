/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/DBStoreEntry.h>
#include <framework/database/IntraRunDependency.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <algorithm>
#include <iomanip>
#include <TFile.h>
#include <TClonesArray.h>
#include <TClass.h>

namespace {
  /** do nothing, needed for variadic template below */
  void deleteAndSetNullptr() {}
  /** simple helper to delete a pointer and set it to nullptr. This can take
   * any number of pointers as arguments and will delete and reset all of them
   * in order */
  template<class T, class... Args> void deleteAndSetNullptr(T*& ptr, Args&& ... args)
  {
    delete ptr;
    ptr = nullptr;
    deleteAndSetNullptr(std::forward<Args>(args)...);
  }
}

namespace Belle2 {
  DBStoreEntry DBStoreEntry::fromObject(const std::string& name, const TObject* obj, bool isRequired)
  {
    bool isArray = dynamic_cast<const TClonesArray*>(obj) != nullptr;
    TClass* objClass = isArray ? (dynamic_cast<const TClonesArray*>(obj)->GetClass()) : (obj->IsA());
    return DBStoreEntry(c_Object, name, objClass, isArray, isRequired);
  }

  DBStoreEntry::~DBStoreEntry()
  {
    // if intra run dependency: object owned by the IntraRunDependency object so don't delete the object itself.
    if (m_intraRunDependency) m_object = nullptr;
    // and free all memory
    deleteAndSetNullptr(m_object, m_intraRunDependency, m_tfile);
    if (!m_callbacks.empty()) B2DEBUG(38, "DBEntry " << m_name << " destroyed, running callbacks");
    runCallbacks(true);
  }

  void DBStoreEntry::updateObject(const EventMetaData& event)
  {
    // if we don't have intra run dependency we don't care about event number
    if (!m_intraRunDependency) return;
    // otherwise update the object and call all callbacks on change
    TObject* old = m_object;
    m_object = m_intraRunDependency->getObject(event);
    if (old != m_object) {
      B2DEBUG(35, "IntraRunDependency for " << m_name << ": new object (" << old << ", " << m_object << "), running callbacks");
      runCallbacks();
    }
  }

  void DBStoreEntry::resetPayload()
  {
    m_revision = 0;
    m_iov = IntervalOfValidity();
    m_keep = false;
    m_checksum = "";
    m_checksum = "";
    // If the old object was intra run dependent then m_object is owned by the
    // intra run dependency object. So set the pointer to null to not delete the object.
    if (m_intraRunDependency) m_object = nullptr;
    // and delete the old intra run dependency if it exists
    deleteAndSetNullptr(m_object, m_intraRunDependency, m_tfile);
  }

  void DBStoreEntry::updatePayload(unsigned int revision, const IntervalOfValidity& iov, const std::string& filename,
                                   const std::string& checksum, const EventMetaData& event)
  {
    m_iov = iov;
    // File is the same as before, no need to update anything else. We could
    // check all identifiers like revision, filename and checksum
    // > if (revision == m_revision && filename == m_filename && m_checksum == checksum) return;
    // but this is a bit strict as we know we have some payloads with the same
    // checksum but multiple revisions and there's no need to actually update
    // the object. But at least lets make sure the revision number is reported the same as in the db
    m_revision = revision;
    if (m_checksum == checksum) return;
    m_keep = false;
    m_checksum = checksum;
    m_filename = filename;
    // If the old object was intra run dependent then m_object is owned by the
    // intra run dependency object. So set the pointer to null to not delete the object.
    if (m_intraRunDependency) m_object = nullptr;
    // and delete the old intra run dependency if it exists
    deleteAndSetNullptr(m_object, m_intraRunDependency, m_tfile);
    loadPayload(event);
    B2DEBUG(34, "DBEntry " << std::quoted(m_name) << " changed: " << std::endl
            << "    revision = " << m_revision << std::endl
            << "    filename = " << m_filename << std::endl
            << "    checksum = " << m_checksum << std::endl
            << "    validity = " << m_iov << std::endl
            << "  -> running callbacks");
    runCallbacks();
  }

  void DBStoreEntry::loadPayload(const EventMetaData& event)
  {
    if (m_filename == "") {
      // invalid payload nothing else to do
      return;
    }
    if (m_payloadType != c_RawFile) {
      // Open the payload file but make sure to go back to the previous
      // directory to not disturb other code.
      TDirectory* oldDirectory = gDirectory;
      m_tfile = TFile::Open(m_filename.c_str());
      gDirectory = oldDirectory;
      // Check if the file is open
      if (!m_tfile || !m_tfile->IsOpen()) {
        B2ERROR("Cannot open " << std::quoted(m_filename) << " for reading");
        deleteAndSetNullptr(m_tfile);
        return;
      }
      m_tfile->SetBit(kCanDelete, false);
      // File is open and we are a normal object entry?
      if (m_payloadType == c_Object) {
        // get the object
        m_object = m_tfile->Get(m_name.c_str());
        if (!m_object) {
          B2ERROR("Failed to get " << std::quoted(m_name) << " from payload file "
                  << std::quoted(m_filename) << ".");
          return;
        }
        m_object->SetBit(kCanDelete, false);
        // check that it is compatible with what it should be
        checkType(m_object);
        // resolve run dependency
        if (m_object->InheritsFrom(IntraRunDependency::Class())) {
          m_intraRunDependency = static_cast<IntraRunDependency*>(m_object);
          m_object = m_intraRunDependency->getObject(event);
          B2DEBUG(34, "Found intra run dependency for " << m_name << ": " << m_intraRunDependency << ", " << m_object);
        }
        // TODO: depending on the object type we could now close the file. I
        // guess we cannot close the file in case of TTree but we have to find
        // out exactly which object types are safe before doing that?
      }
    }
  }

  void DBStoreEntry::overrideObject(TObject* obj, const IntervalOfValidity& iov)
  {
    if (!checkType(obj)) return;

    m_revision = 0;
    m_filename = "";
    m_iov = iov;
    m_keep = true;
    // object unchanged, nothing to do
    if (m_object == obj) {
      B2DEBUG(34, "DBEntry " << std::quoted(m_name) << " override unchanged");
      return;
    }
    // We need to clean up the object but if someone hands us the same object
    // we already own this would lead to invalid access later so make sure we
    // only delete it when appropriate
    if (!isIntraRunDependent()) {
      delete m_object;
    }
    // clean up the other memory
    deleteAndSetNullptr(m_intraRunDependency, m_tfile);
    // and set, ready go
    m_object = obj;
    B2DEBUG(34, "DBEntry " << this << " " << std::quoted(m_name) << " override created: " << std::endl
            << "    object = " << m_object << std::endl
            << "    validity = " << m_iov << std::endl
            << "  -> running callbacks");
    runCallbacks();
  }

  void DBStoreEntry::runCallbacks(bool onDestruction)
  {
    // Check if a given callback function is still valid, that is the weak_ptr
    // still points to a valid shared_ptr. If not return true to mark for
    // removal, otherwise call it if appropriate and return false to mark to
    // keep.
    auto remove_if_callback = [onDestruction, this](std::tuple<CallbackType, bool>& e) {
      auto sp = std::get<CallbackType>(e).lock();
      // shared_ptr owning this callback has been destroyed, remove from list
      // of callbacks
      if (!sp) return true;
      // only call the appropriate type of callbacks
      if (onDestruction == std::get<bool>(e)) {(*sp)(m_name);}
      // in any case: callback is still valid, keep it
      return false;
    };
    //clear expired callbacks and call all others using erase-remove idiom: move
    //all valid callback functions (above callback returns true) to the front
    //and delete the remainder of the vector
    m_callbacks.erase(std::remove_if(m_callbacks.begin(), m_callbacks.end(), remove_if_callback), m_callbacks.end());
  }

  bool DBStoreEntry::checkType(EPayloadType type, const TClass* objClass, bool array, bool inverse) const
  {
    if (type != m_payloadType) {
      B2FATAL("Existing entry '" << m_name << "' is of a different type than requested");
    }
    // OK, all other checks only make sense for objects
    if (type != c_Object) return true;
    // Check whether the existing entry and the requested object are both arrays or both single objects
    if (m_isArray != array) {
      B2FATAL("Existing entry '" << m_name << "' is an " << ((m_isArray) ? "array" : "object") <<
              " and the requested one an " << ((array) ? "array" : "object"));
    }

    // Check whether the existing entry and the requested object have the same type
    if (!inverse) {
      if (!m_objClass->InheritsFrom(objClass)) {
        B2FATAL("Existing entry '" << m_name << "' of type " <<
                m_objClass->GetName() << " doesn't match requested type " <<
                objClass->GetName());
      }
    } else {
      if (!objClass->InheritsFrom(m_objClass)) {
        B2FATAL("Existing entry '" << m_name << "' of type " <<
                m_objClass->GetName() << " doesn't match actual type " <<
                objClass->GetName());
      }
    }
    return true;
  }

  bool DBStoreEntry::checkType(const TObject* object) const
  {
    // Get class information from object
    if (object->InheritsFrom(IntraRunDependency::Class())) {
      object = static_cast<const IntraRunDependency*>(object)->getAnyObject();
    }
    TClass* objClass = object->IsA();
    bool array = (objClass == TClonesArray::Class());
    if (array) {
      objClass = static_cast<const TClonesArray*>(object)->GetClass();
    }

    return checkType(c_Object, objClass, array, true);
  }

}
