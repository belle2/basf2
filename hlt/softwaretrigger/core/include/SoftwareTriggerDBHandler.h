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

#include <hlt/softwaretrigger/core/SoftwareTriggerCut.h>
#include <hlt/softwaretrigger/dbobjects/DBRepresentationOfSoftwareTriggerCut.h>
#include <framework/database/DBObjPtr.h>

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
       *  <package_identifier>&<base_name>&<cut_name>
       *
       * Make sure to not include & into the base or cut name.
       */
      static std::string makeFullCutName(const std::string& baseCutIdentifier,
                                         const std::string& cutIdentifier);

      /**
       * Upload a new (or replace an old version) cut with the given base and specific name. Neither the base nor the
       * cut name are allowed to have '&' in it. Please make sure that the base name must correspond to the identifiers
       * of the calculation objects created in the SoftwareTriggerModule.
       */
      static void upload(const std::unique_ptr<SoftwareTriggerCut>& cut, const std::string& baseCutIdentifier,
                         const std::string& cutIdentifier, const IntervalOfValidity& iov);

      /**
       * Download a cut from the database. This function should only
       * be called from python to interact with/edit single cuts and not from your module
       * to check the cuts (use the initialize method etc. for this).
       * @param baseCutIdentifier The base name of the cut to download.
       * @param cutIdentifier The identifier of the cut to download.
       * @return A unique pointer to the downloaded cut or a nullptr of no cut with this name is in the DB.
       */
      static std::unique_ptr<SoftwareTriggerCut> download(const std::string& baseCutIdentifier, const std::string& cutIdentifier);

      /** Use the default constructor (needed as we delete the copy constructor) */
      SoftwareTriggerDBHandler() = default;

      /**
       * Download cuts with the given base name and specific names from the database and register them here.
       * When calling the checkForChangedDBEntries, these cuts will be checked for changes.
       *
       * To get the cuts with their identifiers, call the getCutsWithNames function.
       */
      void initialize(const std::string& baseIdentifier, const std::vector<std::string>& cutIdentifiers);

      /// Helper function to check for changes in the DB of all cuts registered in the initialize function.
      void checkForChangedDBEntries();

      /// Get the already downloaded list of constant cuts with their identifiers.
      const std::map<std::string, std::unique_ptr<const SoftwareTriggerCut>>& getCutsWithNames() const;

    private:
      /// Delete the copy constructor
      SoftwareTriggerDBHandler(const SoftwareTriggerDBHandler& rhs) = delete;

      /// Delete the assignment constructror
      SoftwareTriggerDBHandler& operator=(SoftwareTriggerDBHandler& rhs) = delete;

      /// Common suffix to identify all software trigger cuts in the database.
      static const std::string s_dbPackageIdentifier;

      /// Database entries of the cuts, which where created in the initialize function.
      std::vector<DBObjPtr<DBRepresentationOfSoftwareTriggerCut>> m_databaseObjects;
      /// Map of cuts with their identifiers, downloaded from the database.
      std::map<std::string, std::unique_ptr<const SoftwareTriggerCut>> m_cutsWithIdentifier;
    };
  }
}