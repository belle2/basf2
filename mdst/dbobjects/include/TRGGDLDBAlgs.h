/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <unordered_map>
#include <vector>

namespace Belle2 {

  /** The payload class for GDL psnm
   *
   *  The payload class stores the number of psnm bit and their prescale values
   *  value of a missing bit is "0"
   */


  class TRGGDLDBAlgs: public TObject {
  public:

    /** Default constructor */
    TRGGDLDBAlgs(): m_noutbit{0} {}
    /** copy constructor */
    TRGGDLDBAlgs(const TRGGDLDBAlgs& b)
    {
      m_noutbit = b.m_noutbit;
      for (int i = 0; i < b.m_noutbit; i++) {
        m_algs[i] = b.m_algs[i];
      }
    }
    /** assignment operator */
    TRGGDLDBAlgs& operator=(const TRGGDLDBAlgs& b)
    {
      m_noutbit = b.m_noutbit;
      for (int i = 0; i < b.m_noutbit; i++) {
        m_algs[i] = b.m_algs[i];
      }
      return *this;
    }

    void setalg(std::string s)
    {
      m_algs.push_back(s);
    }
    void setnoutbit(int i)
    {
      m_noutbit = i;
    }

    std::string getalg(int i) const
    {
      return m_algs[i];
    }
    int getnoutbit() const
    {
      return m_noutbit;
    }

  private:

    /** Number of bit */
    int m_noutbit;

    /** Number of bins per sensor along u side */
    std::vector<std::string> m_algs;

    ClassDef(TRGGDLDBAlgs, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
