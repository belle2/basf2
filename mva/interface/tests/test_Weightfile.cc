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
#include <boost/filesystem/operations.hpp>

#include <TFile.h>

#include <fstream>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  class TestOptions : public MVA::Options {

  public:
    TestOptions(const std::string& _x, const std::string& _y) : x(_x), y(_y) { }
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

  TEST(WeightfileTest, FeatureImportance)
  {
    std::map<std::string, float> importance;
    importance["a"] = 1.0;
    importance["b"] = 2.0;
    importance["c"] = 3.0;
    MVA::Weightfile weightfile;
    weightfile.addFeatureImportance(importance);

    EXPECT_EQ(weightfile.getElement<unsigned int>("number_of_importance_vars"), 3);
    EXPECT_EQ(weightfile.getElement<std::string>("importance_key0"), "a");
    EXPECT_EQ(weightfile.getElement<float>("importance_value0"), 1.0);

    auto importance2 = weightfile.getFeatureImportance();

    EXPECT_EQ(importance2.size(), 3);
    EXPECT_EQ(importance2["a"], 1.0);
    EXPECT_EQ(importance2["b"], 2.0);
    EXPECT_EQ(importance2["c"], 3.0);

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

  TEST(WeightfileTest, StaticSaveLoadDatabase)
  {

    TestHelpers::TempDirCreator tmp_dir;
    LocalDatabase::createInstance("testPayloads/TestDatabase.txt");

    MVA::Weightfile weightfile;
    weightfile.addElement("Test", "a");

    MVA::Weightfile::save(weightfile, "MVAInterfaceTest");
    auto loaded = MVA::Weightfile::loadFromDatabase("MVAInterfaceTest");
    EXPECT_EQ(loaded.getElement<std::string>("Test"), "a");

    EXPECT_THROW(MVA::Weightfile::loadFromFile("MVAInterfaceTest"), std::runtime_error);

    loaded = MVA::Weightfile::load("MVAInterfaceTest");
    EXPECT_EQ(loaded.getElement<std::string>("Test"), "a");

    EXPECT_THROW(MVA::Weightfile::loadFromDatabase("DOES_NOT_EXIST"), std::runtime_error);

    boost::filesystem::remove_all("testPayloads");
    Database::reset();

  }

  TEST(WeightfileTest, StaticSaveLoadXML)
  {

    TestHelpers::TempDirCreator tmp_dir;

    MVA::Weightfile weightfile;
    weightfile.addElement("Test", "a");

    MVA::Weightfile::save(weightfile, "MVAInterfaceTest.xml");
    auto loaded = MVA::Weightfile::loadFromXMLFile("MVAInterfaceTest.xml");
    EXPECT_EQ(loaded.getElement<std::string>("Test"), "a");

    loaded = MVA::Weightfile::loadFromFile("MVAInterfaceTest.xml");
    EXPECT_EQ(loaded.getElement<std::string>("Test"), "a");

    loaded = MVA::Weightfile::load("MVAInterfaceTest.xml");
    EXPECT_EQ(loaded.getElement<std::string>("Test"), "a");

    EXPECT_THROW(MVA::Weightfile::loadFromXMLFile("DOES_NOT_EXIST.xml"), std::runtime_error);

  }

  TEST(WeightfileTest, StaticSaveLoadROOT)
  {

    TestHelpers::TempDirCreator tmp_dir;

    MVA::Weightfile weightfile;
    weightfile.addElement("Test", "a");

    MVA::Weightfile::save(weightfile, "MVAInterfaceTest.root");
    auto loaded = MVA::Weightfile::loadFromROOTFile("MVAInterfaceTest.root");
    EXPECT_EQ(loaded.getElement<std::string>("Test"), "a");

    loaded = MVA::Weightfile::loadFromFile("MVAInterfaceTest.root");
    EXPECT_EQ(loaded.getElement<std::string>("Test"), "a");

    loaded = MVA::Weightfile::load("MVAInterfaceTest.root");
    EXPECT_EQ(loaded.getElement<std::string>("Test"), "a");

    EXPECT_THROW(MVA::Weightfile::loadFromROOTFile("DOES_NOT_EXIST.root"), std::runtime_error);

    {
      std::fstream file("INVALID.root");
    }
    EXPECT_THROW(MVA::Weightfile::loadFromROOTFile("INVALID.root"), std::runtime_error);

  }

  TEST(WeightfileTest, StaticDatabase)
  {

    TestHelpers::TempDirCreator tmp_dir;
    LocalDatabase::createInstance("testPayloads/TestDatabase.txt");

    MVA::Weightfile weightfile;
    weightfile.addElement("Test", "a");

    MVA::Weightfile::saveToDatabase(weightfile, "MVAInterfaceTest");

    auto loaded = MVA::Weightfile::loadFromDatabase("MVAInterfaceTest");

    EXPECT_EQ(loaded.getElement<std::string>("Test"), "a");

    boost::filesystem::remove_all("testPayloads");
    Database::reset();

  }

  TEST(WeightfileTest, StaticDatabaseBadSymbols)
  {

    TestHelpers::TempDirCreator tmp_dir;
    LocalDatabase::createInstance("testPayloads/TestDatabase.txt");

    MVA::Weightfile weightfile;
    weightfile.addElement("Test", "a");

    std::string evilIdentifier = "==> *+:";
    MVA::Weightfile::saveToDatabase(weightfile, evilIdentifier);

    auto loaded = MVA::Weightfile::loadFromDatabase(evilIdentifier);

    EXPECT_EQ(loaded.getElement<std::string>("Test"), "a");

    boost::filesystem::remove_all("testPayloads");
    Database::reset();

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

    TFile file("invalid_weightfile.root", "RECREATE");
    file.Close();
    EXPECT_THROW(MVA::Weightfile::loadFromROOTFile("invalid_weightfile.root"), std::runtime_error);

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
    std::string filename = weightfile.generateFileName(".xml");
    unsigned int length = filename.size();
    EXPECT_TRUE(filename.substr(length - 4, length) == ".xml");

    {
      MVA::Weightfile weightfile2;
      weightfile2.setRemoveTemporaryDirectories(true);
      filename = weightfile2.generateFileName(".xml");
      {
        std::ofstream a(filename);
      }
      EXPECT_TRUE(boost::filesystem::exists(filename));
    }
    EXPECT_FALSE(boost::filesystem::exists(filename));

    {
      MVA::Weightfile weightfile2;
      weightfile2.setRemoveTemporaryDirectories(false);
      filename = weightfile2.generateFileName(".xml");
      {
        std::ofstream a(filename);
      }
      EXPECT_TRUE(boost::filesystem::exists(filename));
    }
    EXPECT_TRUE(boost::filesystem::exists(filename));
    boost::filesystem::remove_all(boost::filesystem::path(filename).parent_path());
    EXPECT_FALSE(boost::filesystem::exists(boost::filesystem::path(filename).parent_path()));

    char* directory_template = strdup("/tmp/Basf2Sub.XXXXXX");
    auto tempdir = std::string(mkdtemp(directory_template));
    {
      MVA::Weightfile weightfile2;
      weightfile2.setTemporaryDirectory(tempdir);
      filename = weightfile2.generateFileName(".xml");
      EXPECT_EQ(filename.substr(0, tempdir.size()), tempdir);
    }
    free(directory_template);
    boost::filesystem::remove_all(tempdir);
    EXPECT_FALSE(boost::filesystem::exists(tempdir));

  }

}
