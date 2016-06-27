/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/interface/Interface.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>

#include <framework/database/LocalDatabase.h>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  class TestOptions : public MVA::SpecificOptions {

  public:
    virtual void load(const boost::property_tree::ptree&) override { }
    virtual void save(boost::property_tree::ptree&) const override { }
    virtual po::options_description getDescription() override
    {
      po::options_description description("General options");
      description.add_options()
      ("help", "print this message");
      return description;
    }
    virtual std::string getMethod() const override { return "Test"; }
  };

  class TestTeacher : public MVA::Teacher {
  public:
    TestTeacher(const MVA::GeneralOptions& g, const TestOptions&) : MVA::Teacher(g) { }
    virtual MVA::Weightfile train(MVA::Dataset&) const { return MVA::Weightfile(); }
  };

  class TestExpert : public MVA::Expert {
  public:
    virtual void load(MVA::Weightfile&) { }
    virtual std::vector<float> apply(MVA::Dataset&) const { return std::vector<float>(); };
  };

  TEST(InterfaceTest, InterfaceCreation)
  {
    {
      MVA::Interface<TestOptions, TestTeacher, TestExpert> interface;

      EXPECT_EQ(interface.getName(), "Test");
      auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
      EXPECT_TRUE(supported_interfaces.find("Test") != supported_interfaces.end());
      EXPECT_EQ(supported_interfaces["Test"], &interface);

      // These should just work
      interface.getOptions();
      interface.getExpert();
      interface.getTeacher(MVA::GeneralOptions(), TestOptions());

      // Adding the same interface twice should emit a warning
      EXPECT_B2WARNING((MVA::Interface<TestOptions, TestTeacher, TestExpert>()));
    }

    // Now it should be removed again
    auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
    EXPECT_TRUE(supported_interfaces.find("Test") == supported_interfaces.end());


  }
}
