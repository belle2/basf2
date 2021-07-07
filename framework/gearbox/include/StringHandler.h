/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/gearbox/InputHandler.h>
#include <sstream>

namespace Belle2 {
  namespace gearbox {
    /** InputContext which just returns the value of a string previously assigned */
    class StringContext: public InputContext {
    public:
      /** Take an XML fragment as parameter and create a context for it */
      explicit StringContext(const std::string& data): m_stream(data) {};
      /** Return a block of the data */
      virtual int readXmlData(char* buffer, int buffsize) override
      {
        m_stream.read(buffer, buffsize);
        return m_stream.gcount();
      }
    protected:
      /** Assigned data string */
      std::istringstream m_stream;
    };

    /**
     * InputHandler which just takes the uri as XML data.
     * Intended for ease of unit testing, beware that the current implementation
     * does always return the same fragment regardles of resource name, so
     * xincludes will lead to endless loop
     */
    class StringHandler: public InputHandler {
    public:
      /** Create a new InputHandler which always returns uri as XML content */
      explicit StringHandler(const std::string& uri): InputHandler(uri) {}
      /** empty, virtual destructor */
      virtual ~StringHandler() {}
      /** Create a new context, path will be ignored */
      virtual InputContext* open(const std::string& path) override;
    };
  }
}
