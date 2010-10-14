/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCDIGI_H
#define CDCDIGI_H

#include <cdc/modules/cdcDigitizer/CDCSignal.h>

#include <framework/core/Module.h>
#include <string>
//#include <boost/shared_ptr.hpp>

#include <cdc/hitcdc/SimHitCDC.h>

#include <vector>
#include <queue>
#include <map>

#include "TRandom.h"

namespace Belle2 {

//! Typedefs
  typedef       std::vector<SimHitCDC*>              SimHitCDCVec;
  typedef       std::map< int, CDCSignal *>          CDCSignalMap; // Cell ID, signal
  typedef       std::pair< int, CDCSignal *>         vpair; // pair of CDCSignalMap
  typedef const std::vector< std::string >           ConstStringVec;
  typedef       std::vector< std::string >           StringVec;

//! The Class for Detailed Digitization of CDC
  /*! This class is a CDC digitizer.
      Now, I only use a simple gaussian function to smear drift length, and
      the sigma is spatial resolution which can be set by users.
      If there are two or more hits in one cell, only the shortest drift length is selected.
      And the signal amplitude is the sum of all hits deposited energy in this cell.

      More details will be considered soon, like X-T function, transfer time and so on.
  */

  class CDCDigi : public Module {

  public:

    //! Constructor
    CDCDigi();

    //! Destructor
    virtual ~CDCDigi();

    //! Initialize the Module
    /*! This method is called at the beginning of data processing - used for initialization
    */
    virtual void initialize();

    //! Called when entering a new run
    /*! At the beginning of each run, the function gives you the chance to change run dependent constants like alignment parameters, etc.

        Used for run header processing
    */
    virtual void beginRun();

    //! Running over all events
    /*! Function is called for each evRunning over all events
        This means, this function is called very often, and good performance of the code is of strong interest.

        The method to process a event digitization.
    */
    virtual void event();

    //! Is called after processing the last event of a run
    /*! Good e.g. for storing stuff, that you want to aggregate over one run.
    */
    virtual void endRun();

    //! Is called at the end of your Module
    /*! Function is called only once at the end of your job at the end of the corresponding module.
        This function is for cleaning up, closing files, etc.

        The method to terminate current module.
    */
    virtual void terminate();

  protected:


    //! Method used to smear drift length
    /*!
        \param driftLength The value of drift length.
        \param fraction Fraction of the first Gaussian used to smear drift length.
        \param mean1 Mean value of the first Gassian used to smear drift length.
        \param resolution1 Resolution of the first Gassian used to smear drift length.
        \param mean2 Mean value of the second Gassian used to smear drift length.
        \param resolution2 Resolution of the second Gassian used to smear drift length.

        \return Drift length after smearing.
    */
    double smearDriftLength(double driftLength, double fraction, double mean1, double resolution1, double mean2, double resolution2);

    //! Method to add noise to pure mc signal
    /*! Method generating random noise using Gaussian distribution and add this effect
        to the final results.
        \param cdcSignalMap A map is used to store CDC signals and will be returned after adding noise.
    */
    void genNoise(CDCSignalMap & cdcSignalMap);

    //! The method to get drift time based on drift length
    /*! In this method, X-T function will be used to calculate drift time.
        \param driftLength The value of drift length.

        \return Drift time.
    */
    double getDriftTime(double driftLength);

    //! The method to print hit information
    /*!
        \param hit A CDC hit.
    */
    void printSimCDCHitInfo(const SimHitCDC & hit) const;

    //! The method to print hits information
    /*!
        \param info Extra information.
        \param hitVec A vector to store hits.
    */
    void printSimCDCHitsInfo(std::string info, const SimHitCDCVec & hitVec) const;

    //! The method to print module parameters
    void printModuleParams() const;

    //! The method to print CDC signal information
    /*!
        \param info Extra information
        \param cdcSignalMap A map to store CDC signals.
    */
    void printCDCSignalInfo(std::string info, const CDCSignalMap & cdcSignalMap) const;


    std::string m_inColName;         /*!< Input collection name */
    std::string m_outColName;        /*!< Output collection name */
    std::string m_relColNamePlsToMC; /*!< Relation collection name - cdc signal (Digit)  <-> MCParticle */
    std::string m_relColNameMCToSim; /*!< Relation collection name - MCParticle        <-> SimTrkHit */

    double m_fraction;          /*!< Fraction of the first Gaussian used to smear drift length */
    double m_mean1;             /*!< Mean value of the first Gassian used to smear drift length */
    double m_resolution1;       /*!< Resolution of the first Gassian used to smear drift length */
    double m_mean2;             /*!< Mean value of the second Gassian used to smear drift length */
    double m_resolution2;       /*!< Resolution of the second Gassian used to smear drift length */

    int   m_electronicEffects;       /*!< Add noise? */
    double m_elNoise;                 /*!< Noise added to the signal */

    int   m_randomSeed;   /*!< Random number seed */

    TRandom * m_random;   /*!< Random number generator - Gaussian distribution */

  private:

    double m_timeCPU;                /*!< CPU time     */
    int    m_nRun;                   /*!< Run number   */
    int    m_nEvent;                 /*!< Event number */
  };

} // end of Belle2 namespace

#endif // CDCDIGI_H
