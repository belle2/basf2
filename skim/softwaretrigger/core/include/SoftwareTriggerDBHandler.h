/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <skim/softwaretrigger/core/SoftwareTriggerCut.h>
#include <skim/softwaretrigger/dbobjects/DBRepresentationOfSoftwareTriggerCut.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Helper class for performing up- and downloads
     * of SoftwareTriggerCuts from the database.
     * In the typical module setup, you would create an instance of this class as a module member,
     * call the initialize function with the cuts you want to process in the module's initialize
     * method and look for changed DB entries in the beginRun function.
     *
     * In the normal event function of the module, you can get a list of cuts with their names
     * by calling the getCutsWithNames method.
     *
     * You can also upload new cuts (or new versions of cuts) with the upload function.
     *
     * In all download and upload methods, you have to provide both a base name and a specific name for the cut.
     * Both names are used (together with a package name) to construct the identifier of the cut
     * (using the getFullCutName method). You can only download cuts from the same set (defined with the same
     * base name), to not mix e.g. level3 and hlt cuts.
     */
    class SoftwareTriggerDBHandler {
    public:
      /**
       * Helper function to compile the full identifier from the base an the specific cut name.
       * The full name is then created as:
       *  <package_identifier>_<base_name>_<cut_name>
       */
      static std::string makeFullCutName(const std::string& baseCutIdentifier,
                                         const std::string& cutIdentifier)
      {
        return s_dbPackageIdentifier + "_" + baseCutIdentifier + "_" + cutIdentifier;
      }

      /**
       * Upload a new (or replace an old version) cut with the given base and specific name
       */
      static void upload(const std::unique_ptr<SoftwareTriggerCut>& cut, const std::string& baseCutIdentifier,
                         const std::string& cutIdentifier, const IntervalOfValidity& iov)
      {
        const std::string& fullCutName = makeFullCutName(baseCutIdentifier, cutIdentifier);
        DBImportObjPtr<DBRepresentationOfSoftwareTriggerCut> cutToUpload(fullCutName);
        cutToUpload.construct(cut);
        cutToUpload.import(iov);
      }

      /**
       * Download cuts with the given base name and specific names from the database and register them here.
       * When calling the checkForChangedDBEntries, these cuts will be checked for changes.
       *
       * To get the cuts with their identifiers, call the getCutsWithNames function.
       */
      void initialize(const std::string& baseIdentifier, const std::vector<std::string>& cutIdentifiers)
      {
        m_databaseObjects.clear();
        m_cutsWithIdentifier.clear();

        m_databaseObjects.reserve(cutIdentifiers.size());

        for (const std::string& cutIdentifier : cutIdentifiers) {
          const std::string& fullIdentifier = makeFullCutName(baseIdentifier, cutIdentifier);
          m_databaseObjects.emplace_back(fullIdentifier);
          if (m_databaseObjects.back()) {
            m_cutsWithIdentifier[fullIdentifier] = m_databaseObjects.back()->getCut();
          } else {
            B2FATAL("There is no DB object with the name " << fullIdentifier);
          }
        }
      }

      /// Helper function to check for changes in the DB of all cuts registered in the initialize function.
      void checkForChangedDBEntries()
      {
        for (auto& databaseCutEntry : m_databaseObjects) {
          if (databaseCutEntry.hasChanged()) {
            B2INFO(databaseCutEntry.getName());
            m_cutsWithIdentifier[databaseCutEntry.getName()] = databaseCutEntry->getCut();
          }
        }
      }

      /// Get the already downloaded list of constant cuts with their identifiers.
      const std::map<std::string, std::unique_ptr<const SoftwareTriggerCut>>& getCutsWithNames() const
      {
        return m_cutsWithIdentifier;
      };

    private:
      /// Common suffix to identify all software trigger cuts in the database.
      static const std::string s_dbPackageIdentifier;

      /// Database entries of the cuts, which where created in the initialize function.
      std::vector<DBObjPtr<DBRepresentationOfSoftwareTriggerCut>> m_databaseObjects;
      /// Map of cuts with their identifiers, downloaded from the database.
      std::map<std::string, std::unique_ptr<const SoftwareTriggerCut>> m_cutsWithIdentifier;
    };
  }
}