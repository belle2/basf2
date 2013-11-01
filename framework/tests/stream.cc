#include <framework/utilities/Stream.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/RelationContainer.h>
#include <framework/dataobjects/RelationElement.h>
#include <framework/datastore/DataStore.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>

#include <TVector3.h>

#include <fstream>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  /** command x should exit using B2FATAL. */
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
    TObject* obj = Stream::deserialize(vStr);
    TVector3* v2 = dynamic_cast<TVector3*>(obj);
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
    const RelationContainer* relCont2 = dynamic_cast<const RelationContainer*>(obj);
    EXPECT_TRUE(relCont2 != NULL);
    EXPECT_EQ(relCont2->getEntries(), 1);
    EXPECT_EQ(relCont2->elements(0).getToIndex(0), 1u);
    EXPECT_DOUBLE_EQ(relCont2->elements(0).getWeight(0), 42.0);

    //creating file for next test..
    //std::ofstream file("serialized.xml");
    //file << Stream::escapeXML(relStr);


    //try converting something broken
    obj = Stream::deserialize("this is not actually XML!");
    EXPECT_FALSE(obj != NULL);
  }

  /** Read things from gearbox. */
  TEST_F(StreamTest, Gearbox)
  {
    Gearbox& gb = Gearbox::getInstance();
    vector<string> backends;
    backends.push_back("file:/framework/tests/");
    gb.setBackends(backends);
    gb.open("object.xml");

    const RelationContainer* relCont2 = dynamic_cast<const RelationContainer*>(gb.getTObject("/A/RelationContainer"));
    EXPECT_TRUE(relCont2 != NULL);
    EXPECT_EQ(relCont2->getEntries(), 1);
    EXPECT_EQ(relCont2->elements(0).getToIndex(0), 1u);
    EXPECT_DOUBLE_EQ(relCont2->elements(0).getWeight(0), 42.0);

    //Access from GearDir should work
    GearDir detector("/A/");
    relCont2 = dynamic_cast<const RelationContainer*>(detector.getTObject("RelationContainer"));
    EXPECT_TRUE(relCont2 != NULL);
    EXPECT_EQ(relCont2->getEntries(), 1);
    EXPECT_EQ(relCont2->elements(0).getToIndex(0), 1u);
    EXPECT_DOUBLE_EQ(relCont2->elements(0).getWeight(0), 42.0);

    //not an object, should throw error
    EXPECT_THROW(gb.getTObject("/A/Description"), gearbox::TObjectConversionError);
    //non-exstant path
    EXPECT_THROW(gb.getTObject("/A/NothingHere"), gearbox::PathEmptyError);
  }

}  // namespace
