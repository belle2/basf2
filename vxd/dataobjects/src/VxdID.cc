/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vxd/dataobjects/VxdID.h>
#include <sstream>

namespace Belle2 {

  namespace {
    /**
     * Small helper function to parse VxdID string representation
     *
     * This function takes an input stream and will return the next component of the VxdID
     * */
    int getPart(std::istream& in)
    {
      if (!in.eof()) {
        //Get next char, if it is a dot, ignore it and get the next one
        int next = in.get();
        if (next == '.') next = in.get();
        //If it is a wildcard we return 0 as id, otherwise we put it back in the stream
        if (next == '*' or in.eof()) {
          return 0;
        } else {
          in.unget();
        }
        //If it is the segment separator, we assume the remaining parts to be missing, so return 0
        if (next == '#') return 0;

        //Now get the actual value out of the stream. If this fails something is wrong and it is not
        //a valid id
        int value(0);
        in >> value;
        if (in.fail() && !in.eof()) {
          throw std::runtime_error("Failed to parse Number");
        }
        return value;
      }
      return 0;
    }
  }

  VxdID::VxdID(const std::string& sensor)
  {
    //We parse the Id from string, so set it to 0 first
    m_id.id = 0;
    //create a stream from the string
    std::istringstream in(sensor);
    try {
      //Get all the parts
      m_id.parts.layer  = getPart(in);
      m_id.parts.ladder = getPart(in);
      m_id.parts.sensor = getPart(in);
      //Check if we also have a segment specified, if so get it
      if (in.peek() == '#') {
        in.get();
        m_id.parts.segment = getPart(in);
      }
    } catch (std::runtime_error&) {
      //Something went wrong parsing the parts
      m_id.id = 0;
      throw std::invalid_argument("Could not parse VxdID: '" + sensor + "'");
    }
    //There is stuff left we also throw an exception as we cannot warn the user
    //without the logging system
    if (!in.eof()) {
      std::string rest;
      //Get the remainder: get everything in the stream until the next NULL
      //character which should only occur at the end of the string.
      getline(in, rest, '\0');
      throw std::invalid_argument("Trailing characters after VxdID " + std::string(*this) + ": '" + rest + "'");
    }
  }

  VxdID::operator std::string() const
  {
    std::stringstream out;
    if (m_id.parts.layer) {
      out << m_id.parts.layer;
    } else {
      out << "*";
    }
    if (m_id.parts.ladder || m_id.parts.sensor) {
      out << ".";
      if (m_id.parts.ladder) {
        out << m_id.parts.ladder;
      } else {
        out << "*";
      }
    }
    if (m_id.parts.sensor) {
      out << "." << m_id.parts.sensor;
    }
    if (m_id.parts.segment) {
      out << "#" << m_id.parts.segment;
    }
    return out.str();
  }

  std::ostream& operator<<(std::ostream& out, const VxdID& id)
  {
    out << ((std::string)id);
    return out;
  }

}
