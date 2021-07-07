/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>

class TEveElement;
class TEveCaloData;
class TObject;

namespace Belle2 {
  class EveTower;
  /** defines a bidirectional (many to many) mapping between TObjects in DataStore and their visual representation.
   *
   * This is used to allow retrieving the original objects for a given TEveElement (to show additional info),
   * and vice versa.
   */
  class VisualRepMap {
  public:
    /** get instance pointer. */
    static VisualRepMap* getInstance();

    /** Select the representation of the given object. */
    void select(const TObject* object) const;

    /** Deselect all other objects. */
    void selectOnly(TEveElement* eveObj) const;

    /** Select related objects. */
    void selectRelated(TEveElement* eveObj) const;

    /** Clear existing selection in Eve Browser. */
    void clearSelection() const;

    /** are we currently in a select() call?
     *
     * Call this in your selection handler and abort if true.
     * (otherwise we end up recursively calling ourselves).
     */
    bool isCurrentlySelecting() const { return m_currentlySelecting; }

    /** Get object represented by given visual representation.
     *
     * (elem is pretty much const, but boost refuses to search for it then. )
     */
    const TObject* getDataStoreObject(TEveElement* elem) const;

    /** Get (first) visual representation of given object. */
    TEveElement* getEveElement(const TObject* obj) const;

    /** Does obj have a visualization? */
    bool isVisualized(const TObject* obj) { return getEveElement(obj) != nullptr; }

    /** Remove all contents in map. (call this after each event) */
    void clear();

    /** Generic function to keep track of which objects have which visual representation.
     *
     * Should be called by functions adding TEveElements to the event scene
     * (Hits are currently excluded).
     *
     * Does not take ownership of given objects, just stores mapping between pointers.
     */
    void add(const TObject* dataStoreObject, TEveElement* visualRepresentation);

    /** Selection inside TEveCalo* is complicated, use this to keep track of ECL clusters. */
    void addCluster(const TObject* dataStoreObject, TEveCaloData* caloData, int towerID);
  private:
    /** default constructor */
    VisualRepMap();
    /** no copy ctor */
    VisualRepMap(const VisualRepMap&) = delete;
    /** no assignment */
    VisualRepMap& operator=(const VisualRepMap&) = delete;
    /** destructor */
    ~VisualRepMap();

    class DataStoreEveElementMap;

    mutable bool m_currentlySelecting; /**< are we currently in a select() call? */

    /** Map DataStore contents in current event with their visual representation. */
    DataStoreEveElementMap* m_dataStoreEveElementMap;

    /** individual ECL towers (getting them out of TEve is hard). */
    std::vector<EveTower*> m_eclTowers;

  };
}
