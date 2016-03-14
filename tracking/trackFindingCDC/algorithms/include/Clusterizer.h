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
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>


#include <tracking/trackFindingCDC/algorithms/AutomatonCell.h>
#include <tracking/trackFindingCDC/algorithms/WeightedNeighborhood.h>

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
    template<class Item, class Cluster>
    class Clusterizer {

    public:

      /** Constructor. */
      Clusterizer() {;}

      /** Destructor.*/
      ~Clusterizer() {;}

    private:
      /// Type for the neighborhood of elements in the algorithm
      typedef WeightedNeighborhood<const Item> Neighborhood;

    public:

      /// Creates the clusters.
      /** Take the collection and its assoziated neighborhood and appends the clusters to the cluster vector give by non const reference*/
      template<class ItemRange>
      void create(const ItemRange& items,
                  const Neighborhood& neighborhood,
                  std::vector<Cluster>& clusters) const {

        clusters.reserve(30);

        //Prepare states
        m_cellStates.clear();
        for (Item const & item : items) {
          setCellState(item, -1);
        }

        int iCluster = -1;
        for (Item const & item : items) {

          if (getCellState(item) == -1) {

            clusters.push_back(Cluster());
            Cluster& newCluster = clusters.back();
            ++iCluster;

            startCluster(neighborhood, newCluster, iCluster, item);

          }
        }
      }


      /** Creates the clusters.
       *  This is a variation of create() taking pointers to objects as vertices instead of references
       */
      template<class PtrItemRange>
      void createFromPointers(const PtrItemRange& ptrItems,
                              const Neighborhood& neighborhood,
                              std::vector<Cluster>& clusters) const {


        clusters.reserve(30);

        //Prepare states
        m_cellStates.clear();
        for (Item const * ptrItem : ptrItems) {

          if (ptrItem == nullptr) {
            B2WARNING("Nullptr given as item in Clusterizer");
            continue;
          }
          const Item& item = *ptrItem;
          setCellState(item, -1);
        }

        int iCluster = -1;
        for (const Item * ptrItem : ptrItems) {

          if (ptrItem == nullptr) {
            B2WARNING("Nullptr given as item in Clusterizer");
            continue;
          }
          const Item& item = *ptrItem;

          if (getCellState(item) == -1) {

            clusters.push_back(Cluster());
            Cluster& newCluster = clusters.back();
            ++iCluster;

            startCluster(neighborhood, newCluster, iCluster, item);

          }
        }
      }


      /// Setter for the cell state if the Item inherits from AutomatonCell - use the cell state internal to the AutomtonCell.
      template<class ConvertableToAutomaton>
      typename boost::enable_if <
      boost::is_convertible<ConvertableToAutomaton, const AutomatonCell& >,
            void >::type
      setCellState(const ConvertableToAutomaton& item, const CellState& cellState) const {
        const AutomatonCell& automatonCell = item;
        automatonCell.setCellState(cellState);
      }

      /// Getter for the cell state if the Item inherits from Automaton cell - use the cell state internal to the AutomtonCell.
      template<class ConvertableToAutomaton>
      typename boost::enable_if <
      boost::is_convertible<ConvertableToAutomaton, const AutomatonCell& >,
            CellState >::type
      getCellState(const ConvertableToAutomaton& item) const {
        const AutomatonCell& automatonCell = item;
        return automatonCell.getCellState();
      }

      /// Setter for the cell state if the Item does not inherit from AutomatonCell.
      template<class NotConvertableToAutomaton>
      typename boost::disable_if <
      boost::is_convertible<NotConvertableToAutomaton, const AutomatonCell& >,
            void >::type
            setCellState(const NotConvertableToAutomaton& item, const CellState& cellState) const
      { m_cellStates[&item] = cellState; }

      /// Getter for the cell state if the Item does not inherit from AutomatonCell.
      template<class NotConvertableToAutomaton>
      typename boost::disable_if <
      boost::is_convertible<NotConvertableToAutomaton, const AutomatonCell& >,
            CellState >::type
            getCellState(const NotConvertableToAutomaton& item) const
      { return m_cellStates[&item]; }


    private:
      /// Helper function. Starting a new cluster and iterativelly (not recursively) expands it.
      inline void startCluster(const Neighborhood& neighborhood,
                               Cluster& newCluster,
                               int iCluster,
                               const Item& seedItem) const {

        //Cluster uses pointers as items instead of objects
        const Item* ptrSeedItem = &seedItem;

        setCellState(*ptrSeedItem, iCluster);
        newCluster.insert(newCluster.end(), ptrSeedItem);

        //grow the cluster
        std::vector<const Item*> itemsToCheckNow;
        std::vector<const Item*> itemsToCheckNext;

        itemsToCheckNow.reserve(10);
        itemsToCheckNext.reserve(10);

        itemsToCheckNext.push_back(ptrSeedItem);

        while (! itemsToCheckNext.empty()) {

          itemsToCheckNow.swap(itemsToCheckNext);
          itemsToCheckNext.clear();
          // Check registered items for neighbors
          BOOST_FOREACH(const Item *  clusterItem, itemsToCheckNow) {
            // Consider each neighbor
            for (const typename Neighborhood::WeightedRelation & relation : neighborhood.equal_range(clusterItem)) {

              const Item* neighborItem = getNeighbor(relation);

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

          } //end for itItem
        } // end while  !itemsToCheckNext.empty()

      } // end  startCluster(...)

    private:
      mutable std::map<const Item*, CellState> m_cellStates; ///< Memory for the cell state, if the Item does not inherit from AutomatonCell.

    }; // end class Clusterizer

  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //CLUSTERIZER_H_
