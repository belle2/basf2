//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGCDC.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDC_FLAG_
#define TRGCDC_FLAG_

#include <cmath>
#include <string>
#include <vector>
#include "trg/trg/Clock.h"

#define TRGCDC_UNDEFINED 999999

namespace HepGeom {
    template <class T> class Point3D;
}

namespace Belle2 {
    class TRGTime;
}

namespace Belle2 {

class TRGLink;
class CDCGeometryPar;
class TRGCDC;
class TRGCDCWire;
class TRGCDCLayer;
class TRGCDCWireHit;
class TRGCDCWireHitMC;
class TRGCDCTrackSegment;
class TRGCDCFrontEnd;
class TRGCDCMerger;

/** A class to represent CDC.

  The instance of TRGCDC is a singleton. 'TRGCDC::getTRGCDC()' gives you a
  pointer to access the TRGCDC. Geometrical information is initialized
  automatically. Before accessing hit information, user has to call
  'update()' to initialize hit information event by event. */

class TRGCDC {

  public:
    /// returns TRGCDC object.
    static TRGCDC * getTRGCDC(const std::string & configFile);
    static TRGCDC * getTRGCDC(void);

  private:
    /// Constructor and destructor
    TRGCDC(const std::string & configFile);
    virtual ~TRGCDC();

    /// initializes CDC geometry.
    void initialize(void);

    /// configures trigger modules for detail simulation.
    void configure(void);

  public:
    /// simulates CDC trigger.
    void simulate(void);

  public:// Selectors
    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// returns CDC version.
    std::string versionCDC(void) const;

    /// dumps debug information.
    void dump(const std::string & message) const;

    /// returns debug level.
    int debugLevel(void) const;

    /// sets and returns debug level.
    int debugLevel(int) const;

    /// returns fudge factor for drift time error.
    float fudgeFactor(void) const;

    /// sets and returns fudge factor for drift time error.
    float fudgeFactor(float);

  public:// Geometry
    /// returns a pointer to a wire. 0 will be returned if 'wireId' is invalid.
    const TRGCDCWire * const wire(unsigned wireId) const;

    /// returns a pointer to a wire. 'localId' can be negative. 0 will be returned if 'layerId' is invalid.
    const TRGCDCWire * const wire(unsigned layerId, int localId) const;

    /// returns a pointer to a wire.
//    const TRGCDCWire * const wire(const HepGeom::Point3D<double> & point) const;

    /// returns a pointer to a wire.
    const TRGCDCWire * const wire(float r, float phi) const;

    /// returns a pointer to a layer. 0 will be returned if 'id' is invalid.
    const TRGCDCLayer * const layer(unsigned id) const;

    /// returns a pointer to a super-layer. 0 will be returned if 'id' is invalid.
    const std::vector<TRGCDCLayer *> * const superLayer(unsigned id) const;

    /// returns \# of wire layers in a super layer. 0 will be returned if 'superLayerId' is invalid.
    unsigned nLocalLayer(unsigned superLayerId) const;

    unsigned nWires(void) const;
    unsigned nSuperLayers(void) const;
    unsigned nLayers(void) const;
    unsigned nAxialLayers(void) const;
    unsigned nStereoLayers(void) const;
    unsigned nAxialSuperLayers(void) const;
    unsigned nStereoSuperLayers(void) const;

    /// returns cell width in unit of radian.
    float cellWidth(unsigned superLayerId) const;

    /// returns inner radius of super layer.
    float superLayerR(unsigned superLayerId) const;

    /// returns (inner radius)^2 of super layer.
    float superLayerR2(unsigned superLayerId) const;

    /// returns \# of track segments.
    unsigned nTrackSegments(void) const;

    /// returns a track segment.
    const TRGCDCTrackSegment * trackSegment(unsigned id) const;

    /// returns \# of track segment layers.
    unsigned nTrackSegmentLayers(void) const;

    /// returns a pointer to a track segment layer. 0 will be returned if 'id' is invalid.
    const TRGCDCLayer * trackSegmentLayer(unsigned id) const;

  public:// Event by event hit information.

    /// clears all TRGCDC hit information.
    void clear(void);

    /// clears TRGCDC information.
    void fastClear(void);

    /// updates TRGCDC information. clear() is called in this function.
    void update(bool mcAnalysis = true);

    /// returns a list of TRGCDCWireHit. 'update()' must be called before calling this function.
    std::vector<const TRGCDCWireHit *> hits(void) const;

    /// returns a list of axial hits. 'update()' must be called before calling this function.
    std::vector<const TRGCDCWireHit *> axialHits(void) const;

    /// returns a list of stereo hits. 'update()' must be called before calling this function.
    std::vector<const TRGCDCWireHit *> stereoHits(void) const;

    /// returns a list of TRGCDCWireHitMC. 'updateMC()' must be called before calling this function.
    std::vector<const TRGCDCWireHitMC *> hitsMC(void) const;

    /// returns bad hits(finding invalid hits).
    std::vector<const TRGCDCWireHit *> badHits(void) const;

  public:// Utility functions

    /// returns wire name.
    std::string wireName(unsigned wireId) const;

    /// returns local ID in a layer. This function is expensive.
    unsigned localId(unsigned wireId) const;

    /// returns layer ID. This function is expensive.
    unsigned layerId(unsigned wireId) const;

    /// returns layer ID from axialStereoLayerId. This function is expensive.
    unsigned layerId(unsigned axialStereoSwitch,
		     unsigned axialStereoLayerId) const;

    /// returns super layer ID. This function is expensive.
    unsigned superLayerId(unsigned wireId) const;

    /// returns local layer ID in a super layer. This function is expensive.
    unsigned localLayerId(unsigned wireId) const;

    /// returns axialStereo super layer ID. This function is expensive.
    unsigned axialStereoSuperLayerId(unsigned axialStereo,
				     unsigned axialStereoLayerId) const;

    /// returns true if w0 and w1 are neighbor.
    bool neighbor(const TRGCDCWire & w0, const TRGCDCWire & w1) const;

    /// calculates corrected drift time. correctionFlag(bit 0:tof, 1:T0 offset, 2:propagation delay, 3:tan(lambda) correction)
//     static void driftDistance(TLink & link,
// 			      const TTrack & track,
// 			      unsigned correctionFlag = 0,
// 			      float T0Offset = 0.);

  public:// TRG information

    /// returns the system clock.
    const TRGClock & systemClock(void) const;

    /// returns the system offset in MC.
    double systemOffsetMC(void) const;

    /// returns a vector of hit TSs.
    const std::vector<const TRGCDCTrackSegment *> tsHits(void) const;

    /// returns a front-end board.
    const TRGCDCFrontEnd * frontEnd(unsigned id) const;

    /// returns a merger board.
    const TRGCDCMerger * merger(unsigned id) const;

  private:
    /// classify hits.
    void classification(void);

    /// updates TRGCDC information for MC.
    void updateMC(void);

  private:

    /// CDC trigger singleton.
    static TRGCDC * _cdc;

    /// Debug level.
    mutable int _debugLevel;

    /// CDC version.
    const std::string _cdcVersion;

    /// CDC trigger configuration filename.
    std::string _configFilename;

    /// Super layers.
    std::vector<std::vector<TRGCDCLayer *> *> _superLayers;

    /// Axial super layers.
    std::vector<std::vector<TRGCDCLayer *> *> _axialSuperLayers;

    /// Stereo super layers.
    std::vector<std::vector<TRGCDCLayer *> *> _stereoSuperLayers;

    /// All layers.
    std::vector<TRGCDCLayer *> _layers;

    /// Axial layers.
    std::vector<TRGCDCLayer *> _axialLayers;

    /// Stereo layers.
    std::vector<TRGCDCLayer *> _stereoLayers;

    /// All wires.
    std::vector<TRGCDCWire *> _wires;

    /// Wires with a hit.
    std::vector<TRGCDCWire *> _hitWires;

    /// CDC hits.
    std::vector<TRGCDCWireHit *> _hits;

    /// CDC hits on axial wires.
    std::vector<TRGCDCWireHit *> _axialHits;

    /// CDC hits on stereo wires.
    std::vector<TRGCDCWireHit *> _stereoHits;

    /// Bad CDC hits.(not used now)
    std::vector<TRGCDCWireHit *> _badHits;

    /// MC info. of CDC hits.
    std::vector<TRGCDCWireHitMC *> _hitsMC;

    /// Track Segments.
    std::vector<TRGCDCTrackSegment *> _tss;

    /// Track Segments with hits.
    std::vector<TRGCDCTrackSegment *> _tsHits;

    /// Track Segment layers.
    std::vector<TRGCDCLayer *> _tsLayers;

    /// Fudge factor for position error.
    float _fudgeFactor;

    /// Cell width in radian.
    float * _width;

    /// R of cell.
    float * _r;

    /// R^2 of cell.
    float * _r2;

    /// CDC trigger system clock.
    const TRGClock _clock;

    /// Timing offset of CDC trigger.
    const double _offset;

    /// All serial links.
    std::vector<TRGLink *> _links;

    /// CDC front-end boards.
    std::vector<TRGCDCFrontEnd *> _fronts;

    /// CDC trigger merger boards.
    std::vector<TRGCDCMerger *> _mergers;

  public: // Utility functions

    /// Converts int to string.
    static std::string itostring(int i);

    /// Converts double to string.
    static std::string dtostring(double d, unsigned int precision);

    /// CERNLIB car.
    static std::string carstring(const std::string &s);

    /// CERNLIB cdr.
    static std::string cdrstring(const std::string &s);

    /// Dumps bit contents to cout.
    static void bitDisplay(unsigned);

    /// Dumps bit contents to cout.
    static void bitDisplay(unsigned val,
			   unsigned firstDigit,
			   unsigned lastDigit);
};

//-----------------------------------------------------------------------------

#ifdef TRGCDC_NO_INLINE
#define inline
#else
#undef inline
#define TRGCDC_INLINE_DEFINE_HERE
#endif

#ifdef TRGCDC_INLINE_DEFINE_HERE

inline
int
TRGCDC::debugLevel(void) const {
    return _debugLevel;
}

inline
int
TRGCDC::debugLevel(int a) const {
    return _debugLevel = a;
}

inline
unsigned
TRGCDC::nLocalLayer(unsigned superLayerId) const {
    std::vector<TRGCDCLayer *> * superLayer = _superLayers[superLayerId];
    if (! superLayer) return 0;
    return superLayer->size();
}

inline
const TRGCDCLayer * const
TRGCDC::layer(unsigned id) const {
    return _layers[id];
}

inline
const std::vector<TRGCDCLayer *> * const
TRGCDC::superLayer(unsigned id) const {
    return _superLayers[id];
}

inline
float
TRGCDC::fudgeFactor(void) const {
    return _fudgeFactor;
}

inline
float
TRGCDC::fudgeFactor(float a) {
    return _fudgeFactor = a;
}

inline
std::string
TRGCDC::versionCDC(void) const {
    return std::string(_cdcVersion);
}

inline
unsigned
TRGCDC::nWires(void) const {
    return _wires.size();
}

inline
unsigned
TRGCDC::nSuperLayers(void) const {
    return _superLayers.size();
}

inline
unsigned
TRGCDC::nStereoLayers(void) const {
    return _stereoLayers.size();
}

inline
unsigned
TRGCDC::nAxialLayers(void) const {
    return _axialLayers.size();
}

inline
unsigned
TRGCDC::nAxialSuperLayers(void) const {
    return _axialSuperLayers.size();
}

inline
unsigned
TRGCDC::nStereoSuperLayers(void) const {
    return _stereoSuperLayers.size();
}

inline
unsigned
TRGCDC::nLayers(void) const {
    return _layers.size();
}

inline
float
TRGCDC::cellWidth(unsigned a) const {
    return _width[a];
}

inline
float
TRGCDC::superLayerR(unsigned i) const {
    return _r[i];
}

inline
float
TRGCDC::superLayerR2(unsigned i) const {
    return _r2[i];
}

inline
const TRGCDCTrackSegment *
TRGCDC::trackSegment(unsigned id) const {
    return _tss[id];
}

inline
unsigned
TRGCDC::nTrackSegments(void) const {
    return _tss.size();
}

inline
const TRGClock &
TRGCDC::systemClock(void) const {
    return _clock;
}

inline
double
TRGCDC::systemOffsetMC(void) const {
    return _offset;
}

inline
const std::vector<const TRGCDCTrackSegment *>
TRGCDC::tsHits(void) const {
    std::vector<const TRGCDCTrackSegment *> t;
    t.assign(_tsHits.begin(), _tsHits.end());
    return t;
}

inline
const TRGCDCFrontEnd *
TRGCDC::frontEnd(unsigned a) const {
    return _fronts[a];
}

inline
const TRGCDCMerger *
TRGCDC::merger(unsigned a) const {
    return _mergers[a];
}

inline
const TRGCDCLayer *
TRGCDC::trackSegmentLayer(unsigned id) const {
    if (id < _tsLayers.size())
	return _tsLayers[id];
    return 0;
}

inline
unsigned
TRGCDC::nTrackSegmentLayers(void) const {
    return _tsLayers.size();
}

#endif

#undef inline

} // namespace Belle2

#endif /* TRGCDC_FLAG_ */
