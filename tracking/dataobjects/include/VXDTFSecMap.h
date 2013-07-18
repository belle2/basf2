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
    VXDTFSecMap() {}

    /** getter - returns full sectorMapInformation */
//    const std::vector< std::pair< unsigned int, std::vector< std::pair< unsigned int, std::vector< std::pair< unsigned int, std::pair<double, double> > > > > > >& getSectorMap() const { return m_sectorMap; }
    const SecMapCopy& getSectorMap() const { return m_sectorMap; }

    /** getter - returns name of sectorMap */
    std::string getMapName() { return m_nameOfSecMap; }

    /** setter - add new index value (currently not used) */
//    void setSectorMap(std::vector< std::pair< unsigned int, std::vector< std::pair< unsigned int, std::vector< std::pair< unsigned int, std::pair<double, double> > > > > > > newMap) { m_sectorMap = newMap; }
    void setSectorMap(SecMapCopy newMap) { m_sectorMap = newMap; }

    /** setter - set name of sectorMap */
    void setMapName(std::string newName) { m_nameOfSecMap = newName; }

  protected:
//    std::vector< std::pair< unsigned int, std::vector< std::pair< unsigned int, std::vector< std::pair< unsigned int, std::pair<double, double> > > > > > > m_sectorMap; /**< contains full information of the sectorMap */
    SecMapCopy m_sectorMap; /**< contains full information of the sectorMap */
    std::string  m_nameOfSecMap; /**< Name of the sectorMap */

    ClassDef(VXDTFSecMap, 1)
  };

  /** @}*/
} //Belle2 namespace
#endif

// LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/jkl/DocFW/genfit2/genfit2/lib scons --extra-libpath=/home/jkl/DocFW/genfit2/genfit2/lib --extra-ccflags='-I/home/jkl/DocFW/genfit2/genfit2/include' --symlink
