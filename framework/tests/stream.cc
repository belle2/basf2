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

  /** create a more or less complex RelationContainer. */
  RelationContainer* createObject()
  {
    RelationContainer* relCont = new RelationContainer;
    relCont->setFromName("a");
    relCont->setToName("b");
    relCont->setFromDurability(DataStore::c_Event);
    relCont->setToDurability(DataStore::c_Event);
    TClonesArray& relations = relCont->elements();
    //let's make this a bit larger (very small objects are never compressed)
    for (int i = 0; i < 100; i++)
      new(relations.AddrAt(relations.GetLast() + 1)) RelationElement(0, i + 1, 42.0);

    return relCont;
  }

  /** common content checks for RelationContainer. */
  void checkObject(const RelationContainer* rel)
  {
    EXPECT_TRUE(rel != NULL);
    EXPECT_TRUE(rel->getFromName() == "a");
    EXPECT_TRUE(rel->getToName() == "b");
    EXPECT_EQ(rel->getEntries(), 100);
    for (int i = 0; i < 100; i++) {
      EXPECT_EQ(rel->elements(i).getToIndex(0), (unsigned int)(i + 1));
      EXPECT_DOUBLE_EQ(rel->elements(i).getWeight(0), 42.0);
    }
  }

  /** common content checks for data retrieved from Gearbox. */
  void checkGbContents()
  {
    Gearbox& gb = Gearbox::getInstance();

    checkObject(dynamic_cast<const RelationContainer*>(gb.getTObject("/A/RelationContainer")));

    //Access from GearDir should work
    GearDir detector("/A/");
    checkObject(dynamic_cast<const RelationContainer*>(detector.getTObject("RelationContainer")));
  }


  /** Check XML conversion. */
  TEST_F(StreamTest, XML)
  {
    TVector3 v(1.0, 2.0, 3.0);
    std::string vStr = Stream::serializeXML(&v);
    //B2INFO(vStr);

    //restore
    TObject* obj = Stream::deserializeXML(vStr);
    TVector3* v2 = dynamic_cast<TVector3*>(obj);
    EXPECT_TRUE(v2 != NULL);

    EXPECT_TRUE(*v2 == v);

    //something more complex
    RelationContainer* relCont = createObject();

    std::string relStr = Stream::serializeXML(relCont);
    //B2INFO(relStr);
    obj = Stream::deserializeXML(relStr);
    checkObject(dynamic_cast<const RelationContainer*>(obj));

    //creating file for next test..
    /*
    std::ofstream file("object.xml");
    file << Stream::escapeXML(relStr);
    */

    //try converting something broken
    obj = Stream::deserializeXML("this is not actually XML!");
    EXPECT_FALSE(obj != NULL);
  }

  /** Check raw conversion. */
  TEST_F(StreamTest, raw)
  {
    TVector3 v(1.0, 2.0, 3.0);
    std::string vStr = Stream::serializeAndEncode(&v);
    //B2INFO(vStr);

    //restore
    TObject* obj = Stream::deserializeEncodedRawData(vStr);
    TVector3* v2 = dynamic_cast<TVector3*>(obj);
    EXPECT_TRUE(v2 != NULL);
    EXPECT_TRUE(*v2 == v);

    //something more complex
    RelationContainer* relCont = createObject();

    std::string relStr = Stream::serializeAndEncode(relCont);
    //B2INFO(relStr);
    obj = Stream::deserializeEncodedRawData(relStr);
    checkObject(dynamic_cast<const RelationContainer*>(obj));

    //creating file for next test..
    /*
    std::ofstream file("object_base64.xml");
    file << Stream::escapeXML(relStr);
    */

    // Given random input, TBase64 will most likely just crash, this specific truncation seems ok.
    std::string truncated = vStr.substr(0, 10);
    TObject* broken_obj = Stream::deserializeEncodedRawData(truncated);
    EXPECT_TRUE(broken_obj == NULL);
  }
  /** Read things from gearbox. */
  TEST_F(StreamTest, GearboxXML)
  {
    Gearbox& gb = Gearbox::getInstance();
    vector<string> backends;
    backends.push_back("file:/framework/tests/");
    gb.setBackends(backends);
    gb.open("object.xml");

    checkGbContents();
  }

  /** Read things from gearbox. */
  TEST_F(StreamTest, GearboxRaw)
  {
    Gearbox& gb = Gearbox::getInstance();
    vector<string> backends;
    backends.push_back("file:/framework/tests/");
    gb.setBackends(backends);
    gb.open("object_base64.xml");

    checkGbContents();
  }

}  // namespace
