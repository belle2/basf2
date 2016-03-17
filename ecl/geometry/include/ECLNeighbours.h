/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Returns the list of neighbours for a given crystal ID.                 *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef ECLNEIGHBOURS_H
#define ECLNEIGHBOURS_H

#include <vector>
#include <string>

namespace Belle2 {
  namespace ECL {

    /**
     * Class to get the neighbours for a given crystal id.
     */
    class ECLNeighbours {

    public:

      /**  Constructor: Default. */
//      ECLNeighbours();

      /**  Constructor: Fix number of neighbours ("N") in the seed theta ring or radius ("R") with par = n or par = radius [cm]. */
      ECLNeighbours(const std::string neighbourDef, const float par);

      /**  Destructor. */
      ~ECLNeighbours();

      /** Return the neighbours for a given crystal ID.*/
      std::vector<short  int> getNeighbours(short int cid);

    private:
      std::vector < std::vector < short int > > m_neighbourMap; // list of list of neighbour cids
      std::vector < std::vector < short int > > m_neighbourMapTemp; // temp list of list of neighbour cids

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
      void initializeN(const int nneighbours);

      /**  initialize the radius neighbout list. */
      void initializeR(const float radius);

      /**  return the previous phi id. */
      short int decreasePhiId(const short int phiid, const short int thetaid, const short int n);

      /**  return the next phi id. */
      short int increasePhiId(const short int phiid, const short int thetaid, const short int n);

      /** return a list of phi ids between two phi ids */
      std::vector<short int> getPhiIdsInBetween(const short int phiInc, const short int phiDec, const short int theta);

      /** return the chord length between crystals */
      float getDistance(float alpha, float R);

    };

  } // end of namespace ECL
} // end of namespace Belle2

#endif
