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

#include <skim/softwaretrigger/core/SoftwareTriggerCut.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

#include <TObject.h>

namespace Belle2 {
  class DBRepresentationOfSoftwareTriggerCut : public TObject {
  public:
    DBRepresentationOfSoftwareTriggerCut() {}

    explicit DBRepresentationOfSoftwareTriggerCut(const std::unique_ptr<SoftwareTrigger::SoftwareTriggerCut>& softwareTriggerCut) :
      m_cutString(softwareTriggerCut->decompile()), m_preScaleFactor(softwareTriggerCut->getPreScaleFactor())
    {
    }

    std::unique_ptr<SoftwareTrigger::SoftwareTriggerCut> getCut() const
    {
      return SoftwareTrigger::SoftwareTriggerCut::compile(m_cutString, m_preScaleFactor);
    }

  private:
    /// Do not copy/assign this object.
    DBRepresentationOfSoftwareTriggerCut& operator=(const DBRepresentationOfSoftwareTriggerCut&) = delete;
    /// Do not copy this object.
    DBRepresentationOfSoftwareTriggerCut(const DBRepresentationOfSoftwareTriggerCut& rhs) = delete;

    std::string m_cutString = "";
    double m_preScaleFactor = 1;

    ClassDef(DBRepresentationOfSoftwareTriggerCut, 1);
  };
}
