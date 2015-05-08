#pragma once

#include <TGHtml.h> //huge number of defines, might want to hide that part.
#include <TString.h>

#include <set>
#include <vector>
#include <algorithm>

class TObject;
class TGWindow;

namespace Belle2 {
  /** text-based info viewer showing DataStore contents.
   *
   **/
  class InfoWidget : public TGHtml {
  public:
    InfoWidget(const TGWindow* p);
    ~InfoWidget();

    /** reset for new event. */
    void newEvent();

    /** Navigate to given URI of the form "entryName/arrayIndex".
     *
     * "/" will show DataStore overview, "entryName/" shows the given array or object.
     *
     * @param clearSelection when showing an object, this determines  wether to clear
     *        an existing selection
     */
    void show(const char* uri = "/", bool clearSelection = true);

    /** Navigate to page belonging to given object.
     *
     * Special in that it doesn't clear the current selection.
     */
    void show(const TObject* obj) { show(getURI(obj).Data(), false); }

    /** navigate to previous page, clearing current page from history. */
    void back();

    /** Used to colour visited links. */
    virtual int IsVisited(const char* uri); //please add 'override' here once CINT is gone

  private:
    /** a parsed URI. */
    struct URI {
      URI(): object(0), entryName(""), arrayIndex(-1) { }

      const TObject* object; /**< object referenced (or NULL). */
      TString entryName; /**< name of DataStore entry. */
      int arrayIndex; /**< index in array, only valid if arrayName and object are filled. */
    };
    /** Construct URI by parsing given string. */
    URI parseURI(const TString& uri);

    /** create DataStore overview. */
    TString createMainPage() const;
    /** create list of array contents. */
    TString createArrayPage(const URI& uri) const;
    /** create object info. */
    TString createObjectPage(const URI& uri) const;

    /** returns string with title, breadcrumbs, menu. */
    TString getHeader(const URI& uri = URI()) const;

    /** human-readable name (e.g. pi+)
     *
     *  use TObject::GetName()? default is ok.
     */
    static TString getName(const TObject* obj);

    /** Where is this object in the datastore? */
    static TString getIdentifier(const TObject* obj);

    /** get URI to this object. */
    static TString getURI(const TObject* obj);

    /** return entry name & index for arrays, with index = -1 for objects. */
    static std::pair<std::string, int> getDataStorePosition(const TObject* obj);

    /** Get object info (e.g. via RelationsObject::getInfoHTML()). */
    static TString getInfo(const TObject* obj);

    /** Get object contents (member data). */
    static TString getContents(const TObject* obj);

    /** return HTML-formatted list of related objects. */
    static TString getRelatedInfo(const TObject* obj);

    std::set<TString> m_visited; /**< list of all pages viewed in current event. */

    std::vector<TString> m_history; /**< ordered list of all pages viewed in current event. */

    ClassDef(InfoWidget, 0); /**< text-based info viewer showing DataStore contents. */
  };

}
