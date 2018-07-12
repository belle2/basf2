/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/dataobjects/BKLMElementID.h>
#include <sstream>

using namespace std;

namespace Belle2 {

  namespace {
    /**
     * Small helper function to parse BKLM string representation
     *
     * This function takes an input stream and will return the component of the BKLM
     * */
    int getPart(istream& in)
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
          throw runtime_error("Failed to parse Number");
        }
        return value;
      }
      return 0;
    }
  }

  BKLMElementID::BKLMElementID(const std::string& module)
  {
    //We parse the Id from string, so set it to 0 first
    m_id.id = 0;
    //create a stream from the string
    istringstream in(module);
    try {
      //Get all the parts
      m_id.parts.isForward = getPart(in);
      m_id.parts.sector = getPart(in);
      m_id.parts.layer  = getPart(in);
      /*//Check if we also have a segment specified, if so get it
      if (in.peek() == '#') {
        in.get();
        m_id.parts.segment = getPart(in);
      }
      */
    } catch (runtime_error&) {
      //Something went wrong parsing the parts
      m_id.id = 0;
      throw invalid_argument("Could not parse BKLMElementID: '" + module + "'");
    }
    //There is stuff left we also throw an exception as we cannot warn the user
    //without the logging system
    if (!in.eof()) {
      string rest;
      //Get the remainder: get everything in the stream until the next NULL
      //character which should only occur at the end of the string.
      getline(in, rest, '\0');
      throw invalid_argument("Trailing characters after BKLMElementID " + (string)*this + ": '" + rest + "'");
    }
  }

  BKLMElementID::operator string() const
  {
    stringstream out;
    if (m_id.parts.isForward) {
      out << m_id.parts.isForward;
    } else {
      out << "*";
    }
    if (m_id.parts.sector) {
      out << "." << m_id.parts.sector;
    } else {
      out << "*";
    }
    if (m_id.parts.layer) {
      out << "." << m_id.parts.layer;
    }
    return out.str();
  }

  std::ostream& operator<<(std::ostream& out, const BKLMElementID& id)
  {
    out << ((string)id);
    return out;
  }

}
