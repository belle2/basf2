/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <gtest/gtest.h>

#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>
#include <tracking/trackFindingCDC/ca/WithAutomatonCell.h>
#include <framework/logging/Logger.h>

#include <utility>

using namespace Belle2;
using namespace TrackFindingCDC;

TEST(TrackFindingCDCTest, CellularPathFollower_followSingle)
{
  using Element = WithAutomatonCell<std::pair<int, int> >;
  Element startElement(std::make_pair(1, 1));
  startElement.getAutomatonCell().setCellWeight(1);

  Element secondElement(std::make_pair(1, 1));
  secondElement.getAutomatonCell().setCellWeight(1);

  std::vector<WeightedRelation<Element>> relations;
  relations.emplace_back(&startElement, 1, &secondElement);

  // Mimic execution of the cellular automaton
  startElement.getAutomatonCell().setStartFlag();
  startElement.getAutomatonCell().setCellState(3);
  startElement.getAutomatonCell().setAssignedFlag();

  secondElement.getAutomatonCell().setCellState(1);
  secondElement.getAutomatonCell().setAssignedFlag();

  CellularPathFollower<Element> cellularPathFollower;
  const std::vector<Element*> elementPath =
    cellularPathFollower.followSingle(&startElement, relations, -INFINITY);
  EXPECT_EQ(2, elementPath.size());
  EXPECT_EQ(&startElement, elementPath.front());
  EXPECT_EQ(&secondElement, elementPath.back());
}
