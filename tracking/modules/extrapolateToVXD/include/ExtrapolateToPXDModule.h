/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef EXTRAPOLATETOPXDMODULE_H
#define EXTRAPOLATETOPXDMODULE_H

#include <framework/core/Module.h>
#include <fstream>

namespace Belle2 {

  /** Module to extrapolate Tracks found in the CDC and SVD to PXD.
   *  This module needs as input GFTracks created by fitting CDC+SVD Hits.
   *  This module is basically very similar to the ExtrapolateToSVDModule.
   *  The search for corresponding PXDHits is perfomed layer wise.
   *  For each layer the GFTrack is extrapolated to each PXDHit and the distance between the PXDHit and the found point of closest approach is considered.
   *  The PXDHit with the shortest distance is taken (one hit per layer).
   *  The output are GFTrackCands, which are basically copies of GFTrackCands from GFTracks, but with new momentum seed (fit result) and the additional PXDHits.
   *  @todo So far the module only proves that the principle is working, there is plenty of room for improvement (preselection of PXDHits to be considered, more elaborate criteria to assign hits, possibility to have 2 PXDHits per layer etc)
   */

  class ExtrapolateToPXDModule : public Module {

  public:

    /** Constructor .
     */
    ExtrapolateToPXDModule();

    /** Destructor.
     */
    virtual ~ExtrapolateToPXDModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    virtual void initialize();

    /** Called when entering a new run.
     */
    virtual void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    virtual void event();

    /** This method is called if the current run ends.
     */
    virtual void endRun();

    /** This method is called at the end of the event processing.
     */
    virtual void terminate();


  protected:


  private:

    std::string m_pxdHitsColName;                    /**< PXDHits collection name */
    std::string m_svdHitsColName;                    /**< SVDHits collection name */
    std::string m_cdcHitsColName;                    /**< CDCHits collection name */
    std::string m_gfTrackCandsColName;               /**< GFTrackCandidates collection name */
    std::string m_gfTracksColName;                   /**< GFTracks collection name */

    bool m_textFileOutput;                           /**< Boolean to mark if text output files with hit coordinates should be created*/
    std::ofstream Hitsfile;                          /**< Simple text file to write out the coordinates of the digitized hits*/
    std::ofstream Tracksfile;                        /**< Simple text file to write out the coordinates of the digitized hits ordered by their belonging to a track candidate*/

  };
} // end namespace Belle2


#endif /* EXTRAPOLATETOPXDMODULE_H */

