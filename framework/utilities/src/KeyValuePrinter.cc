/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/utilities/KeyValuePrinter.h>
#include <boost/property_tree/json_parser.hpp>

using namespace Belle2;

std::string KeyValuePrinter::escape(const std::string& value) const
{
  using namespace boost::property_tree::json_parser;

  if (m_json)
    return "\"" + create_escapes(value) + "\"";
  else
    return value;
}
