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
   * Database object for energy-deposit to ADC-count conversion.
   */
  class CDCEDepToADCConversions: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCEDepToADCConversions() {}

    /**
     * Set conversion parameterization mode
     */
    void setParamMode(unsigned short mode)
    {
      m_paramMode = mode;
    }

    /**
     * Set group id (parameterized per group)
     * id=0: superLayerID; =1: layerID; =1: wireID
     */
    void setGroupID(unsigned short mode)
    {
      m_groupID = mode;
    }

    /**
     * Set the conv. paramseters in the list
     * @param id superLayerid(0-8), laerID(0-55) or wireID
     * @param params parameters for conversion
     */
    void setParams(unsigned short id, const std::vector<float>& params)
    {
      m_cvs.insert(std::pair<unsigned short, std::vector<float>>(id, params));
    }

    /**
     * Get mode of conversion parameterization
     */
    unsigned short getParamMode() const
    {
      return m_paramMode;
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
      return m_cvs.size();
    }

    /**
     * Get the whole list
     */
    std::map<unsigned short, std::vector<float>> getParams() const
    {
      return m_cvs;
    }

    /**
     * Get the conv. parameters for the id
     * @param  id layerID or wireID
     * @return conversion paramseters for the id
     */
    const std::vector<float>& getParams(unsigned short id) const
    {
      std::map<unsigned short, std::vector<float>>::const_iterator it = m_cvs.find(id);
      if (it != m_cvs.end()) {
        return it->second;
      } else {
        B2FATAL("Specified id not found in getParams !");
      }
    }

    /**
     * Print all contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "Edep-to-ADC conversion list" << std::endl;
      std::cout << "#entries= " << m_cvs.size() << std::endl;
      std::cout << m_paramMode << std::endl;
      std::cout << "in order of id and parameters" << std::endl;

      for (auto const& ent : m_cvs) {
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
    unsigned short m_paramMode = 0; /*!< Mode for parameterization */
    unsigned short m_groupID = 0;   /*!< Group id (parameterized per group) */
    std::map<unsigned short, std::vector<float>> m_cvs; /**< cv list */

    ClassDef(CDCEDepToADCConversions, 2); /**< ClassDef */
  };

} // end namespace Belle2
