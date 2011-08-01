/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/vxd/VxdID.h>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cassert>

using namespace std;

namespace Belle2 {

  /**
   * Small helper function to parse VxdID string representation
   *
   * This function takes an input stream and will return the next component of the VxdID
   * */
  static int getPart(istream &in)
  {
    if (!in.eof()) {
      //Get next char, if it is a dot, ignore it and get the next one
      char next = in.get();
      if (next == '.') next = in.get();
      //If it is a wildcard we return 0 as id, otherwise we put it back in the stream
      if (next == '*') {
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
        throw runtime_error("Failed to parse Number");
      }
      return value;
    }
    return 0;
  }

  VxdID::VxdID(const std::string& sensor)
  {
    //We parse the Id from string, so set it to 0 first
    m_id.id = 0;
    //create a stream from the string
    istringstream in(sensor);
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
    } catch (runtime_error &e) {
      //Something went wrong parsing the parts
      cout << "Could not parse VtxID '" << sensor << "'" << endl;
      m_id.id = 0;
    }
    //There is stuff left, warn about it
    if (!in.eof()) {
      cout << "Stuff at end of ID" << endl;
    }
  }

  VxdID::operator string() const
  {
    stringstream out;
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

  std::ostream& operator<<(std::ostream &out, const VxdID &id)
  {
    out << ((string)id);
    return out;
  }

}
