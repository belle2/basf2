#pragma once

#include <TString.h>
#include <TMemberInspector.h>

namespace Belle2 {
  /** Pass to TObject::ShowMembers() to get tabular view of object data. */
  class HtmlClassInspector : public TMemberInspector {
  public:
    HtmlClassInspector() { }
    /** Return finished table. */
    TString getTable() const;

    /** Implementation mostly copied from TDumpMembers. */
    void Inspect(TClass* cl, const char* pname, const char* mname, const void* add) override;

  private:
    TString m_info; /**< used to store output. */
  };
}
