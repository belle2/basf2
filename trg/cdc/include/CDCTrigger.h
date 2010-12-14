//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTrigger.h
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef CDCTrigger_FLAG_
#define CDCTrigger_FLAG_

#include <cmath>
#include <string>
#include <vector>
#include "trigger/gdl/GDLClock.h"

#define CDCTrigger_UNDEFINED 999999

namespace HepGeom {
    template <class T> class Point3D;
}

namespace Belle2 {
    class GDLTime;
}

namespace Belle2 {

class CDCGeometryPar;
class CDCTrigger;
class CDCTriggerWire;
class CDCTriggerLayer;
class CDCTriggerWireHit;
class CDCTriggerWireHitMC;
class CDCTriggerTrackSegment;

/** A class to represent CDC.

  The instance of CDCTrigger is a singleton. 'CDCTrigger::getCDCTrigger()' gives you a
  pointer to access the CDCTrigger. Geometrical information is initialized
  automatically. Before accessing hit information, user has to call
  'update()' to initialize hit information event by event. */

class CDCTrigger {

  public:
    /// returns CDCTrigger object.
    static CDCTrigger * getCDCTrigger(const std::string & cdcVersion);
    static CDCTrigger * getCDCTrigger(void);

  private:
    /// Constructor and destructor
    CDCTrigger(const std::string & cdcVersion);
    virtual ~CDCTrigger();

    /// initializes CDC geometry.
    void initialize(void);

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
    const CDCTriggerWire * const wire(unsigned wireId) const;

    /// returns a pointer to a wire. 'localId' can be negative. 0 will be returned if 'layerId' is invalid.
    const CDCTriggerWire * const wire(unsigned layerId, int localId) const;

    /// returns a pointer to a wire.
//    const CDCTriggerWire * const wire(const HepGeom::Point3D<double> & point) const;

    /// returns a pointer to a wire.
//    const CDCTriggerWire * const wire(float r, float phi) const;

    /// returns a pointer to a layer. 0 will be returned if 'id' is invalid.
    const CDCTriggerLayer * const layer(unsigned id) const;

    /// returns a pointer to a super-layer. 0 will be returned if 'id' is invalid.
    const std::vector<CDCTriggerLayer *> * const superLayer(unsigned id) const;

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
    const CDCTriggerTrackSegment * trackSegment(unsigned id) const;

  public:// Event by event hit information.

    /// clears all CDCTrigger hit information.
    void clear(void);

    /// clears CDCTrigger information.
    void fastClear(void);

    /// updates CDCTrigger information. clear() is called in this function.
    void update(bool mcAnalysis = true);

    /// returns a list of CDCTriggerWireHit. 'update()' must be called before calling this function.
    std::vector<const CDCTriggerWireHit *> hits(void) const;

    /// returns a list of axial hits. 'update()' must be called before calling this function.
    std::vector<const CDCTriggerWireHit *> axialHits(void) const;

    /// returns a list of stereo hits. 'update()' must be called before calling this function.
    std::vector<const CDCTriggerWireHit *> stereoHits(void) const;

    /// returns a list of CDCTriggerWireHitMC. 'updateMC()' must be called before calling this function.
    std::vector<const CDCTriggerWireHitMC *> hitsMC(void) const;

    /// returns bad hits(finding invalid hits).
    std::vector<const CDCTriggerWireHit *> badHits(void) const;

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
    bool neighbor(const CDCTriggerWire & w0, const CDCTriggerWire & w1) const;

    /// calculates corrected drift time. correctionFlag(bit 0:tof, 1:T0 offset, 2:propagation delay, 3:tan(lambda) correction)
//     static void driftDistance(TLink & link,
// 			      const TTrack & track,
// 			      unsigned correctionFlag = 0,
// 			      float T0Offset = 0.);

  public:// Trigger information

    /// returns the system clock.
    const GDLClock & systemClock(void) const;

    /// returns the system offset in MC.
    double systemOffsetMC(void) const;

    /// returns a vector of hit TSs.
    const std::vector<const CDCTriggerTrackSegment *> tsHits(void) const;

  private:
    /// classify hits.
    void classification(void);

    /// updates CDCTrigger information for MC.
    void updateMC(void);

  private:
    static CDCTrigger * _cdc;
    mutable int _debugLevel;
    const std::string _cdcVersion;
    std::vector<std::vector<CDCTriggerLayer *> *> _superLayers;
    std::vector<std::vector<CDCTriggerLayer *> *> _axialSuperLayers;
    std::vector<std::vector<CDCTriggerLayer *> *> _stereoSuperLayers;
    std::vector<CDCTriggerLayer *> _layers;
    std::vector<CDCTriggerLayer *> _axialLayers;
    std::vector<CDCTriggerLayer *> _stereoLayers;
    std::vector<CDCTriggerWire *> _wires;
    std::vector<CDCTriggerWire *> _hitWires;
    std::vector<CDCTriggerWireHit *> _hits;
    std::vector<CDCTriggerWireHit *> _axialHits;
    std::vector<CDCTriggerWireHit *> _stereoHits;
    std::vector<CDCTriggerWireHit *> _badHits;
    std::vector<CDCTriggerWireHitMC *> _hitsMC;
    std::vector<CDCTriggerTrackSegment *> _tss;
    std::vector<CDCTriggerTrackSegment *> _tsHits;
    std::vector<CDCTriggerLayer *> _tsLayers;

    float _fudgeFactor;
    float * _width;
    float * _r;
    float * _r2;

    const GDLClock _clock;
    const double _offset;

  public: // Utility functions
    static std::string itostring(int i);
    static std::string dtostring(double d, unsigned int precision);
    static std::string carstring(const std::string &s);
    static std::string cdrstring(const std::string &s);
    static void bitDisplay(unsigned);
    static void bitDisplay(unsigned val,
			   unsigned firstDigit,
			   unsigned lastDigit);
};

//-----------------------------------------------------------------------------

#ifdef CDCTrigger_NO_INLINE
#define inline
#else
#undef inline
#define CDCTrigger_INLINE_DEFINE_HERE
#endif

#ifdef CDCTrigger_INLINE_DEFINE_HERE

inline
int
CDCTrigger::debugLevel(void) const {
    return _debugLevel;
}

inline
int
CDCTrigger::debugLevel(int a) const {
    return _debugLevel = a;
}

inline
unsigned
CDCTrigger::nLocalLayer(unsigned superLayerId) const {
    std::vector<CDCTriggerLayer *> * superLayer = _superLayers[superLayerId];
    if (! superLayer) return 0;
    return superLayer->size();
}

inline
const CDCTriggerLayer * const
CDCTrigger::layer(unsigned id) const {
    return _layers[id];
}

inline
const std::vector<CDCTriggerLayer *> * const
CDCTrigger::superLayer(unsigned id) const {
    return _superLayers[id];
}

inline
float
CDCTrigger::fudgeFactor(void) const {
    return _fudgeFactor;
}

inline
float
CDCTrigger::fudgeFactor(float a) {
    return _fudgeFactor = a;
}

inline
std::string
CDCTrigger::versionCDC(void) const {
    return std::string(_cdcVersion);
}

inline
unsigned
CDCTrigger::nWires(void) const {
    return _wires.size();
}

inline
unsigned
CDCTrigger::nSuperLayers(void) const {
    return _superLayers.size();
}

inline
unsigned
CDCTrigger::nStereoLayers(void) const {
    return _stereoLayers.size();
}

inline
unsigned
CDCTrigger::nAxialLayers(void) const {
    return _axialLayers.size();
}

inline
unsigned
CDCTrigger::nAxialSuperLayers(void) const {
    return _axialSuperLayers.size();
}

inline
unsigned
CDCTrigger::nStereoSuperLayers(void) const {
    return _stereoSuperLayers.size();
}

inline
unsigned
CDCTrigger::nLayers(void) const {
    return _layers.size();
}

inline
float
CDCTrigger::cellWidth(unsigned a) const {
    return _width[a];
}

inline
float
CDCTrigger::superLayerR(unsigned i) const {
    return _r[i];
}

inline
float
CDCTrigger::superLayerR2(unsigned i) const {
    return _r2[i];
}

inline
const CDCTriggerTrackSegment *
CDCTrigger::trackSegment(unsigned id) const {
    return _tss[id];
}

inline
unsigned
CDCTrigger::nTrackSegments(void) const {
    return _tss.size();
}

inline
const GDLClock &
CDCTrigger::systemClock(void) const {
    return _clock;
}

inline
double
CDCTrigger::systemOffsetMC(void) const {
    return _offset;
}

inline
const std::vector<const CDCTriggerTrackSegment *>
CDCTrigger::tsHits(void) const {
    std::vector<const CDCTriggerTrackSegment *> t;
    t.assign(_tsHits.begin(), _tsHits.end());
    return t;
}

#endif

#undef inline

} // namespace Belle2

#endif /* CDCTrigger_FLAG_ */
