/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/svdCATF/svdCATFModule.h"

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <pxd/geopxd/SiGeoCache.h>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(svdCATF)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

svdCATFModule::svdCATFModule() : Module()
{
  //Set module properties
  setDescription("svdCATFMod module");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  //Parameter definition

}


svdCATFModule::~svdCATFModule()
{

}

void svdCATFModule::initialize()
{


}


void svdCATFModule::beginRun()
{
  m_sectors.clear();
// holen Info über coordpositionen der sektoren aus xml-datei (sind normierte relativcoordinaten (also zwischen 0 und 1)):
// da diese relativkoords für alle sensoren gleich sind, kann man die ruhig ein einziges mal auslesen...

  struct { //because of normalized relative coordinates we need only 4 values even for trapezoidal sensorplanes
    vector<double> uMins; // eigentlich arrays schneller, aber falls sich die sektorenzahl pro sensor ändert, kann man den Teil so lassen...
    vector<double> vMins;
    vector<double> uMaxes;
    vector<double> vMaxes;
  } sectorPositions; // gleich hier deklariert, brauche es eh nur einmal. Daher brauch ich auch keinen allgemeinen Typnamen (anonym definiert)

  GearDir sectorDefs("/Detector/CATFParameters/Sector[@type='SVD']"); // öffne den angegebenen Pfad

  m_numUColumns = sectorDefs.getParamNumValue("uCol"); // darf ich das so machen?
  m_numVColumns = sectorDefs.getParamNumValue("vCol");

  GearDir sectorIDs(sectorDefs, "sID"); // nimm Pfad von sectorDefs und hänge den zusätzlichen Pfad dran.

  int nSecIDs = sectorIDs.getNumberNodes();
  if (nSecIDs != m_numUColumns * m_numVColumns) {
    B2FATAL("CA-trackfinder: /tracking/data/SVDSectors.xml is set wrong, please repair it.");
  }
  for (int aSecID = 1; aSecID <= nSecIDs; ++aSecID) {
    GearDir actualSector(sectorDefs, (boost::format("sID[id=%1%]") % aSecID).str());
    double pointUmin = actualSector.getParamNumValue("u0"); // siehe B2-Doxygen GearDir...
    double pointVmin = actualSector.getParamNumValue("v0");
    double pointUmax = actualSector.getParamNumValue("u1");
    double pointVmax = actualSector.getParamNumValue("v1");
    sectorPositions.uMins.push_back(pointUmin);
    sectorPositions.vMins.push_back(pointVmin);
    sectorPositions.uMaxes.push_back(pointUmax);
    sectorPositions.vMaxes.push_back(pointVmax);
  }

// bauen unsere sektoren (konvertieren koords, importieren friends). da auch trapezoide Sensoren möglich, muss das ein wenig umständlich gehandlet werden:
  SiGeoCache* geometry = SiGeoCache::instance();

  // alyerId = acutalSensor.getParamIntValue("@id");
  // GearDir ladders(acutalSensor,"Ladder");

  BOOST_FOREACH(int aLayer, geometry->getSVDLayerIDs()) { // müssten hier vom äußersten Layer nach innen zählen, wegen node-basiertem
    // hier theoretisch auch für SVD+PXD möglich, bei Bedarf wird das aber einfach über eine separate Schleife geschickt.
    GearDir xmlLayers((boost::format("/Detector/CATFParameters/Layer[id=%1%]") % aLayer).str()); // aLayer muss erst passend für das xml-Zeugs formatiert werden.

    GearDir ladders(xmlLayers, "Ladder");
    for (int iLadder = 1; iLadder <= ladders.getNumberNodes(); ++iLadder) {
      GearDir xmlLadders(ladders, iLadder);
      int aLadder = xmlLadders.getParamIntValue("@id");

      GearDir sensors(xmlLadders, "Sensor");
      for (int iSensor = 1; iSensor <= sensors.getNumberNodes(); ++iSensor) {
        GearDir xmlSensors(sensors, iSensor);
        int aSensor = xmlSensors.getParamIntValue("@id");

        double vSize = 0.5 * geometry->getVSensorSize(aLayer, aLadder, aSensor);

        GearDir sectors(xmlLadders, "Sector");
        for (int iSector = 1; iSector <= sectors.getNumberNodes(); ++iSector) {
          GearDir xmlSectors(sectors, iSector);
          int aSector = xmlSectors.getParamIntValue("@id");

          // füllen sektoren mit Coordinfos... hier sind sie aber nicht mehr normiert, sondern einfach lokale coords (besser zum schnellen Einsortieren der Hits)
          SectorID sectorID(geometry->getSensorUniID(aLayer, aLadder, aSensor), aSector);
          // Beim Zugriff auf ein Element wird automatisch eines erstellt sollte es noch nicht vorhanden sein.
          Sector& sector = m_sectors[sectorID];

          // füllen sektoren mit friends... muss natürlich aus der xml-datei brav einzeln ausgelesen werden, wird daher ein relativ langsamer Prozess werden.
          GearDir friends(xmlLadders, "Friend");
          for (int iFriend = 1; iFriend <= friends.getNumberNodes(); ++iFriend) {
            GearDir xmlFriend(friends, iFriend);
            string newFriend = xmlFriend.getParamString(); // check doxygen for getParamType
            size_t posMarker = newFriend.find("#");
            string strUniID = newFriend.substr(0, posMarker);
            string strSecID = newFriend.substr(posMarker + 2); // npos, also das Ende ist defaulteinstellung, also brauch ich das nicht angeben
            int uniID = boost::lexical_cast<int>(strUniID);
            int secID = boost::lexical_cast<int>(strSecID);
            sector.addFriend(&m_sectors[SectorID(uniID, secID)]);// geht auch, wenn verlinkter Sektor nicht existiert hat.

          } // loop over friends
        } // loop over sectors
      } // loop over sensors
    } // loop over ladders
  } // loop over layers
}


void svdCATFModule::event()
{

  for (map<SectorID, Sector>::iterator it = m_sectors.begin(); it != m_sectors.end(); ++it) {
    //FIXME: need clear member in sector
    //it->second.clear();
  }


  /** POOL:
  // changing from normalized coordinates to normal local coordinates means, that we need at least 6 values (trapezoidal sensors ftw -.-)
  tempCoords.v0 = -vSize + sectorPositions.vMins(actualsID)*2*vSize;
  tempCoords.v1 = -vSize + sectorPositions.vMaxes(actualsID)*2*vSize;
  // uSizev0 and uSizev1 are sectordependent, thats why they have to be calculated separately for each sector (faster: there are 15 sectors so far, trios of them have got the same v-positions so you could filter it by their sectornumber. Makes sense or not, depending on the speed of getUSensorSize vs if-check)
  double uSizev0 = 0.5*geometry->getUSensorSize(aLayer,aLadder,aSensor, tempCoords.v0);
  double uSizev1 = 0.5*geometry->getUSensorSize(aLayer,aLadder,aSensor, tempCoords.v1);
  tempCoords.u0atv0 = -uSizev0 + sectorPositions.vMins(actualsID)*2*uSizev0;
  tempCoords.u1atv0 = -uSizev0 + sectorPositions.vMaxes(actualsID)*2*uSizev0;
  tempCoords.u0atv1 = -uSizev1 + sectorPositions.vMins(actualsID)*2*uSizev1;
  tempCoords.u1atv1 = -uSizev1 + sectorPositions.vMaxes(actualsID)*2*uSizev1;

   **/

  if (m_numUColumns == 1) { // simplest case, only vColumns there.

  } else {/** TODO **/}

}
/*
  laden der Hits aus dem TObject-Raum und speichern in eigener Klasse. alle Hits kommen in eine Liste
  Zuweisen der Hits zu den Sektoren:
  for (int listIterator = 0; listIterator not_eq 7; ++listIterator) {

  int layID TFHit(listIterator).getLayerID();
  int ladID TFHit(listIterator).getLadderID();
  int senID TFHit(listIterator).getSensorID();

  hier noch ein Prinzip überlegen wie man die Hits jetzt schnell und unkompliziert in die passenden Sektoren packt.
  alle sektoren, die mind. einen Hit bekommen haben, werden in ner Liste verlinkt ( da aktuell 2805 sektoren und wir wohl nie so
  viele Hits pro event haben werden, wäre es gescheiter, nur die aktiven Sektoren zu speichern. Friendsektoren sind da natürlich
  umständlicher, im schlimmsten Fall werden halt alle friendsektoren gecheckt, ob was drin ist. Alternativ (oder zusätzlich
  interessant ist auf jeden Fall der Gedanke, statt 15 Sektoren, nur noch 6 Sektoren (nur 2 in z-Richtung) zu nehmen. Man bekommt
  auf diese Art zwar deutlich mehr overhead an sinnlosen Hits, aber dafür würde sich die Anzahl der Sektoren auf 1122 reduzieren.
  Minimalwert ist 561, da wir auf jeden Fall 3 Sektoren pro Sensor brauchen (overlapping parts des Detektors)



  segfinder-Abschnitt: iteriere über alle sektoren (vllt besser, ne Liste aller sektoren mit Friends zu machen, würde zumindest
  theoretisch ein bisserl was sparen), checke ob sie Hits drin haben, wenn ja, suche in all den Freundsektoren nach passenden hits.
  Die Längencutoffs sind Sektorabhängig (vllt auch gekoppelt an Freundsektor) (oder alternativ: z-Abhängig, je näher an z0, desto
  kürzer der Cutoff). Details erst nach ausreichender Studie mit helpermodulen möglich. Gespeichert werden die so gefundenen
  segmentCells in den dazugehörigen Hits (inner und outer nicht vergessen!) und in ner Liste (optionaler Bonus).

  Neighbourfinder-Abschnitt: iteriere über alle Hits (ist schneller als über die Segmente drüberzuiterieren, da man sonst noch die
  verbundenen segmentCells separat finden muss, was einen kombinatorisch irrsinnigen Aufwand bedeutet.), wie es mit den Hits im
  Innersten Layer aussieht, muss man noch checken: ist es schneller über alle Hits zu iterieren, oder ne Liste zu machen, in der nur
  die Hits mit attached segments stehen (im innersten Layer haben ja nur die mit Friendsektoren partner). teste bei jedem Hit jede
  mögliche Kombination von inneren und äußeren segmentCells und berechne ihre Winkel. Die mit passendem Winkel (wieder mit helpermodulen
  herauszufinden) werden bei den äußeren segmentCells als Nachbarn eingetragen. weiters bekommen segmentCells mit Nachbarn den Zustand
  "getnewstate" == TRUE und dann noch in einer Liste abgespeichert. So werden im CA nur die abgearbeitet, die auch interessant sind.
  Wie die Cutoffs effizient für alle Hits (abh. ihrer Position) zu bekommen sind, ist noch zu überlegen.

vSeg: entweder in den Neighbourfinder rein, einfach als weitere segmentCell hinzufügen, oder ähnlich in alter Fassung, in den CA rein und
extra Schleife rein. bei Neighborufinderversion müssten im Segfinder im innersten Layer vSegs mit inner Hit (0,0,0) erzeugt werden (
wie der selber eingebunden wird, muss man noch checken. einfach in das TArray rein ist wohl keine gute Idee. Im Neighbourfinder würden
dann einfach wie bei normalen SegmentCells die Winkel berechnet und so weiter. Weiters wie soll das mit den Mehrstufigen vSeg-Prinzip
einfließen? idealerweise wäre es ja schön, wenn man das in den CA einpflegen könnte (einfach höhere States für die mit besserem bSeg).
Beim Einteilen in TC-Klassen würde dann einfach der Zustand des innersten Segments als Kriterium genommen (ob die Tracklänge innerhalb
der Klassen oder Klassenübergreifend einfließen soll, ist dann wieder Ziel einer kleinen Studie.

cellular Automaton-Abschnitt:
Checkstep: nimm die Liste aus dem Neighbourfinder, iteriere durch sie durch und mach das übliche CA-Spiel (checke  ob >= 1 Nachbar den
gleichen Zustand hat, wenn ja, tu nichts (getnewstate ist schon auf TRUE), wenn nein, setze wert auf FALSE.
Updatestep: iteriere durch Liste durch, alle, die getnewstate auf TRUE haben, belassen ihn dabei, bekommen erhöhten State und werden in
neuer Liste abgespeichert (für nächsten Durchgang), alle mit FALSE werden rausgefiltert (so schrumpft die Liste mit jedem neuen Zeitschritt
und es werden wirklich nur die Zellen gecheckt, bei denen sich was ändern kann.
Beide Schritte werden so lange wiederholt, bis keine Cells mehr aktualisiert wurden und damit die neue Liste leer bleibt.

TrackCandidateCollector-Abschnitt: schauen, ob ein besseres Sammelsystem gefunden werden kann. Weiters, zickzack-filter und Kinkfilter(deltapT)
hier einbauen (wenn nicht sogar in den Neighbourfinder (wegen Folgeeffekte evtl schneller) -> austesten was schneller ist!) einfließen lassen.
Dann Treebased Kalmanfiltering (many TCs enter, one TC leaves!!!einseinself). Qi berechnen und Bad-Qi-Cutoff-Filter nicht vergessen. Surviving
TCs kommen in Liste. Allgemein Überlegenswert: ist es möglich, die eDeposits der Hits in den KF reinzubekommen, wenn ja wie, wenn ja, bringts
überhaupt irgendwas?

Neuronales Netzerk-Abschnitt: straight-forward wie in Matlab-Version, Liste mit verknüpfungen zu TCs gehen samt QIs rein, die Gewinner verlassen
den NN-Ring (NN genau wie in Matlab-Version. Ob eine andere NN-Variante mehr Sinn macht, kann man danach immer noch checken).

die Survivors werden als final TCs ausgegeben, Party, Jubel und viel Freu...


}*/


void svdCATFModule::endRun()
{

}


void svdCATFModule::terminate()
{

}
