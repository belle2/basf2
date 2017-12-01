/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>

namespace Belle2 {
  namespace gearbox {
    /**
     * Class representing a resource context for gearbox.
     *
     * This class is responsible for providing read access to an XML resource
     * and to free any memory associated with the resource on destruction
     */
    class InputContext {
    public:
      /**
       * Read a block of XML data into buffer, at most buffsize bytes and
       * return the number of bytes read.
       * @param buffer   pointer where to write XML data
       * @param buffsize number of bytes to write, actual number of bytes can
       *                 be less if XML is shorter
       * @return number of bytes actually written
       */
      virtual int readXmlData(char* buffer, int buffsize) = 0;
      /** Close the context */
      virtual ~InputContext() {}
    protected:
    };

    /**
     * Class to provide an InputContext for a given XML resource name.
     *
     * This class is responsible for opening XML resources on request hand
     * returning an InputContext representing the resource on success
     */
    class InputHandler {
    public:
      /**
       * Factory function which takes a backend uri and returns
       * an InputHandler instance
       * */
      typedef InputHandler* Factory(const std::string& uri);

      /**
       * Constructor accepting the uri for the InputHandler.
       *
       * The uri is used to specify any parameters for the backend. The content is
       * dependent on the actual InputHandler implementation but could be a
       * base directory, a database host, an archive filename or any combination.
       * Parsing of this string is left to the actual implementation
       */
      explicit InputHandler(const std::string& uri): m_uri(uri) {};

      /** empty, virtual destructor */
      virtual ~InputHandler() {}

      /**
       * Open an XML resource.
       *
       * Open the XML resource specified by path and return an InputContext
       * instance on success.
       * @param path name of the XML resource
       * @return pointer to an InputContext instance on success, 0 otherwise
       */
      virtual InputContext* open(const std::string& path) = 0;
    protected:
      /** URI for the InputHandler */
      std::string m_uri;
    };
  }
}
