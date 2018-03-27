#ifndef PARAMETERMAP_H
#define PARAMETERMAP_H
#include <map>
#include <string>
#include <iostream>
#include <cassert>
#include <fstream>

namespace Belle2 {
  namespace ECL {
    class ParameterMap {
    public:
      ParameterMap() {}

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

      double param(const std::string& name) const
      {
        const_iterator i = find(name);
        return i->second;
      }

      void print(std::ostream& out) const
      {
        const_iterator i;
        for (i = par_.begin(); i != par_.end(); ++i)
          out << i->first << " " << i->second << std::endl;
      }

      void  write(const char* fileName) const
      {
        std::ofstream f(fileName);
        print(f);
        f.close();
      }

      void setPar(const std::string& s, double c)
      {
        par_[ s ] = c;
      }

    private:
      typedef std::map<std::string, double> container;
      typedef container::const_iterator const_iterator;
      typedef container::iterator iterator;
      container par_;

      const_iterator find(const std::string& name) const
      {
        const_iterator i = par_.find(name);
        assert(i != par_.end());
        return i;
      }

      iterator find(const std::string& name)
      {
        iterator i = par_.find(name);
        assert(i != par_.end());
        return i;
      }


    };

  }
}
#endif



