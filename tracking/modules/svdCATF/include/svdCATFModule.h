/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* This module does nothing (so far).
* Important note: if you find comments in two different languages in this module,
* please ignore the non-english ones. Only english comments are final. The other
* ones are only for temporal purposes (e.g. marking nonworking or nonexistent
* parts of the code)
*
*/

#ifndef svdCATFModule_H_
#define svdCATFModule_H_

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <generators/dataobjects/MCParticle.h>
#include <framework/dataobjects/Relation.h>
//#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <TVector3.h>
#include <svd/dataobjects/VXDSpacePoint.h>
#include <pxd/geopxd/CIDManager.h>
#include <framework/gearbox/GearDir.h> // braucht man zum Auslesen von xml-files


//#include <iostream>
//#include <sstream>

namespace Belle2 {

  struct coordStore {
    /* within each sector, its normalized local coordinates get stored. Because
    of normalized coordinates, it is sufficent to store only 4 values, even for
    trapezoidal sensors */
    double u0;
    double v0;
    double u1;
    double v1;
  };

  class SegmentCell {/** TODO **/};

  class TrackCandidate {/** TODO **/};

  //Key struct zur identifikation
  /// es enthaelt die eindeutige Nummer zum auffinden des gewuenschten sektors in der map
  // pro sensor sind es 15 sektoren, daher muesste globale sensor id und lokale sector id reichen

  class SectorID {
  public:
    SectorID(int uID, short int sectID): m_uniID(uID), m_sectorID(sectID) {} // defaultwerte
    bool operator<(const SectorID &b)  const { return getUniID() < b.getUniID() || getSectorID() < b.getSectorID(); }
    bool operator==(const SectorID &b) const { return getUniID() < b.getUniID() && getSectorID() == b.getSectorID(); }
    short int getSectorID() const { return m_sectorID; }
    int getUniID() const { return m_uniID; }

  protected:
    int m_uniID;
    short int m_sectorID;
  };

  // persönliche Hitklasse, benütze ich nur solange nichts anderes geht
  class TFHit {
  public:
    /** constructor**/
    TFHit(VXDSpacePoint* hit): m_hit(hit) {}

    /** hit braucht keine sektorinfo, der Sektor hat alles was wir brauchen**/

    int getLayerID() const { return m_hit->getLayerID(); }
    TVector3 getHitCoordinates() const {return TVector3(m_hit->getX(), m_hit->getY(), m_hit->getZ()); }

    const std::list<SegmentCell*>& getAttachedCell() const  { return m_attachedCells; }
    const std::vector<TrackCandidate*>& getTrackCandidate() const  { return m_attachedTrackCandidates; }
    const TrackCandidate* getTrackCandidate(int i) const { return m_attachedTrackCandidates[i]; }

    //selbes Spiel mit den pointers auf sector und Hauptcontainer

    /* setter */  // no setter for layer, ladder, sensor because they should not be changed.
    void addCell(SegmentCell* newCell) { m_attachedCells.push_back(newCell); }
    void addTrackCandidate(TrackCandidate* newTC) { m_attachedTrackCandidates.push_back(newTC); }

  protected:
    VXDSpacePoint* m_hit;
    std::list<SegmentCell*> m_attachedCells; //Info fließt eh nur in eine Richtung, also reicht der innere Zellenbaum
    std::vector<TrackCandidate*> m_attachedTrackCandidates; // brauch ich das überhaupt?

  };

  //Value class mit den Werten
  class Sector {
  public:
    Sector() {} // defaultwerte? (passt das so?)
    void addHit(TFHit* newSpacePoint) {
      m_hits.push_back(newSpacePoint);
    } // eigentlich brauchen wir mehr info als nur diese spacepoints. notwendig sind die verknüpften segmente (cells) und fürs testen wäre noch ne Verknüpfung zum        originalpartikel nett ( auch klassifizierung obs ein background- oder trackhit ist), vllt hier mit vererbung passendes basteln?
    void addFriend(Sector* newSector) { m_friends.push_back(newSector); }
    const std::vector<TFHit*>& getHits() const { return m_hits; } // sind das nicht zu viele
    const std::vector<Sector*>& getFriends() const { return m_friends; } // s.o.

  protected:
    std::vector<Sector*> m_friends; // liste mit pointer auf die Schlüssel für kompatible Sektoren (da manche Freunde beim Einschlichten einfach noch nicht existieren,         müssen die UniID und die SectorID herhalten. .first() ist uniID, .second() ist sectorID
    std::vector<TFHit*> m_hits; // liste mit hits im sektor
  };







  // eigentliches CATF-modul
  class svdCATFModule : public Module {

  public:
    typedef std::map<SectorID, Sector> SectorMap;
    /* beim Iterieren über die Map ist der Hauptiterator die UniID und dann pro UniID
    macht er dann die SectorID durch. */

    //! Constructor
    svdCATFModule();

    //! Destructor
    virtual ~svdCATFModule();

    //! Initialize the Module
    /*! Function is called only once at the beginning of your job at the beginning of the corresponding module.
        Things that can be done here, should be done here, as it is relatively cheap in terms of CPU time.

        This method has to be implemented by subclasses.
    */
    virtual void initialize();

    //! Called when entering a new run
    /*! At the beginning of each run, the function gives you the chance to change run dependent constants like alignment parameters, etc.

        This method has to be implemented by subclasses.
    */
    virtual void beginRun();

    //! Running over all events
    /*! Function is called for each evRunning over all events
        This means, this function is called very often, and good performance of the code is of strong interest.

        This method has to be implemented by subclasses.
    */
    virtual void event();

    //! Is called after processing the last event of a run
    /*! Good e.g. for storing stuff, that you want to aggregate over one run.

        This method has to be implemented by subclasses.
    */
    virtual void endRun();

    //! Is called at the end of your Module
    /*! Function is called only once at the end of your job at the end of the corresponding module.
        This function is for cleaning up, closing files, etc.

        This method has to be implemented by subclasses.
    */
    virtual void terminate();


  protected:
    //Map ala phython dict
    SectorMap m_sectors; // sectors (subunit of sensors) are stored into a map.


    int m_numUColumns; // delivers the number of columns (sectors) in the U-axis of the sensor
    int m_numVColumns; // delivers the number of columns (sectors) in the V-axis of the sensor


  private:

  };
}

#endif /* svdCATFModule_H_ */
