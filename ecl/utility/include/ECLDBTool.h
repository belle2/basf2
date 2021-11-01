/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

// FRAMEWORK
#include <framework/database/IntervalOfValidity.h>
#include <framework/dataobjects/EventMetaData.h>
namespace Belle2 {
  /**
   * The ECLDBTool class
   * is designed to read / write
   * object from / to database.
   */
  class ECLDBTool {
  public:
    /**
     * Constructor.
     * @param isLocal enables local database
     usage.
     * @param dbName is a tag of the central
     database or path to a local database.
     * @param payloadName is name of payload.
     */
    ECLDBTool(bool isLocal,
              const char* dbName,
              const char* payloadName);
    /**
     * Destructor.
     */
    ~ECLDBTool();
    /**
     * Connect to a database.
     */
    void connect() const;
    /**
     * Write object and validity interval
     * to a database.
     * @param obj is object.
     * @param iov is validity interval.
     */
    void write(TObject* const obj,
               const IntervalOfValidity& iov) const;
    /**
     * Read object and validity interval
     * from a database.
     * @param obj is object.
     * @param iov is validity interval.
     * @param event is event metadata.
     */
    void read(TObject** obj,
              IntervalOfValidity** iov,
              const EventMetaData& event) const;
    /**
     * Read just validity interval.
     * @param iov is validity interval.
     * @param event is the event metadata.
     */
    void read(IntervalOfValidity** iov,
              const EventMetaData& event) const;
    /**
     * Change interval of validity (Make
     * a copy of the current payload with other
     * validity interval).
     * @param event is the event metadata.
     * @param iov is the validity interval.
     */
    void changeIoV(const EventMetaData& event,
                   const IntervalOfValidity& iov) const;
  private:
    /**
     * If m_isLocal is true,
     * local database is used.
     * Otherwise central database
     * is used.
     */
    bool m_isLocal;
    /**
     * Tag in the central database
     * or path to a local database.
     */
    std::string m_dbName;
    /**
     * Name of payload.
     */
    std::string m_payloadName;
  };
}

