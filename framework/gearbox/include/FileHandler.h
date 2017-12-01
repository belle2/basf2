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

#include <framework/gearbox/InputHandler.h>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/format.hpp>

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
      virtual int readXmlData(char* buffer, int buffsize)
      {
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
     *
     * The uri is the base dir for all files to be searched and can contain the
     * placeholders {EXP} and {RUN} to be replaced with the experiment and run
     * number respectively. Optionally, the placeholders can contain the number
     * of digits the result should have separated by a colon, filled with
     * zeros, e.g. {EXP:4} to have the experiment number starting as 0001
     *
     * If an file cannot be found (with or without .gz extension), the
     * directory separators "/" will be successively replaced by dashes "-"
     * (starting at the back) to allow for flat file structure. For example, if
     * the uri is /data/ and the filename to be opened is geometry/Belle2.xml,
     * the inputhandler will try to find
     * /data/geometry/Belle2.xml
     * /data/geometry/Belle2.xml.gz
     * /data/geometry-Belle2.xml
     * /data/geometry-Belle2.xml.gz
     * /data-geometry-Belle2.xml
     * /data-geometry-Belle2.xml.gz
     * in that order the search path, stopping at the first success.
     */
    class FileHandler: public InputHandler {
    public:
      /** Instantiate a new file handler, using the uri as base search path.
       * If uri is empty, the default data search path will be used.
       * @param uri * path to search files in
       */
      explicit FileHandler(const std::string& uri);
      /** empty, virtual destructor */
      virtual ~FileHandler() {}
      /** create a new FileContext by searching the file system for a file
       * named like path.
       * @param path name of the file, relative to the FileHandler search path
       */
      virtual InputContext* open(const std::string& path);
    protected:
      /** Search path to look for files */
      std::string m_path;
      /** format object in case of run-dependent data path */
      boost::format m_pathformat;
      /** bool indicating whether the data path is run-dependent */
      bool m_rundependence {false};
    };
  } //namespace gearbox
} //namespace Belle2
