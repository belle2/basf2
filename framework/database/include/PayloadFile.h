/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/database/DBObjPtr.h>
#include <string>


namespace Belle2 {
  /**
   * A wrapper class for payload files used by the Database and DBStore classes.
   */
  class PayloadFile {
  public:
    /**
     * Constructor.
     * @param fileName       Name of the payload file obtained from the database.
     */
    explicit PayloadFile(const std::string& fileName): m_fileName(fileName) {};

    /**
     * Get the name of the downloaded payload file.
     * @return    Local file name.
     */
    std::string getFileName() const {return m_fileName;};

    /**
     * Read the content of the payload (text) file.
     * @return    The content of the payload file
     */
    std::string getContent() const;

  private:
    std::string m_fileName; /**< Name of the payload file. **/
  };

  /** Specialization of DBObjPtr in case of PayloadFiles.
   * This is for backwards compatibility when we used to distinuigsh Root
   * objects and plain files with a rather weird and different scheme.
   *
   * Now this is a specialization of the base DBObjPtr which does not open the
   * file but just keeps a reference to the filename.
   */
  template<> class DBObjPtr<PayloadFile>: public DBAccessorBase {
  public:
    /** Constructor of a DBObjPtr object */
    explicit DBObjPtr(const std::string& name = "", bool required = true):
      DBAccessorBase(DBStoreEntry::c_RawFile, name, required), m_payloadFile(isValid() ? m_entry->getFilename() : "")
    {
      // if the payload changes also change the filename
      addCallback([this](const std::string&) {m_payloadFile = PayloadFile(isValid() ? m_entry->getFilename() : "");});
    }
    /** isValid is always true if we have a filename */
    inline bool isValid() const {return (m_entry) && (m_entry->getFilename() != "");}
    inline const PayloadFile& operator *()  const {return m_payloadFile; }  /**< Imitate pointer functionality. */
    inline const PayloadFile* operator ->() const {return &m_payloadFile; }   /**< Imitate pointer functionality. */
  private:
    /** PayloadFile object which can be queried for the filename */
    PayloadFile m_payloadFile;
  };
}
