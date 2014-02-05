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
#include <limits>

//#include <Rtypes.h> //for nullptr remove for c++11  !!

namespace Belle2 {

  namespace CDCLocalTracking {

    /** @name Basic integer types*/
    /**@{*/

    typedef int Index;

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


    /** @name Cellular automata types */
    /**@{*/
    typedef Weight CellState;  ///< Type for the cell states in the cellular automata
    typedef Weight CellWeight;  ///< Type for the cell weight in the cellular automata
    typedef Weight NeighborWeight;  ///< Type for the cell weight in the cellular automata
    //const CellState CYCLE_MARKER = 999;

    typedef unsigned CellFlags; ///< Type for the additional status flags of cells in the cellular automata
    const CellFlags IS_SET = 1; ///< Constant for a already updated cell
    const CellFlags IS_START = 2; ///< Constant for a cell that marks the start of a path
    const CellFlags IS_CYCLE = 4; ///< Constant marker for the detection of cycles in the cellular automata
    const CellFlags DO_NOT_USE = 8; ///< Constant for a cell that should not be used

    /// Constant summing all possible cell flags
    const CellFlags ALL_FLAGS = IS_SET + IS_START + IS_CYCLE + DO_NOT_USE;
    /**@}*/

    /** @name Wire and layer ids */
    /**@{*/
    ///The type of the wire ids
    typedef signed short IWireType;

    ///The type of the layer and superlayer ids
    typedef signed short ILayerType;
    //typedef signed char ILayerType;    // This can be used for performance maximization but makes lousy output
    const ILayerType INVALIDSUPERLAYER = 128; ///< Constant making an invalid superlayer id
    const ILayerType INVALIDLAYER = 128; ///< Constant making an invalid layer id
    const IWireType INVALIDWIRE = 32767; ///< Constant making an invalid wire id
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
