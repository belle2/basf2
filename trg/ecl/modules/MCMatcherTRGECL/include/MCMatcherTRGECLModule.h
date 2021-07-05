/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef MCMATCHERTRGECLMODULE_H_
#define MCMATCHERTRGECLMODULE_H_

//basf2 framework headers
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include "trg/ecl/TrgEclMapping.h"
#include "trg/ecl/dataobjects/TRGECLDigi0MC.h"
#include "trg/ecl/dataobjects/TRGECLHitMC.h"

//C++/C standard lib elements.
#include <map>

namespace Belle2 {
  //namespace ECL {

  /** Class to represent the hit of one cell */
  class MCMatcherTRGECLModule : public Module {

  public:
    /** Constructor.*/
    MCMatcherTRGECLModule();

    /** Destructor.*/
    virtual ~MCMatcherTRGECLModule();

    /** Copy constructor, deleted. */
    MCMatcherTRGECLModule(MCMatcherTRGECLModule&) = delete;

    /** Assignment operator, deleted. */
    MCMatcherTRGECLModule& operator=(MCMatcherTRGECLModule&) = delete;

    /** Initialize variables, print info, and start CPU clock. */
    virtual void initialize() override;

    /** Nothing so far.*/
    virtual void beginRun() override;

    /** Actual digitization of all hits in the ECL.
     *
     *  The digitized hits are written into the DataStore.
     */
    virtual void event() override;

    /** Nothing so far. */
    virtual void endRun() override;

    /** Stopping of CPU clock.*/
    virtual void terminate() override;

    /** define a map for Primary Track*/
    typedef std::map< int, int>  PrimaryTrackMap;
    /** define a multimap for hits in showers */
    typedef std::multimap< int, int> MultiMap;


  private:
    StoreArray<TRGECLDigi0MC> m_trgECLDigi0MC; /**< output for TRGECLDigi0MC */
    StoreArray<TRGECLHitMC> m_trgECLHitMC;  /**< output for TRGECLHitMC */



    /** CPU time     */
    double m_timeCPU = 0.;
    /** Run number   */
    int    m_nRun = 0;
    /** Event number */
    int    m_nEvent = 0;
    /** object of TC Mapping*/
    TrgEclMapping* _TCMap;
    /** TCId  */
    int TCId[100];
    /**TC raw energy */
    double TCRawEnergy[100];
    /** TC raw timing */
    double TCRawTiming[100];
    /** Signal Contibution in a TC*/
    double SignalContribution[100];
    /** Backgroun Contribution in a TC  */
    double BKGContribution[100];
    /** Energy of maximum contribtion particle  */
    double maxEnergy[100][3] ;
    /**  particles contribution  */
    double contribution[100][3];
    /** Primary Index in TC hit  */
    int TCPrimaryIndex[100][3] ;
    /** XtalId in TC  */
    int XtalId[100][3] ;
    /** Momentum X of particle */
    double px[100][3] ;
    /** Momentum Y of particle */
    double py[100][3] ;
    /** Momentum Z of particle */
    double pz[100][3] ;
    /** Track Id */
    int trackId[100][3];
    /** Beam background tag */
    int background_tag[100][3];
    /** Particle ID */
    int pid[100][3];
    /** Mother ID   */
    int mother[100][3]  ;
    /** Grand mother ID   */
    int gmother[100][3] ;
    /** Grand Grand Mother ID   */
    int ggmother[100][3] ;
    /** Grand Grand Grand Mother ID   */
    int gggmother[100][3] ;

    /** Raw Energy of particle    */
    double MCEnergy[100][3];
    /** eclhit id  */
    int ieclhit[100][3];
    /** TCId  */
    int TCIdHit[100];
    /** TC Hit energy  */
    double TCHitEnergy[100];
    /** TC Hit Timking  */
    double TCHitTiming[100];
    /** Primary Index in TC hit  */
    int TCPrimaryIndexHit[100][3] ;
    /** XtalId in TC  */
    int XtalIdHit[100][3] ;
    /** Momentum X of particle */
    double pxHit[100][3] ;
    /** Momentum X of particle */
    double pyHit[100][3] ;
    /** Momentum X of particle */
    double pzHit[100][3] ;
    /** Track Id */
    int trackIdHit[100][3];
    /** Beam background tag */
    int background_tagHit[100][3];
    /** Particle ID */
    int pidHit[100][3];
    /** Mother ID   */
    int motherHit[100][3]  ;
    /** Grand mother ID   */
    int gmotherHit[100][3] ;
    /** Grand Grand Mother ID   */
    int ggmotherHit[100][3] ;
    /** Grand Grand Grand Mother ID   */
    int gggmotherHit[100][3] ;
    /** Raw Energy of particle    */
    double MCEnergyHit[100][3];
    /** Signal Contibution in a TC*/
    double SignalContributionHit[100];
    /** Backgroun Contribution in a TC  */
    double BKGContributionHit[100];
    /**  particles contribution  */
    double contributionHit[100][3];


  };
  //} //TRGECL
} // end of Belle2 namespace

#endif // TRGECLDIGI_H
