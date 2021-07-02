/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <string>
#include <fstream>
#include <boost/property_tree/xml_parser.hpp>

namespace Belle2 {
  /// DB object which stores whole xml
  class XmlFile : public TObject {

  public:
    /// Constructor
    XmlFile() {};

    /// Destructor
    virtual ~XmlFile()
    {
    }

    /// Read content from xml file
    void readXml(std::string xmlFileName)
    {
      std::ifstream t(xmlFileName);
      std::stringstream buffer;
      buffer << t.rdbuf();
      m_data = buffer.str();
    }

    /// Get content as string
    std::string getData()
    {
      return m_data;
    }

    /// Populate boost PropertyTree with stored data
    void fillPropertyTree(boost::property_tree::ptree& tree)
    {
      std::stringstream ss; ss << m_data;
      using boost::property_tree::ptree;
      read_xml(ss, tree);
    }

  private:
    /// The xml data as string
    std::string m_data{""};

    ClassDef(XmlFile, 1) /**< DB object which stores whole xml */
  };
}
