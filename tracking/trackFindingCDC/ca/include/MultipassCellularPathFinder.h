/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/ca/CellularAutomaton.h>
#include <tracking/trackFindingCDC/ca/CellularPathFollower.h>

#include <tracking/trackFindingCDC/ca/Path.h>
#include <tracking/trackFindingCDC/ca/CellHolder.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>
#include <framework/logging/Logger.h>

#include <vector>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     *  Class to combine the run of the cellular automaton and the repeated path extraction.
     *  Execute the cellular automaton and extracting paths interatively blocking the already used
     *  knots until there is no more path fullfilling the minimal length / energy requirement given
     *  as minStateToFollow to the constructor.
     */
    template <class ACellHolder>
    class MultipassCellularPathFinder {
    public:
      /// Default constructor also checking the validity of the template arguments
      MultipassCellularPathFinder()
      {
        // Experiment in how to specify the requirements of the template parameters
        // Somewhat incomplete
        static_assert_isCellHolder<ACellHolder>();
      }

    public:
      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
      {
        moduleParamList->addParameter(prefixed(prefix, "caMode"),
                                      m_param_caMode,
                                      "Mode for the cellular automaton application"
                                      "*  * 'normal'    normal path search for high value paths"
                                      "*  * 'cells'     make path for each individual cell for debugging"
                                      "*  * 'relations' make path for each individual relation for debugging",
                                      m_param_caMode);

        moduleParamList->addParameter(prefixed(prefix, "minState"),
                                      m_param_minState,
                                      "The minimal accumulated state to follow",
                                      m_param_minState);

        moduleParamList->addParameter(prefixed(prefix, "minPathLength"),
                                      m_param_minPathLength,
                                      "The minimal path length to that is written to output",
                                      m_param_minPathLength);

      }

      /// Applies the cellular automaton to the collection and its relations
      void apply(const std::vector<ACellHolder*>& cellHolders,
                 const std::vector<WeightedRelation<ACellHolder>>& cellHolderRelations,
                 std::vector<Path<ACellHolder> >& paths)
      {
        B2ASSERT("Expected the relations to be sorted",
                 std::is_sorted(cellHolderRelations.begin(), cellHolderRelations.end()));

        // Forward all cells as paths
        if (m_param_caMode == "cells") {
          for (ACellHolder* cellHolder : cellHolders) {
            paths.push_back({cellHolder});
          }
          return;
        }

        // Forward all relations as paths
        if (m_param_caMode == "relations") {
          for (const WeightedRelation<ACellHolder>& cellHolderRelation : cellHolderRelations) {
            paths.push_back({cellHolderRelation.getFrom(), cellHolderRelation.getTo()});
          }
          return;
        }

        // Everything else is just normal
        if (m_param_caMode != "normal") {
          B2WARNING("Unrecognised caMode parameter value " << m_param_caMode);
          m_param_caMode = "normal";
        }

        // Multiple passes of the cellular automaton. One path is created
        // at a time denying all knots it picked up, applying the
        // cellular automaton again and so on. No best candidate
        // analysis needed
        for (ACellHolder* cellHolder : cellHolders) {
          cellHolder->unsetAndForwardMaskedFlag();
        }

        B2DEBUG(100, "Apply multipass cellular automat");
        do {
          m_cellularAutomaton.applyTo(cellHolders, cellHolderRelations);

          auto lessStartCellState = [this](ACellHolder * lhs, ACellHolder * rhs) {
            AutomatonCell& lhsCell = lhs->getAutomatonCell();
            AutomatonCell& rhsCell = rhs->getAutomatonCell();

            // Cells with state lower than the minimal cell state are one lowest category
            if (rhsCell.getCellState() < m_param_minState) return false;
            if (lhsCell.getCellState() < m_param_minState) return true;

            return (std::make_tuple(lhsCell.hasPriorityPathFlag(),
                                    lhsCell.hasStartFlag(),
                                    lhsCell.getCellState()) <
                    std::make_tuple(rhsCell.hasPriorityPathFlag(),
                                    rhsCell.hasStartFlag(),
                                    rhsCell.getCellState()));
          };

          auto itStartCellHolder =
            std::max_element(cellHolders.begin(), cellHolders.end(), lessStartCellState);
          if (itStartCellHolder == cellHolders.end()) break;
          else if (not(*itStartCellHolder)->getAutomatonCell().hasStartFlag()) break;
          else if ((*itStartCellHolder)->getAutomatonCell().getCellState() < m_param_minState) break;

          const ACellHolder* highestCellHolder = *itStartCellHolder;

          Path<ACellHolder> newPath = m_cellularPathFollower.followSingle(highestCellHolder,
                                      cellHolderRelations,
                                      m_param_minState);

          if (newPath.empty()) break;

          // Block the used items
          for (ACellHolder* cellHolderPtr : newPath) {
            cellHolderPtr->setAndForwardMaskedFlag();
          }

          // Block the items that have already used components
          for (ACellHolder* cellHolder : cellHolders) {
            cellHolder->receiveMaskedFlag();
          }

          if (static_cast<int>(newPath.size()) >= m_param_minPathLength) {
            paths.push_back(std::move(newPath));
          }

        } while (true);
      }

    private:
      /**
       *  Mode for the cellular automaton application
       *
       *  * 'normal'    normal path search for high value paths
       *  * 'cells'     make path for each individual cell for debugging
       *  * 'relations' make path for each individual relation for debugging
       */
      std::string m_param_caMode{"normal"};

      /// The minimal accumulated state of the paths to follow
      Weight m_param_minState = -INFINITY;

      /// The minimal path length to write to output
      int m_param_minPathLength = 0;

      /// The cellular automaton to be used.
      CellularAutomaton<ACellHolder> m_cellularAutomaton;

      /// The path follower used to extract the path from the graph processed by the cellular automaton.
      CellularPathFollower<ACellHolder> m_cellularPathFollower;
    };
  }
}
