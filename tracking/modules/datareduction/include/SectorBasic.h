#ifndef SECTORBASIC_H_
#define SECTORBASIC_H_

#include <list>

#include "lcio.h"
#include <EVENT/MCParticle.h>
#include <EVENT/TrackerHit.h>

#include "PXDLadder.h"


#ifdef CAIRO_OUTPUT
#include <cairo.h>
#endif

struct LadderEntry {
  LadderEntry(): ladder(0), start(0), end(1) {};
  PXDLadder* ladder;
  double start;
  double end;
  double minRadius;
  double maxRadius;
};


class SectorBasic {
public:

  SectorBasic(double RotationAngle, double DownShift, double SectorWidth): _RotationAngle(RotationAngle), _DownShift(DownShift), _SectorWidth(SectorWidth / 2.0) {
    color[0] = 0;
    color[1] = 0;
    color[2] = 1;
  };
  virtual ~SectorBasic() {};

  bool addHit(EVENT::TrackerHit* hit);
  bool addPXDLadder(PXDLadder* ladder);

  std::list<EVENT::TrackerHit*>& getTrackerHitList() {return _hitList; };
  std::list<LadderEntry>& getPXDLadderList() {return _ladderList; };

  unsigned int getHitNumber();
  void clearHits();

  unsigned int getLadderNumber();
  void clearLadders();

#ifdef CAIRO_OUTPUT
  virtual void draw(cairo_t* cairo);
  virtual void makePoint(cairo_t* cairo, double x, double y, int status = -10);
  virtual std::string name();
#endif
  double color[3];
  void setColor(double red, double green, double blue) {
    color[0] = red;
    color[1] = green;
    color[2] = blue;
  }

protected:
  double _RotationAngle;
  double _DownShift;
  double _SectorWidth;
  std::list<EVENT::TrackerHit*> _hitList;
  std::list<LadderEntry> _ladderList;

  virtual bool isHitInSector(EVENT::TrackerHit* hit);
  virtual bool isLadderInSector(PXDLadder* ladder);

  /**
   * Function to check if given point lies in sector;
   * @return  0 if point is in sector, -1 if point is below sector, 1 if point is above sector,
   *         -2 if point is wrong side
   */
  virtual int checkPoint(double x, double y);
  virtual void setIntersection(LadderEntry& ladderEntry);

  void setMinMaxRadius(LadderEntry& ladderEntry);

private:

};

#endif /* SECTORBASIC_H_ */
