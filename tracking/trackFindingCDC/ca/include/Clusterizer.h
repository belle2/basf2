/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <boost/foreach.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>


#include <tracking/trackFindingCDC/ca/AutomatonCell.h>
#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <framework/logging/Logger.h>


namespace Belle2 {
  namespace TrackFindingCDC {


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
     * The neighborhood given to the clusterizer must be of type WeightedNeighborhood<value_type>*/
    template<class AItem, class ACluster>
    class Clusterizer {

    private:
      /// Type for the neighborhood of elements in the algorithm
      typedef WeightedNeighborhood<AItem> Neighborhood;

    public:

      /// Creates the clusters.
      /** Take the collection and its assoziated neighborhood and appends the clusters to the cluster vector give by non const reference*/
      template<class AItemRange>
      void create(AItemRange& items,
                  const Neighborhood& neighborhood,
                  std::vector<ACluster>& clusters) const
      {

        clusters.reserve(30);

        //Prepare states
        for (AItem& item : items) {
          setCellState(item, -1);
        }

        int iCluster = -1;
        for (AItem& item : items) {

          if (getCellState(item) == -1) {

            clusters.push_back(ACluster());
            ACluster& newCluster = clusters.back();
            ++iCluster;

            startCluster(neighborhood, newCluster, iCluster, item);

          }
        }
      }


      /** Creates the clusters.
       *  This is a variation of create() taking pointers to objects as vertices instead of references
       */
      template<class APtrItemRange>
      void createFromPointers(APtrItemRange& ptrItems,
                              const Neighborhood& neighborhood,
                              std::vector<ACluster>& clusters) const
      {


        clusters.reserve(30);

        //Prepare states
        for (AItem* ptrItem : ptrItems) {

          if (ptrItem == nullptr) {
            B2WARNING("Nullptr given as item in Clusterizer");
            continue;
          }
          AItem& item = *ptrItem;
          setCellState(item, -1);
        }

        int iCluster = -1;
        for (AItem* ptrItem : ptrItems) {

          if (ptrItem == nullptr) {
            B2WARNING("Nullptr given as item in Clusterizer");
            continue;
          }
          AItem& item = *ptrItem;

          if (getCellState(item) == -1) {

            clusters.push_back(ACluster());
            ACluster& newCluster = clusters.back();
            ++iCluster;

            startCluster(neighborhood, newCluster, iCluster, item);

          }
        }
      }

      /// Setter for the cell state if the AItem inherits from AutomatonCell - use the cell state internal to the AutomtonCell.
      void setCellState(AItem& item, CellState cellState) const
      {
        AutomatonCell& automatonCell = item.getAutomatonCell();
        automatonCell.setCellState(cellState);
      }

      /// Getter for the cell state if the AItem inherits from Automaton cell - use the cell state internal to the AutomtonCell.
      CellState getCellState(const AItem& item) const
      {
        const AutomatonCell& automatonCell = item.getAutomatonCell();
        return automatonCell.getCellState();
      }

      // Set the cell weight - use the cell weight internal to the AutomtonCell.
      void setCellWeight(AItem& item, CellWeight cellWeight) const
      {
        AutomatonCell& automatonCell = item.getAutomatonCell();
        automatonCell.setCellWeight(cellWeight);
      }


    private:
      /// Helper function. Starting a new cluster and iterativelly (not recursively) expands it.
      inline void startCluster(const Neighborhood& neighborhood,
                               ACluster& newCluster,
                               int iCluster,
                               AItem& seedItem) const
      {
        //ACluster uses pointers as items instead of objects
        AItem* ptrSeedItem = &seedItem;

        setCellState(*ptrSeedItem, iCluster);
        newCluster.insert(newCluster.end(), ptrSeedItem);

        //grow the cluster
        std::vector<AItem*> itemsToCheckNow;
        std::vector<AItem*> itemsToCheckNext;

        itemsToCheckNow.reserve(10);
        itemsToCheckNext.reserve(10);

        itemsToCheckNext.push_back(ptrSeedItem);

        while (not itemsToCheckNext.empty()) {

          itemsToCheckNow.swap(itemsToCheckNext);
          itemsToCheckNext.clear();
          // Check registered items for neighbors
          for (AItem* clusterItem : itemsToCheckNow) {
            size_t nNeighbors = 0;

            // Consider each neighbor
            for (const typename Neighborhood::WeightedRelation& relation : neighborhood.equal_range(clusterItem)) {
              ++nNeighbors;

              AItem* neighborItem = getNeighbor(relation);

              CellState neighborICluster = getCellState(*neighborItem);
              if (neighborICluster == -1) {
                // Element not yet in cluster
                //Add the element
                setCellState(*neighborItem, iCluster);
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

            setCellWeight(*clusterItem, nNeighbors);

          } //end for itItem
        } // end while  !itemsToCheckNext.empty()

      } // end  startCluster(...)

    }; // end class Clusterizer

  } //end namespace TrackFindingCDC
} //end namespace Belle2

