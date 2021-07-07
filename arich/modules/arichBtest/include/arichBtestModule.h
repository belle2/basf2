
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHBTESTMODULE_H
#define ARICHBTESTMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <vector>
#include <time.h>
#include <zlib.h>

class TFile;
class TVector3;

namespace Belle2 {

  //class MCParticle;
  class  ARICHTracking;

  /** The UserTutorial module.
   * Prints something
   * as tree using the B2INFO message to the logging system.
   */
  class arichBtestModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    arichBtestModule();

    /** Destructor. */
    virtual ~arichBtestModule() {}

    //! Initialize the Module
    /*! Function is called only once at the beginning of your job at the beginning of the corresponding module.
        Things that can be done here, should be done here, as it is relatively cheap in terms of CPU time.
    */
    virtual void initialize() override;

    //! Called when entering a new run
    /*! At the beginning of each run, the function gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun() override;

    //! Running over all events
    /*! Function is called for each evRunning over all events
        This means, this function is called very often, and good performance of the code is of strong interest.
    */
    virtual void event() override;

    //! Is called after processing the last event of a run
    /*! Good e.g. for storing stuff, that you want to aggregate over one run.
     */
    virtual void endRun() override;

    //! Is called at the end of your Module
    /*! Function is called only once at the end of your job at the end of the corresponding module.
        This function is for cleaning up, closing files, etc.
    */
    virtual void terminate() override;

  protected:


    //! Skip the data part of the record
    int skipdata(gzFile fp);
    //! Read the MWPC information from the data buffer
    void readmwpc(unsigned int* dbuf, unsigned int len, int print = 0);
    //! Read the HAPD hits from the data buffer
    int readhapd(unsigned int len, unsigned int* data);
    //! Read the data from the file (can be compressed)
    int readdata(gzFile fp, int rec_id, int print);
    //! file desriptor of the data file
    gzFile m_fp;
    //! EOF flag
    int m_end;
    //! number of events
    int m_events;
    //! pointer to the root file
    TFile* m_file;

    std::vector<std::string> m_runList; /**< The filenames of the runs */
    std::vector<int> m_eveList;         /**< The eventnumbers for each of the runs */
    //! current run
    std::vector<std::string>::iterator  m_runCurrent;
    //! output file name
    std::string m_outFile;
    //! Bit mask of the MWPC tracking chambers used for the track creation
    std::vector<int> m_MwpcTrackMask;
    //! time of the first processed event
    time_t m_timestart;
    //! Pointer to the tracking chambers
    ARICHTracking* m_mwpc;
    //! raw MWPC TDC buffer
    int m_tdc[32];
    //! Beamtest Track reconstruction
    int getTrack(int mask, TVector3& r, TVector3& dir);

  private:

    //! Momentum of the particles in the beam [GeV].
    double m_beamMomentum;
  };

} // end namespace Belle2

#endif // ARICHBTESTMODULE_H
