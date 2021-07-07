/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <TObject.h>

namespace Belle2 {
  /**
   * Class to handle storing the trigger menu in the database.
   * It stores together with a base identifier the list of cuts attached to this base identifier.
   * When evaluating the software trigger module, this trigger menu is used to know,
   * which trigger cuts are downloaded from the database and evaluated.
   *
   * The result of the SoftwareTrigger module depends directly on these trigger cuts and the setting of the
   * mode, which defines if accept cuts or reject cuts are handled preferential over the other type.
   * See more information in the SoftwareTriggerModule.
   */
  class SoftwareTriggerMenu : public TObject {
  public:
    /// Empty constructor for ROOT
    SoftwareTriggerMenu() {}

    /// Normal constructor
    SoftwareTriggerMenu(const std::vector<std::string>& cutIdentifiers, bool acceptMode) :
      m_cutIdentifiers(cutIdentifiers), m_acceptMode(acceptMode)
    {
      for (const std::string& cutIdentifier : cutIdentifiers) {
        B2ASSERT("Cut Identifier should not include a &", cutIdentifier.find("&") == std::string::npos);
      }
    }

    /// Getter for the cut identifiers
    const std::vector<std::string> getCutIdentifiers() const
    {
      return m_cutIdentifiers;
    }

    /// Returns true if the mode of this cut is "accept" (instead of "reject").
    bool isAcceptMode() const
    {
      return m_acceptMode;
    }

  private:
    /// List of cut identifiers in this trigger menu
    std::vector<std::string> m_cutIdentifiers = {};

    /// Mode setting, if in accept mode or not (if not, we are in reject mode).
    bool m_acceptMode = false;

    /// Make this class ready for ROOT.
    ClassDef(SoftwareTriggerMenu, 1);
  };
}
