/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/FANN.h>
#include <mva/interface/Interface.h>
#include <framework/utilities/FileSystem.h>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  TEST(FANNTest, WeightfilesAreReadCorrectly)
  {
    MVA::Interface<MVA::FANNOptions, MVA::FANNTeacher, MVA::FANNExpert> interface;

    MVA::GeneralOptions general_options;
    general_options.m_variables = {"M", "p", "pt"};
    MVA::MultiDataset dataset(general_options, {{1.835127, 1.179507, 1.164944},
      {1.873689, 1.881940, 1.843310},
      {1.863657, 1.774831, 1.753773},
      {1.858293, 1.605311, 0.631336},
      {1.837129, 1.575739, 1.490166},
      {1.811395, 1.524029, 0.565220}
    },
    {}, {0.0, 1.0, 0.0, 1.0, 0.0, 1.0});

    auto expert = interface.getExpert();

    auto weightfile = MVA::Weightfile::loadFromFile(FileSystem::findFile("mva/methods/tests/FANN.xml"));
    expert->load(weightfile);
    auto probabilities = expert->apply(dataset);
    EXPECT_NEAR(probabilities[0], 0.047535836696624756, 0.0001);
    EXPECT_NEAR(probabilities[1], 0.7130427360534668, 0.0001);
    EXPECT_NEAR(probabilities[2], 0.7729528546333313, 0.0001);
    EXPECT_NEAR(probabilities[3], 0.16526281833648682, 0.0001);
    EXPECT_NEAR(probabilities[4], 0.0091879460960626602, 0.0001);
    EXPECT_NEAR(probabilities[5], -0.21771839261054993, 0.0001);
  }

}
