/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <list>
#include <string>

class G4LogicalVolume;
class G4Polycone;

namespace Belle2 {
  class GearDir;
  /** Common code concerning the geometry representation of the detector */
  namespace geometry {

    /** Set the color of a logical volume.
     * This function will set the visualization color of a logical volume from a string representation of the color
     * Recognized formats for the color are:
     *  - #rgb where r,g,b are hexadecimal values from 0 to f representing the
     *    color values for red, green and blue respectively
     *  - #rgba where r,g,b,a are hexadecimal values from 0 to f and a
     *    represents the alpha value
     *  - #rrggbb where rr,gg,bb are hexadecimal values from 00 to ff
     *    representing the color values for red, green and blue respectively
     *  - #rrggbbaa where rr,gg,bb,aa are hexadecimal values from 00 to ff and
     *    aa represents the alpha value
     *  - rgb(r,g,b) where r,g,b are float values between 0.0 and 1.0
     *    representing the color values for red, green and blue respectively
     *  - rgb(r,g,b,a) where r,g,b,a are float values between 0.0 and 1.0
     *    representing the color values for red, green, blue and alpha
     *    respectively
     *
     * @param volume Volume for which to set the color
     * @param color  String representation of the color
     */
    void setColor(G4LogicalVolume& volume, const std::string& color);

    /** Helper function to quickly set the visibility of a given volume.
     * @param volume Volume for which to set the visibility
     * @param visible true if the volume should be visible, false otherwise
     */
    void setVisibility(G4LogicalVolume& volume, bool visible);

    /** Create Polycone Shape from XML Parameters.
     * This function will create a polycone shape directly from Gearbox Parameters of the form
     *
     * <minPhi unit="deg">  0</minPhi>
     * <maxPhi unit="deg">360</maxPhi>
     * <Plane>
     *   <posZ unit="mm">-10.0</posZ>
     *   <innerRadius unit="mm"> 20.000</innerRadius>
     *   <outerRadius unit="mm"> 20.000</outerRadius>
     * </Plane>
     * ...
     * <Plane>
     *   <posZ unit="mm">10.0</posZ>
     *   <innerRadius unit="mm"> 15.000</innerRadius>
     *   <outerRadius unit="mm"> 30.000</outerRadius>
     * </Plane>
     *
     * There must be at least two Plane definitions, minPhi and maxPhi can be
     * omitted. minZ and maxZ will return the extents of the Polycone along z
     *
     * @param name   Name of the shape to create
     * @param params GearDir pointing to the parameters
     * @param[out] minZ will contain the minimal z coordinate of the polycone
     * @param[out] maxZ will contain the maximal z coordinate of the polycone
     */
    G4Polycone* createPolyCone(const std::string& name, const GearDir& params, double& minZ, double& maxZ);

    /** Create a solid by roating two polylines around the Z-Axis.
     * This function will create a polycone shape directly from Gearbox
     * Parameters describing the inner and the outer envelope of the polycone.
     * The XML Parameters should be of the form
     *
     * <minPhi unit="deg">  0</minPhi>
     * <maxPhi unit="deg">360</maxPhi>
     * <OuterPoints>
     *   <point><z unit="mm">-393.000</z><x unit="mm">100.000</x></point>
     *   <point><z unit="mm">-337.000</z><x unit="mm"> 85.500</x></point>
     *   ...
     *   <point><z unit="mm">-138.000</z><x unit="mm"> 35.000</x></point>
     * </OuterPoints>
     * <InnerPoints>
     *   <point><z unit="mm">-393.000</z><x unit="mm"> 97.934</x></point>
     *   <point><z unit="mm">-339.000</z><x unit="mm"> 83.952</x></point>
     *   ...
     *   <point><z unit="mm">-138.000</z><x unit="mm"> 33.000</x></point>
     * </InnerPoints>
     *
     * Where OuterPoints and InnerPoints specify a polyline which is the outer
     * respective inner envelope of the Polycone. The number of points doe s
     * not have to be the same for Outer- and InnerPoints. Needed positions
     * will be interpolated when creating the Polycone.
     *
     * The Positions for Outer- and InnerPoints have to be in ascending Z
     * coordinates. The first and last point of OuterPoints will be connected to the
     * first respective last point of InnerPoints. The resulting shape will be
     * rotated around the z axis to create the polycone.
     *
     * @param name Name of the Solid
     * @param params Parameters to use for the Solid
     * @param[out] minZ will contain the minimal z coordinate of the polycone
     * @param[out] maxZ will contain the maximal z coordinate of the polycone
     */
    G4Polycone* createRotationSolid(const std::string& name, const GearDir& params, double& minZ, double& maxZ);

    /** Create a solid by roating two polylines around the Z-Axis.
     * This function will create a polycone shape. The InnerPoints
     * and OuterPoints are passed directly as stl::lists to avoid dependence on
     * gearbox.
     *
     * Where OuterPoints and InnerPoints specify a polyline which is the outer
     * respective inner envelope of the Polycone. The number of points does
     * not have to be the same for Outer- and InnerPoints. Needed positions
     * will be interpolated when creating the Polycone.
     *
     * The Positions for Outer- and InnerPoints have to be in ascending Z
     * coordinates. The first and last point of OuterPoints will be connected to the
     * first respective last point of InnerPoints. The resulting shape will be
     * rotated around the z axis to create the polycone.
     *
     * @param name Name of the Solid
     * @param innerPoints List of inner points
     * @param outerPoints List of outer points
     * @param[out] minZ will contain the minimal z coordinate of the polycone
     * @param[out] maxZ will contain the maximal z coordinate of the polycone
     */
    G4Polycone* createRotationSolid(const std::string& name,
                                    std::list< std::pair<double, double> > innerPoints,
                                    std::list< std::pair<double, double> > outerPoints,
                                    double minPhi, double maxPhi, double& minZ, double& maxZ);


  }
} //Belle2 namespace
