/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEARBOX_FILEHANDLER_H
#define GEARBOX_FILEHANDLER_H

#include <framework/gearbox/InputHandler.h>
#include <boost/iostreams/filtering_stream.hpp>


namespace Belle2 {
  namespace gearbox {

    /** InputContext which serves the content of a plain file */
    class FileContext: public InputContext {
    public:
      /** Open the given filename.
       * @param filename absolute filename name of the file
       * @param compressed if true, decompress the file on the fly
       */
      FileContext(const std::string& filename, bool compressed);
      /** Return a block of the data */
      virtual int readXmlData(char* buffer, int buffsize) {
        m_stream.read(buffer, buffsize);
        return m_stream.gcount();
      }
      /** Close the file */
      virtual ~FileContext() {}
    protected:
      /** stream to read data from */
      boost::iostreams::filtering_istream m_stream;
    };

    /** InputHandler which will read XML from plain files, optionally gzip compressed.
     * This Handler will try to find the XML files as plain files in the file
     * system.  If a file with the exact name cannot be found, the Handler
     * tries to append .gz to the filename and decompress it with gzip.
     */
    class FileHandler: public InputHandler {
    public:
      /** Instantiate a new file hanlder, using the uri as base search path.
       * If uri is empty, the default data search path will be used
       * @param uri path to search files in
       */
      FileHandler(const std::string& uri);
      /** empty, virtual destructor */
      virtual ~FileHandler() {}
      /** create a new FileContext by searching the file system for a file named like path.
       * @param path name of the file, relative to the FileHandler search path
       */
      virtual InputContext* open(const std::string& path);
    protected:
      /** Search path to look for files */
      std::string m_path;
    };

  }
}

#endif
