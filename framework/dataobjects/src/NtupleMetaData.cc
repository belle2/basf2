/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <boost/python/class.hpp>
#include <boost/python/copy_const_reference.hpp>

#include <framework/dataobjects/NtupleMetaData.h>
#include <framework/utilities/HTML.h>
#include <framework/utilities/KeyValuePrinter.h>
#include <framework/io/RootIOUtilities.h>


#include <boost/algorithm/string.hpp>

#include <nlohmann/json.hpp>

#include <iostream>

using namespace std;
using namespace Belle2;
using namespace boost::python;

NtupleMetaData::NtupleMetaData() {}

void NtupleMetaData::Print(Option_t* option) const
{
  FileMetaData::Print(option);
  KeyValuePrinter printer(false);
  // printer.put("inputs", m_inputLfns);
  std::cout << "=== NtupleMetaData ===\n";
  std::cout << printer.string();
}

bool NtupleMetaData::write(std::ostream& output, const std::string& physicalFileName) const
{
  FileMetaData::write(output, physicalFileName);
  output << "  <Ntuple>\n";
  if (!m_inputLfns.empty()) {
    output << "    <Inputs>" << HTML::escape(m_inputLfns[0]);
    for (unsigned int input = 1; input < m_inputLfns.size(); input++) {
      output << "," << HTML::escape(m_inputLfns[input]);
    }
    output << "</Inputs>\n";
  }
  output << "  </Ntuple>\n";

  return true;
}

std::string NtupleMetaData::getJsonStr() const
{
  nlohmann::json metadata = FileMetaData::getJsonStr();
  metadata.push_back(nlohmann::json::object_t::value_type("inputs", m_inputLfns));
  return metadata.dump(2);
}

void NtupleMetaData::setCreationData()
{
  std::string site;
  char date[100];
  auto now = time(nullptr);
  strftime(date, 100, "%Y-%m-%d %H:%M:%S", gmtime(&now));
  const char* belle2_site = getenv("BELLE2_SITE");
  if (belle2_site) {
    site = belle2_site;
  } else {
    char hostname[1024];
    gethostname(hostname, 1023); //will not work well for ipv6
    hostname[1023] = '\0'; //if result is truncated, terminating null byte may be missing
    site = hostname;
  }
  const char* user = getenv("BELLE2_USER");
  if (!user) user = getenv("USER");
  if (!user) user = getlogin();
  if (!user) user = "unknown";
  auto commitid = "development";
  FileMetaData::setCreationData(date, site, user, commitid);
}
