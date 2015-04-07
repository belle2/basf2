#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/utilities/TestHelpers.h>

#include <TNamed.h>
#include <TClonesArray.h>

#include <gtest/gtest.h>

#include <list>
#include <cstdio>

using namespace std;
using namespace Belle2;

namespace {

  /** Set up a a test database file */
  class DataBaseTest : public ::testing::Test {
  protected:

    /** Create a database with a TNamed object and an array of TObjects for experiment 1 to 5 each. */
    virtual void SetUp()
    {
      StoreObjPtr<EventMetaData> evtPtr;
      DataStore::Instance().setInitializeActive(true);
      evtPtr.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);
      evtPtr.construct(0, 1, 1);

      list<Database::DBQuery> query;
      Database::setGlobalTag("TestDatabase.root");
      TClonesArray array("TObject");
      for (int experiment = 1; experiment <= 5; experiment++) {
        IntervalOfValidity iov(experiment, 0, experiment, 0);

        TString name = "Experiment ";
        name += experiment;
        query.push_back(Database::DBQuery("TNamed", new TNamed(name, name), iov));

        new(array[experiment - 1]) TObject;
        array[experiment - 1]->SetUniqueID(experiment);
        Database::Instance().storeData("TObjects", &array, iov);
      }
      Database::Instance().storeData(query);
    }

    /** clear datastore */
    virtual void TearDown()
    {
      remove("TestDatabase.root");
    }

  };

  /** Test range check of validity intervals */
  TEST_F(DataBaseTest, IntervalOfValidity)
  {
    EXPECT_TRUE(IntervalOfValidity().empty());
    EXPECT_FALSE(IntervalOfValidity(1, 0, 0, 0).empty());
    EXPECT_FALSE(IntervalOfValidity(0, 0, 1, 0).empty());
    EXPECT_FALSE(IntervalOfValidity(1, 2, 3, 4).empty());

    EventMetaData event(0, 8, 15);  // experiment 15, run 8
    EXPECT_FALSE(IntervalOfValidity().contains(event));
    EXPECT_FALSE(IntervalOfValidity(16, 1, 0, 0).contains(event));
    EXPECT_FALSE(IntervalOfValidity(16, 0, 0, 0).contains(event));
    EXPECT_FALSE(IntervalOfValidity(15, 9, 0, 0).contains(event));
    EXPECT_TRUE(IntervalOfValidity(15, 8, 0, 0).contains(event));
    EXPECT_TRUE(IntervalOfValidity(15, 8, 15, 0).contains(event));
    EXPECT_TRUE(IntervalOfValidity(15, 8, 15, 8).contains(event));
    EXPECT_TRUE(IntervalOfValidity(15, 0, 15, 8).contains(event));
    EXPECT_TRUE(IntervalOfValidity(0, 0, 15, 8).contains(event));
    EXPECT_FALSE(IntervalOfValidity(0, 0, 15, 7).contains(event));
    EXPECT_FALSE(IntervalOfValidity(0, 0, 14, 0).contains(event));
    EXPECT_FALSE(IntervalOfValidity(0, 0, 14, 1).contains(event));
  }

  /** Test database access via DBObjPtr */
  TEST_F(DataBaseTest, DBObjPtr)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    DBObjPtr<TNamed> named;

    evtPtr->setExperiment(1);
    DBStore::Instance().update();
    EXPECT_TRUE(named);
    EXPECT_TRUE(strcmp(named->GetName(), "Experiment 1") == 0);
    evtPtr->setExperiment(4);
    EXPECT_TRUE(strcmp(named->GetName(), "Experiment 1") == 0);
    DBStore::Instance().update();
    EXPECT_TRUE(strcmp(named->GetName(), "Experiment 4") == 0);
    evtPtr->setExperiment(7);
    DBStore::Instance().update();
    EXPECT_FALSE(named);
  }

  /** Test database access via DBArray */
  TEST_F(DataBaseTest, DBArray)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    DBArray<TObject> objects;

    evtPtr->setExperiment(1);
    DBStore::Instance().update();
    EXPECT_TRUE(objects);
    EXPECT_EQ(objects.getEntries(), 1);
    EXPECT_EQ(objects[0]->GetUniqueID(), 1);
    evtPtr->setExperiment(4);
    EXPECT_EQ(objects.getEntries(), 1);
    DBStore::Instance().update();
    EXPECT_EQ(objects.getEntries(), 4);
    EXPECT_EQ(objects[0]->GetUniqueID(), 1);
    EXPECT_EQ(objects[3]->GetUniqueID(), 4);
    evtPtr->setExperiment(7);
    DBStore::Instance().update();
    EXPECT_FALSE(objects);
  }

  /** Test range checks of DBArray */
  TEST_F(DataBaseTest, DBArrayRange)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    DBArray<TObject> objects;

    evtPtr->setExperiment(3);
    DBStore::Instance().update();
    EXPECT_THROW(objects[-1], std::out_of_range);
    EXPECT_THROW(objects[3], std::out_of_range);
  }

  /** Test type check of DBObjPtr and DBArray */
  TEST_F(DataBaseTest, TypeCheck)
  {
    DBObjPtr<TNamed> named;
    EXPECT_B2FATAL(DBObjPtr<EventMetaData> wrongType("TNamed"));

    DBArray<TObject> objects;
    EXPECT_B2FATAL(DBArray<EventMetaData> wrongType("TObjects"));
  }

  /** Test the database content change notification */
  TEST_F(DataBaseTest, HasChanged)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    evtPtr->setExperiment(0);
    DBStore::Instance().update();

    DBObjPtr<TNamed> named;
    EXPECT_FALSE(named.hasChanged());

    evtPtr->setExperiment(1);
    EXPECT_FALSE(named.hasChanged());

    DBStore::Instance().update();
    EXPECT_TRUE(named.hasChanged());

    evtPtr->setRun(8);
    DBStore::Instance().update();
    EXPECT_FALSE(named.hasChanged());

    evtPtr->setExperiment(5);
    DBStore::Instance().update();
    EXPECT_TRUE(named.hasChanged());

    evtPtr->setExperiment(7);
    DBStore::Instance().update();
    EXPECT_TRUE(named.hasChanged());
  }

}  // namespace
