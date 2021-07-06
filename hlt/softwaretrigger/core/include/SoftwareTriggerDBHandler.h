/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <hlt/softwaretrigger/core/SoftwareTriggerCut.h>
#include <mdst/dbobjects/DBRepresentationOfSoftwareTriggerCut.h>
#include <mdst/dbobjects/SoftwareTriggerMenu.h>
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
      /// Common prefix to identify all software trigger cuts in the database.
      static const std::string s_dbPackageIdentifier;
      /// Common suffix to identify all total results in the stored results.
      static const std::string s_totalResultIdentifier;

      /**
       * Helper factory function to generate a unique cut pointer
       * from its representation in the database.
       * Was part of the DBRepresentation before, but is now a standalone function.
       */
      static std::unique_ptr<SoftwareTriggerCut> createCutFromDB(const DBRepresentationOfSoftwareTriggerCut& dbCut);

      /**
       * Helper function to compile the full identifier from the base and the specific cut name.
       * The full name is then created as:
       *  <package_identifier>&<base_name>&<cut_name>
       *
       * Make sure to not include & into the base or cut name.
       */
      static std::string makeFullCutName(const std::string& baseCutIdentifier,
                                         const std::string& cutIdentifier);

      /**
       * Handy function to create the name related to the total result of a specific
       * trigger stage (either filter or skim) in the stored results or the total result. It is in the form
       *   <package_identifier>&<base_name>&total_result
       *
       * @param baseIdentifier The baseIdentifier (either filter or skim or all)
       * @return then name.
       */
      static std::string makeTotalResultName(const std::string& baseIdentifier = "all");

      /**
       * Helper function to compile the full menu identifier from the base name.
       * The full name is then created as:
       *  <package_identifier>&<base_name>
       *
       * Make sure to not include & into the base name.
       */
      static std::string makeFullTriggerMenuName(const std::string& baseIdentifier);

      /**
       * Check if a given cut name in the form
       *  <package_identifier>&<base_name>&<cut_name>
       * has the given base name.
       */
      static bool hasBaseIdentifier(const std::string& cutName, const std::string& baseIdentifier);

      /**
       * Upload a new (or replace an old version) cut with the given base and specific name. Neither the base nor the
       * cut name are allowed to have '&' in it. Please make sure that the base name must correspond to the identifiers
       * of the calculation objects created in the SoftwareTriggerModule.
       */
      static void upload(const std::unique_ptr<SoftwareTriggerCut>& cut, const std::string& baseCutIdentifier,
                         const std::string& cutIdentifier, const IntervalOfValidity& iov);

      /**
       * Upload a new (or replace an old version) trigger menu with the given base and specific names.
       * Neither the base nor the cut names are allowed to have '&' in it.
       * Please make sure that the base name must correspond to the identifiers
       * of the calculation objects created in the SoftwareTriggerModule and the cut names must correspond to
       * cuts uploaded into the database.
       */
      static void uploadTriggerMenu(const std::string& baseCutIdentifier,
                                    const std::vector<std::string>& cutIdentifiers,
                                    bool acceptMode,
                                    const IntervalOfValidity& iov);

      /**
       * Download a cut from the database. This function should only
       * be called from python to interact with/edit single cuts and not from your module
       * to check the cuts (use the initialize method etc. for this).
       * @param baseCutIdentifier The base name of the cut to download.
       * @param cutIdentifier The identifier of the cut to download.
       * @return A unique pointer to the downloaded cut or a nullptr of no cut with this name is in the DB.
       */
      static std::unique_ptr<SoftwareTriggerCut> download(const std::string& baseCutIdentifier, const std::string& cutIdentifier);

      /**
       * Download a trigger menu from the database. This function should only
       * be called from python to interact with/edit single menus and not from your module
       * to check the cuts (use the initialize method etc. for this).
       * @param baseCutIdentifier The base name of the trigger menu to download.
       * @return A unique pointer to the downloaded menu or a nullptr of no menu with this name is in the DB.
       */
      static std::unique_ptr<SoftwareTriggerMenu> downloadTriggerMenu(const std::string& baseCutIdentifier);

      /** Use the default constructor (needed as we delete the copy constructor) */
      explicit SoftwareTriggerDBHandler(const std::string& baseIdentifier) :
        m_baseIdentifier(baseIdentifier),
        m_softwareTriggerMenu(makeFullTriggerMenuName(baseIdentifier))
      {
        initialize();
      }

      /**
       * Download the trigger menu and afterwards the cuts with the given base name and
       * specific names from the database and register them here.
       * When calling the checkForChangedDBEntries, these cuts will be checked for changes.
       *
       * To get the cuts with their identifiers, call the getCutsWithNames function.
       */
      void initialize();

      /// Helper function to check for changes in the DB of all cuts registered in the initialize function.
      void checkForChangedDBEntries();

      /// Get the already downloaded list of constant cuts with their identifiers.
      const std::map<std::string, std::unique_ptr<const SoftwareTriggerCut>>& getCutsWithNames() const;

      /// Return true of the trigger menu is in accept mode.
      bool getAcceptOverridesReject() const;

    private:
      /// Delete the copy constructor
      SoftwareTriggerDBHandler(const SoftwareTriggerDBHandler& rhs) = delete;

      /// Delete the assignment constructror
      SoftwareTriggerDBHandler& operator=(SoftwareTriggerDBHandler& rhs) = delete;

      /// Base identifier
      std::string m_baseIdentifier = "";
      /// Database entry of the software trigger menu.
      DBObjPtr<SoftwareTriggerMenu> m_softwareTriggerMenu;
      /// Database entries of the cuts, which where created in the initialize function.
      std::vector<DBObjPtr<DBRepresentationOfSoftwareTriggerCut>> m_databaseObjects;
      /// Map of cuts with their identifiers, downloaded from the database.
      std::map<std::string, std::unique_ptr<const SoftwareTriggerCut>> m_cutsWithIdentifier;
    };
  }
}
