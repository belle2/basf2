/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// includes - rootStuff:
// includes - stl:
#include <vector>
#include <string>

// includes - tf-related stuff
// includes - general fw stuff
#include <framework/datastore/RelationsObject.h>


namespace Belle2 {


  /** forward declaration for the ActivatedSector */
  class ActivatedSector;


  /** forward declaration for the SectorFriendship */
  class SectorFriendship;


  /** Sector is a central part of storing information for VXD trackFinders.
   *
   * - is created by the ExportSectorMapModule, where relations between Sectors are set
   * - is linked (1:1) to an ActivatedSector
   * - carries SectorFriendships which link sectors and store Filters
   * - and the SegmentMaker which takes a related ActivatedSector (socalled FriendSector) to create segments
   **/
  class Sector : public RelationsObject {
  public:


    /** constructor */
    Sector():
      m_myActiveSector(NULL),
      m_sectorID(0),
      m_distance2Origin(0),
      m_useDistance4sort(false)
    {}


    /** useful constructor for cases where sectors are treated by fullSecID (parameter 1) */
    explicit Sector(unsigned int secID):
      m_myActiveSector(NULL),
      m_sectorID(secID),
      m_distance2Origin(0),
      m_useDistance4sort(false)
    {}


    /** useful constructor both cases of sector sorting:
     * sectors treated by fullSecID (parameter 1) and distance2origin (parameter 2) if you use this constructor,
     * sorting by distance is activated automatically but can be set by parameter 3
     */
    Sector(unsigned int secID, float distance2origin, bool sortByDistance = true):
      m_myActiveSector(NULL),
      m_sectorID(secID),
      m_distance2Origin(distance2origin),
      m_useDistance4sort(sortByDistance)
    {}

    /**
     * Generate the default copy constructor
     */
    Sector(const Sector&) = default;


    /** overloaded assignment operator */
    Sector& operator=(const Sector& aSector)
    {
      m_myActiveSector = aSector.getMyActiveSector();
      m_sectorID = aSector.getSecID();
      m_distance2Origin = aSector.getDistance();
      m_useDistance4sort = aSector.useDistance4sort();
      return *this;
    }


    /** overloaded '<'-operator for sorting algorithms - sorts by distance2origin or fullSecID depending on setting */
    bool operator<(const Sector& b)  const
    {
      if (m_useDistance4sort == false) { return getSecID() < b.getSecID(); }
      return getDistance() < b.getDistance();
    }


    /** overloaded '=='-operator for sorting algorithms - sorts by distance2origin or fullSecID depending on setting */
    bool operator==(const Sector& b) const
    {
      if (useDistance4sort() == false) { return getSecID() == b.getSecID(); }
      return getDistance() == b.getDistance();
    }


    /** overloaded '>'-operator for sorting algorithms - sorts by distance2origin or fullSecID depending on setting */
    bool operator>(const Sector& b)  const
    {
      if (useDistance4sort() == false) { return getSecID() > b.getSecID(); }
      return getDistance() > b.getDistance();
    }


    /** called each event - takes all spacePoints from the activated Sector and its friend Sectors to produce segments */
    void segmentMaker();


    /** removes link to activated sector. should be called at the end of each run. TODO: shall this activate the destructor of the activated sector too? */
    virtual void clear() { m_myActiveSector = NULL; }


    /** getter - returns a pointer to the currently connected activatedSector */
    ActivatedSector* getMyActiveSector() const { return m_myActiveSector; }


    /** setter - set distance of sector to origin defined by sectorMap */
    void setDistance(float distance) { m_distance2Origin = distance; }


    /** getter - get distance of sector to origin defined by sectorMap */
    float getDistance() const { return m_distance2Origin; }


    /** getter - getSecID returns the ID of the sector (for definition of secID, see m_sectorID). */
    unsigned getSecID() const { return m_sectorID; }


    /** printing member, delivers string of interesting features of current sector */
    std::string printSector();


    /** if true, usingDistance for sector sorting is activated, if false, the sectorID is used */
    bool useDistance4sort() const { return m_useDistance4sort; }


    /** if you pass a true here, the sorting will be set to using the distance to origind instead of the sectorID. If you set to false, it's the other way round */
    void setDistance4sort(bool sortByDistance) { m_useDistance4sort = sortByDistance; }


  protected:


    /** The activated sector is created each event where this sector inhabits a spacePoint. */
    ActivatedSector* m_myActiveSector;


    /** This vector carries a pointer to each SectorFriendship for faster access during events */
    std::vector<SectorFriendship*> m_myFriends;


    /** secID allows identification of sector.
     *
     * Current definition AB_C_D (more details can be found in FullSecID.h):
     * A: layerNumber(1-7), -> 7 used for testing purposes
     * B: subLayerNumber(0,1)-defines whether sector has friends on same layer (=1) or not (=0),
     * C: uniID/complete VxdID-info,
     * D: sectorID on sensor (0-X), whole info stored in an int, can be converted to human readable code by using FullSecID-class
     */
    unsigned int m_sectorID;


    /** carries info about the distance of the sector-center to the origin.
     *
     * The origin is a value depending on the sectorMap which represents the IP (does not have to be 0,0,0)
     * especially relevant for testbeam- and other testing scenarios.
     * Needed for the CA (directed graph) e.g. if layerNumbers are not consecutive
     */
    float m_distance2Origin;


    /** if activated, sectors are sorted by distance to origin, if false, they are sorted by layerID. */
    bool m_useDistance4sort;


    ClassDef(Sector, 1)
  };
} //Belle2 namespace
