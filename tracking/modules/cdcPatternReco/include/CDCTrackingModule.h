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

#include <boost/tuple/tuple.hpp>

namespace Belle2 {
  /**
   *  \addtogroup modules
   *  @{
   *  \addtogroup tracking_modules
   *  \ingroup modules
   *  @{ CDCTrackingModule @} @}
   */


  /** Module to perform pattern recognition in the CDC through conformal transformation.
   *
   *  The CDCTrackingModule performs pattern recognition in the CDC through conformal transformation of hit coordinates.
   *  First Digitized CDCHits (CDCDigi module should be executed before this module) are combined to segments.
   *  Then segments from axial superlayers are combined to track candidates.
   *  In the following step stereo segments are assigned to these candidates.
   *  As output GFTrackCands are created, which can be directly passed to GenFit.
   */
  class CDCTrackingModule : public Module {

  public:

    /** Constructor.
     *  Create and allocate memory for variables here. Add the module parameters in this method.
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

    /** This method sorts hit indices to bring them in a correct order, which is needed for the fitting
     *  First parameter is a vector with the hit indices, this vector is charged within the function.
     *  Second parameter is the name of the CDCTrackHits array. In this way the sort funtion can get all necessary information about the hits.
     *  Third parameter is the estimated charge of the track, which is needed for hits from the same layer to be ordered correctly.
     */
    static void sortHits(std::vector<int> & hitIndices, std::string CDCTrackHits, double charge);

    /** This method is a comparison function used in the sortHits() function.
     *  This method is there to compare 4-tuples with <hitId, rho, wireId, charge>.
     *  It this way also hits from the same layer can be ordered.
     */
    static bool tupleComp(boost::tuple<int, double, int, double> tuple1, boost::tuple<int, double, int, double> tuple2);

  protected:


  private:

    std::string m_cdcSimHitsColName;              /**< Input simulated hits collection name (should already be created by the CDCSensitiveDetector module) */
    std::string m_cdcHitsColName;                 /**< Input digitized hits collection name (output of CDCDigitizer module) */
    std::string m_cdcTrackCandsColName;           /**< Output tracks collection name*/
    std::string m_gfTrackCandsColName;            /**< Output genfit track candidates collection name*/

    int m_nTracks;                                /**< Counter for the number of found tracks*/

    bool m_textFileOutput;                          /**< Boolean to create output text files with hit coordinates (needed for development purposes, wont be needed later on)*/

    std::ofstream SimHitsfile;                      /**< Simple text file to write out the coordinates of the simulated hits*/
    std::ofstream Hitsfile;                         /**< Simple text file to write out the coordinates of the digitized hits*/
    std::ofstream ConfHitsfile;                     /**< Simple text file to write out the coordinates of the digitized hits in the conformal plane*/
    std::ofstream Tracksfile;                       /**< Simple text file to write out the coordinates of the digitized hits ordered by their belonging to a track candidate*/
    std::ofstream ConfTracksfile;                   /**< Simple text file to write out the coordinates of the digitized hits in the conformal plane ordered by their belonging to a track candidate*/



  };
} // end namespace Belle2
#endif


