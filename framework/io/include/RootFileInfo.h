/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <memory>
#include <string>
#include <set>

class TTree;
class TFile;

namespace Belle2 {
  class FileMetaData;
  namespace RootIOUtilities {
    /** Helper class to factorize some necessary tasks when working with Belle2
     * output files. Like the correct way to obtain the FileMetaData from a
     * file or to check the list of existing branches */
    class RootFileInfo {
    public:
      /** Create an object from a given filename or url.
       * This will open the file and read the tree headers. It will throw exceptions if it encounters any problems:
       *
       * - std::invalid_argument if the file cannot be opened at all
       * - std::runtime_error if any of the trees is missing or the number of entries in the persistent tree is !=1
       */
      explicit RootFileInfo(const std::string& filename);
      /** Close the file and delete all structures associated with it */
      ~RootFileInfo();

      /** Return a reference to the persistent tree */
      TTree& getPersistentTree() { return *m_persistent; }
      /** Return a reference to the event tree */
      TTree& getEventTree() { return *m_events; }
      /** Return the event metadata from the file. On the first call it will be
       * read from the persistent tree. If this fails a std::runtime_error is
       * thrown. */
      const FileMetaData& getFileMetaData();
      /** Return a set of branch names for either the event or the persistent tree */
      const std::set<std::string>& getBranchNames(bool persistent = false);
      /** Check if the event or persistent tree contain at least all the
       * branches in the set of required branches. Throw a std::runtime_error
       * if not */
      void checkMissingBranches(const std::set<std::string>& required, bool persistent = false);
    private:
      /** Pointer to the file object */
      std::unique_ptr<TFile> m_file;
      /** Pointer to the persistent tree */
      std::unique_ptr<TTree> m_persistent;
      /** Pointer to the event tree */
      std::unique_ptr<TTree> m_events;
      /** Pointer to the file metadata once it has been read */
      std::unique_ptr<FileMetaData> m_metadata;
      /** Cached set of persistent branch names */
      std::optional<std::set<std::string>> m_persistentBranches;
      /** Cached set of event branch names */
      std::optional<std::set<std::string>> m_eventBranches;
    };
  }
}
