/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/database/PayloadMetadata.h>

#include <functional>
#include <unordered_map>

namespace Belle2 {
  class EventMetaData;

  namespace Conditions {

    /** Class to store and retrieve temporary payloads.
     *
     * Temporary payloads are stored in a directory where we create a root files
     * and a plain text file with name, revision and iov for each of these
     * files. This is not very safe but ideal for testing and manual adjustments
     * before final validation and upload to the cental server.
     *
     * The interface is very basic: We create an instance with a filename and
     * then we either get() payload information for a given event and payload
     * metadata struct or we store a given payload either from an object
     * (storeData()) or from an existing file (storePayload()).
     *
     * Both call the underlying store() which tries to be robust against
     * multiple processes trying to create payloads at the same time.
     */
    class TestingPayloadStorage {
    public:
      /** Create a new instance to work on a given filename */
      explicit TestingPayloadStorage(const std::string& filename);
      /** Try to fill the PayloadMetaData for the given EventMetaData, return
       * true on success, false if no machting payload could be found */
      bool get(const EventMetaData& event, PayloadMetadata& info);
      /** Store a TObject instance as a payload with given name and interval of
       * validity. This will create a new ROOT file containing the object in the
       * directory of the storage file */
      bool storeData(const std::string& name, TObject* object, const IntervalOfValidity& iov);
      /** Store an existing file as payload with given name and interval of
       * validity. This will create a copy of the file with a different name in
       * the directory of the storage file */
      bool storePayload(const std::string& name, const std::string& fileName, const IntervalOfValidity& iov);
      /** Reset the list of known payloads. Will trigger re-reading the file on next access to get() */
      void reset() { m_initialized = false; }
    private:
      /** Read the given storage file, done lazily on first access to get() after construction or call to reset() */
      void read();
      /** Map of known payloads to a list of known revisions and their interval of validity */
      std::unordered_map<std::string, std::vector<std::tuple<std::string, IntervalOfValidity>>> m_payloads;
      /** Storage file where to look for payloads. This is the logical file name as given by the user */
      std::string m_filename;
      /** Storage file where to look for payloads converted to an absolute path to be robust against directory changes */
      std::string m_absoluteFilename;
      /** Directory containing the storage file as absolute file name. Will try
       * to create it if it doesn't exist and storeData() or storePayload() is
       * called */
      std::string m_payloadDir;
      /** Remember whether we read the file already */
      bool m_initialized{false};
      /** Build the filename for a new payload with a given name and revision in a directory */
      static std::string payloadFilename(const std::string& path, const std::string& name, const std::string& revision);
      /** Write a payload file from the given object and name. Will create new
       * root file containing object under the Key name with the name fileName
       */
      bool writePayload(const std::string& fileName, const std::string& name, const TObject* object);
      /** Try to store a new payload with the given name and interval of
       * validity. This function first tries to find the next free revision and
       * then call the writer function to write the payload to the given
       * filename. If the writer function returns success the payload is added
       * to the storage file
       *
       * @param name payload name
       * @param iov iov for the payload
       * @param source source filename to use. If this is empty there is no file yet and it has to be created first
       * @param writer callback function to create a file. Will be called with a destination filename if the source parameter was emty
       */
      bool store(const std::string& name, const IntervalOfValidity& iov, const std::string& source,
                 const std::function<bool(const std::string&)>& writer);
    };
  } // Conditions namespace
} // Belle2 namespace
