#ifndef PYRELATIONARRAY_H
#define PYRELATIONARRAY_H

#include <framework/dataobjects/RelationContainer.h>

#include <string>
#include <vector>
#include <map>

namespace Belle2 {
  /** a Python wrapper for RelationArray.
   *
   * \note For saved objects deriving from RelationsObject you don't need to use this class.
   *       Instead you can access relations directly from each object, see
   *       framework/examples/cdcplotmodule.py for a full example.
   *
   * Note that constructing the map used by getToIndices() requires looping
   * over all relations, so you'll want to have only one PyRelationArray per
   * name and event.
   *
   * Example (Loop over MCParticles and find corresponding PXDSimHits):
     \code{.py}
      mcpart_to_pxdhits = Belle2.PyRelationArray('MCParticlesToPXDSimHits')
      #...
      for mcpart_index in range(num_mcparticles):
      pxdhits_for_current_particle = mcpart_to_pxdhits.getToIndices(mcpart_index)
      for hit_index in pxdhits_for_current_particle:
        hit = pxdhits[hit_index]
        #do something with hit
     \endcode
   */
  class PyRelationArray {
  public:
    /** Constructor
    * @param name Name of the relation array
    * @param durability 0: event, 1: persistent
    */
    PyRelationArray(const std::string& name, int durability = 0);

    /** returns number of relations */
    int getEntries() const {
      return isValid() ? m_relations->getEntries() : 0;
    }

    /** returns true if the associated RelationArray exists, false otherwise.
     *
     * Note that getEntries() and getToIndices() are safe even for invalid
     * arrays.
     */
    bool isValid() const { return m_relations; }

    /** returns a vector of 'to' indices corresponding to the given 'from' index, or an empty list if 'from' cannot be found.
     *
     * This function internally uses a map to ensure fast lookups.
     */
    std::vector<unsigned int> getToIndices(unsigned int from) const;

    /** returns the RelationElement at index i.
     *
     * @param i Index, should be in range [0, getEntries()-1]
     */
    const RelationElement& operator [](int i) const { return m_relations->elements(i);}

  private:
    /** wrapped object */
    RelationContainer* m_relations;

    /** maps from-index to to-indices */
    std::map<unsigned int, std::vector<unsigned int> > m_toindicesMap;
  };
}
#endif
