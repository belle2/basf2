/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDTFSecMap_H
#define VXDTFSecMap_H

// stl:
#include <vector>
#include <string>
#include <utility> // std::pair

// root
#include <TVector3.h>
#include <TObject.h>

namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /** The VXD Track Finder Sector Map Container
   * This class is needed for importing a sector map (lookup-table generated using simulated data).
   */
  class VXDTFSecMap: public TObject {
  public:
    typedef std::pair< double, double> CutoffValue; /**< .first is minValue, .second is maxValue */
    typedef std::pair<unsigned int, CutoffValue> Cutoff; /**< .first is code of filter, .second is CutoffValue */
    typedef std::vector< Cutoff > FriendValue; /**< stores all Cutoffs */
    typedef std::pair<unsigned int, FriendValue > Friend; /**< .first is secID of current Friend, second is FriendValue */
    typedef std::vector< Friend > SectorValue; /**< stores all Friends */
    typedef std::pair<unsigned int, SectorValue> Sector; /**< .first is secID of current sector, second is SectorValue */
    typedef std::vector < Sector > SecMapCopy; /**< stores all Sectors */
    // well, without typedef this would be:
    // vector< pair< unsigned int, vector< pair< unsigned int, vector< pair< unsigned int, pair<double, double> > > > > > > .... hail typedefs -.-

    /** Default constructor for the ROOT IO. */
    VXDTFSecMap():
      m_magneticFieldStrength(1.5),
      m_additionalInfo("") {}

    /** getter - returns full sectorMapInformation */
    const SecMapCopy& getSectorMap() const { return m_sectorMap; }

    /** getter - returns name of sectorMap */
    std::string getMapName() const { return m_nameOfSecMap; }

    /** getter - returns name of detectorType */
    std::string getDetectorType() const { return m_detectorType; }

    /** getter - returns config of the sectors in U direction */
    std::vector<double> getSectorConfigU() const { return m_sectorConfigU; }

    /** getter - returns config of the sectors in V direction */
    std::vector<double> getSectorConfigV() const { return m_sectorConfigV; }

    /** getter - returns the position of the assumed primary vertex */
    TVector3 getOrigin() const { return m_origin; }

    /** getter - returns the strength of magnetic field in Tesla */
    double getMagneticFieldStrength() const { return m_magneticFieldStrength; }

    /** getter - returns some interesting additional info */
    std::string getAdditionalInfo() const { return m_additionalInfo; }



    /** setter - add new index value (currently not used) */
    void setSectorMap(SecMapCopy newMap) { m_sectorMap = newMap; }

    /** setter - set name of sectorMap */
    void setMapName(std::string newName) { m_nameOfSecMap = newName; }

    /** setter - set name of detectorType */
    void setDetectorType(std::string newName) { m_detectorType = newName; }

    /** setter - defines the config of the sectors in U direction, value is valid for each sensor of chosen detector setup, minimum 2 values, all values: 0.0 <= x <= 1.0 */
    void setSectorConfigU(std::vector<double> newUconfig) { m_sectorConfigU = newUconfig; }

    /** setter - defines the config of the sectors in V direction, value is valid for each sensor of chosen detector setup, minimum 2 values, all values: 0.0 <= x <= 1.0 */
    void setSectorConfigV(std::vector<double> newVconfig) { m_sectorConfigV = newVconfig; }

    /** setter - sets the position of the assumed primary vertex */
    void setOrigin(TVector3 origin) { m_origin = origin; }

    /** setter - sets the strength of magnetic field in Tesla, standard is 1.5T */
    void setMagneticFieldStrength(double mField) { m_magneticFieldStrength = mField; }

    /** setter - set some interesting additional info here */
    void setAdditionalInfo(std::string newInfo) { m_additionalInfo = newInfo; }

  protected:
    SecMapCopy m_sectorMap; /**< contains full information of the sectorMap */
    std::string  m_nameOfSecMap; /**< Name of the sectorMap */
    std::string  m_detectorType; /**< Name of the detectorType (PXD, SVD, VXD) */
    std::vector<double> m_sectorConfigU; /**< allows defining the the config of the sectors in U direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
    std::vector<double> m_sectorConfigV; /**< allows defining the the config of the sectors in V direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
    TVector3 m_origin; /**< defines the position of the assumed primary vertex */
    double m_magneticFieldStrength; /**< strength of magnetic field in Tesla, standard is 1.5T */
    std::string m_additionalInfo; /**< this variable is reserved for extra info which shall be stored in the container, e.g. date of production or other useful info for the user(it shall be formatted before storing it), this info will be displayed by the VXDTF on Info-level */

    ClassDef(VXDTFSecMap, 1)
  };

  /** @}*/
} //Belle2 namespace
#endif
