/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <display/InfoWidget.h>
#include <display/HtmlClassInspector.h>
#include <display/VisualRepMap.h>
#include <display/ObjectInfo.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationVector.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/ColorPalette.h>

#include <utility>

using namespace Belle2;

namespace {
  DataStore::EDurability getDurability(TString d)
  {
    if (d == "persistent")
      return DataStore::c_Persistent;
    if (d == "event")
      return DataStore::c_Event;
    B2FATAL("Invalid URI scheme '" << d << "' specified!");
  }
}
InfoWidget::InfoWidget(const TGWindow* p):
  TGHtml(p, 400, 600, -1)
{
  this->Connect("MouseDown(const char*)", "Belle2::InfoWidget", this, "show(const char*)");

  //magic to prevent the frame being empty.
  MapSubwindows();
  Resize();
  MapWindow();
}

InfoWidget::~InfoWidget()
{
}

int InfoWidget::IsVisited(const char* uri)
{
  //not actually sure why return type is 'int'...
  return (bool)m_visited.count(uri);
}

void InfoWidget::update()
{
  TString lastURI = "";
  if (!m_history.empty()) {
    lastURI = m_history.back();
  }
  m_visited.clear();
  m_history.clear();

  //check if the object given by lastURI exists in the new event, too.
  //array pages are ok, too
  if (lastURI != "") {
    URI parsedURI(lastURI);
    if (!parsedURI.object and !lastURI.EndsWith("/")) {
      //doesn't exist, go to main page
      lastURI = "";
    }
  }

  if (lastURI == "")
    lastURI = "main:";
  show(lastURI);
}

void InfoWidget::back()
{
  m_history.pop_back(); //drop current page
  const TString lastURI = m_history.back();
  m_history.pop_back(); //we'll go there promptly and re-add it

  show(lastURI);
}

void InfoWidget::show(const TObject* obj)
{
  show(URI::getURI(obj).Data(), false);
}

void InfoWidget::show(const char* uri, bool clearSelection)
{
  B2DEBUG(100, "Navigating to: " << uri);

  if (std::string(uri) == "back:") {
    back();
    return;
  }

  m_visited.insert(uri);
  m_history.push_back(uri);

  URI parsedURI = URI(uri);
  TString info;
  if (parsedURI.object) {
    info = createObjectPage(parsedURI);

    //highlight in display
    if (clearSelection)
      VisualRepMap::getInstance()->clearSelection();
    VisualRepMap::getInstance()->select(parsedURI.object);
  } else if (parsedURI.entryName != "") {
    info = createArrayPage(parsedURI);
  } else {
    info = createMainPage();
  }
  info = "<html><body>" + info + "</body></html>";

  Clear();
  //string only passed to function taking const char*...
  ParseText(const_cast<char*>(info.Data()));
  Layout();
}

TString InfoWidget::createMainPage() const
{
  TString info = getHeader();

  const char* schemes[] = {"event", "persistent"};
  const DataStore::EDurability durabilities[] = { DataStore::c_Event, DataStore::c_Persistent };
  const char* durabilyNames[] = {"c_Event", "c_Persistent"};

  for (int i = 0; i < 2; i++) {
    const TString scheme = schemes[i];
    auto arrayNames = DataStore::Instance().getListOfArrays(TObject::Class(), durabilities[i]);
    if (!arrayNames.empty()) {
      info += "<h2>Arrays";
      if (i > 0) {
        info += " (<tt>";
        info += durabilyNames[i];
        info += "</tt>)";
      }
      info += "</h2>";
    }
    for (std::string name : arrayNames) {
      const StoreArray<TObject> array(name, durabilities[i]);
      int nEntries = array.getEntries();
      if (nEntries)
        info += TString::Format("<a href='%s:%s/'>%s (%d)</a><br>", scheme.Data(), name.c_str(), name.c_str(), nEntries);
      else
        info += TString::Format("%s (%d)<br>", name.c_str(), nEntries);
    }

    auto objNames = DataStore::Instance().getListOfObjects(TObject::Class(), durabilities[i]);
    if (!objNames.empty()) {
      info += "<h2>Objects";
      if (i > 0) {
        info += " (<tt>";
        info += durabilyNames[i];
        info += "</tt>)";
      }
      info += "</h2>";
    }
    for (std::string name : objNames) {
      const StoreObjPtr<TObject> obj(name, durabilities[i]);
      if (obj)
        info += TString::Format("<a href='%s:%s/'>%s</a><br>", scheme.Data(), name.c_str(), name.c_str());
      else
        info += TString::Format("%s<br>", name.c_str());
    }
    if (i == 0)
      info += "<hr>";
  }
  return info;
}

TString InfoWidget::createArrayPage(const URI& uri) const
{
  const StoreArray<TObject> array(uri.entryName.Data(), getDurability(uri.scheme));
  TString info = getHeader(uri);
  if (array.getEntries() != 0) {
    info += HtmlClassInspector::getClassInfo(array[0]->IsA());
  }

  for (int i = 0; i < array.getEntries(); i++) {
    TString name = ObjectInfo::getName(array[i]);
    if (name != "")
      name = " - " + name;
    info += TString::Format("<a href='%s:%s/%d'>%s[%d]%s</a><br>",
                            uri.scheme.Data(), uri.entryName.Data(), i,
                            uri.entryName.Data(), i, name.Data());
  }
  return info;
}

TString InfoWidget::createObjectPage(const URI& uri) const
{
  TString info = getHeader(uri);
  info += ObjectInfo::getInfo(uri.object);
  info += getRelatedInfo(uri.object);
  info += getContents(uri.object);

  return info;
}
TString InfoWidget::getHeader(const URI& uri) const
{
  int numEntries = -1;
  if (uri.entryName.Length() != 0 and (!uri.object or uri.arrayIndex != -1)) {
    const StoreArray<TObject> array(uri.entryName.Data(), getDurability(uri.scheme));
    numEntries = array.getEntries();
  }

  TString col;
  if (uri.object and VisualRepMap::getInstance()->isVisualized(uri.object))
    col = "#d2ede4"; //needs to be lighter than purple/blue used for links
  else
    col = TangoPalette::getHex("Aluminium", 1);
  TString info;
  info += "<table border=0 width=100% bgcolor=" + col + "><tr>";
  //breadcrumbs
  info += "<td>";
  info += "<a href='main:'>DataStore</a> / ";
  if (uri.arrayIndex != -1) {
    info += "<a href='" + uri.scheme + ":" + uri.entryName + "/'>" + uri.entryName + "</a>";
    info += TString::Format("<b>[%d]</b>", uri.arrayIndex);
  } else {
    info += "<b>" + uri.entryName + "</b>";
  }
  info += "</td>";

  //back button
  if (m_history.size() <= 1) //current page is part of history, so we need at least two
    info += "<td align=right>Back</td>";
  else
    info += "<td align=right><a href='back:'>Back</a></td>";


  info += "</tr></table>";

  //short header for DataStore overview
  if (uri.entryName == "")
    return info;

  //title
  if (uri.object) {
    TString name = ObjectInfo::getName(uri.object);
    if (name != "")
      name = " - " + name;
    info += "<h2>" + ObjectInfo::getIdentifier(uri.object) + name + "</h2>";
  } else {
    info += TString::Format("<h2>%s (%d)</h2>", uri.entryName.Data(), numEntries);
  }


  if (uri.arrayIndex != -1) { //this is an array
    if (uri.arrayIndex == 0)
      info += "Previous";
    else
      info += "<a href='" + uri.scheme + ":" + TString::Format("%s/%d", uri.entryName.Data(), uri.arrayIndex - 1) + "'>Previous</a>";
    info += " ";
    if (uri.arrayIndex == numEntries - 1)
      info += "Next";
    else
      info += "<a href='" + uri.scheme + ":" + TString::Format("%s/%d", uri.entryName.Data(), uri.arrayIndex + 1) + "'>Next</a>";
    info += "<br> <br>";
  }

  return info;
}
TString InfoWidget::URI::getURI(const TObject* obj)
{
  auto pos = ObjectInfo::getDataStorePosition(obj);
  if (pos.first.empty()) {
    B2DEBUG(100, "No DataStore entry found for " << obj->GetName() << ", using raw pointer.");
    return TString::Format("raw:%lu", (long)obj);
  }
  return TString::Format("event:%s/%d", pos.first.c_str(), pos.second);
}

TString InfoWidget::getRelatedInfo(const TObject* obj)
{
  TString info;
  info += "<h4>Related Objects</h4>";

  StoreEntry* storeEntry = nullptr;
  int index = -1;
  {
    //relations from this
    const RelationVector<TObject> relatedObjects(DataStore::Instance().getRelationsWith(DataStore::c_ToSide, obj, storeEntry, index,
                                                 TObject::Class(), "ALL", ""));
    const TString pref = "this <b>-&gt;</b> ";
    for (size_t i = 0; i < relatedObjects.size(); i++) {
      const TObject* relObj = relatedObjects.object(i);
      double weight = relatedObjects.weight(i);
      TString name = ObjectInfo::getName(relObj);
      if (name != "")
        name = " - " + name;
      info += pref + "<a href='" + URI::getURI(relObj) + "'>" + ObjectInfo::getIdentifier(relObj) + name + "</a>";
      if (weight != 1.0)
        info += TString::Format(" (weight: %.3g)", weight);
      info += "<br>";
    }
  }

  info += " <br>"; //extra space needed!
  {
    //relations to this
    const RelationVector<TObject> relatedObjects(DataStore::Instance().getRelationsWith(DataStore::c_FromSide, obj, storeEntry, index,
                                                 TObject::Class(), "ALL", ""));

    const TString pref = "this <b>&lt;-</b> ";
    for (size_t i = 0; i < relatedObjects.size(); i++) {
      const TObject* relObj = relatedObjects.object(i);
      double weight = relatedObjects.weight(i);
      TString name = ObjectInfo::getName(relObj);
      if (name != "")
        name = " - " + name;
      info += pref + "<a href='" + URI::getURI(relObj) + "'>" + ObjectInfo::getIdentifier(relObj) + name + "</a>";
      if (weight != 1.0)
        info += TString::Format(" (weight: %.3g)", weight);
      info += "<br>";
    }
  }
  return info;
}

TString InfoWidget::getContents(const TObject* obj)
{
  TString info;

  info += "<h4>Object Details</h4>";
  info += HtmlClassInspector::getClassInfo(obj->IsA());
  info += HtmlClassInspector::getMemberData(obj);

  return info;
}

InfoWidget::URI::URI(const TString& uri)
{
  //split uri into schema:path (no double slash: only path after scheme)
  Ssiz_t protStart = uri.First(":");
  Ssiz_t protEnd = protStart + 1;
  if (protStart >= uri.Length())
    B2FATAL("URI has invalid format: " << uri);
  scheme = uri(0, protStart);
  TString path = uri(protEnd, uri.Length() - 1);

  if (scheme == "raw") {
    //interpret path as pointer
    object = reinterpret_cast<TObject*>(path.Atoll());
  } else if (path.Length() > 0 and path != "/") {
    //event/persistent
    DataStore::EDurability durability = getDurability(scheme);

    Ssiz_t delim = path.Last('/');
    Ssiz_t idxFieldLength = path.Length() - delim - 1;
    if (delim >= path.Length())
      B2FATAL("URI has invalid format: " << path);
    //ok, set entryName
    entryName = path(0, delim);
    if (idxFieldLength > 0) {
      //array index found
      arrayIndex = TString(path(delim + 1, idxFieldLength)).Atoi();
      const StoreArray<TObject> arr(entryName.Data(), durability);
      if (arrayIndex < arr.getEntries())
        object = arr[arrayIndex];
    }
    const auto& entries = DataStore::Instance().getStoreEntryMap(durability);
    const auto& it = entries.find(entryName.Data());
    if (it == entries.end()) {
      B2ERROR("Given entry '" << entryName << "' not found in DataStore, invalid URI?");
    } else if (!it->second.isArray) {
      //also set object for StoreObjPtr
      object = it->second.object;
    }
  }
}
