/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef ARICHPHOTON_H
#define ARICHPHOTON_H

namespace Belle2 {

  /**
   * Struct for ARICH reconstructed photon (digit related to track) information
   */
  struct ARICHPhoton {
    /**
     * Constructor to allow initialization
     * @param thetaCer    reconstructed cherenkov angle theta
     * @param phiCer      reconstructed cherenkov angle phi
     * @param aergel      assumed aerogel layer of photon emission point
     * @param mirror      assumed mirror plane of photon reflection
     */
    ARICHPhoton(float thetaCer = 0, float phiCer = 0, int aerogel = 0, int mirror = 0): thetaCer(thetaCer), phiCer(phiCer),
      aerogel(aerogel), mirrorId(mirror) {};
    float thetaCer;  /**< reconstructed theta angle */
    float phiCer;    /**< reconstructed phi angle */
    int aerogel;     /**< assumed aerogel layer of photon emission */
    int mirrorId;    /**< assumed reflection of mirror plate */
    //float lkhCont;   /**<
  };
} //Belle2 namespace

#endif // ARICHPHOTON_H
