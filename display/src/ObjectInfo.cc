#include <display/ObjectInfo.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/utilities/HTML.h>

#include <genfit/GFRaveVertex.h>

#include <TString.h>
#include <TMath.h>

using namespace Belle2;

TString ObjectInfo::getName(const TObject* obj)
{
  if (!obj)
    B2ERROR("ObjectInfo::getName() got null?");
  if (const RelationsObject* relObj = dynamic_cast<const RelationsObject*>(obj)) {
    return relObj->getName();
  }
  return "";
}

TString ObjectInfo::getInfo(const TObject* obj)
{
  if (!obj)
    B2ERROR("ObjectInfo::getInfo() got null?");
  if (auto relObj = dynamic_cast<const RelationsObject*>(obj)) {
    return relObj->getInfoHTML();
  } else if (auto vertex = dynamic_cast<const genfit::GFRaveVertex*>(obj)) {
    return "<b>V</b>=" + HTML::getString(vertex->getPos()) + "<br>" +
           TString::Format("pVal=%e", TMath::Prob(vertex->getChi2(), vertex->getNdf()));
  }
  return "";
}

TString ObjectInfo::getTitle(const TObject* obj)
{
  TString out(getIdentifier(obj));
  const TString& name = getName(obj);
  if (name.Length() != 0)
    out += " - " + name;

  return out + "\n" + HTML::htmlToPlainText(getInfo(obj).Data()).c_str();
}

std::pair<std::string, int> ObjectInfo::getDataStorePosition(const TObject* obj)
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

TString ObjectInfo::getIdentifier(const TObject* obj)
{
  auto pos = getDataStorePosition(obj);
  if (pos.second != -1)
    return TString::Format("%s[%d]", pos.first.c_str(), pos.second);
  return pos.first;
}

