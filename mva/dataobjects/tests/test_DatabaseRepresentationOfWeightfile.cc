/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  TEST(DatabaseRepresentationOfWeightfileTest, Construction)
  {

    DatabaseRepresentationOfWeightfile x;
    EXPECT_EQ(x.m_data, std::string());

  }

  TEST(DatabaseRepresentationOfWeightfileTest, TObjectClassNameDoesNotChange)
  {

    // Renaming this class probably invalids all the stored weighfiles in the database,
    // hence this test
    DatabaseRepresentationOfWeightfile x;
    EXPECT_EQ(std::string(x.ClassName()), std::string("Belle2::DatabaseRepresentationOfWeightfile"));

  }

}
