/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <map>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <TObject.h>

namespace Belle2 {

  /**
   * Database object for fudge factors for CDC space resol. Needed both for MC and data production. This object would probably be tentative; may be merged into the CDCSpaceResols object in future...
   */
  class CDCFudgeFactorsForSigma: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCFudgeFactorsForSigma() {}

    /**
     * Set group id (factors per group)
     * id=0: all-wires; >=1: not ready
     */
    void setGroupID(unsigned short mode)
    {
      m_groupID = mode;
    }

    /**
     * Set the factors in the list
     * @param id laerID(0-55) or wireID
     * @param factors factors
     */
    void setFactors(unsigned short id, const std::vector<float>& factors)
    {
      m_ffs.insert(std::pair<unsigned short, std::vector<float>>(id, factors));
    }

    /**
     * Get group id
     */
    unsigned short getGroupID() const
    {
      return m_groupID;
    }

    /**
     * Get the no. of entries in the list
     */
    unsigned short getEntries() const
    {
      return m_ffs.size();
    }

    /**
     * Get the whole list
     */
    std::map<unsigned short, std::vector<float>> getFactors() const
    {
      return m_ffs;
    }

    /**
     * Get the factors for the id
     * @param  id layerID or wireID
     * @return fudge factors for the id
     */
    const std::vector<float>& getFactors(unsigned short id) const
    {
      std::map<unsigned short, std::vector<float>>::const_iterator it = m_ffs.find(id);
      if (it != m_ffs.end()) {
        return it->second;
      } else {
        B2FATAL("Specified id not found in getFactors !");
      }
    }

    /**
     * Print all contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "Fudge factor list" << std::endl;
      std::cout << "#entries= " << m_ffs.size() << std::endl;
      std::cout << "in order of id and factors" << std::endl;

      for (auto const& ent : m_ffs) {
        std::cout << ent.first;
        unsigned short np = (ent.second).size();
        for (unsigned short i = 0; i < np; ++i) {
          std::cout << "  " << (ent.second)[i];
        }
        std::cout << std::endl;
      }
    }

    /**
     * Output the contents in text file format
     */
    /*
    void outputToFile(std::string fileName) const
    {
      std::ofstream fout(fileName);

      if (fout.bad()) {
        B2ERROR("Specified output file could not be opened!");
      } else {
        fout << m_twParamMode << "  " << m_nTwParams << std::endl;
        for (auto const& ent : m_tws) {
          fout << std::setw(3) << std::right << ent.first;
          for (unsigned short i = 0; i < m_nTwParams; ++i) {
            fout << "  " << std::setw(15) << std::scientific << std::setprecision(8) << ent.second[i];
          }
          fout << std::endl;
        }
        fout.close();
      }
    }
    */

  private:
    unsigned short m_groupID = 0;   /*!< Group id (factors per group) */
    std::map<unsigned short, std::vector<float>> m_ffs; /**< cv list */

    ClassDef(CDCFudgeFactorsForSigma, 1); /**< ClassDef */
  };

} // end namespace Belle2

