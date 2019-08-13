#pragma once

#include <TObject.h>
#include <unordered_map>
#include <vector>

namespace Belle2 {

  /** The payload class for GDL algorithm.
   *
   *  The payload class stores the number of psnm bit and their prescale values
   */


  class TRGGDLDBAlgs: public TObject {
  public:

    /** Default constructor */
    TRGGDLDBAlgs() {}
    /** copy constructor */
    TRGGDLDBAlgs(const TRGGDLDBAlgs& b)
    {
      m_algs.clear();
      m_algs.reserve(b.m_algs.size());
      for (long unsigned int i = 0; i < b.m_algs.size(); i++) {
        m_algs[i] = b.m_algs[i];
      }
    }
    /** assignment operator */
    TRGGDLDBAlgs& operator=(const TRGGDLDBAlgs& b)
    {
      m_algs.clear();
      m_algs.reserve(b.m_algs.size());
      for (long unsigned int i = 0; i < b.m_algs.size(); i++) {
        m_algs[i] = b.m_algs[i];
      }
      return *this;
    }

    /** Setter for a GDL logic alghrithm */
    void setalg(std::string s)
    {
      m_algs.push_back(s);
    }

    /** Getter for a GDL logic alghrithm */
    std::string getalg(int i) const
    {
      return m_algs[i];
    }

    /** Getter of the number of algorithms */
    int getnalgs() const
    {
      return m_algs.size();
    }

  private:

    /** Algorithm strings of GDL logic */
    std::vector<std::string> m_algs;

    ClassDef(TRGGDLDBAlgs, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
