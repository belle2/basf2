//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TCDC.h
// Section  : CDC Tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TCDC_FLAG_
#define TCDC_FLAG_

#define TCDC_UNDEFINED 999999

#include <string>
#include <vector>
#include <TFile.h>
#include <TTree.h>
#include <TVectorD.h>
#include <TH1.h>
#include "tracking/modules/trasan/TLayer.h"

namespace HepGeom {
    template <class T> class Point3D;
}

namespace Belle {

class CDCGeometryPar;
class TLayer;
class TWire;
class TWireHit;
class TWireHitMC;

///  The instance of TCDC is a singleton. 'TCDC::getTCDC()'
///  gives you a pointer to access the TCDC. Geometrical information
///  is initialized automatically. Before accessing hit information,
///  user has to call 'update()' to initialize hit information event
///  by event.

class TCDC {

  public:

    /// returns TCDC object with specific configuration.
    static TCDC * getTCDC(const std::string & configFile);
    
    /// returns TCDC object. TCDC should be created with specific
    /// configuration before calling this function.
    static TCDC * getTCDC(void);

  private:

    /// Constructor
    TCDC(const std::string & configFile);

    /// Destructor
    virtual ~TCDC();

    /// initializes CDC geometry.
    void initialize(void);

    /// configures trigger modules for firmware simulation.
    void configure(void);

    /// terminates when run is finished
    void terminate(void);

    /// simulates track segment decisions.

  public:// Selectors

    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// returns CDC version.
    std::string versionCDC(void) const;

    /// sets simulation mode.
    unsigned mode(void) const;

    /// sets simulation mode.
    unsigned mode(unsigned);

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

    /// returns a pointer to a wire. 0 will be returned if 'wireId' is
    /// invalid.
    const TWire * wire(unsigned wireId) const;

    /// returns a pointer to a wire. 'localId' can be negative. 0 will
    /// be returned if 'layerId' is invalid.
    const TWire * wire(unsigned layerId, int localId) const;

    /// returns a pointer to a wire.
//    const TWire * wire(const HepGeom::Point3D<double> & point) const;

    /// returns a pointer to a wire.
    const TWire * wire(float r, float phi) const;

    /// returns a pointer to a layer. 0 will be returned if 'id' is invalid.
    const TLayer * layer(unsigned id) const;

    /// returns a pointer to a super-layer. 0 will be returned if 'id' is invalid.
    const std::vector<TLayer *> * superLayer(unsigned id) const;

    /// returns \# of wire layers in a super layer. 0 will be returned
    /// if 'superLayerId' is invalid.
    unsigned nLocalLayers(unsigned superLayerId) const;

    /// return \# of wires.
    unsigned nWires(void) const;

    /// returns \# of super layers.
    unsigned nSuperLayers(void) const;

    /// return \# of layers.
    unsigned nLayers(void) const;

    /// return \# of axial layers.
    unsigned nAxialLayers(void) const;

    /// returns \# of stereo layers.
    unsigned nStereoLayers(void) const;

    /// return \# of axial super layers.
    unsigned nAxialSuperLayers(void) const;

    /// returns \# of stereo super layers.
    unsigned nStereoSuperLayers(void) const;

    /// returns cell width in unit of radian.
    float cellWidth(unsigned superLayerId) const;

    /// returns inner radius of super layer.
    float superLayerR(unsigned superLayerId) const;

    /// returns (inner radius)^2 of super layer.
    float superLayerR2(unsigned superLayerId) const;

  public:// Event by event hit information.

    /// clears all TCDC hit information.
    void clear(void);

    /// clears TCDC information.
    void fastClear(void);

    /// updates TCDC wire information. clear() is called in this function.
    void update(bool mcAnalysis = true);

    /// updates TCDC wire information by Hardware data
    /// 0: From CDC FE ASCII file (Implementing)
    /// 1: From CDC FE-DAQ root file (Not implemented)
    /// 2: From TSIM root file (Not implemented)
    void updateByData(int inputMode);

    /// returns a list of TWireHit. 'update()' must be called
    /// before calling this function.
    std::vector<const TWireHit *> hits(void) const;

    /// returns a list of axial hits. 'update()' must be called before
    /// calling this function.
    std::vector<const TWireHit *> axialHits(void) const;

    /// returns a list of stereo hits. 'update()' must be called
    /// before calling this function.
    std::vector<const TWireHit *> stereoHits(void) const;

    /// returns a list of TWireHitMC. 'updateMC()' must be called
    /// before calling this function.
    std::vector<const TWireHitMC *> hitsMC(void) const;

    /// returns bad hits(finding invalid hits).
//  std::vector<const TWireHit *> badHits(void) const;

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
    bool neighbor(const TWire & w0, const TWire & w1) const;

  private:

    /// classify hits.
    void classification(void);

    /// updates TCDC information for MC.
    void updateMC(void);

  private:

    /// CDC trigger singleton.
    static TCDC * _cdc;

    /// Debug level.
    mutable int _debugLevel;

    /// Super layers.
    std::vector<std::vector<TLayer *> *> _superLayers;

    /// Axial super layers.
    std::vector<std::vector<TLayer *> *> _axialSuperLayers;

    /// Stereo super layers.
    std::vector<std::vector<TLayer *> *> _stereoSuperLayers;

    /// All layers.
    std::vector<TLayer *> _layers;

    /// Axial layers.
    std::vector<TLayer *> _axialLayers;

    /// Stereo layers.
    std::vector<TLayer *> _stereoLayers;

    /// All wires.
    std::vector<TWire *> _wires;

    /// Wires with a hit.
    std::vector<TWire *> _hitWires;

    /// CDC hits.
    std::vector<TWireHit *> _hits;

    /// CDC hits on axial wires.
    std::vector<TWireHit *> _axialHits;

    /// CDC hits on stereo wires.
    std::vector<TWireHit *> _stereoHits;

    /// Bad CDC hits.(not used now)
    std::vector<TWireHit *> _badHits;

    /// MC info. of CDC hits.
    std::vector<TWireHitMC *> _hitsMC;

    /// Fudge factor for position error.
    float _fudgeFactor;

    /// Cell width in radian.
    float * _width;

    /// R of cell.
    float * _r;

    /// R^2 of cell.
    float * _r2;
};

//-----------------------------------------------------------------------------

inline
int
TCDC::debugLevel(void) const {
    return _debugLevel;
}

inline
int
TCDC::debugLevel(int a) const {
    return _debugLevel = a;
}

inline
unsigned
TCDC::nLocalLayers(unsigned superLayerId) const {
    std::vector<TLayer *> * superLayer = _superLayers[superLayerId];
    if (! superLayer) return 0;
    return superLayer->size();
}

inline
const TLayer *
TCDC::layer(unsigned id) const {
    return _layers[id];
}

inline
const std::vector<TLayer *> *
TCDC::superLayer(unsigned id) const {
    return _superLayers[id];
}

inline
float
TCDC::fudgeFactor(void) const {
    return _fudgeFactor;
}

inline
float
TCDC::fudgeFactor(float a) {
    return _fudgeFactor = a;
}

inline
std::string
TCDC::versionCDC(void) const {
    return std::string("standard");
}

inline
unsigned
TCDC::nWires(void) const {
    return _wires.size();
}

inline
unsigned
TCDC::nSuperLayers(void) const {
    return _superLayers.size();
}

inline
unsigned
TCDC::nStereoLayers(void) const {
    return _stereoLayers.size();
}

inline
unsigned
TCDC::nAxialLayers(void) const {
    return _axialLayers.size();
}

inline
unsigned
TCDC::nAxialSuperLayers(void) const {
    return _axialSuperLayers.size();
}

inline
unsigned
TCDC::nStereoSuperLayers(void) const {
    return _stereoSuperLayers.size();
}

inline
unsigned
TCDC::nLayers(void) const {
    return _layers.size();
}

inline
float
TCDC::cellWidth(unsigned a) const {
    return _width[a];
}

inline
float
TCDC::superLayerR(unsigned i) const {
    return _r[i];
}

inline
float
TCDC::superLayerR2(unsigned i) const {
    return _r2[i];
}

} // namespace Belle

#endif /* TCDC_FLAG_ */
