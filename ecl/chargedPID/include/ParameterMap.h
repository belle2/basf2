/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Class to parse .dat file content into a std::map for later usage       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Alon Hershenhorn                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <map>
#include <string>
#include <iostream>
#include <cassert>
#include <fstream>

namespace Belle2 {
  namespace ECL {

    /** Class to parse .dat file content into a std::map for later usage.
     */
    class ParameterMap {
    public:
      ParameterMap() {}

      /** Constructor. Reads an input text file and stores its
      content into a map.
       */
      explicit ParameterMap(const char* fileName)
      {
        std::ifstream f(fileName);
        if (!f.good()) return;
        while (! f.eof()) {
          std::string s;
          double d;
          f >> s >> d;
          if (f.good()) {
            if (s[0] != '#') par_[ s ] = d;
          }
        }
      }

      /** Getter for parameter:
      @param name the name of the parameter as in the input text file.
       */
      double param(const std::string& name) const
      {
        const_iterator i = find(name);
        return i->second;
      }

      /** Print content of map to an output stream. */
      void print(std::ostream& out) const
      {
        const_iterator i;
        for (i = par_.begin(); i != par_.end(); ++i)
          out << i->first << " " << i->second << std::endl;
      }

      /** Write parameter map to an output file. */
      void write(const char* fileName) const
      {
        std::ofstream f(fileName);
        print(f);
        f.close();
      }

      /** Setter for parameter:
      @param name the name of the parameter.
      @param c the value to be set.
       */
      void setPar(const std::string& s, double c)
      {
        par_[ s ] = c;
      }

    private:
      typedef std::map<std::string, double> container;
      typedef container::const_iterator const_iterator;
      typedef container::iterator iterator;

      /** The map containing the paramters */
      container par_;

      /** Check if parameter is in map:
      @param name the name of the parameter.
      @return const iterator to the map element, if an element with specified key is found. If not, the execution is aborted.
       */
      const_iterator find(const std::string& name) const
      {
        const_iterator i = par_.find(name);
        assert(i != par_.end());
        return i;
      }

      /** Check if parameter is in map:
      @param name the name of the parameter.
      @return iterator to the map element, if an element with specified key is found. If not, the execution is aborted.
       */
      iterator find(const std::string& name)
      {
        iterator i = par_.find(name);
        assert(i != par_.end());
        return i;
      }


    };

  }
}

