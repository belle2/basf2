/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/IntervalOfValidity.h>

#include <string>
#include <unordered_set>

class TFile;

namespace Belle2 {
  class EventMetaData;
  class IntraRunDependency;
  class DBAccessorBase;

  /** Class to hold one entry from the ConditionsDB in the DBStore
   *
   * This class is what every DBObjPtr and DBArray are pointing to. It owns the
   * TFile pointer for the payload and the payload object itself.
   *
   * Any update of the Payload goes through here: when the payload changes
   * after a run the updatePayload() will be called with the new revision, iov
   * and filename. This class then opens the File and obtains the payload
   * object.
   *
   * \warning Users should not need to use this class at all but just need
   *          DBObjPtr and DBArray.
   *
   * This class also supports raw file payloads (where we do not even open the
   * file but will just return the filename to interested parties) or ROOTFile
   * payloads (where we make sure the file can be read by ROOT but don't
   * extract anything from it) But at the moment there is no api to use these
   * types.
   *
   * \sa DBObjPtr DBArray DBStore
   */
  class DBStoreEntry {
  public:
    /** Possible Store entry types */
    enum EPayloadType {
      /** Just a plain old file, we don't look at it just provide the filename */
      c_RawFile,
      /** Normal ROOT file but no extraction of any specific object */
      c_ROOTFile,
      /** A ROOT file containing a object with the name of the DBStoreEntry */
      c_Object
    };
    /** Construct a new DBStoreEntry
     * @param name name of the payload, ususally the class name of the payload
     *        but can be any string identifying the payload in the database
     * @param objClass TClass* pointer of the payload, used to check that we
     *        actually obtained the correct payload from the database
     * @param isArray if true the object is required to be a TClonesArray
     *        containing objClass objects.
     */
    DBStoreEntry(EPayloadType type, const std::string& name, const TClass* objClass, bool isArray, bool isRequired):
      m_payloadType(type), m_name(name), m_objClass(objClass), m_isArray(isArray), m_isRequired(isRequired) {}
    /** Construct a new DBStoreEntry with a requested name and an object
     * directly. The constructor arguments type, objClass and isArray will be
     * filled correctly by looking at obj
     *
     * @param name name of the payload, ususally the class name of the payload
     *        but can be any string identifying the payload in the database
     * @param obj instance of an object representing the correct type
     * @return new DBStoreEntry instance with the correct name and types set
     * */
    static DBStoreEntry fromObject(const std::string& name, const TObject* obj, bool isRequired);
    /** Don't copy the entries around */
    DBStoreEntry(const DBStoreEntry&) = delete;
    /** Also don't just assign them to each other */
    DBStoreEntry& operator=(const DBStoreEntry&) = delete;
    /** But we're fine with moving them */
    DBStoreEntry(DBStoreEntry&&) = default;
    /** Clean up memory. Some of the members are raw pointers to limit the
     * include burden on this class as it will be included by anyone using
     * Conditions data */
    ~DBStoreEntry();
    /** get the name of the payload */
    const std::string& getName() const { return m_name; }
    /** get the revision of the payload, this is an abitrary number which
     * indicates the conditions version */
    unsigned int getRevision() const { return m_revision; }
    /** get the validity of the payload */
    IntervalOfValidity getIoV() const { return m_iov; }
    /** get the filename for this payload */
    const std::string& getFilename() const { return m_filename; }
    /** get the checksum of the payload. This is a fixed size string calculated from the file contents.
     * If this changes the payload data also changed */
    const std::string& getChecksum() const { return m_checksum; }
    /** get the object for this payload, can be nullptr if the payload is not
     * loaded or not of type c_Object */
    const TObject* getObject() const { return m_object; }
    /** get the ROOT TFile pointer for this payload. Can be nullptr if the payload is not loaded or neither c_ROOTFile or c_Object */
    const TFile* getTFile() const { return m_tfile; }
    /** get the class of this payload */
    const TClass* getClass() const { return m_objClass; }
    /** get whether this payload is an array of objects or a single objects */
    bool isArray() const { return m_isArray; }
    /** check whether this payload is required for operation */
    bool isRequired() const { return m_isRequired; }
    /** check whether the payload should be kept until expired */
    bool keepUntilExpired() const { return m_keep; }
    /** return whether or not the payload might change even during the run */
    bool isIntraRunDependent() const { return (bool)m_intraRunDependency; }
    /** Register an Accessor object to be notified on changes by calling DBAccessorBase::storeEntryChanged() */
    void registerAccessor(DBAccessorBase* object) { m_accessors.insert(object); }
    /** Deregister an Accessor object and remove it from the list of registered objects */
    void removeAccessor(DBAccessorBase* object) { m_accessors.erase(object); }

  private:
    /** reset the payload to nothing */
    void resetPayload();
    /** Update the payload information for this entry and if appropriate open the
     * new file and extract the object as needed. If another version is already loaded
     * the object will be deleted and the file closed.
     *
     * If neither revision or filename change we assume the payload is not
     * modified and will not update anything and not call any update notifications.
     *
     * @param revision new revision of the payload
     * @param iov new interval of validity for the payload
     * @param filename filename for the payload
     */
    void updatePayload(unsigned int revision, const IntervalOfValidity& iov, const std::string& filename, const std::string& checksum,
                       const EventMetaData& event);
    /** Actual load the payload from file after all info is set */
    void loadPayload(const EventMetaData& event);
    /** update the payload object according to the new event information.
     * If the payload has no intra run dependency this does nothing, otherwise
     * it will load the appropirate object for the given event and call the
     * update notifications on changes to the object.
     */
    void updateObject(const EventMetaData& event);
    /** Set an override object in case we want to use a different object then
     * actually provided by the database
     * @param obj Instance to use as payload, this entry will take over ownership.
     * @param iov interval how long this object should be valid
     */
    void overrideObject(TObject* obj, const IntervalOfValidity& iov);
    /** Return the pointer to the current object and release ownership: The caller is responsible to clean up the object */
    TObject* releaseObject() { TObject* obj{nullptr}; std::swap(obj, m_object); return obj; }
    /** Check if a given TObject instance is compatible with the type of this entry */
    bool checkType(const TObject* object) const;
    /** Check if a given TClass is compatible with the type of this entry */
    bool checkType(EPayloadType type, const TClass* objClass, bool array, bool inverse = false) const;
    /** Notify all the registered accessors */
    void notifyAccessors(bool onDestruction = false);
    /** Change status of this payload to required */
    void require() { m_isRequired = true; }
    /** Type of this payload */
    const EPayloadType m_payloadType;
    /** Name of this payload */
    const std::string m_name;
    /** Class of this payload */
    const TClass* const m_objClass;
    /** True if this payload is an array of objects */
    const bool m_isArray;
    /** True if at least one user marked this payload as required, false if
     * this payload is marked optional */
    bool m_isRequired{false};
    /** if True this payload should not be updated unless it's really out of date.
     * Usually only set for overrides */
    bool m_keep{false};
    /** revision of this payload */
    unsigned int m_revision{0};
    /** validity of this payload */
    IntervalOfValidity m_iov{0, 0, 0, 0};
    /** filename containing which contains the actual payload data */
    std::string m_filename{""};
    /** checksum of the payload file */
    std::string m_checksum;
    /** Pointer to the open ROOT TFile pointer for m_filename */
    TFile* m_tfile{nullptr};
    /** Pointer to the actual payload object */
    TObject* m_object{nullptr};
    /** If the payload has intra run dependency this will point to the whole
     * payload and m_object will just point to the part currently valid */
    IntraRunDependency* m_intraRunDependency{nullptr};
    /** Vector of all the accessors registered with this entry */
    std::unordered_set<DBAccessorBase*> m_accessors;
    /** Allow only the DBStore class to update the payload contents */
    friend class DBStore;
    /** Also allow the Database class to return TObject* pointers for now */
    friend class Database;
  };
}

