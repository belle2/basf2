#include <analysis/utility/VariableManager.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/datastore/StoreArray.h>

#include <framework/logging/Logger.h>

#include <TVector3.h>

#include <fstream>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  /** command x should exit using B2FATAL. */
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Test Variable class. */
  class VariableTest : public ::testing::Test {
  protected:

  };


  /** Return correct answer. */
  double dummyVar(const Particle*) { return 42.0; }

  /** test VariableManager. */
  TEST_F(VariableTest, VariableManager)
  {
    const VariableManager::Var* v = VariableManager::Instance().getVariable("THISDOESNTEXIST");
    EXPECT_TRUE(v == nullptr);

    //this exists
    const VariableManager::Var* pvar = VariableManager::Instance().getVariable("p");
    EXPECT_TRUE(pvar != nullptr);

    //re-registration not allowed
    EXPECT_FATAL(VariableManager::Instance().registerVariable("p", &dummyVar, "description"));

    EXPECT_FATAL(VariableManager::Instance().registerVariable("something", nullptr, "blah"));


    VariableManager::Instance().registerVariable("testingthedummyvar", &dummyVar, "blah");
    const VariableManager::Var* dummy = VariableManager::Instance().getVariable("testingthedummyvar");
    EXPECT_TRUE(dummy != nullptr);
    EXPECT_TRUE(dummy->description == "blah");
    EXPECT_DOUBLE_EQ(dummy->function(nullptr), 42.0);

    //also test the macro (with other name)
    REGISTER_VARIABLE("testingthedummyvar2", dummyVar, "something else");
    dummy = VariableManager::Instance().getVariable("testingthedummyvar2");
    EXPECT_TRUE(dummy != nullptr);
    EXPECT_TRUE(dummy->description == "something else");
    EXPECT_DOUBLE_EQ(dummy->function(nullptr), 42.0);

    //test list
    /*
    for (const VariableManager::Var* v : VariableManager::Instance().getVariables()) {
      B2WARNING(v->name);
    }
    */
    EXPECT_TRUE(VariableManager::Instance().getVariables().size() > 0);

    //special characters are not allowed!
    EXPECT_FATAL(VariableManager::Instance().registerVariable(" space", dummyVar, "blah"));
    EXPECT_FATAL(VariableManager::Instance().registerVariable("star*", dummyVar, "blah"));
    EXPECT_FATAL(VariableManager::Instance().registerVariable("*", dummyVar, "blah"));

    //this is ok, though
    VariableManager::Instance().registerVariable("abcdef0123945859432689_ZEFUEONHSUTNSXA", dummyVar, "blah");
  }

}  // namespace
