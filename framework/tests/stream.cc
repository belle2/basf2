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
using namespace Belle2;

namespace {
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
    ASSERT_NE(rel, nullptr);
    EXPECT_TRUE(rel->getFromName() == "a");
    EXPECT_TRUE(rel->getToName() == "b");
    EXPECT_EQ(rel->getEntries(), 100);
    for (int i = 0; i < 100; i++) {
      EXPECT_EQ(rel->getElement(i).getToIndex(0), (unsigned int)(i + 1));
      EXPECT_DOUBLE_EQ(rel->getElement(i).getWeight(0), 42.0);
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

  /** Check raw conversion. */
  TEST(StreamTest, raw)
  {
    TVector3 v(1.0, 2.0, 3.0);
    std::string vStr = Stream::serializeAndEncode(&v);
    //B2INFO(vStr);

    //restore
    TObject* obj = Stream::deserializeEncodedRawData(vStr);
    TVector3* v2 = dynamic_cast<TVector3*>(obj);
    ASSERT_NE(v2, nullptr);
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
    // Note that the string length needs to be a multiple of 4 because
    // otherwise TBase64 will definitely access memory it should not as it just
    // assumes multiple of four and then performs an invalid read of a char
    // which it casts to unsigned int.
    std::string truncated = vStr.substr(0, 12);
    TObject* broken_obj = Stream::deserializeEncodedRawData(truncated);
    EXPECT_TRUE(broken_obj == NULL);
  }

  /** Read things from gearbox. */
  TEST(StreamTest, GearboxRaw)
  {
    Gearbox& gb = Gearbox::getInstance();
    vector<string> backends;
    backends.push_back("file:/framework/tests/");
    gb.setBackends(backends);
    gb.open("object_base64.xml");

    checkGbContents();
  }

}  // namespace
