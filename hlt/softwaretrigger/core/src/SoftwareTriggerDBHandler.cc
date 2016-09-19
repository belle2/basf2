/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/database/DBImportObjPtr.h>
#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    const std::string SoftwareTriggerDBHandler::s_dbPackageIdentifier = "software_trigger_cut";

    /// Helper function to check for changes in the DB of all cuts registered in the initialize function.
    void SoftwareTriggerDBHandler::checkForChangedDBEntries()
    {
      for (auto& databaseCutEntry : m_databaseObjects) {
        if (databaseCutEntry.hasChanged()) {
          B2INFO(databaseCutEntry.getName());
          m_cutsWithIdentifier[databaseCutEntry.getName()] = databaseCutEntry->getCut();
        }
      }
    }

    void SoftwareTriggerDBHandler::initialize(const std::string& baseIdentifier, const std::vector<std::string>& cutIdentifiers)
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

    std::string SoftwareTriggerDBHandler::makeFullCutName(const std::string& baseCutIdentifier,
                                                          const std::string& cutIdentifier)
    {
      assert(baseCutIdentifier.find("&") == std::string::npos);
      assert(cutIdentifier.find("&") == std::string::npos);

      return s_dbPackageIdentifier + "&" + baseCutIdentifier + "&" + cutIdentifier;
    }

    void SoftwareTriggerDBHandler::upload(const std::unique_ptr<SoftwareTriggerCut>& cut, const std::string& baseCutIdentifier,
                                          const std::string& cutIdentifier, const IntervalOfValidity& iov)
    {
      const std::string& fullCutName = makeFullCutName(baseCutIdentifier, cutIdentifier);
      DBImportObjPtr<DBRepresentationOfSoftwareTriggerCut> cutToUpload(fullCutName);
      cutToUpload.construct(cut);
      cutToUpload.import(iov);
    }

    std::unique_ptr<SoftwareTriggerCut> SoftwareTriggerDBHandler::download(const std::string& baseCutIdentifier,
        const std::string& cutIdentifier)
    {
      const std::string& fullCutName = makeFullCutName(baseCutIdentifier, cutIdentifier);
      DBObjPtr<DBRepresentationOfSoftwareTriggerCut> downloadedCut(fullCutName);
      if (downloadedCut) {
        return downloadedCut->getCut();
      } else {
        return std::unique_ptr<SoftwareTriggerCut>();
      }
    }

    const std::map<std::string, std::unique_ptr<const SoftwareTriggerCut>>& SoftwareTriggerDBHandler::getCutsWithNames() const
    {
      return m_cutsWithIdentifier;
    };
  }
}
