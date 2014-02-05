/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CLUSTERIZER_H_
#define CLUSTERIZER_H_

#include <boost/foreach.hpp>

#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>

#include <framework/logging/Logger.h>


namespace Belle2 {
  namespace CDCLocalTracking {


    /// Implementation of the clustering
    /** Clusters elements of a given collection using the connections presented by a neighorhood.
     *  A cluster is essentially a number of items that can reach each other by one more connections
     *  in a neighborhood. A cluster therefore presents a minimal group to focus on for local tracking
     *  purposes and can be used not dimish memory consumption as well as time complexity.
     *  The algorithm is essentially an iterative expansion of the neighborhood relations keeping
     *  track of the alraedy used adopted items by using the cellState of the automaton cell.
     *  The cellState after the clusterization could therefore be used as grouping tag directly,
     *  but we return the clusters as a std::vector of clusters to avoid a reiteration over all elements
     *  and also avoid the problems arising from the cellstate being a floating point number not
     *  suitable for indexing.
     *  The collection container must support BOOST_FOREACH for iteration over its ::value_type.
     *  Cluster can be anything that is default constructable and supports .insert(end(),const value_type *item).
     * The neighborhood given to the clusterizer must be of type WeightedNeighborhood<const value_type>*/
    template<class Collection, class Cluster>
    class Clusterizer {

    public:

      /** Constructor. */
      Clusterizer() {;}

      /** Destructor.*/
      ~Clusterizer() {;}

    private:
      typedef typename Collection::value_type Item; /// Contained item type
      typedef WeightedNeighborhood<const Item> Neighborhood; /// Neighborhood type

    public:

      /// Creates the clusters.
      /** Take the collection and its assoziated neighborhood and appends the clusters to the cluster vector give by non const reference*/
      void create(const Collection& items,
                  const Neighborhood& neighborhood,
                  std::vector<Cluster>& clusters) const {

        prepareCellStates(items);

        clusters.reserve(30);

        int iCluster = -1;
        BOOST_FOREACH(Item const & item , items) {

          if (item.getAutomatonCell().getCellState() == -1) {

            clusters.push_back(Cluster());
            Cluster& newCluster = clusters.back();
            ++iCluster;

            startCluster(neighborhood, newCluster, iCluster, item);

          }
        }
      }

    private:
      /// Helper function. Sets all cell states to -1.
      inline void prepareCellStates(const Collection& items) const {
        BOOST_FOREACH(Item const & item , items) {
          item.getAutomatonCell().setCellState(-1);
        }
      }

      /// Helper function. Starting a new cluster and iterativelly (not recursively) expands it.
      inline void startCluster(const Neighborhood& neighborhood,
                               Cluster& newCluster,
                               int iCluster,
                               const Item& seedItem) const {

        //Cluster uses pointers as items instead of objects
        seedItem.getAutomatonCell().setCellState(iCluster);
        const Item* clusterSeedItem = &seedItem;
        newCluster.insert(newCluster.end(), clusterSeedItem);

        //grow the cluster
        std::vector<const Item*> itemsToCheckNow;
        std::vector<const Item*> itemsToCheckNext;

        itemsToCheckNow.reserve(10);
        itemsToCheckNext.reserve(10);

        itemsToCheckNext.push_back(clusterSeedItem);


        while (! itemsToCheckNext.empty()) {

          itemsToCheckNow.swap(itemsToCheckNext);
          itemsToCheckNext.clear();
          // Check registered items for neighbors
          BOOST_FOREACH(const Item *  clusterItem, itemsToCheckNow) {
            // Get neighbors
            typename Neighborhood::range neighborRange = neighborhood.equal_range(clusterItem);

            // Consider each neighbor
            for (typename Neighborhood::iterator itNeighbor = neighborRange.first;
                 itNeighbor != neighborRange.second; ++itNeighbor) {

              const Item* const& neighborItem = itNeighbor.getNeighbor();

              CellState neighborICluster = neighborItem->getAutomatonCell().getCellState();
              if (neighborICluster == -1) {
                // Element not yet in cluster
                //Add the element
                neighborItem->getAutomatonCell().setCellState(iCluster);
                newCluster.insert(newCluster.end(), neighborItem);

                //register it for further expansion
                itemsToCheckNext.insert(itemsToCheckNext.end(), neighborItem);

              } else if (neighborICluster != iCluster) {
                B2WARNING("Neighboring hit was already assigned to different cluster. Check if the neighborhood is symmetric.");
              } else {
                //nothing to do
                //continue;
              }
            } //end for itNeighbor

          } //end for itItem
        } // end while  !itemsToCheckNext.empty()

      } // end  startCluster(...)

    }; // end class Clusterizer

  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //CLUSTERIZER_H_
