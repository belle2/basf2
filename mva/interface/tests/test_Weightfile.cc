/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/interface/Weightfile.h>
#include <mva/interface/Options.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>

#include <framework/database/LocalDatabase.h>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  class TestOptions : public MVA::Options {

  public:
    TestOptions(std::string x, std::string y) : x(x), y(y) { }
    virtual void load(const boost::property_tree::ptree& pt) override { y = pt.get<std::string>(x); }
    virtual void save(boost::property_tree::ptree& pt) const override { pt.put(x, y); }
    virtual po::options_description getDescription() override
    {
      po::options_description description("General options");
      description.add_options()
      ("help", "print this message");
      return description;
    }

    std::string x;
    std::string y;
  };

  TEST(WeightfileTest, Options)
  {
    TestOptions options1("Test1", "a");
    TestOptions options2("Test2", "b");
    MVA::Weightfile weightfile;
    weightfile.addOptions(options1);
    weightfile.addOptions(options2);

    EXPECT_EQ(weightfile.getElement<std::string>("Test1"), "a");
    EXPECT_EQ(weightfile.getElement<std::string>("Test2"), "b");

    TestOptions options3("Test2", "c");
    weightfile.getOptions(options3);
    EXPECT_EQ(options3.y, "b");
  }

  TEST(WeightfileTest, SignalFraction)
  {

    MVA::Weightfile weightfile;
    weightfile.addSignalFraction(0.7);
    EXPECT_FLOAT_EQ(weightfile.getSignalFraction(), 0.7);

  }

  TEST(WeightfileTest, Element)
  {

    MVA::Weightfile weightfile;
    weightfile.addElement("Test", 1);
    EXPECT_EQ(weightfile.getElement<int>("Test"), 1);

  }

  TEST(WeightfileTest, Stream)
  {

    MVA::Weightfile weightfile;
    std::stringstream sstream("MyStream");
    weightfile.addStream("Test", sstream);
    EXPECT_EQ(weightfile.getStream("Test"), "MyStream");

  }

  TEST(WeightfileTest, File)
  {

    TestHelpers::TempDirCreator tmp_dir;
    std::ofstream ofile("file.txt");
    ofile << "MyFile";
    ofile.close();

    MVA::Weightfile weightfile;
    weightfile.addFile("Test", "file.txt");

    weightfile.getFile("Test", "file2.txt");
    std::string content;
    std::ifstream ifile("file2.txt");
    ifile >> content;
    ifile.close();

    EXPECT_EQ(content, "MyFile");

  }

  TEST(WeightfileTest, StaticDatabase)
  {

    TestHelpers::TempDirCreator tmp_dir;
    /*StoreObjPtr<EventMetaData> evtPtr;
    DataStore::Instance().setInitializeActive(true);
    evtPtr.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);
    evtPtr.construct(0, 0, 1);*/
    LocalDatabase::createInstance("TestDatabase.txt");

    MVA::Weightfile weightfile;
    weightfile.addElement("Test", "a");

    MVA::Weightfile::saveToDatabase(weightfile, "MVAInterfaceTest");

    auto loaded = MVA::Weightfile::loadFromDatabase("MVAInterfaceTest");

    EXPECT_EQ(loaded.getElement<std::string>("Test"), "a");

  }

  TEST(WeightfileTest, StaticXMLFile)
  {

    TestHelpers::TempDirCreator tmp_dir;
    MVA::Weightfile weightfile;
    weightfile.addElement("Test", "a");

    MVA::Weightfile::saveToXMLFile(weightfile, "MVAInterfaceTest.xml");

    auto loaded = MVA::Weightfile::loadFromXMLFile("MVAInterfaceTest.xml");

    EXPECT_EQ(loaded.getElement<std::string>("Test"), "a");

  }

  TEST(WeightfileTest, StaticROOTFile)
  {

    TestHelpers::TempDirCreator tmp_dir;
    MVA::Weightfile weightfile;
    weightfile.addElement("Test", "a");

    MVA::Weightfile::saveToROOTFile(weightfile, "MVAInterfaceTest.root");

    auto loaded = MVA::Weightfile::loadFromROOTFile("MVAInterfaceTest.root");

    EXPECT_EQ(loaded.getElement<std::string>("Test"), "a");

  }

  TEST(WeightfileTest, StaticStream)
  {

    TestHelpers::TempDirCreator tmp_dir;
    MVA::Weightfile weightfile;
    weightfile.addElement("Test", "a");

    std::ofstream ofile("file.txt");
    MVA::Weightfile::saveToStream(weightfile, ofile);
    ofile.close();

    std::ifstream ifile("file.txt");
    auto loaded = MVA::Weightfile::loadFromStream(ifile);
    EXPECT_EQ(loaded.getElement<std::string>("Test"), "a");

  }

  TEST(WeightfileTest, GetFileName)
  {

    MVA::Weightfile weightfile;
    std::string filename = weightfile.getFileName(".xml");
    unsigned int length = filename.size();
    EXPECT_TRUE(filename.substr(length - 4, length) == ".xml");

  }

}
