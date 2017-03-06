#include <framework/pcore/MsgHandler.h>

#include <TVector3.h>
#include <TNamed.h>

#include <fstream>
#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  TEST(MsgHandlerTest, conversion)
  {
    MsgHandler handler;
    TVector3 a(1.0, 2.0, 3.0);
    handler.add(&a, "vector");
    TVector3 b(42.0, 7.0, -20.0);
    handler.add(&b, "vectorb");
    TNamed c("abc", "def");
    handler.add(&c, "named");
    EvtMessage* msg = handler.encode_msg(MSG_EVENT);

    {
      MsgHandler handler2;
      vector<TObject*> objs;
      vector<string> names;
      handler2.decode_msg(msg, objs, names);
      ASSERT_EQ(3, objs.size());
      ASSERT_EQ(3, names.size());
      ASSERT_EQ("vector", names[0]);
      ASSERT_EQ("vectorb", names[1]);
      ASSERT_EQ("named", names[2]);

      auto a2 = dynamic_cast<TVector3*>(objs[0]);
      ASSERT_NE(a2, nullptr);
      EXPECT_TRUE(*a2 == a);

      auto b2 = dynamic_cast<TVector3*>(objs[1]);
      ASSERT_NE(b2, nullptr);
      EXPECT_TRUE(*b2 == b);

      auto c2 = dynamic_cast<TNamed*>(objs[2]);
      ASSERT_NE(c2, nullptr);
      EXPECT_TRUE(std::string(c2->GetName()) == c.GetName());
      EXPECT_TRUE(std::string(c2->GetTitle()) == c.GetTitle());
    }
    delete msg;
  }
}  // namespace
