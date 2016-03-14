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

    const Manager::Var* daughter1P = Manager::Instance().getVariable("daughter(1, p)");
    EXPECT_TRUE(daughter1P != nullptr);

    const Manager::Var* extraInfo = Manager::Instance().getVariable("extraInfo(signalProbability)");
    EXPECT_TRUE(extraInfo != nullptr);

    const Manager::Var* nested = Manager::Instance().getVariable("daughterSumOf(daughter(1, extraInfo(signalProbability)))");
    EXPECT_TRUE(nested != nullptr);

    const Manager::Var* funcDoesNotExists = Manager::Instance().getVariable("funcDoesNotExist(p)");
    EXPECT_TRUE(funcDoesNotExists == nullptr);

    const Manager::Var* nestedDoesNotExist =
      Manager::Instance().getVariable("daughterSumOf(daughter(1, ExtraInfoWrongName(signalProbability)))");
    EXPECT_TRUE(nestedDoesNotExist != nullptr); // TODO This should actually return nullptr, but this is not easy to implement.

    // Test alias
    const Manager::Var* aliasDoesNotExists = Manager::Instance().getVariable("myAlias");
    EXPECT_TRUE(aliasDoesNotExists == nullptr);
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

    std::unique_ptr<Cut> a = Cut::Compile("1.2 < 1.5 ");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile(" 1.5<1.2");
    EXPECT_FALSE(a->check(nullptr));

    a = Cut::Compile(" 12.3 >1.5 ");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("12 > 15");
    EXPECT_FALSE(a->check(nullptr));

    a = Cut::Compile("1.2 == 1.2");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile(" 1.5!=1.2");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile("1.2 == 1.2 == 1.2");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile(" 1.5 == 1.5!=1.2");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile(" 1.5 == 1.5!=1.5");
    EXPECT_FALSE(a->check(nullptr));

    a = Cut::Compile("1.0 < 1.2 == 1.2");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile(" 1.5 < 1.6 != 1.6");
    EXPECT_FALSE(a->check(nullptr));
    a = Cut::Compile(" 1.5 < 1.6 != 1.7");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile(" [12 >= 12 ]");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("[ 15>= 16 ]");
    EXPECT_FALSE(a->check(nullptr));

    a = Cut::Compile(" [12 <= 12 ]");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("  [ 17<= 16.7 ]");
    EXPECT_FALSE(a->check(nullptr));

    a = Cut::Compile(" [12 <= 12 < 13]");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("  [ 17<= 16.7 < 18 ]");
    EXPECT_FALSE(a->check(nullptr));

    a = Cut::Compile(" [12 >= 12 < 13]");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("  [ 15> 16.7 <= 18 ]");
    EXPECT_FALSE(a->check(nullptr));

    a = Cut::Compile("dummyvar > 1.0");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("1.0 < dummyvar <= dummyvar");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile("dummyvar < 100.0");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("dummyvar <= dummyvar <= dummyvar");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile("dummymetavar(123) < 100.0");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("dummymetavar(1) <= dummymetavar(1<) <= dummymetavar(1<3)");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile("1 < 2 and 3 < 4");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("1 < 2 and 4 < 3");
    EXPECT_FALSE(a->check(nullptr));
    a = Cut::Compile("2 < 1 and 4 < 3");
    EXPECT_FALSE(a->check(nullptr));
    a = Cut::Compile("2 < 1 and 3 < 4");
    EXPECT_FALSE(a->check(nullptr));

    a = Cut::Compile("1 < 2 or 3 < 4");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("1 < 2 or 4 < 3");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("2 < 1 or 4 < 3");
    EXPECT_FALSE(a->check(nullptr));
    a = Cut::Compile("2 < 1 or 3 < 4");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile("1 < 2 and 3 < 4 and [ 5 < 6 or 7 > 6 ]");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("[1 < 2 < 3 or 3 > 4 ] and [ 5 < 6 or 7 > 6 ]");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("[1 < 2 < 3 or 3 > 4 ] or [ 5 < 6 and 7 > 6 ]");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile("1 < 2 and 3 < 4 or 5 > 6");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("1 < 2 or 3 < 4 and 5 > 6");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("1 < 2 and 4 < 3 or 6 > 5");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("1 < 2 or 4 < 3 and 6 > 5");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile("1 != 2 and 3 < 4 or 5 > 6");
    EXPECT_TRUE(a->check(nullptr));
    a = Cut::Compile("1 < 2 or 3 != 4 and 5 > 6");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile("1 != 2 and 3 == 4 or 5 > 6");
    EXPECT_FALSE(a->check(nullptr));
    a = Cut::Compile("1 < 2 or 3 == 4 and 5 > 6");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile("15 != 0x15");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile("15 == 0xF");
    EXPECT_TRUE(a->check(nullptr));

    // Should throw an exception
    EXPECT_THROW(a = Cut::Compile("15 == 15.0 bla"), std::runtime_error);
    EXPECT_TRUE(a->check(nullptr));
    EXPECT_THROW(a = Cut::Compile("15 == 15e1000"), std::out_of_range);
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile("1e-3 < 1e3");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile("1e-3 == 0.001");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile("1000 < infinity");
    EXPECT_TRUE(a->check(nullptr));

    a = Cut::Compile("1000 > infinity");
    EXPECT_FALSE(a->check(nullptr));

    a = Cut::Compile("1000 < nan");
    EXPECT_FALSE(a->check(nullptr));

    a = Cut::Compile("1000 > nan");
    EXPECT_FALSE(a->check(nullptr));


  }

  TEST(VariableTest, makeROOTCompatible)
  {
    EXPECT_EQ("", makeROOTCompatible(""));
    EXPECT_EQ("test", makeROOTCompatible("test"));
    EXPECT_EQ("test213", makeROOTCompatible("test213"));
    EXPECT_EQ("test_23", makeROOTCompatible("test_23"));

    EXPECT_EQ("", makeROOTCompatible(""));
    EXPECT_EQ("test", makeROOTCompatible("test"));
    EXPECT_EQ("test213", makeROOTCompatible("test213"));
    EXPECT_EQ("test_23", makeROOTCompatible("test_23"));

    EXPECT_EQ("test__bo23__bc", makeROOTCompatible("test(23)"));
    EXPECT_EQ("test__so23__sc", makeROOTCompatible("test[23]"));
    EXPECT_EQ("test__co23__cc", makeROOTCompatible("test{23}"));
    EXPECT_EQ("test__boe__pl__clgamma__cm__spM__sp__st__sp3__bc", makeROOTCompatible("test(e+:gamma, M < 3)"));
    EXPECT_EQ("__cl__bo__bc__sp__bo__bc__cm", makeROOTCompatible(":() (),"));
    EXPECT_EQ("c__cl__bo__bcb__sp__bo__bc__cma", makeROOTCompatible("c:()b (),a"));
    EXPECT_EQ("c__cl__bo__cm__pl8951aBZWVZUEOH_Rhtnsq__bcb__sp__bo__bc__cma", makeROOTCompatible("c:(,+8951aBZWVZUEOH_Rhtnsq)b (),a"));
    EXPECT_EQ("foo__bobar__cm__mi0__pt123__cm__sp94__bc", makeROOTCompatible("foo(bar,-0.123, 94)"));

    for (auto pair : getSubstitutionMap()) {
      EXPECT_EQ(pair.second, makeROOTCompatible(pair.first));
    }

    EXPECT_NO_B2WARNING(makeROOTCompatible("test_a"));
    EXPECT_B2WARNING(makeROOTCompatible("test__a"));
  }

  TEST(VariableTest, invertMakeROOTCompatible)
  {
    EXPECT_EQ(invertMakeROOTCompatible(""), "");
    EXPECT_EQ(invertMakeROOTCompatible("test"), "test");
    EXPECT_EQ(invertMakeROOTCompatible("test213"), "test213");
    EXPECT_EQ(invertMakeROOTCompatible("test_23"), "test_23");

    EXPECT_EQ(invertMakeROOTCompatible(""), "");
    EXPECT_EQ(invertMakeROOTCompatible("test"), "test");
    EXPECT_EQ(invertMakeROOTCompatible("test213"), "test213");
    EXPECT_EQ(invertMakeROOTCompatible("test_23"), "test_23");

    EXPECT_EQ(invertMakeROOTCompatible("test__bo23__bc"), "test(23)");
    EXPECT_EQ(invertMakeROOTCompatible("test__so23__sc"), "test[23]");
    EXPECT_EQ(invertMakeROOTCompatible("test__co23__cc"), "test{23}");
    EXPECT_EQ(invertMakeROOTCompatible("test__boe__pl__clgamma__cm__spM__sp__st__sp3__bc"), "test(e+:gamma, M < 3)");
    EXPECT_EQ(invertMakeROOTCompatible("__cl__bo__bc__sp__bo__bc__cm"), ":() (),");
    EXPECT_EQ(invertMakeROOTCompatible("c__cl__bo__bcb__sp__bo__bc__cma"), "c:()b (),a");
    EXPECT_EQ(invertMakeROOTCompatible("c__cl__bo__cm__pl8951aBZWVZUEOH_Rhtnsq__bcb__sp__bo__bc__cma"),
              "c:(,+8951aBZWVZUEOH_Rhtnsq)b (),a");
    EXPECT_EQ(invertMakeROOTCompatible("foo__bobar__cm__mi0__pt123__cm__sp94__bc"), "foo(bar,-0.123, 94)");

    for (auto pair : getSubstitutionMap()) {
      EXPECT_EQ(pair.first, invertMakeROOTCompatible(pair.second));
    }
  }

}  // namespace
