// // // // // #ifndef SEGMENTFILTERVXDTF_HH
// // // // // #define SEGMENTFILTERVXDTF_HH
// // // // //
// // // // // #include "tracking/dataobjects/FullSecID.h"
// // // // // #include "tracking/trackFindingVXD/twoHitFilters/Distance1DZ.h"
// // // // // #include "tracking/trackFindingVXD/twoHitFilters/Distance1DZTemp.h"
// // // // // #include "tracking/trackFindingVXD/twoHitFilters/Distance3DNormed.h"
// // // // // #include "tracking/trackFindingVXD/twoHitFilters/SlopeRZ.h"
// // // // // #include "tracking/trackFindingVXD/twoHitFilters/Distance1DZSquared.h"
// // // // // #include "tracking/trackFindingVXD/twoHitFilters/Distance2DXYSquared.h"
// // // // // #include "tracking/trackFindingVXD/twoHitFilters/Distance3DSquared.h"
// // // // // #include "tracking/trackFindingVXD/filterTools/Shortcuts.h"
// // // // // #include "tracking/trackFindingVXD/filterTools/Observer.h"
// // // // //
// // // // // #include "tracking/vxdCaTracking/VXDTFHit.h"
// // // // // #include "tracking/dataobjects/FullSecID.h"
// // // // // #include <unordered_map>
// // // // // #include <functional>
// // // // //
// // // // // #include <iostream>
// // // // //
// // // // //
// // // // // namespace Belle2 {
// // // // //
// // // // //
// // // // //   class VXDTFSegmentFilter {
// // // // //   public:
// // // // //
// // // // //     typedef
// // // // //     decltype((0. < Distance3DSquared<VXDTFHit>()   < 0.).observe(Observer()).enable()&&
// // // // //              (0. < Distance2DXYSquared<VXDTFHit>() < 0.).observe(Observer()).enable()&&
// // // // //              (0. < Distance1DZ<VXDTFHit>()         < 0.)/*.observe(Observer())*/.enable()&&
// // // // //              (0. < SlopeRZ<VXDTFHit>()             < 0.).observe(Observer()).enable()&&
// // // // //              (Distance3DNormed<VXDTFHit>() < 0.).enable()) filter_t;
// // // // //
// // // // //     typedef std::pair< FullSecID, FullSecID > key_t;
// // // // //
// // // // //     VXDTFSegmentFilter(int filterSize = 10000);
// // // // //
// // // // //     void addFriendsSectorFilter(const FullSecID& inner,
// // // // //                                 const FullSecID& outer,
// // // // //                                 filter_t& filter)
// // // // //     {
// // // // //
// // // // //       m_filter[ key_t(inner, outer) ] = filter;
// // // // //
// // // // //     }
// // // // //
// // // // //     const filter_t& friendsSectorFilter(const FullSecID& inner,
// // // // //                                         const FullSecID& outer) const
// // // // //     {
// // // // //       static filter_t just_in_case;
// // // // //       auto filter = m_filter.find(key_t(inner, outer));
// // // // //       if (filter != m_filter.end())
// // // // //         return filter->second;
// // // // //
// // // // //       return just_in_case;
// // // // //     }
// // // // //
// // // // //   private:
// // // // //     static size_t secIdPairHasher(const key_t& pair)
// // // // //     {
// // // // //       auto first = (size_t) pair.first.getFullSecID();
// // // // //       auto second = (size_t) pair.second.getFullSecID();
// // // // //
// // // // //       return first ^ (second + 0x9e3779b9 + (first << 6) + (first >> 2));
// // // // //     };
// // // // //
// // // // //
// // // // //     std::unordered_map< key_t,
// // // // //         filter_t , function<decltype(secIdPairHasher)> >  m_filter;
// // // // //   };
// // // // //
// // // // // }
// // // // //
// // // // //
// // // // // #endif
