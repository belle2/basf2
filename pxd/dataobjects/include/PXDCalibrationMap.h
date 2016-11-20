/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Peter Kodys, Peter Kvasnicka                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <tuple>
#include <unordered_map>
#include <boost/functional/hash.hpp>


namespace Belle2 {
  /** Unoredered map used in cluster shape correction calibration, with its related types.
   */
  class PXDCalibrationMap {
  public:
    /** Key type for the map: tuple containing:
     * cluster shape code (see pxd/reconstruction/ClusterShape.h),
     * kind of pixel 1:small pixels layer 1, 2:large pixels layer 1, etc.,
     * axis 0=u, 1=v,
     * incidence angle in u (perpendicular = 0), in steps from -9 to 8,
     * incidence angle in v (perpendicular = 0), in steps from -9 to 8
     */
    typedef std::tuple<short, short, short, short, short> key_type;

    /** Virtual destructor */
    virtual ~PXDCalibrationMap() {}

    /** Return value from the map */
    double getValue(short shape, short pixel, short axis, short angle_u, short angle_v)
    {
      key_type key = std::make_tuple(shape, pixel, axis, angle_u, angle_v);
      auto result = m_map.find(key);
      if (result == m_map.end())
        return 0.0;
      else return result->second;
    }

    /** Add a key-value pair to the map */
    void setValue(short shape, short pixel, short axis, short angle_u, short angle_v, double value)
    {
      auto result = m_map.insert(std::make_pair(std::make_tuple(shape, pixel, axis, angle_u, angle_v), value));
      if (!result.second)
        result.first->second = value;
    }

    /** Hash generator for map keys */
    struct key_hash : public std::unary_function<key_type, std::size_t> {
      std::size_t operator()(const key_type& k) const
      {
        using boost::hash_value;
        using boost::hash_combine;
        std::size_t seed = 0;
        hash_combine(seed, hash_value(std::get<0>(k)));
        hash_combine(seed, hash_value(std::get<1>(k)));
        hash_combine(seed, hash_value(std::get<2>(k)));
        hash_combine(seed, hash_value(std::get<3>(k)));
        hash_combine(seed, hash_value(std::get<4>(k)));
        return seed;
      }
    };
    /** Key comparison functor */
    struct key_equal : public std::binary_function<key_type, key_type, bool> {
      bool operator()(const key_type& v0, const key_type& v1) const
      {
        return (
                 std::get<0>(v0) == std::get<0>(v1) &&
                 std::get<1>(v0) == std::get<1>(v1) &&
                 std::get<2>(v0) == std::get<2>(v1) &&
                 std::get<3>(v0) == std::get<3>(v1) &&
                 std::get<4>(v0) == std::get<4>(v1)
               );
      }
    };
    /** The type of the internal hash map. */
    typedef std::unordered_map<key_type, double, key_hash, key_equal> map_typeCorrs;

  private:
    /** The internal map of the class. */
    map_typeCorrs m_map; /**< map that holds the class's data. */

  public:
    ClassDef(PXDCalibrationMap, 1); /**< Class holding PXD shape calibration data.*/
  }; // class PXDCalibrationMap
} // namespace Belle2


