#pragma once

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

class TEveElement;
class TObject;

namespace Belle2 {
  /** defines a bidirectional mapping between TObjects in DataStore and their visual representation.
   *
   * This is used to allow retrieving the original objects for a given TEveElement (to show additional info),
   * and vice versa.
   */
  class VisualRepMap {

    /** defines a bidirectional mapping between TObjects in DataStore and their visual representation. */
    typedef boost::bimaps::bimap <
    boost::bimaps::unordered_set_of<const TObject*>,
          boost::bimaps::unordered_set_of<TEveElement*>
          > DataStoreEveElementMap;

  public:

    /** Get object represented by given visual representation.
     *
     * (elem is pretty much const, but boost refuses to search for it then. )
     */
    const TObject* getDataStoreObject(TEveElement* elem) const;


    /** Get visual representation of given object. */
    TEveElement* getEveElement(const TObject* obj) const;

    /** Remove all contents in map. */
    void clear() { m_dataStoreEveElementMap.clear(); }

    /** Generic function to keep track of which objects have which visual representation.
     *
     * Should be called by functions adding TEveElements to the event scene
     * (Hits are currently excluded).
     *
     * Does not take ownership of given objects, just stores mapping between pointers.
     */
    void add(const TObject* dataStoreObject, TEveElement* visualRepresentation) {
      m_dataStoreEveElementMap.insert(DataStoreEveElementMap::value_type(dataStoreObject, visualRepresentation));
    }
  private:
    /** Map DataStore contents in current event with their visual representation. */
    DataStoreEveElementMap m_dataStoreEveElementMap;

  };
}
