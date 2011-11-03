#ifndef DATAREDUCTION_H
#define DATAREDUCTION_H
#include <tracking/modules/datareduction/cairo-utils.h> // first to avoid ifdef problems

#include <framework/core/Module.h>

#include <tracking/modules/datareduction/SectorList.h>
#include <tracking/modules/datareduction/PXDLadderList.h>

#include <boost/format.hpp>

namespace Belle2 {


  class DataReductionModule : public Module {
  public:

    DataReductionModule();
    virtual ~DataReductionModule();
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  protected:

    int _nRun;
    int _nEvt;

    std::string _colNameDigiSVDTrueHits; //Name of the digitized SVD hit input collection
    std::string _colNameDigiPXDTrueHits; //Name of the digitized PXD hit input collection
    std::string _colNameSimPXDTrueHits;  //Name of the simulated PXD hit input collection

    SectorList* _sectorList;
    PXDLadderList* _pxdLadderList;

    int _numberPXDTrueHitsTotal;
    int _numberPXDTrueHitsFound;
    int _numberSimPXDTrueHitsTotal;
    int _numberSimPXDTrueHitsFound;

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
} //Namespace
#endif /* DATAREDUCTION_H */
