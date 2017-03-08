/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
