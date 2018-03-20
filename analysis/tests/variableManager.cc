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
  double dummyVarWithParameters(const Particle*, const std::vector<double>& parameters)
  {
    double result = 0;
    for (auto& x : parameters)
      result += x;
    return result;
  }
  Manager::FunctionPtr dummyMetaVar(const std::vector<std::string>& arguments)
  {
    std::string arg = arguments[0];
    auto func = [arg](const Particle*) -> double {
      return arg.size();
    };
    return func;
  }

  /** test VariableManager. */
  TEST(VariableTest, ManagerDeathTest)
  {

    EXPECT_B2FATAL(Manager::Instance().getVariable("THISDOESNTEXIST"));

    //this exists
    const Manager::Var* pvar = Manager::Instance().getVariable("p");
    EXPECT_TRUE(pvar != nullptr);

    //test special variable operations
    const Manager::Var* daughterProductP = Manager::Instance().getVariable("daughterProductOf(p)");
    EXPECT_TRUE(daughterProductP != nullptr);

    const Manager::Var* daughterSumP = Manager::Instance().getVariable("daughterSumOf(p)");
    EXPECT_TRUE(daughterSumP != nullptr);

    const Manager::Var* daughter1P = Manager::Instance().getVariable("daughter(1, p)");
    EXPECT_TRUE(daughter1P != nullptr);

    const Manager::Var* extraInfo = Manager::Instance().getVariable("extraInfo(signalProbability)");
    EXPECT_TRUE(extraInfo != nullptr);

    const Manager::Var* nested = Manager::Instance().getVariable("daughterSumOf(daughter(1, extraInfo(signalProbability)))");
    EXPECT_TRUE(nested != nullptr);

    EXPECT_B2FATAL(Manager::Instance().getVariable("funcDoesNotExist(p)"));

    EXPECT_B2FATAL(Manager::Instance().getVariable("daughterSumOf(daughter(1, ExtraInfoWrongName(signalProbability)))"));

    // Test collection
    auto vec = Manager::Instance().resolveCollections({"myCollection"});
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], "myCollection");
    EXPECT_TRUE(Manager::Instance().addCollection("myCollection", {"p", "px", "py", "pz"}));

    auto vec2 = Manager::Instance().resolveCollections({"myCollection"});
    EXPECT_EQ(vec2.size(), 4);
    EXPECT_EQ(vec2[0], "p");
    EXPECT_EQ(vec2[1], "px");
    EXPECT_EQ(vec2[2], "py");
    EXPECT_EQ(vec2[3], "pz");

    // Test alias
    EXPECT_B2FATAL(Manager::Instance().getVariable("myAlias"));
    Manager::Instance().addAlias("myAlias", "daughterSumOf(daughter(1, extraInfo(signalProbability)))");
    const Manager::Var* aliasDoesExists = Manager::Instance().getVariable("myAlias");
    EXPECT_TRUE(aliasDoesExists != nullptr);

    EXPECT_B2WARNING(Manager::Instance().addAlias("myAlias", "daughterSumOf(daughter(1, extraInfo(signalProbability)))"));
    EXPECT_B2ERROR(Manager::Instance().addAlias("M", "daughterSumOf(daughter(1, extraInfo(signalProbability)))"));

    //re-registration not allowed
    EXPECT_B2FATAL(Manager::Instance().registerVariable("p", (Manager::FunctionPtr)&dummyVar, "description"));

    EXPECT_B2FATAL(Manager::Instance().registerVariable("something", (Manager::FunctionPtr)nullptr, "blah"));


    Manager::Instance().registerVariable("testingthedummyvar", (Manager::FunctionPtr)&dummyVar, "blah");
    const Manager::Var* dummy = Manager::Instance().getVariable("testingthedummyvar");
    ASSERT_NE(dummy, nullptr);
    EXPECT_TRUE(dummy->description == "blah");
    EXPECT_DOUBLE_EQ(dummy->function(nullptr), 42.0);

    //also test the macro (with other name)
    REGISTER_VARIABLE("testingthedummyvar2", dummyVar, "something else");
    dummy = Manager::Instance().getVariable("testingthedummyvar2");
    ASSERT_NE(dummy, nullptr);
    EXPECT_TRUE(dummy->description == "something else");
    EXPECT_DOUBLE_EQ(dummy->function(nullptr), 42.0);


    Manager::Instance().registerVariable("testingthedummyvarwithparameters(n)", (Manager::ParameterFunctionPtr)&dummyVarWithParameters,
                                         "blah");
    dummy = Manager::Instance().getVariable("testingthedummyvarwithparameters(3)");
    ASSERT_NE(dummy, nullptr);
    EXPECT_DOUBLE_EQ(dummy->function(nullptr), 3.0);
    EXPECT_DOUBLE_EQ(Manager::Instance().getVariable("testingthedummyvarwithparameters(3)")->function(nullptr), 3.0);
    EXPECT_DOUBLE_EQ(Manager::Instance().getVariable("testingthedummyvarwithparameters(3,5)")->function(nullptr), 8.0);
    EXPECT_DOUBLE_EQ(Manager::Instance().getVariable("testingthedummyvarwithparameters(3,7,8)")->function(nullptr), 18.0);

    Manager::Instance().registerVariable("testingthedummymetavar(cut)", (Manager::MetaFunctionPtr)&dummyMetaVar,
                                         "blah");
    dummy = Manager::Instance().getVariable("testingthedummymetavar(1 < 2)");
    ASSERT_NE(dummy, nullptr);
    EXPECT_DOUBLE_EQ(dummy->function(nullptr), 5.0);
    EXPECT_DOUBLE_EQ(Manager::Instance().getVariable("testingthedummymetavar(123)")->function(nullptr), 3.0);


    //also test the macro (with other name)
    REGISTER_VARIABLE("testingthedummyvarwithparameters2(n,m)", dummyVarWithParameters, "something else");
    dummy = Manager::Instance().getVariable("testingthedummyvarwithparameters2(4,5)");
    ASSERT_NE(dummy, nullptr);
    EXPECT_DOUBLE_EQ(dummy->function(nullptr), 9.0);

    //test list
    /*
    for (const Manager::Var* v : Manager::Instance().getVariables()) {
      B2WARNING(v->name);
    }
    */
    EXPECT_TRUE(Manager::Instance().getVariables().size() > 0);

    //special characters are not allowed!
    EXPECT_B2FATAL(Manager::Instance().registerVariable(" space", (Manager::FunctionPtr)dummyVar, "blah"));
    EXPECT_B2FATAL(Manager::Instance().registerVariable("star*", (Manager::FunctionPtr)dummyVar, "blah"));
    EXPECT_B2FATAL(Manager::Instance().registerVariable("*", (Manager::FunctionPtr)dummyVar, "blah"));

    //this is ok, though
    Manager::Instance().registerVariable("abcdef0123945859432689_ZEFUEONHSUTNSXA", (Manager::FunctionPtr)dummyVar, "blah");
  }

  TEST(VariableTest, Cut)
  {
    Manager::Instance().registerVariable("dummyvar", (Manager::FunctionPtr)&dummyVar, "blah");
    Manager::Instance().registerVariable("dummymetavar(cut)", (Manager::MetaFunctionPtr)&dummyMetaVar, "blah");

    std::unique_ptr<Cut> a = Cut::compile("dummyvar > 1.0");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::compile("1.0 < dummyvar <= dummyvar");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::compile("dummyvar < 100.0");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::compile("dummyvar <= dummyvar <= dummyvar");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::compile("dummymetavar(123) < 100.0");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::compile("dummymetavar(1) <= dummymetavar(1<) <= dummymetavar(1<3)");
    EXPECT_TRUE(a->check(nullptr));
  }


}  // namespace
