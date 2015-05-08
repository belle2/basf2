#include <display/VisualRepMap.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/DataStore.h>

#include <TEveSelection.h>
#include <TEveManager.h>

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

using namespace Belle2;

namespace {
  /** defines a bidirectional mapping between TObjects in DataStore and their visual representation. */
  typedef boost::bimaps::bimap <
  boost::bimaps::unordered_set_of<const TObject*>,
        boost::bimaps::unordered_set_of<TEveElement*>
        > DataStoreEveElementMap_Base;
}

/** Hide implementation in private class. */
class VisualRepMap::DataStoreEveElementMap : public DataStoreEveElementMap_Base { };

VisualRepMap* VisualRepMap::getInstance()
{
  static VisualRepMap instance;
  return &instance;
}

VisualRepMap::VisualRepMap() : m_currentlySelecting(false), m_dataStoreEveElementMap(new DataStoreEveElementMap) { }

VisualRepMap::~VisualRepMap() { delete m_dataStoreEveElementMap; }

void VisualRepMap::clear() { m_dataStoreEveElementMap->clear(); }

const TObject* VisualRepMap::getDataStoreObject(TEveElement* elem) const
{
  const auto& it = m_dataStoreEveElementMap->right.find(elem);
  if (it != m_dataStoreEveElementMap->right.end())
    return it->second;
  return nullptr;
}

TEveElement* VisualRepMap::getEveElement(const TObject* obj) const
{
  const auto& it = m_dataStoreEveElementMap->left.find(obj);
  if (it != m_dataStoreEveElementMap->left.end())
    return it->second;
  return nullptr;
}


void VisualRepMap::select(const TObject* object) const
{
  if (m_currentlySelecting)
    B2FATAL("recursive select() call detected. Please check isCurrentlySelecting().");
  m_currentlySelecting = true;
  TEveElement* elem = getEveElement(object);
  if (elem and !gEve->GetSelection()->HasChild(elem)) {
    //select this object in addition to existing selection
    gEve->GetSelection()->UserPickedElement(elem, true);
  }
  m_currentlySelecting = false;
}

void VisualRepMap::selectRelated(TEveElement* eveObj) const
{
  const TObject* representedObject = getDataStoreObject(eveObj);
  if (representedObject) {
    //representedObject->Dump();

    const RelationVector<TObject>& relatedObjects = DataStore::Instance().getRelationsWithObj<TObject>(representedObject, "ALL");
    for (const TObject& relObj : relatedObjects) {
      select(&relObj);
    }
  }
}

void VisualRepMap::clearSelection() const
{
  if (!gEve)
    return;
  //equivalent to clicking into empty space
  gEve->GetSelection()->UserPickedElement(nullptr);
}
void VisualRepMap::add(const TObject* dataStoreObject, TEveElement* visualRepresentation)
{
  if (!dataStoreObject) {
    B2ERROR("Trying to insert NULL for object represented by " << visualRepresentation->GetElementName());
    return;
  }
  auto ret = m_dataStoreEveElementMap->insert(DataStoreEveElementMap::value_type(dataStoreObject, visualRepresentation));
  if (!ret.second) {
    B2ERROR("Failed to insert object represented by " << visualRepresentation->GetElementName() << "! Duplicate?");
  }
}
