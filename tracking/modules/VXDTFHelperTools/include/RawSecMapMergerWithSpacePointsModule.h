/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>
#include <framework/gearbox/GearDir.h> // needed for reading xml-files
#include "tracking/dataobjects/VXDTFSecMap.h"

// stl:
#include <string>
#include <vector>
#include <list>
#include <map>
#include <utility> // std::pair

//boost:
#include <boost/tuple/tuple.hpp> // a generalized version of pair
#ifndef __CINT__
#include <boost/chrono.hpp>
#endif

namespace Belle2 {

  /** The RawSecMapMergerWithSpacePointsModule
   *
   * this module takes a root file containing a raw sectorMap created by the SecMapTrainerBaseModule
   * and converts it to a sectormap which can be read by the VXDTF. Please check the parameters to be set...
   *
   */
  class RawSecMapMergerWithSpacePointsModule : public Module {

  public:
    typedef boost::chrono::high_resolution_clock
    boostClock; /**< used for measuring time comsumption */ // high_resolution_clock, process_cpu_clock
    //       typedef boost::chrono::microseconds boostNsec; /**< defines time resolution (currently mictroseconds) */ // microseconds, milliseconds
    typedef boost::chrono::nanoseconds boostNsec; /**< defines time resolution (currently nanoseconds) */ // microseconds, milliseconds
    typedef std::pair< double, double> CutoffValue; /**< .first is minValue, .second is maxValue */
    typedef std::pair<unsigned int, CutoffValue> Cutoff; /**< .first is code of filter, .second is CutoffValue */
    typedef std::vector< Cutoff > FriendValue; /**< stores all Cutoffs */
    typedef std::pair<unsigned int, FriendValue > Friend; /**< .first is secID of current Friend, second is FriendValue */
    typedef std::vector< Friend > SectorValue; /**< stores all Friends */
    typedef std::pair<unsigned int, SectorValue> Sector; /**< .first is secID of current sector, second is SectorValue */
    typedef std::vector < Sector > SecMapCopy; /**< stores all Sectors */
    // well, without typedef this would be:
    // vector< pair< unsigned int, vector< pair< unsigned int, vector< pair< unsigned int, pair<double, double> > > > > > > .... hail typedefs -.-

    /**
     * Constructor of the module.
     */
    RawSecMapMergerWithSpacePointsModule();

    /** Destructor of the module. */
    ~RawSecMapMergerWithSpacePointsModule();

    /** Initializes the Module.
     */
    void initialize() override;

    /**
     * Termination action.
     */
    void terminate() override;

    /** safe way of importing cutoff values from the xml-file */
    double getXMLValue(GearDir& quantiles, std::string& valueType, std::string& filterType);

    /** imports sectorMap from XML-file (WARNING: this way is outdated and should not be used any more, please use it only, if really needed!) */
    std::pair<int, int> importXMLMap();


    /** imports sectorMap from ROOT-file  */
    std::pair<int, int> importROOTMap();


  protected:

    boostNsec m_fillStuff; /**< time consumption of the secMap-creation (initialize) */
    SecMapCopy m_fullSectorMapCopy; /**< contains the full information of the xml-file */
    std::vector< Belle2::VXDTFSecMap> m_importedSectorMaps; /**< contains all sectorMaps imported from file */


    /** if true, sectorMap is imported via rootFile, if false, it is imported via XML-file */
    bool m_PARAMimportROOTorXML;


    // ROOTorXML = True
    bool m_PARAMsortByDistance2origin; /**< only needed if importROOTorXML = true: if set to true, sectors are not treated by layerID but by distance to origin. This has an effect on filtering bad sector combinations. Recommendation: use false for normal Belle2-VXD-case and true for beam test situations where layerIDs are mixed up */
    std::string m_PARAMrootFileName; /**< only needed if importROOTorXML = true: sets the root filename */
    bool m_PARAMprintFinalMaps; /**< only needed if importROOTorXML = true:  if true, a complete list of sectors (B2INFO) and its friends (B2DEBUG-1) will be printed on screen */

    bool m_PARMfilterRareCombinations; /**< only needed if importROOTorXML = true:  use this member if you want to steer whether rare sector-friend-combinations shall be filtered or not. Set true if you want to filter these combinations or set false if otherwise. */

    double
    m_PARAMrarenessFilter; /**< only needed if importROOTorXML = true:  use this member if you want to steer whether rare sector-friend-combinations shall be filtered or not, here you can set the threshold for filter. 100% = 1. 1% = 0.01%. Example: if you choose 0.01, all friendsectors which occur less often than in 1% of all cases when main sector was used, are deleted in the friendship-relations", double(0.0)) */

    std::vector<int>
    m_PARAMsampleThreshold; /**< only needed if importROOTorXML = true:  exactly two entries allowed: first: minimal sample size for sector-combination, second: threshold for 'small samples' where behavior is less strict. If sampleSize is bigger than second, normal behavior is chosen */
    std::vector<double>
    m_PARAMsmallSampleQuantiles; /**< only needed if importROOTorXML = true:  behiavior of small sample sizes, exactly two entries allowed: first: lower quantile, second: upper quantile. only values between 0-1 are allowed */
    std::vector<double>
    m_PARAMsampleQuantiles; /**< only needed if importROOTorXML = true:  behiavior of normal sample sizes, exactly two entries allowed: first: lower quantile, second: upper quantile. only values between 0-1 are allowed */
    std::vector<double>
    m_PARAMstretchFactor; /**< only needed if importROOTorXML = true:  exactly two entries allowed: first: stretchFactor for small sample size for sector-combination, second: stretchFactor for normal sample size for sector-combination: WARNING if you simply want to produce wider cutoffs in the VXDTF, please use the tuning parameters there! This parameter here is only if you know what you are doing, since it changes the values in the XML-file directly */


    // ROOTorXML = False
    std::vector<double>
    m_PARAMsectorConfigU; /**< only needed if importROOTorXML = false:  allows defining the the config of the sectors in U direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
    std::vector<double>
    m_PARAMsectorConfigV; /**< only needed if importROOTorXML = false:  allows defining the the config of the sectors in V direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
    std::vector<double>
    m_PARAMsetOrigin; /**< only needed if importROOTorXML = false:  allows to reset orign (e.g. usefull for special cases like testbeams), only valid if 3 entries are found */
    double m_PARAMmagneticFieldStrength; /**< only needed if importROOTorXML = false:  strength of magnetic field in Tesla, standard is 1.5T */
    std::string m_PARAMdetectorType; /**< only needed if importROOTorXML = false:  defines which detector type has to be exported */
    std::string
    m_PARAMsectorSetup; /**< only needed if importROOTorXML = false:  lets you chose the sectorSetup (compatibility of sensors, individual cutoffs,...) */
    std::string
    m_PARAMadditionalInfo; /**< only needed if importROOTorXML = false:  this variable is reserved for extra info which shall be stored in the container, e.g. date of production or other useful info for the user(it shall be formatted before storing it), this info will be displayed by the VXDTF on Info-level */




  private:

  };

}
