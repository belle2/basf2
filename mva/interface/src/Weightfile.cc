/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mva/interface/Weightfile.h>

#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <framework/database/Database.h>
#include <framework/database/DBImportArray.h>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>

#include <TFile.h>

#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace Belle2 {
  namespace MVA {

    std::string makeSaveForDatabase(std::string str)
    {
      std::map<std::string, std::string> replace {
        {" ", "__sp"},
      };
      for (auto& pair : replace) {
        boost::replace_all(str, pair.first, pair.second);
      }
      //const static boost::regex blackList("[^a-zA-Z0-9_]");
      //return boost::regex_replace(str, blackList, "");
      return str;
    }

    Weightfile::~Weightfile()
    {
      for (auto& filename : m_filenames) {
        if (fs::exists(filename)) {
          if (m_remove_temporary_directories)
            fs::remove_all(filename);
        }
      }
    }

    void Weightfile::addOptions(const Options& options)
    {
      options.save(m_pt);
    }

    void Weightfile::getOptions(Options& options) const
    {
      options.load(m_pt);
    }

    void Weightfile::addFeatureImportance(const std::map<std::string, float>& importance)
    {
      m_pt.put("number_of_importance_vars", importance.size());
      unsigned int i = 0;
      for (auto& pair : importance) {
        m_pt.put(std::string("importance_key") + std::to_string(i), pair.first);
        m_pt.put(std::string("importance_value") + std::to_string(i), pair.second);
        ++i;
      }
    }

    std::map<std::string, float> Weightfile::getFeatureImportance() const
    {
      std::map<std::string, float> importance;
      unsigned int numberOfImportanceVars = m_pt.get<unsigned int>("number_of_importance_vars", 0);
      for (unsigned int i = 0; i < numberOfImportanceVars; ++i) {
        auto key = m_pt.get<std::string>(std::string("importance_key") + std::to_string(i));
        auto value = m_pt.get<float>(std::string("importance_value") + std::to_string(i));
        importance[key] = value;
      }
      return importance;
    }

    void Weightfile::addSignalFraction(float signal_fraction)
    {
      m_pt.put("signal_fraction", signal_fraction);
    }

    float Weightfile::getSignalFraction() const
    {
      return m_pt.get<float>("signal_fraction");
    }

    std::string Weightfile::generateFileName(const std::string& suffix)
    {
      char* directory_template = strdup((fs::temp_directory_path() / "Basf2MVA.XXXXXX").c_str());
      auto directory = mkdtemp(directory_template);
      std::string tmpfile = std::string(directory) + std::string("/weightfile") + suffix;
      m_filenames.emplace_back(directory);
      free(directory_template);
      return tmpfile;
    }

    void Weightfile::addFile(const std::string& identifier, const std::string& custom_weightfile)
    {
      // TODO Test if file is valid
      std::ifstream in(custom_weightfile, std::ios::in | std::ios::binary);
      addStream(identifier, in);
      in.close();
    }

    void Weightfile::addStream(const std::string& identifier, std::istream& in)
    {
      using base64_t =  boost::archive::iterators::base64_from_binary <
                        boost::archive::iterators::transform_width<std::string::const_iterator, 6, 8 >>;

      std::string contents;
      in.seekg(0, std::ios::end);
      contents.resize(in.tellg());
      in.seekg(0, std::ios::beg);
      in.read(&contents[0], contents.size());
      std::string enc(base64_t(contents.begin()), base64_t(contents.end()));

      m_pt.put(identifier, enc);
    }

    void Weightfile::getFile(const std::string& identifier, const std::string& custom_weightfile)
    {
      std::ofstream out(custom_weightfile, std::ios::out | std::ios::binary);
      out << getStream(identifier);
    }

    std::string Weightfile::getStream(const std::string& identifier) const
    {
      using binary_t = boost::archive::iterators::transform_width <
                       boost::archive::iterators::binary_from_base64<std::string::const_iterator>, 8, 6 >;

      auto contents = m_pt.get<std::string>(identifier);
      std::string dec(binary_t(contents.begin()), binary_t(contents.end()));
      return dec;
    }

    void Weightfile::save(Weightfile& weightfile, const std::string& filename, const Belle2::IntervalOfValidity& iov)
    {
      if (boost::ends_with(filename, ".root")) {
        return saveToROOTFile(weightfile, filename);
      } else if (boost::ends_with(filename, ".xml")) {
        return saveToXMLFile(weightfile, filename);
      } else {
        return saveToDatabase(weightfile, filename, iov);
      }
    }

    void Weightfile::saveToROOTFile(Weightfile& weightfile, const std::string& filename)
    {
      std::stringstream ss;
      Weightfile::saveToStream(weightfile, ss);
      DatabaseRepresentationOfWeightfile database_representation_of_weightfile;
      database_representation_of_weightfile.m_data = ss.str();
      TFile file(filename.c_str(), "RECREATE");
      file.WriteObject(&database_representation_of_weightfile, "Weightfile");
    }

    void Weightfile::saveToXMLFile(Weightfile& weightfile, const std::string& filename)
    {
#if BOOST_VERSION < 105600
      boost::property_tree::xml_writer_settings<char> settings('\t', 1);
#else
      boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
#endif
      boost::property_tree::xml_parser::write_xml(filename, weightfile.m_pt, std::locale(), settings);
    }

    void Weightfile::saveToStream(Weightfile& weightfile, std::ostream& stream)
    {
#if BOOST_VERSION < 105600
      boost::property_tree::xml_writer_settings<char> settings('\t', 1);
#else
      boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
#endif
      boost::property_tree::xml_parser::write_xml(stream, weightfile.m_pt, settings);
    }

    Weightfile Weightfile::load(const std::string& filename, const Belle2::EventMetaData& emd)
    {
      if (boost::ends_with(filename, ".root")) {
        return loadFromROOTFile(filename);
      } else if (boost::ends_with(filename, ".xml")) {
        return loadFromXMLFile(filename);
      } else {
        return loadFromDatabase(filename, emd);
      }
    }

    Weightfile Weightfile::loadFromFile(const std::string& filename)
    {
      if (boost::ends_with(filename, ".root")) {
        return loadFromROOTFile(filename);
      } else if (boost::ends_with(filename, ".xml")) {
        return loadFromXMLFile(filename);
      } else {
        throw std::runtime_error("Cannot load file " + filename + " because file extension is not supported");
      }
    }

    Weightfile Weightfile::loadFromROOTFile(const std::string& filename)
    {

      if (not fs::exists(filename)) {
        throw std::runtime_error("Given filename does not exist: " + filename);
      }

      TFile file(filename.c_str(), "READ");
      if (file.IsZombie() or not file.IsOpen()) {
        throw std::runtime_error("Error during open of ROOT file named " + filename);
      }

      DatabaseRepresentationOfWeightfile* database_representation_of_weightfile = nullptr;
      file.GetObject("Weightfile", database_representation_of_weightfile);

      if (database_representation_of_weightfile == nullptr) {
        throw std::runtime_error("The provided ROOT file " + filename + " does not contain a valid MVA weightfile.");
      }
      std::stringstream ss(database_representation_of_weightfile->m_data);
      delete database_representation_of_weightfile;
      return Weightfile::loadFromStream(ss);
    }

    Weightfile Weightfile::loadFromXMLFile(const std::string& filename)
    {
      if (not fs::exists(filename)) {
        throw std::runtime_error("Given filename does not exist: " + filename);
      }

      Weightfile weightfile;
      boost::property_tree::xml_parser::read_xml(filename, weightfile.m_pt);
      return weightfile;
    }

    Weightfile Weightfile::loadFromStream(std::istream& stream)
    {
      Weightfile weightfile;
      boost::property_tree::xml_parser::read_xml(stream, weightfile.m_pt);
      return weightfile;
    }

    void Weightfile::saveToDatabase(Weightfile& weightfile, const std::string& identifier, const Belle2::IntervalOfValidity& iov)
    {
      std::stringstream ss;
      Weightfile::saveToStream(weightfile, ss);
      DatabaseRepresentationOfWeightfile database_representation_of_weightfile;
      database_representation_of_weightfile.m_data = ss.str();
      Belle2::Database::Instance().storeData(makeSaveForDatabase(identifier), &database_representation_of_weightfile, iov);
    }

    void Weightfile::saveArrayToDatabase(const std::vector<Weightfile>& weightfiles, const std::string& identifier,
                                         const Belle2::IntervalOfValidity& iov)
    {
      DBImportArray<DatabaseRepresentationOfWeightfile> dbArray(makeSaveForDatabase(identifier));

      for (auto weightfile : weightfiles) {
        std::stringstream ss;
        Weightfile::saveToStream(weightfile, ss);
        dbArray.appendNew(ss.str());
      }

      dbArray.import(iov);
    }

    Weightfile Weightfile::loadFromDatabase(const std::string& identifier, const Belle2::EventMetaData& emd)
    {
      auto pair = Belle2::Database::Instance().getData(emd, makeSaveForDatabase(identifier));

      if (pair.first == 0) {
        throw std::runtime_error("Given identifier cannot be loaded from the database: " + identifier);
      }

      DatabaseRepresentationOfWeightfile database_representation_of_weightfile = *static_cast<DatabaseRepresentationOfWeightfile*>
          (pair.first);
      std::stringstream ss(database_representation_of_weightfile.m_data);
      return Weightfile::loadFromStream(ss);
    }

  }
}
