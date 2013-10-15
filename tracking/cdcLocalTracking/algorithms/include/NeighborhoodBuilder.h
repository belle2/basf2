/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef NEIGHBORHOODBUILDER_H_
#define NEIGHBORHOODBUILDER_H_

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>

#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>


namespace Belle2 {
  namespace CDCLocalTracking {

    /// Builder for a neigborhood over a sorted collection
    /** Often one face the problem of having to build a neighborhood relation between elements of the \n
     *  same kind. To find suitable neighbors in a general container it would take an amount of time \n
     *  proportional to n*n to compare all available elements to each other, which is often to long. \n
     *  However if we can sort the sequence we can improve look up speed to an element by a great deal. \n
     *  All tracking entities we want to build neighborhoods for are therefore already made sortable. \n
     *  But the improved look up speed only helps if the neighbors are not scattered around randomly over \n
     *  the sorted range but are close together in a specific section of the range. The time complexity drops \n
     *  than to n*log n + n * m where is the number of elements in the collection and m the expected \n
     *  of neighbors.
     *
     *  Since we already sorted out the arrangement of entities during their creation, we have to define \n
     *  the region where to look for neighbors. Since we keep the general logic for look up the same \n
     *  but vary the definition of what a neighborhood is supposed to be we factor the later about into \n
     *  a strategy object called the NeighborChooser which defines the following interface. \n
     *
     *  X lowestPossibleNeighbor(const X & item) const
     *  which returns a lowest possible neighbor for item to be searched for in the collection.
     *  The lowest neighbor must now necessarilly be part of the collection but just marks a lower bound \n
     *  for neighbors to look for. The lower bound for a concrete possible neighbor in the is fetched \n
     *  from the collection by a call to .lower_bound(). Needless to say that the chooser has to know \n
     *  the sorting relation to make an appropriate suggestion where to look.
     *
     *  The returned concret lower_bound from the collection is then checked to be valid by a call to
     *  bool isStillPossibleNeighbor(const X & item,
                                     const X & currentNeighbor,
                                     const X & lowestPossibleNeighbor) const;
     *  giving again the item the found lower bound and the lowest possible neighbor just returned by \n
     *  the chooser. The chooser can signal with true that this concrete lower bound is still valid by \n
     *  returning true. Or it can signal by false that the range of possible neighbors has terminated. \n
     *  In this case we are considered done with this element as and the next element from the collection \n
     *  is taken for a new call to get lowestPossible neighbor.
     *  The lowest possible neighbor is given as a parameter as well in case the computation of it was \n
     *  expensive and should not be done again.
     *
     *  The last methode only signals that we are still in the range of possible neighbors but not if \n
     *  the current one is a neighbor to be added to the neighborhood. The evaluation of the neighbor \n
     *  quality is up to the function
     *  Weight isGoodNeighbor(const X & item, const X & currentNeighbor) const;
     *  It can return NOT_A_NEIGHBOR (NaN) to signal that the current neighbor shall not be added as a \n
     *  valid neighborhood relation. Else it can return a valid weight and a neighborhood relation is \n
     *  inserted to the neighborhood with the weight.
     *
     *  The sorted collection we want the neighborhood to be based on can vary as the template parameter \n
     *  Sorted collections can be
     *  set<X>
     *  CDCGenericHitVector<X> ( that has been sorted )
     *  CDCGenericHitSet<X>
     *  or everything iterable collection that has a Collection::lower_bound(const X &) returning a \n
     *  Collection::const_iterator.
     *  It also provides an initial clear() to erase information gather in previous run of the \n
     *  neighborhood builder.
     */
    template<class Collection, class NeighborChooser>
    class NeighborhoodBuilder {


    private:
      typedef typename Collection::value_type Item; /// type of the item
      typedef WeightedNeighborhood<const Item> Neighborhood; /// type of the neighborhood to be filled

    private:

      NeighborChooser m_chooser; ///< Memory for the neighborhood chooser

    public:
      ///Empty constructor initialzing the default chooser
      NeighborhoodBuilder() : m_chooser()
      {;}

      /// Constructor initialzing the chooser to the given value. Use in case the chooser has parameters you want to set.
      NeighborhoodBuilder(const NeighborChooser& chooser) : m_chooser(chooser)
      {;}

      ///Empty destructor
      ~NeighborhoodBuilder() {;}

      /// Drop all relations in a neighborhood an create a new one
      /** Drops the old neighborhood by clear() and append() the neighborhood as defined by the neighbor chooser.
         @param collection the sorted collection containing the items or pointers to these items
         @param neighborhood an instance of WeightedNeighborhood<Item> to be filled by reference. */
      void create(const Collection& collection, Neighborhood& neighborhood) const {
        neighborhood.clear();
        append(collection, neighborhood);
      }
      /// Append the neighborhood relations to an existing neighborhood
      /** Carries out the algorthim mentioned in the class description and fills the neighborhood given. \n   *  Iterates over the sorted collection and asks the neighbor chooser for the lowest possible neighbor \n
       *  for each item. Then the .lower_bound() of is searched in the collection and yields a concrete \n
       *  iterator marking the lowest concrete possible neighbor. Checks back with the chooser if this   \n
       *  concrete element is still in the valid range of possible neighbors. The neighborhood chooser \n
       *  signals with if the range of possible neighbors was already left. If the range was not left \n
       *  the chooser is asked again if the concrete possilbe neighbor shall be added to the neighborhood \n
       *  and if so with which weight. The chooser signals NOT_A_NEIGHBOR if the possible neighbor shall \n
       *  not be added or with a valid weight to signal the neighborhood relation between the item and the  \n
       *  neighbor shall be added with the returned weight. \n
       *  @param collection the collection containing the items or pointers to these items \n
       *  @param neighborhood an instance of WeightedNeighborhood<Item> to be filled by reference. */
      void append(const Collection& collection, Neighborhood& neighborhood) const {
        //forget everything from former creations
        m_chooser.clear();
        for (typename Collection::const_iterator itItem = collection.begin();
             itItem != collection.end(); ++itItem) {

          const Item& item = *itItem;
          const Item lowestPossibleNeighbor = m_chooser.getLowestPossibleNeighbor(item);

          for (typename Collection::const_iterator itPossibleNeighbor = collection.lower_bound(lowestPossibleNeighbor);
               itPossibleNeighbor != collection.end() and
               m_chooser.isStillPossibleNeighbor(item, *itPossibleNeighbor, lowestPossibleNeighbor);
               ++itPossibleNeighbor) {

            Weight weight = m_chooser.isGoodNeighbor(item,
                                                     *itPossibleNeighbor,
                                                     lowestPossibleNeighbor);

            if (not isNotANeighbor(weight)) {
              // The neighborhood takes references and keeps them
              neighborhood.insert(item, (*itPossibleNeighbor), weight);
            }
          }
        }
      }

    private:

    }; // end class


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //NEIGHBORHOODBUILDER_H_
