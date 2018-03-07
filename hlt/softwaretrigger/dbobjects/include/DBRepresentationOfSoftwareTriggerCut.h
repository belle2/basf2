/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <hlt/softwaretrigger/core/SoftwareTriggerCut.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

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
    /// Empty constructor for ROOT (you will probably want to use the explicit copy from SoftwareTriggerCut).
    DBRepresentationOfSoftwareTriggerCut() {}

    /**
     * Create a new representation for uploading into the database of the given
     * SoftwareTriggerCut. This is done by decompiling the cut into its string
     * representation and storing also the prescale factor.
     * Both can be easily uploaded into the database.
     */
    explicit DBRepresentationOfSoftwareTriggerCut(const std::unique_ptr<SoftwareTrigger::SoftwareTriggerCut>& softwareTriggerCut) :
      SoftwareTriggerCutBase(softwareTriggerCut->getPreScaleFactor(), softwareTriggerCut->isRejectCut()),
      m_cutString(softwareTriggerCut->decompile())
    {
    }

    /**
     * Getter for the cut. This cut is created by compiling the
     * string representation back into a real cut.
     */
    std::unique_ptr<SoftwareTrigger::SoftwareTriggerCut> getCut() const
    {
      return SoftwareTrigger::SoftwareTriggerCut::compile(m_cutString, getPreScaleFactor(), isRejectCut());
    }

  private:
    /// The internal storage of the string representation of the cut.
    std::string m_cutString = "";

    /// Make this class ready for ROOT
    ClassDef(DBRepresentationOfSoftwareTriggerCut, 2);
  };
}
