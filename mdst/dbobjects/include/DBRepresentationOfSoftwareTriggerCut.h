/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <mdst/dbobjects/SoftwareTriggerCutBase.h>

#include <TObject.h>

namespace Belle2 {
  /**
   * Class to handle storing SoftwareTriggerCuts in the database.
   * As we do not want to upload cuts themselve (with all their dependencies and pointers),
   * we decompile them back into a string and save only the string along with the
   * prescale factor. When using the cut, they are recompiled back into a cut
   * from the string representation.
   */
  class DBRepresentationOfSoftwareTriggerCut : public TObject, public SoftwareTriggerCutBase {
  public:
    /// Default constructor for ROOT
    DBRepresentationOfSoftwareTriggerCut() = default;

    /// Constructor from the three components pre scale factor, reject flag and cut string
    DBRepresentationOfSoftwareTriggerCut(unsigned int preScaleFactor, const bool& isRejectCut, const std::string& cutString) :
      SoftwareTriggerCutBase(preScaleFactor, isRejectCut), m_cutString(cutString) {}

    /// Return the cut string stored in this db representation
    const std::string& getCutString() const
    {
      return m_cutString;
    }
  private:
    /// The internal storage of the string representation of the cut.
    std::string m_cutString = "";

    /// Make this class ready for ROOT
    ClassDef(DBRepresentationOfSoftwareTriggerCut, 2);
  };
}
