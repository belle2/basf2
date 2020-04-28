#pragma once

#include <string>

class TObject;
class TString;

namespace Belle2 {
  /** Return information on objects, for use by InfoWidget, Eve titles (popups) etc.
   *
   * Expands on RelationsObject::getInfoHTML()/getName() by also providing info
   * on e.g. genfit2::Track objects.
   */
  namespace ObjectInfo {
    /** human-readable name (e.g. pi+)
     */
    TString getName(const TObject* obj);

    /** Where is this object in the datastore? */
    TString getIdentifier(const TObject* obj);

    /** Get object info HTML (e.g. via RelationsObject::getInfoHTML()). */
    TString getInfo(const TObject* obj);

    /** Get plain text for TEve object titles (shown on mouse-over). */
    TString getTitle(const TObject* obj);

    /** return entry name & index for arrays, with index = -1 for objects. */
    std::pair<std::string, int> getDataStorePosition(const TObject* obj);

  }
}
