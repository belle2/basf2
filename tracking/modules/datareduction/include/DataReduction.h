#ifndef DATAREDUCTION_H_
#define DATAREDUCTION_H_

//Marlin and LCIO includes
#include "marlin/Processor.h"
#include "lcio.h"
#include <EVENT/MCParticle.h>

#include "SectorList.h"
#include "PXDLadderList.h"

#ifdef CAIRO_OUTPUT
#include <cairo.h>
#include <cairo-pdf.h>
#include "cairo-utils.h"
#include <boost/format.hpp>
//#include <boost/foreach.hpp>
#endif

//==========================================================================================
//                                      Processor
//==========================================================================================

class DataReduction : public marlin::Processor {
public:

  virtual marlin::Processor* newProcessor() { return new DataReduction; }

  DataReduction();
  virtual ~DataReduction();
  virtual void init();
  virtual void processRunHeader(lcio::LCRunHeader* run);
  virtual void processEvent(lcio::LCEvent* evt);

  virtual void check(lcio::LCEvent* evt);
  virtual void end();


protected:

  int _nRun;
  int _nEvt;

  std::string _colNameDigiSVDHits; //Name of the digitized SVD hit input collection
  std::string _colNameDigiPXDHits; //Name of the digitized PXD hit input collection
  std::string _colNameSimPXDHits; //Name of the digitized PXD hit input collection

  SectorList* _sectorList;
  PXDLadderList* _pxdLadderList;

  int _numberPXDHitsTotal;
  int _numberPXDHitsFound;
  int _numberSimPXDHitsTotal;
  int _numberSimPXDHitsFound;

  enum LayerType { pixel = 0, strip = 1, slanted = 2 };

private:

  //debug output methods
  void printSectorInfo();

#ifdef CAIRO_OUTPUT
  cairo_t *cairo;
  cairo_surface_t *cairo_surface;
  void makePDF(bool split = true, int group = 1);
#endif

};

#endif /* DATAREDUCTION_H_ */
