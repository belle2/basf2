#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/ConditionsDatabase.h>
#include <framework/database/DatabaseChain.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBArray.h>
#include <framework/database/EventDependency.h>
#include <framework/database/PayloadFile.h>
#include <framework/database/DBPointer.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/geometry/BFieldManager.h>
#include <framework/dbobjects/MagneticField.h>
#include <framework/dbobjects/MagneticFieldComponentConstant.h>

#include <TNamed.h>
#include <TClonesArray.h>

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>
#include <list>
#include <cstdio>

using namespace std;
using namespace Belle2;

namespace {

  /** Set up a a test database file */
  class DataBaseTest : public ::testing::Test {
  protected:

    /** Type of database to be tested */
    enum EDatabaseType {c_local, c_central, c_chain, c_default};

    /** Type of database to be tested */
    EDatabaseType m_dbType = c_local;

    TestHelpers::TempDirCreator m_tempDir; /**< ensure all tests are run inside a temporary directory. */

    /** Create a database with a TNamed object and an array of TObjects for experiment 1 to 5 each. */
    virtual void SetUp()
    {
      StoreObjPtr<EventMetaData> evtPtr;
      DataStore::Instance().setInitializeActive(true);
      evtPtr.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);
      evtPtr.construct(0, 0, 1);

      switch (m_dbType) {
        case c_local:
          LocalDatabase::createInstance("testPayloads/TestDatabase.txt");
          break;
        case c_central:
          ConditionsDatabase::createDefaultInstance("default");
          break;
        case c_chain:
          DatabaseChain::createInstance();
          LocalDatabase::createInstance("testPayloads/TestDatabase.txt");
          ConditionsDatabase::createDefaultInstance("default");
          break;
        case c_default:
          break;
      }

      if (m_dbType != c_central) {
        list<Database::DBImportQuery> query;
        TClonesArray array("TObject");
        for (int experiment = 1; experiment <= 5; experiment++) {
          IntervalOfValidity iov(experiment, -1, experiment, -1);

          TString name = "Experiment ";
          name += experiment;
          query.push_back(Database::DBImportQuery("TNamed", new TNamed(name, name), iov));

          new(array[experiment - 1]) TObject;
          array[experiment - 1]->SetUniqueID(experiment);
          Database::Instance().storeData("TObjects", &array, iov);

          FILE* f = fopen("file.xml", "w");
          fprintf(f, "Experiment %d\n", experiment);
          fclose(f);
          Database::Instance().addPayload("file.xml", "file.xml", iov);
        }
        if (m_dbType != c_chain) {
          Database::Instance().storeData(query);
        }

        EventDependency intraRunDep(new TNamed("A", "A"));
        intraRunDep.add(10, new TNamed("B", "B"));
        intraRunDep.add(50, new TNamed("C", "C"));
        IntervalOfValidity iov1(1, 1, 1, 1);
        Database::Instance().storeData("IntraRun", &intraRunDep, iov1);
        IntervalOfValidity iov2(1, 2, 1, -1);
        Database::Instance().storeData("IntraRun", new TNamed("X", "X"), iov2);
      }
    }

    /** clear datastore */
    virtual void TearDown()
    {
      if (m_dbType != c_central) boost::filesystem::remove_all("testPayloads");
      Database::reset();
      DataStore::Instance().reset();
    }

  };

  /** Test range check of validity intervals */
  TEST_F(DataBaseTest, IntervalOfValidity)
  {
    EXPECT_TRUE(IntervalOfValidity().empty());
    EXPECT_FALSE(IntervalOfValidity(1, -1, -1, -1).empty());
    EXPECT_FALSE(IntervalOfValidity(-1, -1, 1, -1).empty());
    EXPECT_FALSE(IntervalOfValidity(1, 2, 3, 4).empty());

    EventMetaData event(0, 8, 15);  // experiment 15, run 8
    EXPECT_FALSE(IntervalOfValidity().contains(event));
    EXPECT_FALSE(IntervalOfValidity(16, 1, -1, -1).contains(event));
    EXPECT_FALSE(IntervalOfValidity(16, -1, -1, -1).contains(event));
    EXPECT_FALSE(IntervalOfValidity(15, 9, -1, -1).contains(event));
    EXPECT_TRUE(IntervalOfValidity(15, 8, -1, -1).contains(event));
    EXPECT_TRUE(IntervalOfValidity(15, 8, 15, -1).contains(event));
    EXPECT_TRUE(IntervalOfValidity(15, 8, 15, 8).contains(event));
    EXPECT_TRUE(IntervalOfValidity(15, -1, 15, 8).contains(event));
    EXPECT_TRUE(IntervalOfValidity(-1, -1, 15, 8).contains(event));
    EXPECT_FALSE(IntervalOfValidity(-1, -1, 15, 7).contains(event));
    EXPECT_FALSE(IntervalOfValidity(-1, -1, 14, -1).contains(event));
    EXPECT_FALSE(IntervalOfValidity(-1, -1, 14, 1).contains(event));
  }

  /** Test operations of validity intervals */
  TEST_F(DataBaseTest, IntervalOfValidityOperations)
  {
    IntervalOfValidity iov(1, 1, 3, 10);
    EXPECT_TRUE(IntervalOfValidity(1, 1, 3, 10).contains(iov));
    EXPECT_TRUE(IntervalOfValidity(1, -1, 3, 10).contains(iov));
    EXPECT_TRUE(IntervalOfValidity(-1, -1, 3, 10).contains(iov));
    EXPECT_TRUE(IntervalOfValidity(1, 1, 3, -1).contains(iov));
    EXPECT_TRUE(IntervalOfValidity(1, 1, -1, -1).contains(iov));
    EXPECT_FALSE(IntervalOfValidity(1, 2, 3, 10).contains(iov));
    EXPECT_FALSE(IntervalOfValidity(2, -1, 3, 10).contains(iov));
    EXPECT_FALSE(IntervalOfValidity(1, 1, 3, 9).contains(iov));
    EXPECT_FALSE(IntervalOfValidity(1, 1, 2, -1).contains(iov));
    EXPECT_TRUE(iov.contains(IntervalOfValidity(2, -1, 2, -1)));
    EXPECT_FALSE(iov.contains(IntervalOfValidity(1, -1, 2, 1)));
    EXPECT_FALSE(iov.contains(IntervalOfValidity(-1, -1, 3, 10)));
    EXPECT_FALSE(iov.contains(IntervalOfValidity(1, 1, 3, -1)));
    EXPECT_FALSE(iov.contains(IntervalOfValidity(1, 1, -1, -1)));

    EXPECT_TRUE(IntervalOfValidity(1, 1, 3, 10).overlaps(iov));
    EXPECT_TRUE(IntervalOfValidity(1, -1, 1, 1).overlaps(iov));
    EXPECT_TRUE(IntervalOfValidity(-1, -1, 1, 1).overlaps(iov));
    EXPECT_TRUE(IntervalOfValidity(-1, -1, 1, -1).overlaps(iov));
    EXPECT_TRUE(IntervalOfValidity(3, -1, -1, -1).overlaps(iov));
    EXPECT_TRUE(IntervalOfValidity(3, 10, -1, -1).overlaps(iov));
    EXPECT_FALSE(IntervalOfValidity(-1, -1, -1, -1).overlaps(iov));
    EXPECT_FALSE(IntervalOfValidity(-1, -1, 0, -1).overlaps(iov));
    EXPECT_FALSE(IntervalOfValidity(-1, -1, 1, 0).overlaps(iov));
    EXPECT_FALSE(IntervalOfValidity(3, 11, 3, -1).overlaps(iov));
    EXPECT_FALSE(IntervalOfValidity(4, -1, 4, -1).overlaps(iov));

    IntervalOfValidity iov1, iov2;
    iov1 = IntervalOfValidity(1, 1, 3, 10);
    iov2 = IntervalOfValidity(1, 1, 3, 10);
    EXPECT_TRUE(iov1.trimOverlap(iov2, true));
    EXPECT_TRUE(iov1.empty());
    EXPECT_TRUE(iov2 == IntervalOfValidity(1, 1, 3, 10));

    iov1 = IntervalOfValidity(1, 1, 3, 10);
    iov2 = IntervalOfValidity(2, -1, 4, -1);
    EXPECT_TRUE(iov1.trimOverlap(iov2, true));
    EXPECT_TRUE(iov1 == IntervalOfValidity(1, 1, 1, -1));
    EXPECT_TRUE(iov2 == IntervalOfValidity(2, -1, 4, -1));

    iov1 = IntervalOfValidity(1, 1, 3, 11);
    iov2 = IntervalOfValidity(1, 1, 3, 10);
    EXPECT_FALSE(iov1.trimOverlap(iov2, true));
    EXPECT_TRUE(iov1 == IntervalOfValidity(1, 1, 3, 11));
    EXPECT_TRUE(iov2 == IntervalOfValidity(1, 1, 3, 10));

    iov1 = IntervalOfValidity(1, 1, 3, 10);
    iov2 = IntervalOfValidity(1, 1, 3, 10);
    EXPECT_TRUE(iov1.trimOverlap(iov2, false));
    EXPECT_TRUE(iov1 == IntervalOfValidity(1, 1, 3, 10));
    EXPECT_TRUE(iov2.empty());

    iov1 = IntervalOfValidity(1, 1, 3, 10);
    iov2 = IntervalOfValidity(2, -1, 4, -1);
    EXPECT_TRUE(iov1.trimOverlap(iov2, false));
    EXPECT_TRUE(iov1 == IntervalOfValidity(1, 1, 3, 10));
    EXPECT_TRUE(iov2 == IntervalOfValidity(3, 11, 4, -1));
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
    DBArray<TObject> missing("notexisting");
    // check iteration on fresh object
    {
      int i = 0;
      for (auto o : missing) {
        (void)o;
        ++i;
      }
      EXPECT_EQ(i, 0);
    }


    evtPtr->setExperiment(1);
    DBStore::Instance().update();
    EXPECT_TRUE(objects);
    EXPECT_FALSE(missing);
    EXPECT_EQ(objects.getEntries(), 1);
    EXPECT_EQ(objects[0]->GetUniqueID(), 1);
    evtPtr->setExperiment(4);
    EXPECT_EQ(objects.getEntries(), 1);
    DBStore::Instance().update();
    EXPECT_EQ(objects.getEntries(), 4);
    // check iteration on existing
    {
      int i = 0;
      for (auto o : objects) {
        EXPECT_EQ(objects[i]->GetUniqueID(), i + 1);
        ++i;
      }
      EXPECT_EQ(i, 4);
    }

    // check iteration on missing object
    {
      int i = 0;
      for (auto o : missing) {
        (void)o;
        ++i;
      }
      EXPECT_EQ(i, 0);
    }
    evtPtr->setExperiment(7);
    DBStore::Instance().update();
    EXPECT_FALSE(objects);
    // check iteration over missing but previously existing
    {
      int i = 0;
      for (auto o : objects) {
        EXPECT_EQ(objects[i]->GetUniqueID(), i + 1);
        ++i;
      }
      EXPECT_EQ(i, 0);
    }
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

  /** Test intra-run dependency */
  TEST_F(DataBaseTest, IntraRun)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    DBObjPtr<TNamed> intraRun("IntraRun");

    evtPtr->setExperiment(1);
    evtPtr->setRun(1);
    evtPtr->setEvent(9);
    DBStore::Instance().update();
    DBStore::Instance().updateEvent();
    EXPECT_TRUE(strcmp(intraRun->GetName(), "A") == 0);

    evtPtr->setEvent(10);
    DBStore::Instance().updateEvent();
    EXPECT_TRUE(strcmp(intraRun->GetName(), "B") == 0);

    evtPtr->setEvent(49);
    DBStore::Instance().updateEvent();
    EXPECT_TRUE(strcmp(intraRun->GetName(), "B") == 0);

    //let's run an update in between and make sure intra run continues to work
    DBStore::Instance().update();
    EXPECT_TRUE(strcmp(intraRun->GetName(), "B") == 0);

    evtPtr->setEvent(50);
    DBStore::Instance().updateEvent();
    EXPECT_TRUE(strcmp(intraRun->GetName(), "C") == 0);

    evtPtr->setRun(2);
    DBStore::Instance().update();
    EXPECT_TRUE(strcmp(intraRun->GetName(), "X") == 0);
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

    evtPtr->setExperiment(1);
    evtPtr->setRun(1);
    evtPtr->setEvent(1);
    DBObjPtr<TNamed> intraRun("IntraRun");
    DBStore::Instance().update();
    DBStore::Instance().updateEvent();
    EXPECT_TRUE(intraRun.hasChanged());

    evtPtr->setEvent(2);
    DBStore::Instance().updateEvent();
    EXPECT_FALSE(intraRun.hasChanged());

    evtPtr->setEvent(10);
    DBStore::Instance().updateEvent();
    EXPECT_TRUE(intraRun.hasChanged());

    evtPtr->setEvent(1000);
    DBStore::Instance().updateEvent();
    EXPECT_TRUE(intraRun.hasChanged());

    evtPtr->setRun(4);
    DBStore::Instance().update();
    DBStore::Instance().updateEvent();
    EXPECT_TRUE(intraRun.hasChanged());
  }

  /** Test database access to payload files */
  TEST_F(DataBaseTest, PayloadFile)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    DBObjPtr<PayloadFile> payload("file.xml");

    evtPtr->setExperiment(1);
    DBStore::Instance().update();
    EXPECT_EQ(payload->getContent(), "Experiment 1\n") << payload->getFileName();
    evtPtr->setExperiment(4);
    DBStore::Instance().update();
    EXPECT_EQ(payload->getContent(), "Experiment 4\n") << payload->getFileName();
    evtPtr->setExperiment(7);
    DBStore::Instance().update();
    EXPECT_FALSE(payload);
  }

  //disable (wrong) warnings about unused functions
#if defined(__INTEL_COMPILER)
#pragma warning disable 177
#endif

  /** Test callbacks */
  int callbackCounter = 0;

  void callback()
  {
    callbackCounter++;
  }

  class Callback {
  public:
    void callback()
    {
      callbackCounter++;
    }
  };
  Callback callbackObject;

  TEST_F(DataBaseTest, Callbacks)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    evtPtr->setRun(1);
    evtPtr->setEvent(1);
    callbackCounter = 0;

    DBObjPtr<TNamed> named;
    named.addCallback(&callback);
    EXPECT_EQ(callbackCounter, 0);

    evtPtr->setExperiment(2);
    DBStore::Instance().update();
    EXPECT_EQ(callbackCounter, 1);

    evtPtr->setExperiment(4);
    DBStore::Instance().update();
    EXPECT_EQ(callbackCounter, 2);
    DBStore::Instance().update();
    EXPECT_EQ(callbackCounter, 2);

    evtPtr->setExperiment(6);
    DBStore::Instance().update();
    EXPECT_EQ(callbackCounter, 3);

    evtPtr->setExperiment(7);
    DBStore::Instance().update();
    EXPECT_EQ(callbackCounter, 3);

    DBArray<TObject> objects;
    objects.addCallback(&callback);

    evtPtr->setExperiment(1);
    DBStore::Instance().update();
    // callbacks will now be called once for each payload so there is an extra call
    EXPECT_EQ(callbackCounter, 5);

    DBObjPtr<TNamed> intraRun("IntraRun");
    intraRun.addCallback(&callbackObject, &Callback::callback);

    //There won't be any callback here because the correct object is already set on creation
    evtPtr->setEvent(1);
    DBStore::Instance().updateEvent();
    EXPECT_EQ(callbackCounter, 5);

    evtPtr->setEvent(2);
    DBStore::Instance().updateEvent();
    EXPECT_EQ(callbackCounter, 5);

    evtPtr->setEvent(10);
    DBStore::Instance().updateEvent();
    EXPECT_EQ(callbackCounter, 6);

    evtPtr->setEvent(1);
    DBStore::Instance().updateEvent();
    EXPECT_EQ(callbackCounter, 7);
  }

  /** Test the access to arrays by key */
  TEST_F(DataBaseTest, KeyAccess)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    evtPtr->setExperiment(1);
    DBStore::Instance().update();

    DBArray<TObject> objects;
    EXPECT_EQ(objects.getByKey<unsigned int>(&TObject::GetUniqueID, 1)->GetUniqueID(), 1);
    EXPECT_EQ(objects.getByKey<unsigned int>(&TObject::GetUniqueID, 2), nullptr);
    EXPECT_EQ(objects.getByKey(&TObject::IsFolder, false)->GetUniqueID(), 1);
    EXPECT_EQ(objects.getByKey(&TObject::IsFolder, true), nullptr);

    evtPtr->setExperiment(2);
    DBStore::Instance().update();

    EXPECT_EQ(objects.getByKey<unsigned int>(&TObject::GetUniqueID, 1)->GetUniqueID(), 1);
    EXPECT_EQ(objects.getByKey<unsigned int>(&TObject::GetUniqueID, 2)->GetUniqueID(), 2);
  }

  /** Test the access to arrays by key */
  TEST_F(DataBaseTest, DBPointer)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    evtPtr->setExperiment(2);
    DBStore::Instance().update();

    DBPointer<TObject, unsigned int, &TObject::GetUniqueID> ptr(1);
    EXPECT_EQ(ptr.key(), 1);
    EXPECT_TRUE(ptr.isValid());
    EXPECT_EQ(ptr->GetUniqueID(), 1);
    ptr = 2;
    EXPECT_EQ(ptr->GetUniqueID(), 2);
    ptr = 3;
    EXPECT_FALSE(ptr.isValid());
  }

  TEST_F(DataBaseTest, CleanupReattachDeathtest)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    evtPtr->setExperiment(2);
    DBStore::Instance().update();

    DBObjPtr<TNamed> named;
    EXPECT_TRUE(named.isValid());
    double Bz = BFieldManager::getFieldInTesla({0, 0, 0}).Z();
    EXPECT_EQ(Bz, 1.5);
    DBStore::Instance().reset();
    //Ok, on next access the DBObjPtr should try to reconnect
    EXPECT_TRUE(named.isValid());
    //Which means the magnetic field should die a horrible death: It's not
    //found in the database during testing, just a override.
    EXPECT_B2FATAL(BFieldManager::getFieldInTesla({0, 0, 0}));
    //Unless we add a new one
    Belle2::MagneticField* field = new Belle2::MagneticField();
    field->addComponent(new Belle2::MagneticFieldComponentConstant({0, 0, 1.5 * Belle2::Unit::T}));
    Belle2::DBStore::Instance().addConstantOverride("MagneticField", field, false);
    //So now it should work again
    Bz = BFieldManager::getFieldInTesla({0, 0, 0}).Z();
    EXPECT_EQ(Bz, 1.5);
  }
}  // namespace
