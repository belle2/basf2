/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * A local maximum is defined by its energy and the number and energy of  *
 * its neighbours. It holds a relation to to an ECLConnectedRegion and    *
 * an ECLCalDigit.                                                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLLOCALMAXIMUM_H
#define ECLLOCALMAXIMUM_H

#include <framework/datastore/RelationsObject.h>

#include <algorithm>    // std::unique, std::distance
#include <vector>       // std::vector

namespace Belle2 {

  /*! Class to store local maxima (LM)
   */

  class ECLLocalMaximum : public RelationsObject {
  public:

    /** default constructor for ROOT */
    ECLLocalMaximum()
    {
      m_LMId                    = -1;     /**< LM identifier */
      m_NNeighbours             = -1; /**< Number of neighbours with energy > cut */
      m_MaximumEnergyNeighbours = 0.0;  /**< Maximum energy among neighbours */
      m_ListOfNeighbourEnergies.clear(); /**< Clear the neighbour list */
    }

    // setters
    /*! Set LM identifier
     */
    void setLMId(int LMId) { m_LMId = LMId; }

    /*! Set number of neighbours above cut.
     */
    void setNNeighbours(int NoN) { m_NNeighbours = NoN; }

    /*! Set maximum energiy of neighbours.
     */
    void setMaximumEnergyNeighbours(double energy) { m_MaximumEnergyNeighbours = energy; }

    /*! Append an entry to the neighbour energy list.
     */
    void appendToNeighbourEnergiesList(double energy) { m_ListOfNeighbourEnergies.push_back(energy); }

    /*! Set the neighbour energy list.
     */
    void setNeighbourEnergiesList(std::vector<double> list) { m_ListOfNeighbourEnergies = list; }

    /*! Clears the neighbour energy list.
     */
    void clearNeighbourEnergiesList() { m_ListOfNeighbourEnergies.clear(); }

    // getters
    /*! Get LM identifier.
     * @return LMId
     */
    int getLMId() const { return m_LMId; }

    /*! Get number of neighbours above cut.
     * @return number of neighbours
     */
    int getNNeighbours() const { return m_NNeighbours; }

    /*! Get number of neighbours above cut.
     * @return number of neighbours
     */
    double getMaximumEnergyNeighbours() const { return m_MaximumEnergyNeighbours; }

    /*! Get list of seeds (cell ids).
     * @return SeedList
     */
    std::vector<double> getNeighbourEnergiesList() const { return m_ListOfNeighbourEnergies; }

  private:
    int m_LMId;  /**< LM ID */
    int m_NNeighbours; /**< Number of neighbours. */
    double m_MaximumEnergyNeighbours; /**< Maximum energy of neighbours. */
    std::vector<double> m_ListOfNeighbourEnergies;  /**< List of neighbour energies. */

    // 1: initial version
    ClassDef(ECLLocalMaximum, 1); /**< ClassDef */

  };

} // end namespace Belle2

#endif
