/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCLEGENDRETRACKINGMODULE_H
#define CDCLEGENDRETRACKINGMODULE_H

#include <framework/core/Module.h>
#include <fstream>

#include <boost/tuple/tuple.hpp>
#include <vector>

namespace Belle2 {

  /** CDC tracking module, using Legendre transformation of the drift time circles.
   * This is a module, performing tracking in the CDC. It is based on the paper "Implementation of the Legendre Transform for track segment reconstruction in drift tube chambers" by T. Alexopoulus, et al. NIM A592 456-462 (2008)
   *  \addtogroup modules
   *  @{
   *  \addtogroup tracking
   *  @{
   */

  class CDCLegendreTrackingModule: public Module {

  public:

    /** Constructor.
     *  Create and allocate memory for variables here. Add the module parameters in this method.
     */

    CDCLegendreTrackingModule();

    /** Destructor.
     * Use the destructor to release the memory you allocated in the constructor.
     */

    virtual ~CDCLegendreTrackingModule();

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

    std::string m_cdcHitsColName; /**< Input digitized hits collection name (output of CDCDigitizer module) */
    std::string m_cdcLegendreTrackCandsColName; /**< Output tracks collection name*/
    std::string m_gfTrackCandsColName; /**< Output genfit track candidates collection name*/

    int m_nTracks; /**< Counter for the number of found tracks*/

    int m_nbinsTheta; /**< Number of bins in theta direction in the Legendre plane, parameter of the module*/
    int m_nbinsR; /**< Number of bins in r direction in the Legendre plane, parameter of the module*/
    double m_distTheta; /**< Minimal distance of two peaks in theta direction, parameter of the module*/
    double m_distR; /**< Minimal distance of two peaks in t direction, parameter of the module*/
    int m_distThetaInBins; /**< Minimal distance of two peaks in theta direction in bins*/
    int m_distRInBins; /**< Minimal distance of two peaks in t direction ins bins*/
    double m_threshold; /**< Threshold for votes in the legendre plane, parameter of the module*/
    double m_thresholdUnique; /**< Threshold of unique TrackHits for track building*/

    double m_resolutionAxial; /**< Total resolution, used for the assignment of axial hits to tracks*/
    double m_resolutionStereo; /**< Total resolution, used for the assignment of stereo hits to tracks*/

    double m_rMin; /**< Minimum in r direction, initialized in initializer list of the module*/
    double m_rMax; /**< Maximum in r direction, initialized in initializer list of the module*/

    const static double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/

    double* sin_theta; /**< Lookup array for calculation of sin*/
    double* cos_theta; /**< Lookup array for calculation of cos*/

    /** Calculate bin of r for given value.
    * Small helper function to calculate the bin of the r variable in the Legendre plane.
    * @param r continuous value of r
    * @return bin number of given value r
    */
    inline int calcBin(double r);


    /** Calculate center of given bin
    * Small helper function to calculate the center of a bin with given maximum, minimum and number of bins.
    * @param bin Number of bin
    * @param min minimal value
    * @param max maximal value
    * @param nbins total number of bins
    * @return center of given bin
    */
    inline double getBinCenter(int bin, double min, double max, int nbins);


    /** Sort hits for fitting.
     * This method sorts hit indices to bring them in a correct order, which is needed for the fitting
     * @param hitIndices vector with the hit indices, this vector is charged within the function.
     * @param CDCLegendreTrackHits name of the CDCTrackHits array. In this way the sort funtion can get all necessary information about the hits.
     * @param charge estimated charge of the track, which is needed for hits from the same layer to be ordered correctly.
     */
    void sortHits(std::vector<int> & hitIndices,
                  std::string CDCLegendreTrackHits, double charge);

    /**
     *
     */
    static bool tupleComp(boost::tuple<int, double, int, double> tuple1,
                          boost::tuple<int, double, int, double> tuple2);

    std::vector<std::pair<int, int> > DoTrackFinding();

    void createLegendreTrackCandidates(
      std::vector<std::pair<int, int> > tracks);

    void createGFTrackCandidates();
    /** Creates GeantFit Track Candidates from CDCLegendreTrackCandidates */

  };

//helper functor to sort track candidates by votes
  struct SortCandidatesByVotes {
    bool operator()(const std::pair<int, std::pair<int, int> > &lhs,
                    const std::pair<int, std::pair<int, int> > &rhs) const;
  };/**@} @}*/

} // end namespace Belle2
#endif

