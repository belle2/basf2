#include <display/InfoWidget.h>
#include <display/HtmlClassInspector.h>
#include <display/VisualRepMap.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/RelationVector.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/HTML.h>

#include <utility>

using namespace Belle2;

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

void InfoWidget::newEvent()
{
  m_visited.clear();
  m_history.clear();

  show("/");
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

  if (std::string(uri) == "back") {
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
  info += "<p>Arrays/Objects of <tt>c_Event</tt> durability:</p>";
  info += "<h2>Arrays</h2>";
  for (std::string name : DataStore::Instance().getListOfArrays(TObject::Class(), DataStore::c_Event)) {
    const StoreArray<TObject> array(name);
    int nEntries = array.getEntries();
    if (nEntries)
      info += TString::Format("<a href='%s/'>%s (%d)</a><br>", name.c_str(), name.c_str(), nEntries);
    else
      info += TString::Format("%s (%d)<br>", name.c_str(), nEntries);
  }

  info += "<h2>Objects</h2>";
  for (std::string name : DataStore::Instance().getListOfObjects(TObject::Class(), DataStore::c_Event)) {
    const StoreObjPtr<TObject> obj(name);
    if (obj)
      info += TString::Format("<a href='%s/'>%s</a><br>", name.c_str(), name.c_str());
    else
      info += TString::Format("%s<br>", name.c_str());
  }
  return info;
}

TString InfoWidget::createArrayPage(const URI& uri) const
{
  const StoreArray<TObject> array(uri.entryName.Data());
  TString info = getHeader(uri);
  if (array.getEntries() != 0) {
    info += HtmlClassInspector::getClassInfo(array[0]->IsA());
  }

  for (int i = 0; i < array.getEntries(); i++) {
    TString name = getName(array[i]);
    if (name != "")
      name = " - " + name;
    info += TString::Format("<a href='%s/%d'>%s</a><br>", uri.entryName.Data(), i,
                            (getIdentifier(array[i]) + name).Data());
  }
  return info;
}

TString InfoWidget::createObjectPage(const URI& uri) const
{
  TString info = getHeader(uri);
  info += getInfo(uri.object);
  info += getRelatedInfo(uri.object);
  info += getContents(uri.object);

  return info;
}
TString InfoWidget::getHeader(const URI& uri) const
{
  int numEntries = -1;
  if (!uri.object or uri.arrayIndex != -1) {
    const StoreArray<TObject> array(uri.entryName.Data());
    numEntries = array.getEntries();
  }

  TString info;
  info += "<table border=0 width=100\% bgcolor=d2ede4><tr>";
  //TODO: maybe make it small once stupid font issues are gone?
  //breadcrumbs
  info += "<td>";
  //info += "<td><small>";
  info += "<a href='/'>DataStore</a> / ";
  if (uri.arrayIndex != -1) {
    info += "<a href='" + uri.entryName + "/'>" + uri.entryName + "</a> / ";
    info += TString::Format("<b>[%d]</b>", uri.arrayIndex);
  } else {
    info += "<b>" + uri.entryName + "</b>";
  }
  //info += "</small></td>";
  info += "</td>";

  //back button
  if (m_history.size() <= 1) //current page is part of history, so we need at least two
    info += "<td align=right>Back</td>";
  else
    info += "<td align=right><a href='back'>Back</a></td>";


  info += "</tr></table>";

  //short header for DataStore overview
  if (uri.entryName == "")
    return info;

  //title
  if (uri.object) {
    TString name = getName(uri.object);
    if (name != "")
      name = " - " + name;
    info += "<h2>" + getIdentifier(uri.object) + name + "</h2>";
  } else {
    info += TString::Format("<h2>%s (%d)</h2>", uri.entryName.Data(), numEntries);
  }


  if (uri.arrayIndex != -1) { //this is an array
    if (uri.arrayIndex == 0)
      info += "Previous";
    else
      info += "<a href='" + TString::Format("%s/%d", uri.entryName.Data(), uri.arrayIndex - 1) + "'>Previous</a>";
    info += " ";
    if (uri.arrayIndex == numEntries - 1)
      info += "Next";
    else
      info += "<a href='" + TString::Format("%s/%d", uri.entryName.Data(), uri.arrayIndex + 1) + "'>Next</a>";
    info += "<br> <br>";
  }

  return info;
}

TString InfoWidget::getName(const TObject* obj)
{
  if (!obj)
    B2FATAL("InfoWidget::getName() got null?");
  if (const RelationsObject* relObj = dynamic_cast<const RelationsObject*>(obj)) {
    return relObj->getName();
  }
  return "";
}

TString InfoWidget::getInfo(const TObject* obj)
{
  if (!obj)
    B2FATAL("InfoWidget::getInfo() got null?");
  if (const RelationsObject* relObj = dynamic_cast<const RelationsObject*>(obj)) {
    return relObj->getInfoHTML();
  }
  return "";
}

std::pair<std::string, int> InfoWidget::getDataStorePosition(const TObject* obj)
{
  std::string name;
  int index = -1;
  if (const RelationsObject* relObj = dynamic_cast<const RelationsObject*>(obj)) {
    name = relObj->getArrayName();
    index = relObj->getArrayIndex();
  } else {
    //somewhat manual way to find location of object (might not inherit from RelationInterface)
    DataStore::StoreEntry* entry = nullptr;
    DataStore::Instance().findStoreEntry(obj, entry, index);
    if (entry)
      name = entry->name;
  }
  if (index == -1) {
    //this thing might be in a StoreObjPtr...
    for (const auto pair : DataStore::Instance().getStoreEntryMap(DataStore::c_Event)) {
      if (pair.second.object == obj) {
        name = pair.second.name;
        break;
      }
    }
  }

  return std::make_pair(name, index);
}

TString InfoWidget::getIdentifier(const TObject* obj)
{
  auto pos = getDataStorePosition(obj);
  if (pos.second != -1)
    return TString::Format("%s[%d]", pos.first.c_str(), pos.second);
  return pos.first;
}

TString InfoWidget::URI::getURI(const TObject* obj)
{
  auto pos = getDataStorePosition(obj);
  if (pos.first.empty()) {
    B2WARNING("No URI found for " << obj->GetName());
    return "/";
  }
  return TString::Format("%s/%d", pos.first.c_str(), pos.second);
}

TString InfoWidget::getRelatedInfo(const TObject* obj)
{
  TString info;
  info += "<h4>Related Objects</h4>";
  {
    //relations from this
    const RelationVector<TObject>& relatedObjects = DataStore::Instance().getRelationsFromObj<TObject>(obj, "ALL");
    const TString pref = "this <b>-&gt;</b> ";
    for (size_t i = 0; i < relatedObjects.size(); i++) {
      const TObject* relObj = relatedObjects.object(i);
      double weight = relatedObjects.weight(i);
      TString name = getName(relObj);
      if (name != "")
        name = " - " + name;
      info += pref + "<a href='" + URI::getURI(relObj) + "'>" + getIdentifier(relObj) + name + "</a>";
      if (weight != 1.0)
        info += TString::Format(" (weight: %.3g)", weight);
      info += "<br>";
    }
  }

  info += " <br>"; //extra space needed!
  {
    //relations to this
    const RelationVector<TObject>& relatedObjects = DataStore::Instance().getRelationsToObj<TObject>(obj, "ALL");
    const TString pref = "this <b>&lt;-</b> ";
    for (size_t i = 0; i < relatedObjects.size(); i++) {
      const TObject* relObj = relatedObjects.object(i);
      double weight = relatedObjects.weight(i);
      TString name = getName(relObj);
      if (name != "")
        name = " - " + name;
      info += pref + "<a href='" + URI::getURI(relObj) + "'>" + getIdentifier(relObj) + name + "</a>";
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

InfoWidget::URI::URI(const TString& uri):
  object(0), entryName(""), arrayIndex(-1)
{
  if (uri != "/") {
    Ssiz_t delim = uri.Last('/');
    Ssiz_t idxFieldLength = uri.Length() - delim - 1;
    //ok, set.entryName
    entryName = uri(0, delim);
    if (delim >= uri.Length()) {
      B2WARNING("delim: " << delim << " " << idxFieldLength);
      B2FATAL("URI has invalid format: " << uri);
    }
    if (idxFieldLength > 0) {
      //array index found
      arrayIndex = TString(uri(delim + 1, idxFieldLength)).Atoi();
      const StoreArray<TObject> arr(entryName.Data());
      object = arr[arrayIndex];
    }
    const auto& entries = DataStore::Instance().getStoreEntryMap(DataStore::c_Event);
    const auto& it = entries.find(entryName.Data());
    if (it == entries.end()) {
      B2ERROR("Given entry '" << entryName << "' not found in DataStore, invalid URI?");
    } else if (!it->second.isArray) {
      //also set object for StoreObjPtr
      object = it->second.object;
    }
  }
}
