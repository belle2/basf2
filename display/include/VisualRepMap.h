#pragma once

class TEveElement;
class TObject;

namespace Belle2 {
  /** defines a bidirectional mapping between TObjects in DataStore and their visual representation.
   *
   * This is used to allow retrieving the original objects for a given TEveElement (to show additional info),
   * and vice versa.
   */
  class VisualRepMap {
  public:
    static VisualRepMap* getInstance();

    /** Select the representation of the given object. */
    void select(const TObject* object) const;

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

    /** Get visual representation of given object. */
    TEveElement* getEveElement(const TObject* obj) const;

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
  private:
    VisualRepMap();
    /** no copy ctor */
    VisualRepMap(const VisualRepMap&) = delete;
    ~VisualRepMap();

    class DataStoreEveElementMap;

    mutable bool m_currentlySelecting; /**< are we currently in a select() call? */

    /** Map DataStore contents in current event with their visual representation. */
    DataStoreEveElementMap* m_dataStoreEveElementMap;

  };
}
