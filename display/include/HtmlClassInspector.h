/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TString.h>
#include <TMemberInspector.h>

namespace Belle2 {
  /** Pass to TObject::ShowMembers() to get tabular view of object data. */
  class HtmlClassInspector : public TMemberInspector {
  public:
    /** Get class name + description from comment after ClassDef(). */
    static TString getClassInfo(const TClass* obj);
    /** Return table with member data contents. */
    static TString getMemberData(const TObject* obj);

    /** Implementation mostly copied from TDumpMembers. */
    void Inspect(TClass* cl, const char* pname, const char* mname, const void* add) override;

  private:
    /** Constructor */
    HtmlClassInspector() { }

    /** Return finished table. */
    TString getTable() const;

    /** strip comment things */
    static TString stripTitle(TString title);

    TString m_info; /**< used to store output. */
  };
}
