/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* C++ headers. */
#include <string>
#include <vector>

namespace Belle2 {
  namespace ECL {

    /**
     * Class to get the neighbours for a given cell id.
     */
    class ECLNeighbours {

    public:

      /**  Constructor: Fix number of neighbours ("N") in the seed theta ring, fraction cross ("F"),  radius ("R") with par = n or par = fraction (0.1-1.0) or par = radius [cm]. The sorted parameter will sort ascending thetaid and clockwise phi for the "N" case.  */
      ECLNeighbours(const std::string& neighbourDef, const double par, const bool sorted = false);

      /**  Destructor. */
      ~ECLNeighbours();

      /** Return the neighbours for a given cell ID.*/
      const std::vector<short int>& getNeighbours(short int cid) const;

      /** return number of crystals in a given theta ring */
      short int getCrystalsPerRing(const short int thetaid) const { return m_crystalsPerRing[thetaid]; }

    private:
      /** list of list of neighbour cids. */
      std::vector < std::vector < short int > > m_neighbourMap;

      /** temporary list of list of neighbour cids. */
      std::vector < std::vector < short int > > m_neighbourMapTemp;

      /** Number of crystals in each theta ring.*/
      const short m_crystalsPerRing[69] = {
        48, 48, 64, 64, 64, 96, 96, 96, 96, 96, 96, 144, 144, //FWD up to 13
        144, 144, 144, 144, 144, 144, 144,  // BARREL up to 20
        144, 144, 144, 144, 144, 144, 144, 144, 144, 144, // ...30
        144, 144, 144, 144, 144, 144, 144, 144, 144, 144, // ...40
        144, 144, 144, 144, 144, 144, 144, 144, 144, 144, // ...50
        144, 144, 144, 144, 144, 144, 144, 144, 144, // BARREL up to 59
        144, 144, 96, 96, 96, 96, 96, 64, 64, 64
      }; //BWD


      /**  initialize the mask neighbour list. */
      void initializeN(const int nneighbours, const bool sorted = false);

      /**  initialize the mask neighbour list, remove corners. */
      void initializeNC(const int nneighbours);

      /**  initialize the mask neighbour list, legacy code. */
      void initializeNLegacy(const int nneighbours);

      /**  initialize the mask neighbour list, remove corners, legacy code. */
      void initializeNCLegacy(const int nneighbours, const int corners);

      /**  initialize the radius neighbour list. */
      void initializeR(const double radius);

      /**  initialize the fractional cross neighbour list. */
      void initializeF(const double fraction);

      /**  return the previous phi id. */
      short int decreasePhiId(const short int phiid, const short int thetaid, const short int n);

      /**  return the next phi id. */
      short int increasePhiId(const short int phiid, const short int thetaid, const short int n);

      /** return a list of phi ids between two phi ids */
      std::vector<short int> getPhiIdsInBetween(const short int phiInc, const short int phiDec, const short int theta);

      /** return a list of phi ids between two phi ids minus edges*/
      std::vector<short int> getPhiIdsInBetweenC(const short int phiInc, const short int phiDec, const short int theta,
                                                 const int corners);

      /** return the chord length between cells */
      double getDistance(const double alpha, const double R);

    };

  } // end of namespace ECL
} // end of namespace Belle2

