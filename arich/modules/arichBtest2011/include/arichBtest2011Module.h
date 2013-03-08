
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHBTEST2011MODULE_H
#define ARCIHBTEST2011MODULE_H

#include <zlib.h>

#include <framework/core/Module.h>

#include <string>
#include <vector>
#include <time.h>

class TFile;
class TVector3;

namespace Belle2 {

  //class MCParticle;
  class  ARICHTracking;

  /** The UserTutorial module.
   * Prints something
   * as tree using the B2INFO message to the logging system.
   */
  class arichBtest2011Module : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    arichBtest2011Module();

    /** Destructor. */
    virtual ~arichBtest2011Module() {}

    //! Initialize the Module
    /*! Function is called only once at the beginning of your job at the beginning of the corresponding module.
        Things that can be done here, should be done here, as it is relatively cheap in terms of CPU time.
    */
    virtual void initialize();

    //! Called when entering a new run
    /*! At the beginning of each run, the function gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun();

    //! Running over all events
    /*! Function is called for each evRunning over all events
        This means, this function is called very often, and good performance of the code is of strong interest.
    */
    virtual void event();

    //! Is called after processing the last event of a run
    /*! Good e.g. for storing stuff, that you want to aggregate over one run.
     */
    virtual void endRun();

    //! Is called at the end of your Module
    /*! Function is called only once at the end of your job at the end of the corresponding module.
        This function is for cleaning up, closing files, etc.
    */
    virtual void terminate();

  protected:



    int skipdata(gzFile fp);
    void readmwpc(unsigned int* dbuf, unsigned int len);
    int readhapd(unsigned int len, unsigned int* data);
    int readdata(gzFile fp, int rec_id, int print);
    gzFile m_fp;
    int m_end;
    int m_events;
    TFile* m_file;
    /** Loops recursively over the MCParticle list and prints information about each particle.
     * @param mc Reference to the MCParticle whose information should be printed and whose daughters should be visited.
     * @param level The current level of the recursive call of the method.
     */
    //void printTree(const MCParticle& mc, int level = 0);
    std::vector<std::string> m_runList; /**< The filenames of the runs */
    std::vector<int> m_eveList; /**< The eventnumbers for each of the runs */
    std::vector<std::string>::iterator  m_runCurrent;
    std::string m_outFile;
    std::string m_geometry;
    std::vector<int> m_MwpcTrackMask;
    time_t m_time;
    time_t m_timestart;
    ARICHTracking* m_mwpc;
    int m_tdc[32];
    int getTrack(int mask, TVector3& r, TVector3& dir);
    //std::vector<bool> m_seen;   /**< Tag the particles which were already visited using their index. */
    //bool m_onlyPrimaries;       /**< Print only primary particles. */
    //int m_maxLevel;             /**< Show only up to specified depth level. */
  };

} // end namespace Belle2

#endif // ARICHBTEST2011MODULE_H
