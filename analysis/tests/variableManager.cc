#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/datastore/StoreArray.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Variable;
namespace {

  /** Return correct answer. */
  double dummyVar(const Particle*) { return 42.0; }

  /** test VariableManager. */
  TEST(VariableTest, Manager)
  {
    const Manager::Var* v = Manager::Instance().getVariable("THISDOESNTEXIST");
    EXPECT_TRUE(v == nullptr);

    //this exists
    const Manager::Var* pvar = Manager::Instance().getVariable("p");
    EXPECT_TRUE(pvar != nullptr);

    //test special variable operations
    const Manager::Var* daughterProductP = Manager::Instance().getVariable("daughterProductOf(p)");
    EXPECT_TRUE(daughterProductP != nullptr);

    const Manager::Var* daughterSumP = Manager::Instance().getVariable("daughterSumOf(p)");
    EXPECT_TRUE(daughterSumP != nullptr);

    const Manager::Var* daughter1P = Manager::Instance().getVariable("daughter1(p)");
    EXPECT_TRUE(daughter1P != nullptr);

    const Manager::Var* extraInfo = Manager::Instance().getVariable("getExtraInfo(signalProbability)");
    EXPECT_TRUE(extraInfo != nullptr);

    const Manager::Var* nested = Manager::Instance().getVariable("daughterSumOf(daughter1(getExtraInfo(signalProbability)))");
    EXPECT_TRUE(nested != nullptr);

    const Manager::Var* funcDoesNotExists = Manager::Instance().getVariable("funcDoesNotExist(p)");
    EXPECT_TRUE(funcDoesNotExists == nullptr);

    const Manager::Var* nestedDoesNotExist = Manager::Instance().getVariable("daughterSumOf(daughter1(ExtraInfoWrongName(signalProbability)))");
    EXPECT_TRUE(nestedDoesNotExist != nullptr);

    //re-registration not allowed
    EXPECT_B2FATAL(Manager::Instance().registerVariable("p", &dummyVar, "description"));

    EXPECT_B2FATAL(Manager::Instance().registerVariable("something", nullptr, "blah"));


    Manager::Instance().registerVariable("testingthedummyvar", &dummyVar, "blah");
    const Manager::Var* dummy = Manager::Instance().getVariable("testingthedummyvar");
    EXPECT_TRUE(dummy != nullptr);
    EXPECT_TRUE(dummy->description == "blah");
    EXPECT_DOUBLE_EQ(dummy->function(nullptr), 42.0);

    //also test the macro (with other name)
    REGISTER_VARIABLE("testingthedummyvar2", dummyVar, "something else");
    dummy = Manager::Instance().getVariable("testingthedummyvar2");
    EXPECT_TRUE(dummy != nullptr);
    EXPECT_TRUE(dummy->description == "something else");
    EXPECT_DOUBLE_EQ(dummy->function(nullptr), 42.0);

    //test list
    /*
    for (const Manager::Var* v : Manager::Instance().getVariables()) {
      B2WARNING(v->name);
    }
    */
    EXPECT_TRUE(Manager::Instance().getVariables().size() > 0);

    //special characters are not allowed!
    EXPECT_B2FATAL(Manager::Instance().registerVariable(" space", dummyVar, "blah"));
    EXPECT_B2FATAL(Manager::Instance().registerVariable("star*", dummyVar, "blah"));
    EXPECT_B2FATAL(Manager::Instance().registerVariable("*", dummyVar, "blah"));

    //this is ok, though
    Manager::Instance().registerVariable("abcdef0123945859432689_ZEFUEONHSUTNSXA", dummyVar, "blah");
  }

  TEST(VariableTest, Cut)
  {

    Manager::Instance().registerVariable("dummyvar", &dummyVar, "blah");

    Cut a("1.2 < 1.5 ");
    EXPECT_TRUE(a.check(nullptr));
    a.init(" 1.5<1.2");
    EXPECT_FALSE(a.check(nullptr));

    a.init(" 12.3 >1.5 ");
    EXPECT_TRUE(a.check(nullptr));
    a.init("12 > 15");
    EXPECT_FALSE(a.check(nullptr));

    a.init(" [12 >= 12 ]");
    EXPECT_TRUE(a.check(nullptr));
    a.init("[ 15>= 16 ]");
    EXPECT_FALSE(a.check(nullptr));

    a.init(" [12 <= 12 ]");
    EXPECT_TRUE(a.check(nullptr));
    a.init("  [ 17<= 16.7 ]");
    EXPECT_FALSE(a.check(nullptr));

    a.init(" [12 <= 12 < 13]");
    EXPECT_TRUE(a.check(nullptr));
    a.init("  [ 17<= 16.7 < 18 ]");
    EXPECT_FALSE(a.check(nullptr));

    a.init(" [12 >= 12 < 13]");
    EXPECT_TRUE(a.check(nullptr));
    a.init("  [ 15> 16.7 <= 18 ]");
    EXPECT_FALSE(a.check(nullptr));

    a.init("dummyvar > 1.0");
    EXPECT_TRUE(a.check(nullptr));
    a.init("1.0 < dummyvar <= dummyvar");
    EXPECT_TRUE(a.check(nullptr));

    a.init("dummyvar < 100.0");
    EXPECT_TRUE(a.check(nullptr));
    a.init("dummyvar <= dummyvar <= dummyvar");
    EXPECT_TRUE(a.check(nullptr));

    a.init("1 < 2 and 3 < 4");
    EXPECT_TRUE(a.check(nullptr));
    a.init("1 < 2 and 4 < 3");
    EXPECT_FALSE(a.check(nullptr));
    a.init("2 < 1 and 4 < 3");
    EXPECT_FALSE(a.check(nullptr));
    a.init("2 < 1 and 3 < 4");
    EXPECT_FALSE(a.check(nullptr));

    a.init("1 < 2 or 3 < 4");
    EXPECT_TRUE(a.check(nullptr));
    a.init("1 < 2 or 4 < 3");
    EXPECT_TRUE(a.check(nullptr));
    a.init("2 < 1 or 4 < 3");
    EXPECT_FALSE(a.check(nullptr));
    a.init("2 < 1 or 3 < 4");
    EXPECT_TRUE(a.check(nullptr));

    a.init("1 < 2 and 3 < 4 and [ 5 < 6 or 7 > 6 ]");
    EXPECT_TRUE(a.check(nullptr));
    a.init("[1 < 2 < 3 or 3 > 4 ] and [ 5 < 6 or 7 > 6 ]");
    EXPECT_TRUE(a.check(nullptr));

  }

}  // namespace
