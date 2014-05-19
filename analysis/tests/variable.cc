#include <analysis/utility/VariableManager.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/datastore/StoreArray.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {

  /** Return correct answer. */
  double dummyVar(const Particle*) { return 42.0; }

  /** test VariableManager. */
  TEST(VariableTest, VariableManager)
  {
    const VariableManager::Var* v = VariableManager::Instance().getVariable("THISDOESNTEXIST");
    EXPECT_TRUE(v == nullptr);

    //this exists
    const VariableManager::Var* pvar = VariableManager::Instance().getVariable("p");
    EXPECT_TRUE(pvar != nullptr);

    //test special variable operations
    const VariableManager::Var* daughterProductP = VariableManager::Instance().getVariable("daughterProductOf(p)");
    EXPECT_TRUE(daughterProductP != nullptr);

    const VariableManager::Var* daughterSumP = VariableManager::Instance().getVariable("daughterSumOf(p)");
    EXPECT_TRUE(daughterSumP != nullptr);

    const VariableManager::Var* daughter1P = VariableManager::Instance().getVariable("daughter1(p)");
    EXPECT_TRUE(daughter1P != nullptr);

    const VariableManager::Var* extraInfo = VariableManager::Instance().getVariable("ExtraInfo(signalProbability)");
    EXPECT_TRUE(extraInfo != nullptr);

    const VariableManager::Var* nested = VariableManager::Instance().getVariable("daughterSumOf(daughter1(ExtraInfo(signalProbability)))");
    EXPECT_TRUE(nested != nullptr);

    const VariableManager::Var* funcDoesNotExists = VariableManager::Instance().getVariable("funcDoesNotExist(p)");
    EXPECT_TRUE(funcDoesNotExists == nullptr);

    const VariableManager::Var* nestedDoesNotExist = VariableManager::Instance().getVariable("daughterSumOf(daughter1(ExtraInfoWrongName(signalProbability)))");
    EXPECT_TRUE(nestedDoesNotExist != nullptr);

    //re-registration not allowed
    EXPECT_B2FATAL(VariableManager::Instance().registerVariable("p", &dummyVar, "description"));

    EXPECT_B2FATAL(VariableManager::Instance().registerVariable("something", nullptr, "blah"));


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
    EXPECT_B2FATAL(VariableManager::Instance().registerVariable(" space", dummyVar, "blah"));
    EXPECT_B2FATAL(VariableManager::Instance().registerVariable("star*", dummyVar, "blah"));
    EXPECT_B2FATAL(VariableManager::Instance().registerVariable("*", dummyVar, "blah"));

    //this is ok, though
    VariableManager::Instance().registerVariable("abcdef0123945859432689_ZEFUEONHSUTNSXA", dummyVar, "blah");
  }

}  // namespace
