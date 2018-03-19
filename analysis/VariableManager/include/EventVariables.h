/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc, Thomas Keck                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * returns true if event doesn't contain an Y(4S)
     */
    double isContinuumEvent(const Particle*);

    /*
     * Variable used as target for Continuum Suppression Training
     * Returns 1.0 if the Event is not a continuum event, else 0.0
    */
    double isNotContinuumEvent(const Particle*);

    /**
     * return number of tracks in event
     */
    double nTracks(const Particle*);

    /**
     * return number of ECL clusters in event
     */
    double nECLClusters(const Particle*);

    /**
     * return number of KLM clusters in event
     */
    double nKLMClusters(const Particle*);

    /**
     * return total energy in ECL clusters in event
     */
    double ECLEnergy(const Particle*);

    /**
     * return total energy in KLM clusters in event
     */
    double KLMEnergy(const Particle*);

    /**
     * return difference between the total energy of tracks and clusters in CMS and the beam energy
     */
    double missingEnergy(const Particle*);

    /**
    * return experiment number
    */
    double expNum(const Particle*);

    /**
    * return event number
    */
    double evtNum(const Particle*);

    /**
    * return run number
    */
    double runNum(const Particle*);

    /**
    * return productionIdentifier
    */
    double productionIdentifier(const Particle*);

    /**
    * return uniqueEventID
    * Constructs a quantity which is different for each event
    * even if experiment and run number are 0
    */
    double uniqueEventID(const Particle*);

    /**
     * return energy of HER
     */
    double getHEREnergy(const Particle*);

    /**
     * return energy of LER
     */
    double getLEREnergy(const Particle*);

    /**
     * return CMS energy
     */
    double getCMSEnergy(const Particle*);

    /**
     * return crossing angle
     */
    double getCrossingAngle(const Particle*);

    /**
     * return nominal beam Px
     */
    double getBeamPx(const Particle*);

    /**
     * return nominal beam Py
     */
    double getBeamPy(const Particle*);

    /**
     * return nominal beam Pz
     */
    double getBeamPz(const Particle*);

    /**
     * return nominal beam energy
     */
    double getBeamE(const Particle*);

    /**
     * return x coordinate of the IP
     */
    double getIPX(const Particle*);

    /**
     * return y coordinate of the IP
     */
    double getIPY(const Particle*);

    /**
     * return z coordinate of the IP
     */
    double getIPZ(const Particle*);

    /**
     * return (i,j) element of the IP covariance matrix.
     */
    double ipCovMatrixElement(const Particle*, const std::vector<double>& element);

    /**
     * return the magnitude of the thrust vector (thrust is defined for all the event)
     */
    double thrustOfEvent(const Particle*);

    /**
    * return the x component of the thrust axis (thrust is defined for all the event)
    */
    double thrustOfEvent_Px(const Particle*);

    /**
    * return the y component of the thrust axis (thrust is defined for all the event)
    */
    double thrustOfEvent_Py(const Particle*);

    /**
    * return the z component of the thrust axis (thrust is defined for all the event)
    */
    double thrustOfEvent_Pz(const Particle*);

    /**
    * return the x component of the missing momentum of the event in lab
    */
    double missingMomentumOfEvent_Px(const Particle*);

    /**
    * return the y component of the missing momentum of the event in lab
    */
    double missingMomentumOfEvent_Py(const Particle*);

    /**
    * return the z component of the missing momentum of the event in lab
    */
    double missingMomentumOfEvent_Pz(const Particle*);

    /**
        * return the x component of the missing momentum of the event in CMS
        */
    double CMSmissingMomentumOfEvent_Px(const Particle*);

    /**
    * return the y component of the missing momentum of the event in CMS
    */
    double CMSmissingMomentumOfEvent_Py(const Particle*);

    /**
    * return the z component of the missing momentum of the event in CMS
    */
    double CMSmissingMomentumOfEvent_Pz(const Particle*);
  }
} // Belle2 namespace

