/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/****************************************************************
 * This test file for the PID prior classes                     *
 ****************************************************************/

#include <analysis/dbobjects/PIDPriorsTable.h>
#include <analysis/dbobjects/PIDPriors.h>

#include <framework/gearbox/Const.h>
#include <framework/utilities/TestHelpers.h>
#include <TRandom3.h>
#include <TH2F.h>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  class PIDPriorsTest : public ::testing::Test {
  protected:
  };

  /** Test of the PIDPriorsTable class */
  TEST_F(PIDPriorsTest, PIDPriorsTableTest)
  {

    std::vector<float> edgesX = {0., 0.2, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1., 1.1, 1.2, 1.3, 1.4, 1.5, 1.7, 1.9, 2.2, 2.5, 3., 3.5, 4., 5., 6., 7., 10.};
    std::vector<float> edgesY = { -1., -0.95, -0.9, -0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95, 1.};

    PIDPriorsTable PIDTable = PIDPriorsTable();
    PIDTable.setBinEdges(edgesX, edgesY);

    PIDTable.printPrior();

    PIDTable.setPriorValue(1.1, 0.54, 0.1);
    PIDTable.setPriorValue(9.5, -0.67, 0.3);

    std::cout << "after setting (1.1, 0.54) amd (9.5, -0.67)" << std::endl;
    PIDTable.printPrior();

    EXPECT_B2WARNING(PIDTable.setPriorValue(11.0, -10.01, 3.));
    EXPECT_B2WARNING(PIDTable.setPriorValue(4.23, -0.01, 4.));

    std::cout << " ----- Setting the priors from outside ------ " << std::endl;

    std::vector<float>  prior;

    for (int j = 0; j < (int)edgesY.size() - 1; j++) {
      for (int i = 0; i < (int)edgesX.size() - 1; i++) {
        prior.push_back(0.001 * (i + j * (edgesX.size() - 1)));
      }
    }


    PIDTable.setPriorsTable(prior);

    PIDTable.printPrior();

    EXPECT_FLOAT_EQ(PIDTable.getPriorValue(0.1, -0.98), 0.001 * (0 + 0 * (edgesX.size() - 1)));
    EXPECT_FLOAT_EQ(PIDTable.getPriorValue(0.71, -0.65), 0.001 * (5 + 4 * (edgesX.size() - 1)));
    EXPECT_FLOAT_EQ(PIDTable.getPriorValue(0.71, -1.65), 0.);
    EXPECT_FLOAT_EQ(PIDTable.getPriorValue(11.71, 0.876), 0.);
    EXPECT_B2WARNING(PIDTable.getPriorValue(0.71, -1.65));
    EXPECT_B2WARNING(PIDTable.getPriorValue(11.71, 0.876));
    EXPECT_FLOAT_EQ(0., PIDTable.getPriorValue(11.71, 0.876));



    // now let's try to construct a table with no or on edge only
    std::vector<float> badEdges1 = {0.};
    std::vector<float> badEdges2 = {};


    PIDPriorsTable PIDTable2 = PIDPriorsTable();

    EXPECT_B2WARNING(PIDTable2.setBinEdges(edgesX, badEdges1));
    EXPECT_B2WARNING(PIDTable2.setBinEdges(edgesX, badEdges2));


    PIDTable2.setBinEdges(edgesX, badEdges2);

    PIDTable2.printPrior();

    PIDTable2.setPriorValue(1.1, 0.54, 0.3);
    PIDTable2.setPriorValue(9.5, -0.67, 0.3);

    std::cout << "after setting (1.1, 0.54) and (9.5, -0.67)" << std::endl;
    PIDTable2.printPrior();

    EXPECT_B2WARNING(PIDTable2.setPriorValue(11.0, -10.01, 0.3));
    EXPECT_FLOAT_EQ(PIDTable2.getPriorValue(9.516, 0.), 0.3);

  }


  /** Test the PIDPriors dbobject */
  TEST_F(PIDPriorsTest, PIDPriorTest)
  {

    // Creates 6 prior objects in different formats (array, PIDPriorsTable, TH2F) to test different setters

    std::vector<float> edgesX = {0., 1, 2, 3, 4, 5, 6, 7, 8, 9, 10.};
    std::vector<float> edgesY = { -1., -0.9, -0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.};

    std::vector<float>  prior1;
    std::vector<float>  prior2;
    std::vector<float>  prior3;
    std::vector<float>  prior4;
    std::vector<float>  prior5;
    std::vector<float>  prior6;

    TH2F* counts1 = new TH2F("counts1", "counts1",  10, 0, 10., 20, -1, 1);
    TH2F* counts2 = new TH2F("counts2", "counts2",  10, 0, 10., 20, -1, 1);
    TH2F* counts3 = new TH2F("counts3", "counts3",  10, 0, 10., 20, -1, 1);
    TH2F* counts4 = new TH2F("counts4", "counts4",  10, 0, 10., 20, -1, 1);
    TH2F* counts5 = new TH2F("counts5", "counts5",  10, 0, 10., 20, -1, 1);
    TH2F* counts6 = new TH2F("counts6", "counts6",  10, 0, 10., 20, -1, 1);

    TH2F* norms = new TH2F("norms", "norms",  10, 0, 10., 20, -1, 1);

    TRandom3* rn = new TRandom3();



    for (int i = 0; i < (int)edgesX.size() - 1; i++) {
      for (int j = 0; j < (int)edgesY.size() - 1; j++) {

        float n1 = (float)rn->Poisson(100);
        float n2 = (float)rn->Poisson(100);
        float n3 = (float)rn->Poisson(700);
        float n4 = (float)rn->Poisson(200);
        float n5 = (float)rn->Poisson(100);
        float n6 = (float)rn->Poisson(10);

        float n = n1 + n2 + n3 + n4 + n5 + n6;

        prior1.push_back(n1 / n);
        prior2.push_back(n2 / n);
        prior3.push_back(n3 / n);
        prior4.push_back(n4 / n);
        prior5.push_back(n5 / n);
        prior6.push_back(n6 / n);

        counts1->SetBinContent(i + 1, j + 1, n1);
        counts2->SetBinContent(i + 1, j + 1, n2);
        counts3->SetBinContent(i + 1, j + 1, n3);
        counts4->SetBinContent(i + 1, j + 1, n4);
        counts5->SetBinContent(i + 1, j + 1, n5);
        counts6->SetBinContent(i + 1, j + 1, n6);
        norms->SetBinContent(i + 1, j + 1, n);

      }
    }


    PIDPriorsTable PIDTable1 = PIDPriorsTable();
    PIDTable1.setBinEdges(edgesX, edgesY);
    PIDTable1.setPriorsTable(prior1);

    PIDPriorsTable PIDTable2 = PIDPriorsTable();
    PIDTable2.setBinEdges(edgesX, edgesY);
    PIDTable2.setPriorsTable(prior2);

    PIDPriorsTable PIDTable3 = PIDPriorsTable();
    PIDTable3.setBinEdges(edgesX, edgesY);
    PIDTable3.setPriorsTable(prior3);

    PIDPriorsTable PIDTable4 = PIDPriorsTable();
    PIDTable4.setBinEdges(edgesX, edgesY);
    PIDTable4.setPriorsTable(prior4);

    PIDPriorsTable PIDTable5 = PIDPriorsTable();
    PIDTable5.setBinEdges(edgesX, edgesY);
    PIDTable5.setPriorsTable(prior5);

    PIDPriorsTable PIDTable6 = PIDPriorsTable();
    PIDTable6.setBinEdges(edgesX, edgesY);
    PIDTable6.setPriorsTable(prior6);

    Const::ChargedStable part_e = Const::ChargedStable(11);
    Const::ChargedStable part_mu = Const::ChargedStable(13);
    Const::ChargedStable part_pi = Const::ChargedStable(211);
    Const::ChargedStable part_K = Const::ChargedStable(321);
    Const::ChargedStable part_p = Const::ChargedStable(2212);
    Const::ChargedStable part_d = Const::ChargedStable(1000010020);


    std::cout << "setting the tables " << std::endl;
    // setter with the priortable
    PIDPriors priors = PIDPriors();
    priors.setPriors(part_e, PIDTable1);
    priors.setPriors(part_mu, PIDTable2);
    priors.setPriors(part_pi, PIDTable3);
    priors.setPriors(part_K, PIDTable4);
    priors.setPriors(part_p, PIDTable5);
    priors.setPriors(part_d, PIDTable6);

    // setter with the count histograms
    PIDPriors priors2 = PIDPriors();
    priors2.setPriors(part_e, counts1, norms);
    priors2.setPriors(part_mu, counts2, norms);
    priors2.setPriors(part_pi, counts3, norms);
    priors2.setPriors(part_K, counts4, norms);
    priors2.setPriors(part_p, counts5, norms);
    priors2.setPriors(part_d, counts6, norms);

    counts1->Divide(norms);
    counts2->Divide(norms);
    counts3->Divide(norms);
    counts4->Divide(norms);
    counts5->Divide(norms);
    counts6->Divide(norms);

    std::cout << "division done" << std::endl;
    // setter with priors stored in a TH2F
    std::cout << "setting the TH2 " << std::endl;
    PIDPriors priors3 = PIDPriors();
    priors3.setPriors(part_e, counts1);
    priors3.setPriors(part_mu, counts2);
    priors3.setPriors(part_pi, counts3);
    priors3.setPriors(part_K, counts4);
    priors3.setPriors(part_p, counts5);
    priors3.setPriors(part_d, counts6);


    std::vector<Const::ChargedStable> codes = {part_e, part_mu, part_pi, part_K, part_p, part_d};
    for (auto code : codes) {
      for (int i = 0; i < 3; i++) {
        double p = rn->Uniform(0., 10);
        double c = rn->Uniform(-1., 1);
        EXPECT_FLOAT_EQ(priors.getPriorValue(code, p, c),  priors2.getPriorValue(code, p, c));
        EXPECT_FLOAT_EQ(priors.getPriorValue(code, p, c),  priors3.getPriorValue(code, p, c));
      }
    }

    EXPECT_FLOAT_EQ(0.,  priors.getPriorValue(part_pi, 11.1, 0.));
    EXPECT_FLOAT_EQ(0.,  priors.getPriorValue(part_pi, -1, 0.5));
    EXPECT_FLOAT_EQ(0.,  priors.getPriorValue(part_pi, 0.98, -1.001));

    EXPECT_FLOAT_EQ(0.,  priors2.getPriorValue(part_pi, 11.1, 0.));
    EXPECT_FLOAT_EQ(0.,  priors2.getPriorValue(part_pi, -1, 0.5));
    EXPECT_FLOAT_EQ(0.,  priors2.getPriorValue(part_pi, 0.98, -1.001));

    EXPECT_FLOAT_EQ(0.,  priors3.getPriorValue(part_pi, 11.1, 0.));
    EXPECT_FLOAT_EQ(0.,  priors3.getPriorValue(part_pi, -1, 0.5));
    EXPECT_FLOAT_EQ(0.,  priors3.getPriorValue(part_pi, 0.98, -1.001));

  }

}  // namespace
