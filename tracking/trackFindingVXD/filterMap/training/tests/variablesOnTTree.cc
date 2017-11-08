/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/Shortcuts.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/Filter.h>

#include <tracking/trackFindingVXD/filterMap/training/VariablesTTree.h>
#include <TTree.h>


namespace Belle2 {

  SELECTION_VARIABLE(Difference, 2, double,
                     static double value(const double& t1,
                                         const double& t2)
  { return t1 - t2; };
                    );

  SELECTION_VARIABLE(Sum, 2, double,
                     static double value(const double& t1,
                                         const double& t2)
  { return t1 + t2; };
                    );

  class VariablesOnTTree: public ::testing::Test {
  protected:

  public:

    TTree* tree1;
    TTree* tree2;
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

  TEST_F(VariablesOnTTree, basic_test)
  {
    auto filter1 = 0 < Difference() < 1 && 0 < Sum() < 1;
    auto variables1 = VariablesTTree<>::build(filter1 , tree1);

    auto filter2 = 0 < Difference() < 1 && 0 < Sum() < 1;
    auto variables2 = VariablesTTree<>::build(filter2 , tree2);

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
