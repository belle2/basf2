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
#include <boost/filesystem.hpp>


namespace Belle2 {
  namespace gearbox {

    class FileContext: public InputContext {
    public:
      FileContext(const std::string& filename, bool compressed);
      virtual int readXmlData(char *buffer, int buffsize) {
        m_stream.read(buffer, buffsize);
        return m_stream.gcount();
      }
      virtual ~FileContext() {}
    protected:
      boost::iostreams::filtering_istream m_stream;
    };

    class FileHandler: public InputHandler {
    public:
      FileHandler(const std::string& uri);
      virtual InputContext* open(const std::string &path);
    protected:
      std::string m_path;
    };

  }
}

#endif
