/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <vector>
#include <TFile.h>
#include <TTree.h>
#include <TRotation.h>
#include <generators/SAD/ReaderSAD.h>

namespace Belle2 {

  /**
   * Beam background generator based on SAD files
   */
  class BeamBkgGeneratorModule : public Module {

  public:

    /**
     * Constructor
     */
    BeamBkgGeneratorModule();

    /**
     * Destructor
     */
    virtual ~BeamBkgGeneratorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

  private:

    /**
     * Pick up particle randomly from the SAD file according to its rate
     * @return TTree entry number
     */
    int generateEntry() const;

    /**
     * Structure of the TTree in the SAD file
     */
    struct SADTree {
      double s = 0;  /**< lost position measured from IP along the ring [m] */
      double x = 0;  /**< x at lost position [m] */
      double px = 0; /**< px at lost position [GeV] */
      double y = 0;  /**< y at lost position [m] */
      double py = 0; /**< py at lost position [GeV] */
      double E = 0;  /**< E at lost position [GeV] (in fact momentum magnitude!) */
      double rate = 0; /**< lost rate [Hz] */
      double ss = 0; /**< scattered position (|s|<Ltot/2) [m] */
      int nturn = 0; /**< number of turns from scattered to lost */
      double sraw = 0; /**< s at lost position [m] before matching G4 beam pipe inner surface */
      double ssraw = 0; /**< scattered position [m] */
      double xraw = 0; /**< x at lost position [m] before matching G4 beam pipe inner surface */
      double yraw = 0; /**< y at lost position [m] before matching G4 beam pipe inner surface */
      double r = 0; /**< sqrt(x*x+y*y) [m] */
      double rr = 0; /**< sqrt(xraw*xraw+yraw*yraw) [m] */
      double dp_over_p0 = 0; /**< momentum deviation of the lost particle */
      double watt = 0; /**< loss wattage [W] */
    };

    std::string m_fileName; /**< name of the SAD file converted to root */
    std::string m_treeName; /**< name of the TTree in the SAD file */
    std::string m_ringName; /**< name of the superKEKB ring (LER or HER) */
    double m_realTime = 0;  /**< equivalent SuperKEKB running time in [ns] */

    TFile* m_file = 0;  /**< root file pointer */
    TTree* m_tree = 0;  /**< root tree pointer */
    SADTree m_sad;      /**< TTree entry data */
    TRotation m_rotation; /**< rotation from SAD to Belle II frame */
    ReaderSAD m_readerSAD;     /**< the transformation from SAD to Belle II system for the far beamline */
    int m_ring = 0 ; /**< ring number, 1-HER, 2-LER */
    std::vector<int> m_sectionOrdering; /**< superKEKB section ordering */

    std::vector<double> m_rates; /**< cumulative rates of SAD particles [Hz] */
    int m_numEvents = 0 ; /**< number of events to generate */
    int m_eventCounter = 0; /**< event counter */
    std::vector<int> m_counters; /**< counters: how many times SAD particles are used */

  };

} // Belle2 namespace

