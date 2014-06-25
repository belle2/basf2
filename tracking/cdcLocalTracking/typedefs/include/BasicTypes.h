/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef BASICTYPES_H
#define BASICTYPES_H

#include <cstddef> //for size_t
#include <cmath>

//#include <Rtypes.h> //for nullptr remove for c++11  !!

namespace Belle2 {

  namespace CDCLocalTracking {

    /** @name Basic integer types*/
    /**@{*/

    typedef int Index;
    const Index INVALID_INDEX = -999;


    /// Track id type
    typedef int ITrackType;
    const ITrackType INVALID_ITRACK = -998; ///< Constant to mark an invalid track id

    /**@}*/

    /** @name Basic floatig point types*/
    /**@{*/

    /// The float type used by the algorithm. Just as necessary for speed.
    typedef float FloatType;
    /// The famous number
    const FloatType PI = 3.141592653589793238462;

    /// An additive measure of quality (e.g. logarithms of probabilities)
    typedef float Weight;

    /// Constant for the highest possible weight
    extern const Weight HIGHEST_WEIGHT;

    /// Constant for the lowest possible weight
    extern const Weight LOWEST_WEIGHT;
    /**@}*/


    /** @name Definition of signs*/
    /**@{*/
    /// Type transporting a sign
    typedef signed short SignType;
    //typedef signed char SignType; // This can be used for performance maximization but makes lousy outpu
    const SignType PLUS = 1; ///< Constant for plus sign
    const SignType MINUS = -1; ///< Constant for minus sign
    const SignType ZERO = 0;  ///< Constant for undefined sign
    /**@}*/

    /** @name Wire and layer ids */
    /**@{*/
    ///The type of the wire ids
    typedef signed short IWireType;

    ///The type of the layer and superlayer ids
    typedef signed short ILayerType;

    ///The type of the layer and superlayer ids
    typedef ILayerType ISuperLayerType;

    const ISuperLayerType NSUPERLAYERS = 9; ///< Constant representing the total number of cdc superlayers
    const ISuperLayerType INNER_ISUPERLAYER = -1; ///< Constant marking the subdetectors closer to the IP than the CDC.
    const ISuperLayerType OUTER_ISUPERLAYER = NSUPERLAYERS; ///< Constant marking the subdetectors further away from the IP than the CDC.
    const ISuperLayerType INVALID_ISUPERLAYER = 15; ///< Constant making an invalid superlayer id

    /// Indicates if the given number corresponds to a true cdc superlayer - excludes the logic ids for inner and outer volumn
    bool isValidISuperLayer(const ISuperLayerType& iSuperLayer);

    /// Indicates if the given number corresponds to a logical superlayer - includes the logic ids for inner and outer volumn
    bool isLogicISuperLayer(const ISuperLayerType& iSuperLayer);

    /// Returns the logical superlayer number at the given radius
    ISuperLayerType getISuperLayerAtPolarR(const FloatType& polarR);

    /// Returns the logical superlayer number at the given radius
    ISuperLayerType isAxialISuperLayer(const ISuperLayerType& iSuperLayer);



    const ILayerType INVALID_ILAYER = 127; ///< Constant making an invalid layer id
    const IWireType INVALID_IWIRE = 32767; ///< Constant making an invalid wire id

    const ILayerType INVALIDSUPERLAYER = INVALID_ISUPERLAYER; ///Legacy constant
    const ILayerType INVALIDLAYER = INVALID_ILAYER; ///Legacy constant
    const IWireType INVALIDWIRE = INVALID_IWIRE; ///Legacy constant
    /**@}*/

    /** @name Wire neighborhood relation */
    /**@{*/
    /// Type for the neighbor relationship from in wire to an other, imagined in the clock
    typedef signed short WireNeighborType;
    const WireNeighborType CW_OUT_NEIGHBOR = 1;  ///< Constant for clockwise outwards
    const WireNeighborType CW_NEIGHBOR = 3; ///< Constant for clockwise
    const WireNeighborType CW_IN_NEIGHBOR = 5; ///< Constant for clockwise inwards
    const WireNeighborType CCW_IN_NEIGHBOR = 7; ///< Constant for counterclockwise inwards
    const WireNeighborType CCW_NEIGHBOR = 9; ///< Constant for counterclockwise
    const WireNeighborType CCW_OUT_NEIGHBOR = 11; ///< Constant for counterclockwise outwards
    /**@}*/

    /** @name Axial type */
    /**@{*/
    typedef SignType AxialType; ///< Type for the stereo property of the wire
    const AxialType AXIAL = 0; ///< Constant for an axial wire
    const AxialType STEREO_U = 1; ///< Constant for an stereo wire in the U configuration
    const AxialType STEREO_V = -1;  ///< Constant for an stereo wire in the V configuration

    const AxialType INVALID_AXIALTYPE = -127; ///< Constant for an invalid stereo information
    /**@}*/


  } // namespace CDCLocalTracking

} // namespace Belle2
#endif // BASICTYPES
