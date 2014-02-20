/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCWIRELAYER_H
#define CDCWIRELAYER_H

#include <TVector3.h>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/topology/CDCWire.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representating a sense wire layer in the central drift chamber.
    /** This class represents the a CDC layer as a range of wires. \n
     *  The range is sorted with increasing wire ids for maximal fast look up. \n
     *  If offers methods for checking the validity of wire ids, a getter for the wires in this layer \n
     *  additionally to be iterable as a range. \n
     *  Furthermore it stores averaged geometrical quantities combined from the wires it contains \n
     *  The class does not provide the memory for the wires by itself. \n
     *  It uses iterators into a vector of CDCWires. \n
     *  The vector of CDCWires is normally provided by the CDCWireTopology object instance. \n
     * Note : All possible layer object are stored in the CDCWireTopology \n
     * which you can get with the static getInstance() functions. \n
     * There is rarely a need for constructing a layer object and \n
     * it should be avoided for speed reasons. */
    class CDCWireLayer : public CDCLocalTracking::UsedTObject {

    public:
      /// The underlying container type storing range of wires referenced by this class
      typedef std::vector<Belle2::CDCLocalTracking::CDCWire> Container;
      /// The value type of the iterator range
      typedef const Belle2::CDCLocalTracking::CDCWire value_type;

      /// The type of the iterator for the wire range
      typedef Container::iterator iterator;

      /// The type of the const_iterator for the wire range
      typedef Container::const_iterator const_iterator;

    public:
      /// Default constructor for ROOT compatibility
      CDCWireLayer();

      /// Constructor taking the range of wires the layer shall contain. Use rather getInstance() to avoid instance constructions.
      CDCWireLayer(const const_iterator& begin, const const_iterator& end);

      /// Default destructor
      ~CDCWireLayer();

      /** @name Static instance getters
       *  Getter for the already constructed instances from the CDCWireTopology::getInstance() object.
       */
      /**@{*/
      /// Getter from the the continuous layer id. Does not construct a new object.
      static const CDCWireLayer* getInstance(const ILayerType& iCLayer);
      /// Getter from the superlayer id, the layer id. Does not construct a new object.
      static const CDCWireLayer* getInstance(const ILayerType& iSuperLayer,
                                             const ILayerType& iLayer);
      /// Getter for the layer with superlayer id 0, layer id 0.
      static const CDCWireLayer* getLowest() { return getInstance(0, 0); }
      /**@}*/


    public:

      /// Intializes the wire layer variables to the average of according variables in the wire range
      void initialize();


      /** @name Layer index
       */
      /**@{*/
      /// Getter for the continuous layer id unique over all layers
      ILayerType getICLayer() const { return first().getICLayer(); }

      /// Getter for the layer id unique within the superlayer
      ILayerType getILayer()       const { return first().getILayer(); }

      /// Getter for the super layer id
      ILayerType getISuperLayer()  const { return first().getISuperLayer(); }
      /**@}*/


      /** @name Shift property
        */
      /**@{*/
      /// Getter for the numbering shift. See details.
      /** The shift indicates if the wire with index zero in this layer \n
       *  is more clockwise or counterclockwise than \n
       *  the zero wire in the zero layer within this superlayer. \n
       *  This can be used to speed up the lookup inward and outward neighbors \n
       *  of the wires in this layer. For even layers the shift should be ZERO. \n
       *  The shift is not set by this layer since it can not know the shift by its \n
       *  own wires alone, but it is set by the CDCWireSuperLayer. \n
       *  The later is the main user of this property. \n
       *  Watch out : The CDCGeometryPar class just grew a shift property. \n
       *  Check if that is the same as this.
       */
      const CCWInfo& getShift() const { return m_shift; }

      /// Sets the shift of this wire layer
      void setShift(const CCWInfo& shift) const { m_shift = shift; }
      /**@}*/


      /** @name Wire range
       */
      /**@{*/
      /// Getter for the begin iterator of the wire range
      const_iterator begin() const { return m_begin; }

      /// Getter for the end iterator of the wire range
      const_iterator end() const { return m_end; }

      /// Iteration helper for python only
      /** In order to retrieve wires from a layer from python the stl iterators are not usable, \n
       *  so we give an other mechanism for iteration. \n
       *  To get the first wire call the function with no argument. \n
       *  Call the function with the last wire yielded to get the next one. \n
       *  If there is no next wire anymore at the end of the wire range return nullptr */
      const CDCWire* nextWire(const CDCWire* wire = nullptr) const;

      /// Getter for the number of wires in this layer
      IWireType size() const { return m_end - m_begin; }

      /// Checks if the given wire id belongs to a valid wire in this layer
      inline bool isValidIWire(const IWireType& iWire) const
      { return 0 <= iWire and iWire < int(size()); }

      /// Gives the wire by its id in the layer.
      const CDCWire& getWire(const IWireType& iWire) const
      { return *(begin() + iWire); }

      /// Getter for the first wire of the layer
      const CDCWire& first() const { return *(begin()); }
      /// Getter for the last wire of the layer
      const CDCWire& last() const { return *(--end()); }

      /// Setter for the range of the wires
      void setWireRange(const const_iterator& begin, const const_iterator& end)
      { m_begin = begin; m_end = end; initialize(); }
      /**@}*/


      /** @name Neighborhood of wires in this layer
        */
      /**@{*/
      /// Gives the wire by its id in the layer.
      /** Gives the wire by its id, but transforms it to valid range first, \n
       *  so even if the id is out of range you will get a valid wire object. \n
       *  This makes the retrival of neighbors much easier if the neighborhood \n
       *  is around the discontinuity near the zero wires.
       */
      const CDCWire& getWireSafe(const IWireType& iWire) const
      { return *(begin() + (iWire % size() + size())  % size()); }

      /// Getter for the first counterclockwise neighbor by wire id in the layer.
      const CDCWire& getNeighborCCW(const IWireType& iWire) const
      { return getWire((iWire + 1) % size()); }

      /// Getter for the first clockwise neighbor by wire id in the layer.
      const CDCWire& getNeighborCW(const IWireType& iWire) const
      { return getWire((iWire + size() - 1) % size()); }

      /// Getter for the first counterclockwise neighbor by wire id in the layer.
      const CDCWire& getSecondNeighborCCW(const IWireType& iWire) const
      { return getWire((iWire + 2) % size()); }

      /// Getter for the first clockwise neighbor by wire id in the layer.
      const CDCWire& getSecondNeighborCW(const IWireType& iWire) const
      { return getWire((iWire + size() - 2) % size()); }
      /**@}*/


      /** @name Averaged geometry properties
        *  The following getters return variables calculated as average from the contained CDCWire.
        */
      /**@{*/

      /// Indicates if the wire is axial or stereo
      bool isAxial() const { return getAxialType() == AXIAL; }

      /// Getter for the axial type of the wire layer
      /** Gives the axial type of the wire. \n
       *  Result is one of AXIAL, STEREO_U and STEREO_V. \n
       *  The axial type is shared by all wires in the same superlayer. \n
       *  The superlayer pattern for Belle II is AUAVAUAVA according the TDR
       */
      AxialType getAxialType() const { return first().getAxialType(); }

      ///Getter for the averaged skew of all wires in this layer
      const FloatType& getSkew() const { return m_skew; }

      ///Getter for the average stereo angle of all wires in this layer
      FloatType getStereoAngle() const { return std::atan(getSkew() * getMinPolarR()); }

      ///Getter for the closest distance to the beamline ( z-axes ) of all wires in this layer
      const FloatType& getMinPolarR() const { return m_minPolarR; }

      ///Getter for the average distance to the beamline ( z-axes ) at the forward joint points of all wires in this layer
      const FloatType& getForwardPolarR() const { return m_forwardPolarR; };

      ///Getter for the average distance to the beamline ( z-axes ) at the backward joint points of all wires in this layer
      const FloatType& getBackwardPolarR() const { return m_backwardPolarR; };

      ///Getter for the average z coordinate at the forward joint points of all wires in this layer
      const FloatType& getForwardZ() const { return m_forwardZ; }

      ///Getter for the average z coordinate at the backward joint points of all wires in this layer
      const FloatType& getBackwardZ() const { return m_backwardZ; }

      ///Getter for the average polar angle of the forward joint points of the wires relativ to their references
      const FloatType& getForwardPhiToRef() const { return m_forwardPhiToRef; }

      ///Getter for the average polar angle of the backward joint points of the wires relativ to their references
      const FloatType& getBackwardPhiToRef() const { return m_backwardPhiToRef; }

      /// Getter for the average polar angle range covered by the wires relativ to their respective reference points.
      /** Gives the range of polar angles the stereo wires cover relativ to their reference positions. \n
       *  A stereo wire has a certain extension visible as a stretch in the xy projection. \n
       *  This translates to a range of polar angles, which can be used for a quick check \n
       *  if an xy projected track could have hit a wire or not. Taking the average of all wires \n
       *  in the layer makes this value more relieable. \n
       *  The range covers reaches from negativ to positiv values since the references is somewhere \n
       *  in the middle of the wires. \n
       *  The smaller value is always negativ and gets stored as .first of the pair. \n
       *  The bigger value is positiv and gets stored as .second of the pair. \n
       *  For axial wires layer both values or zero since axial wires form a point in the xy projection.
       */
      const std::pair<FloatType, FloatType>& getPhiRangeToRef() const
      { return m_phiRangeToRef; }
      /**@}*/

      /** @name Geometry properties copied from the CDCGeometryPar
        */
      /**@{*/
      ///Getter for inner radius of the layer as taken from the CDCGeometryPar
      const FloatType& getInnerPolarR() const { return m_innerPolarR; }

      ///Getter for outer radius of the layer as taken from the CDCGeometryPar
      const FloatType& getOuterPolarR() const { return m_outerPolarR; }
      /**@}*/

    private:
      /// Begin of the wire range.
      /** Stores iterator pointing to the stored vector<CDCWire> in the related CDCWireTopology object. */
      const_iterator m_begin;
      /// End of the wire range.
      /** Stores iterator pointing to the stored vector<CDCWire> in the related CDCWireTopology object. */
      const_iterator m_end;

      /// Indicates if this layer is shifted clockwise or counterclockwise. \n
      /** The shift indicates if the wire with index zero in this layer \n
       *  is more clockwise or counterclockwise than  \n
       *  the zero wire in the zero layer within this superlayer. \n
       *  This can be used to speed up the lookup inward and outward \n
       *  neighbors of the wires in this layer. For even layers the shift should be 0.
       */
      mutable CCWInfo m_shift;

      /** @name Property averages from the contained CDCWires
       *  The following variables are calculated from the contained CDCWire
       *  on initialization of the CDCWireLayer.
       */
      //@{
      FloatType m_skew; ///< Storage for average skew
      FloatType m_minPolarR; ///< Storage for minimal distance from beamline

      FloatType m_forwardPolarR; ///< Storage for average distance from beamline of forward wire ends
      FloatType m_backwardPolarR; ///< Storage for average distance from beamline of backward wire ends

      FloatType m_forwardZ; ///< Storage for z position of forward wire ends
      FloatType m_backwardZ; ///< Storage for z position of backward wire ends

      FloatType m_forwardPhiToRef; ///< Storage of the average angle between forward and ref position in the xy projection (see getPhiRangeToRef() for details)
      FloatType m_backwardPhiToRef;///< Storage of the average angle between backward and ref position in the xy projection (see getPhiRangeToRef() for details)

      std::pair<FloatType, FloatType> m_phiRangeToRef; ///< Storage of the angle range to ref positions in the xy projection (see getPhiRangeToRef() for details)
      //@}

      /** @name Property averages CDCGeometryPar
       *  The following variables are taken directly from the CDCGeometryPar
       *  on initialization of the CDCWireLayer.
       */
      //@{
      FloatType m_innerPolarR; ///< Storage of the inner radius of the wire layer as taken from the CDCGeometryPar instance
      FloatType m_outerPolarR; ///< Storage of the outer radius of the wire layer as taken from the CDCGeometryPar instance
      //@}

      /** ROOT Macro to make CDCWireLayer a ROOT class.*/
      ClassDefInCDCLocalTracking(CDCWireLayer, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCWIRELAYER
