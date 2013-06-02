#include <framework/utilities/Stream.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/RelationContainer.h>
#include <framework/dataobjects/RelationElement.h>
#include <framework/datastore/DataStore.h>

#include <TVector3.h>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Test (de)serialization routines. */
  class StreamTest : public ::testing::Test {
  protected:

  };

  /** Check XML conversion. */
  TEST_F(StreamTest, XML)
  {
    TVector3 v(1.0, 2.0, 3.0);
    std::string vStr = Stream::serialize(&v);
    //B2INFO(vStr);

    //restore
    TObjectPtr obj = Stream::deserialize(vStr);
    TVector3* v2 = dynamic_cast<TVector3*>(obj.get());
    EXPECT_TRUE(v2 != NULL);

    EXPECT_TRUE(*v2 == v);

    //something more complex
    RelationContainer* relCont = new RelationContainer;
    relCont->setFromName("a");
    relCont->setToName("b");
    relCont->setFromDurability(DataStore::c_Event);
    relCont->setToDurability(DataStore::c_Event);
    TClonesArray& relations = relCont->elements();
    new(relations.AddrAt(relations.GetLast() + 1)) RelationElement(0, 1, 42.0);

    std::string relStr = Stream::serialize(relCont);
    //B2INFO(relStr);
    obj = Stream::deserialize(relStr);
    relCont = dynamic_cast<RelationContainer*>(obj.get());
    EXPECT_TRUE(relCont != NULL);
    EXPECT_EQ(relCont->getEntries(), 1);
    EXPECT_EQ(relCont->elements(0).getToIndex(0), 1);
    EXPECT_DOUBLE_EQ(relCont->elements(0).getWeight(0), 42.0);


    //try converting something broken
    obj = Stream::deserialize("this is not actually XML!");
    EXPECT_FALSE(obj.get() != NULL);
  }

}  // namespace
