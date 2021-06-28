/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2020 - Belle II Collaboration                        *
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
     * returns True if the environment is MC and False for data
     */
    double isMC(const Particle*);

    /**
     * returns true if event doesn't contain an Y(4S)
     */
    double isContinuumEvent(const Particle*);
    /**
     * returns true if event contains a charged B-meson
     */
    double isChargedBEvent(const Particle*);
    /**
     * returns true if event contains opposite flavor neutral B-mesons
     */
    double isUnmixedBEvent(const Particle*);


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
     * return number of problematic charge 0 tracks in event
     * (these get skipped when making particle lists)
     */
    double nChargeZeroTrackFits(const Particle*);

    /**
     * return legacy total energy in ECL clusters in event as used for Belle 1 analysis
     */
    double belleECLEnergy(const Particle*);

    /**
     * return number of KLM clusters in event
     */
    double nKLMClusters(const Particle*);

    /**
     * return number MCParticles in event
     */
    double nMCParticles(const Particle*);

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
     * return CMS energy
     */
    double getCMSEnergy(const Particle*);

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
     * return x coordinate of the IP used for MC generation
     */
    double getGenIPX(const Particle*);

    /**
     * return y coordinate of the IP used for MC generation
     */
    double getGenIPY(const Particle*);

    /**
     * return z coordinate of the IP used for MC generation
     */
    double getGenIPZ(const Particle*);

    /**
     * return x coordinate of the IP measured on data
     */
    double getIPX(const Particle*);

    /**
     * return y coordinate of the IP measured on data
     */
    double getIPY(const Particle*);

    /**
     * return z coordinate of the IP measured on data
     */
    double getIPZ(const Particle*);

    /**
     * return (i,j) element of the IP covariance matrix measured on data
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
    * return the magnitude of the missing momentum of the event in lab
    */
    double missingMomentumOfEvent(const Particle*);

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
    * return the theta angle of the missing momentum of the event in lab
    */
    double missingMomentumOfEvent_theta(const Particle*);

    /**
    * return the magnitude of the missing momentum of the event in CMS
    */
    double missingMomentumOfEventCMS(const Particle*);

    /**
    * return the generated magnitude of the missing momentum of the event in CMS
    */
    double genMissingMomentumOfEventCMS(const Particle*);

    /**
    * return the x component of the missing momentum of the event in CMS
    */
    double missingMomentumOfEventCMS_Px(const Particle*);

    /**
    * return the y component of the missing momentum of the event in CMS
    */
    double missingMomentumOfEventCMS_Py(const Particle*);

    /**
    * return the z component of the missing momentum of the event in CMS
    */
    double missingMomentumOfEventCMS_Pz(const Particle*);

    /**
    * return the theta angle of the missing momentum of the event in CMS
    */
    double missingMomentumOfEventCMS_theta(const Particle*);

    /**
    * return difference between the total energy of tracks (pion mass hypothesis) and photons in CMS and the beam energy
    */
    double missingEnergyOfEventCMS(const Particle*);

    /**
    * return difference between the total energy of tracks (pion mass hypothesis) and photons in CMS and the beam energy
    * using generator information
    */
    double genMissingEnergyOfEventCMS(const Particle*);

    /**
    * return the missing mass squared
    */
    double missingMass2OfEvent(const Particle*);

    /**
    * return the generated missing mass squared
    */
    double genMissingMass2OfEvent(const Particle*);

    /**
    * return the visible energy of the event
    */
    double visibleEnergyOfEventCMS(const Particle*);

    /**
    * return the generated visible energy of the event
    */
    double genVisibleEnergyOfEventCMS(const Particle*);

    /**
    * return the energy of the photons of all the event
    */
    double totalPhotonsEnergyOfEvent(const Particle*);

    /**
    * return the generated energy of the photons of all the event
    */
    double genTotalPhotonsEnergyOfEvent(const Particle*);

    /**
     * return the event date
     */
    double eventYearMonthDay(const Particle*);

    /**
     * return the event year
     */
    double eventYear(const Particle*);

    /**
     * return the event time
     */
    double eventTimeSeconds(const Particle*);

    /**
     * return the event time fractions remainder
     */
    double eventTimeSecondsFractionRemainder(const Particle*);

    /**
    * return the event T0
    */
    double eventT0(const Particle*);

    /**
    * return the time since the previous trigger in clock ticks (127MHz=RF/4 clock)
    */
    double timeSincePrevTriggerClockTicks(const Particle*);

    /**
    * return the time since the previous trigger in micro seconds
    */
    double timeSincePrevTriggerMicroSeconds(const Particle*);

    /**
    * return number of triggered bunch ranging from 0-1279
    * Note: There are a maximum of 5120 buckets, which could each carry one bunch of e+/e-,
    *       but we only have 1280 clock ticks (=5120/4) to identify the bunches
    */
    double triggeredBunchNumber(const Particle*);

    /**
    * return 1 if an injection happened recently
    */
    double hasRecentInjection(const Particle*);

    /**
    * return the time since the last injection pre-kick signal in clock ticks (127MHz=RF/4 clock)
    * Note: this returns the time without the delay until the injected bunch reaches the detector
    *       (which differs for HER/LER)
    */
    double timeSinceLastInjectionSignalClockTicks(const Particle*);

    /**
    * return the time since the last injection pre-kick signal in micro seconds
    * Note: this returns the time without the delay until the injected bunch reaches the detector
    *       (which differs for HER/LER)
    */
    double timeSinceLastInjectionSignalMicroSeconds(const Particle*);

    /**
    * return 1 for injection in HER, 0 for injection in LER
    */
    double injectionInHER(const Particle*);

    /**
    * return lowest bit of revolution counter
    * Note: Mostly important for PXD; needs ~2 revolutions to read out one frame
    */
    double revolutionCounter2(const Particle*);

  }
} // Belle2 namespace

