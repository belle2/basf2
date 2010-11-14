/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CREATORUTILS_H_
#define CREATORUTILS_H_

#include <TGeoVolume.h>
#include <TVector3.h>

namespace Belle2 {

  /**
   * Class that provides various utility methods for geometry Creator developers.
   *
   * This class provides various, static defined methods to support users
   * in developing Creators.
   */
  class CreatorUtils {

  public:

    /**
     * Adds a 3D arrow to the geometry for visualizing momentum, field etc. vectors.
     *
     * The arrow is either added as a daughter to the specified mother volume or to
     * the global subgroup "Arrows". Please note: Remove all errors before simulating the
     * geometry.
     *
     * @param position Reference to a TVector3 specifying the start position of the arrow.
     * @param direction Reference to a TVector3 specifying the direction the arrow should point to.
     *                  The length of the direction vector specifies the length of the arrow.
     * @param mother Optional: The mother volume to which the arrow should be added as daughter.
     *               If this value is set to NULL, the arrow is added to the subgroup "Arrows".
     * */
    static void addArrow(const TVector3& position, const TVector3& direction, TGeoVolume* mother = NULL);


  protected:

    static int s_arrowCount; /**< Static variable, counting the number of created arrows. */

  private:

  };

  /**Initializes the static variable counting the number of arrows. */
  int CreatorUtils::s_arrowCount = 0;

}

#endif /* CREATORUTILS_H_ */
