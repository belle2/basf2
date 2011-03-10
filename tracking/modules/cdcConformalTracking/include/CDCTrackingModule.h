/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCTRACKINGMODULE_H
#define CDCTRACKINGMODULE_H

#include <framework/core/Module.h>
#include <fstream>

namespace Belle2 {

  /** Module to perform pattern recognition in the CDC through conformal transformation.
   * The CDCTrackingModule performs the first pattern recognition step in the CDC through conformal transformation of hit coordinates.
   * First Digitized CDCHits (CDCDigi module should be executed before this module) are combined to segments.
   * Then segments from axial superlayers are combined to track candidates.
   * In the following step stereo segments are assigned to these candidates.
   */

  class CDCTrackingModule : public Module {

  public:

    /** Constructor.
     *  Create and allocate memory for variables here. Add the module parameters in this method.
     *  \param selfRegisterType True if this module is self-registering, otherwise false.
     */

    CDCTrackingModule();

    /** Destructor.
     * Use the destructor to release the memory you allocated in the constructor.
     */


    virtual ~CDCTrackingModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     * Use this method to initialize variables, open files etc.
     */

    virtual void initialize();

    /** Called when entering a new run;
     * Called at the beginning of each run, the method gives you the chance to change run dependent constants like alignment parameters, etc.
     */

    virtual void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */

    virtual void event();

    /** This method is called if the current run ends.
     * Use this method to store information, which should be aggregated over one run.
     */

    virtual void endRun();

    /** This method is called at the end of the event processing.
     *  Use this method for cleaning up, closing files, etc.
     */

    virtual void terminate();

  protected:


  private:

    std::string m_inCDCSimHitsColName;              /**< Input simulated hits collection name (should already be used by the CDCDigitizer module) */
    std::string m_inCDCHitsColName;                 /**< Input digitized hits collection name (output of CDCDigitizer module) */
    std::string m_outCDCTrackCandsColName;          /**< Output tracks collection name*/
    std::string m_inCDCRecoHitsColName;             /**< Input reco hits collection name (needed for relation creation)*/
    std::string m_outCDCTrackCandsToRecoHits;       /**< Output relation (track candidates to cdc recohits) name */

    bool m_textFileOutput;                          /**< Boolean to create output text files with hit coordinates (needed for development purposes, wont be needed later on)*/

    std::ofstream SimHitsfile;                      /**< Simple text file to write out the coordinates of the simulated hits*/
    std::ofstream Hitsfile;                         /**< Simple text file to write out the coordinates of the digitized hits*/
    std::ofstream ConfHitsfile;                     /**< Simple text file to write out the coordinates of the digitized hits in the conformal plane*/
    std::ofstream Tracksfile;                       /**< Simple text file to write out the coordinates of the digitized hits ordered by their belonging to a track candidate*/
    std::ofstream ConfTracksfile;                   /**< Simple text file to write out the coordinates of the digitized hits in the conformal plane ordered by their belonging to a track candidate*/


  };
} // end namespace Belle2
#endif

