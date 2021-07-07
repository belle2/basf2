/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <vector>

namespace Belle2 {

  /** The payload class for GDL algorithm.
   *
   *  The payload class stores strings of GDL logic algorithm.
   */


  class TRGGDLDBAlgs: public TObject {
  public:

    /** Default constructor */
    TRGGDLDBAlgs() {}

    /** copy constructor */
    TRGGDLDBAlgs(const TRGGDLDBAlgs&) = default;

    /** assignment operator */
    TRGGDLDBAlgs& operator=(const TRGGDLDBAlgs&) = default;

    /** Setter for a GDL logic alghrithm */
    void setalg(const std::string& s)
    {
      m_algs.push_back(s);
    }

    /** clear vector */
    void clear(void)
    {
      m_algs.clear();
    }

    /** Getter for a GDL logic alghrithm */
    std::string getalg(int i) const
    {
      return m_algs.at(i);
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
