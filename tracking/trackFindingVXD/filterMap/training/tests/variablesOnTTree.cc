/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <gtest/gtest.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/Shortcuts.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/Filter.h>

#include <tracking/trackFindingVXD/filterMap/training/VariablesTTree.h>
#include <TTree.h>


namespace Belle2 {

  /// Quick definition of a selection variable implementing the difference of 2 doubles
  SELECTION_VARIABLE(Difference, 2, double,
                     static double value(const double& t1,
                                         const double& t2)
                     // cppcheck-suppress unknownMacro
  { return t1 - t2; };
                    );

  /// Quick definition of a selection variable implementing the sum of 2 doubles
  SELECTION_VARIABLE(Sum, 2, double,
                     static double value(const double& t1,
                                         const double& t2)
  { return t1 + t2; };
                    );

  /// Test for VariablesTTree
  class VariablesOnTTree: public ::testing::Test {
  protected:

  public:
    TTree* tree1; /**< a TTree */
    TTree* tree2; /**< another TTree */
    VariablesOnTTree() : tree1(nullptr), tree2(nullptr)
    {
      tree1 = new TTree("t1", "t1");
      tree2 = new TTree("t2", "t2");

    }

    ~VariablesOnTTree()
    {
      delete tree1;
      delete tree2;
    }
  };

  /// Basic test of the class
  TEST_F(VariablesOnTTree, basic_test)
  {
    auto filter1 = 0 < Difference() < 1 && 0 < Sum() < 1;
    auto variables1 = VariablesTTree<>::build(filter1, tree1);

    auto filter2 = 0 < Difference() < 1 && 0 < Sum() < 1;
    auto variables2 = VariablesTTree<>::build(filter2, tree2);

    double a(0.), b(1.0);
    variables1.evaluateOn(a, b);
    tree1->Fill();

    double c(1.), d(0.0);
    variables2.evaluateOn(c, d);
    tree2->Fill();

    tree1->Scan();
    tree2->Scan();
  }
}
