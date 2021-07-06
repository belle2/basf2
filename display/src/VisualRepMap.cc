/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <display/VisualRepMap.h>

#include <display/EveTower.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/DataStore.h>

#include <TEveSelection.h>
#include <TEveCaloData.h>
#include <TEveManager.h>

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>

using namespace Belle2;

namespace {
  /** defines a bidirectional mapping between TObjects in DataStore and their visual representation. */
  typedef boost::bimaps::bimap <
  boost::bimaps::unordered_multiset_of<const TObject*>,
        boost::bimaps::unordered_multiset_of<TEveElement*>
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

VisualRepMap::~VisualRepMap() { clear(); delete m_dataStoreEveElementMap; }

void VisualRepMap::clear()
{
  m_dataStoreEveElementMap->clear();

  for (EveTower* t : m_eclTowers)
    delete t;
  m_eclTowers.clear();
}


const TObject* VisualRepMap::getDataStoreObject(TEveElement* elem) const
{
  //special handling for TEveCaloData
  if (TEveCaloData* caloData = dynamic_cast<TEveCaloData*>(elem)) {
    const auto& selectedCells = caloData->GetCellsSelected();
    if (selectedCells.empty())
      return nullptr;
    int id = selectedCells[0].fTower;
    for (EveTower* eveTower : m_eclTowers) {
      if (eveTower->getID() == id) {
        elem = eveTower;
        break;
      }
    }
  }

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
  auto range = m_dataStoreEveElementMap->left.equal_range(object);
  for (auto it = range.first; it != range.second; ++it) {
    TEveElement* elem = it->second;
    if (elem and !gEve->GetSelection()->HasChild(elem)) {
      //select this object in addition to existing selection
      gEve->GetSelection()->UserPickedElement(elem, true);
    }
  }
  m_currentlySelecting = false;
}

void VisualRepMap::selectOnly(TEveElement* eveObj) const
{
  //copy current selection, then deselect each element
  const std::list<TEveElement*> sel(gEve->GetSelection()->BeginChildren(), gEve->GetSelection()->EndChildren());
  for (TEveElement* el : sel) {
    if (el == eveObj or el->IsA() == EveTower::Class())
      continue;
    gEve->GetSelection()->UserUnPickedElement(el);
  }
}

void VisualRepMap::selectRelated(TEveElement* eveObj) const
{
  const TObject* representedObject = getDataStoreObject(eveObj);
  if (representedObject) {
    //representedObject->Dump();

    const RelationVector<TObject>& relatedObjects = DataStore::getRelationsWithObj<TObject>(representedObject, "ALL");
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
    B2DEBUG(100, "Failed to insert object represented by " << visualRepresentation->GetElementName() << "! Duplicate?");
  }
}

void VisualRepMap::addCluster(const TObject* dataStoreObject, TEveCaloData* caloData, int towerID)
{
  EveTower* tower = new EveTower(caloData, towerID);
  tower->IncDenyDestroy(); //otherwise the selection wants to own this.
  m_eclTowers.push_back(tower);

  add(dataStoreObject, tower);
}
