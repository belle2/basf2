/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/database/DBImportObjPtr.h>
#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>

#include <boost/algorithm/string/predicate.hpp>

namespace Belle2 {
  namespace SoftwareTrigger {
    const std::string SoftwareTriggerDBHandler::s_dbPackageIdentifier = "software_trigger_cut";
    const std::string SoftwareTriggerDBHandler::s_totalResultIdentifier = "total_result";

    std::unique_ptr<SoftwareTriggerCut> SoftwareTriggerDBHandler::createCutFromDB(const DBRepresentationOfSoftwareTriggerCut& dbCut)
    {
      return SoftwareTrigger::SoftwareTriggerCut::compile(dbCut.getCutString(),
                                                          dbCut.getPreScaleFactor(),
                                                          dbCut.isRejectCut());
    }

    std::string SoftwareTriggerDBHandler::makeFullCutName(const std::string& baseCutIdentifier,
                                                          const std::string& cutIdentifier)
    {
      assert(baseCutIdentifier.find("&") == std::string::npos);
      assert(cutIdentifier.find("&") == std::string::npos);

      return s_dbPackageIdentifier + "&" + baseCutIdentifier + "&" + cutIdentifier;
    }

    std::string SoftwareTriggerDBHandler::makeTotalResultName(const std::string& baseIdentifier)
    {
      return makeFullCutName(baseIdentifier, s_totalResultIdentifier);
    }

    std::string SoftwareTriggerDBHandler::makeFullTriggerMenuName(const std::string& baseIdentifier)
    {
      assert(baseIdentifier.find("&") == std::string::npos);

      return s_dbPackageIdentifier + "&" + baseIdentifier;
    }

    bool SoftwareTriggerDBHandler::hasBaseIdentifier(const std::string& cutName, const std::string& baseIdentifier)
    {
      assert(baseIdentifier.find("&") == std::string::npos);
      assert(std::count(cutName.begin(), cutName.end(), '&') == 2);

      return boost::starts_with(cutName, makeFullCutName(baseIdentifier, ""));
    }

    void SoftwareTriggerDBHandler::upload(const std::unique_ptr<SoftwareTriggerCut>& cut, const std::string& baseCutIdentifier,
                                          const std::string& cutIdentifier, const IntervalOfValidity& iov)
    {
      B2ASSERT("The name " << s_totalResultIdentifier << " is already used for the total result of each trigger stage. "
               "You can not create a cut with the same name.", cutIdentifier != s_totalResultIdentifier);
      const std::string& fullCutName = makeFullCutName(baseCutIdentifier, cutIdentifier);
      DBImportObjPtr<DBRepresentationOfSoftwareTriggerCut> cutToUpload(fullCutName);
      cutToUpload.construct(cut->getPreScaleFactor(), cut->isRejectCut(), cut->decompile());
      cutToUpload.import(iov);
    }

    void SoftwareTriggerDBHandler::uploadTriggerMenu(const std::string& baseCutIdentifier,
                                                     const std::vector<std::string>& cutIdentifiers,
                                                     bool acceptMode,
                                                     const IntervalOfValidity& iov)
    {
      const std::string& fullMenuName = makeFullTriggerMenuName(baseCutIdentifier);
      DBImportObjPtr<SoftwareTriggerMenu> menuToUpload(fullMenuName);
      menuToUpload.construct(cutIdentifiers, acceptMode);
      menuToUpload.import(iov);
    }

    std::unique_ptr<SoftwareTriggerCut> SoftwareTriggerDBHandler::download(const std::string& baseCutIdentifier,
        const std::string& cutIdentifier)
    {
      const std::string& fullCutName = makeFullCutName(baseCutIdentifier, cutIdentifier);
      DBObjPtr<DBRepresentationOfSoftwareTriggerCut> downloadedCut(fullCutName);
      if (downloadedCut) {
        return createCutFromDB(*downloadedCut);
      } else {
        return nullptr;
      }
    }

    std::unique_ptr<SoftwareTriggerMenu>
    SoftwareTriggerDBHandler::downloadTriggerMenu(const std::string& baseCutIdentifier)
    {
      const std::string& fullMenuName = makeFullTriggerMenuName(baseCutIdentifier);
      DBObjPtr<SoftwareTriggerMenu> downloadedMenu(fullMenuName);
      if (downloadedMenu) {
        return std::make_unique<SoftwareTriggerMenu>(*downloadedMenu);
      } else {
        return nullptr;
      }
    }

    void SoftwareTriggerDBHandler::checkForChangedDBEntries()
    {
      // In case the whole trigger menu has changed, we start from scratch and reload all triggers.
      if (m_softwareTriggerMenu.hasChanged()) {
        initialize();
        return;
      }

      // In all other cases we just check each downloaded cut, if it has changed.
      for (auto& databaseCutEntry : m_databaseObjects) {
        if (databaseCutEntry.hasChanged()) {
          B2ASSERT("The name of the database entry changed! This is not handled properly by the module.",
                   m_cutsWithIdentifier.find(databaseCutEntry.getName()) != m_cutsWithIdentifier.end());
          m_cutsWithIdentifier[databaseCutEntry.getName()] = createCutFromDB(*databaseCutEntry);
        }
      }
    }

    void SoftwareTriggerDBHandler::initialize()
    {
      B2ASSERT("Could not find a valid trigger name with this "
               "base identifier (" << m_baseIdentifier << ") in the database.", m_softwareTriggerMenu);

      m_databaseObjects.clear();
      m_cutsWithIdentifier.clear();

      const auto& cutIdentifiers = m_softwareTriggerMenu->getCutIdentifiers();
      m_databaseObjects.reserve(cutIdentifiers.size());

      B2DEBUG(20, "Initializing SoftwareTrigger DB with baseIdentifier " << m_baseIdentifier << " and " << cutIdentifiers.size() <<
              " cutIdentifiers");

      for (const std::string& cutIdentifier : cutIdentifiers) {
        B2DEBUG(20, "-> with CutIndentifier " << cutIdentifier);

        const std::string& fullIdentifier = makeFullCutName(m_baseIdentifier, cutIdentifier);
        m_databaseObjects.emplace_back(fullIdentifier);
        if (m_databaseObjects.back()) {
          m_cutsWithIdentifier[fullIdentifier] = createCutFromDB(*m_databaseObjects.back());
        } else {
          B2FATAL("There is no DB object with the name " << fullIdentifier);
        }
      }
    }

    bool SoftwareTriggerDBHandler::getAcceptOverridesReject() const
    {
      return m_softwareTriggerMenu->isAcceptMode();
    }

    const std::map<std::string, std::unique_ptr<const SoftwareTriggerCut>>& SoftwareTriggerDBHandler::getCutsWithNames() const
    {
      return m_cutsWithIdentifier;
    };
  }
}
