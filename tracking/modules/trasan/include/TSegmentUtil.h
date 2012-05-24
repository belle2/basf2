#ifndef TSEGMENTUTIL_FLAG
#define TSEGMENTUTIL_FLAG

#include <vector>
struct reccdc_wirhit;
#if defined(BELLE_NAMESPACE)
namespace Belle {
#endif

// CDC

// returns # of SegId(Segment ID) of CDC, that is, 0, 1, or 2.
  unsigned
  CDCSegId(unsigned localId,   // CDC Local Wire ID
           unsigned layerId,   // CDC Global Layer ID
           unsigned& segId0,   // obtained SegId of CDC(1st)
           unsigned& segId1);  // obtained SegId of CDC(2nd)

// returns CDC Segment Layer Id.(not CDC layer Id!!)
  unsigned
  CDCSegLayerId(unsigned id); // CDC Global Wire ID

// returns superlayer Id.
  unsigned
  CDCSuperLayerId(unsigned id); // CDC Global Wire ID

// returns moved Seg Id of CDC Segment w.r.t refSegId.
  int
  CDCMovedSegId(unsigned refSegId,    // reference CDC Segment Id
                unsigned segId,       // self CDC Segment Id
                unsigned segLayerId,  // self CDC Segment Layer Id
                unsigned type);       // type of "move" method

// returns moved Seg Id of CDC Segment w.r.t refSegId.
// wide region version
  int
  CDCMovedSegId2(unsigned refSegId,    // reference CDC Segment Id
                 unsigned segId,       // self CDC Segment Id
                 unsigned segLayerId,  // self CDC Segment Layer Id
                 unsigned type);       // type of "move" method

// SVD

// returns SVD Segment Layer Id.(== SVD layer Id!!)
  unsigned
  SVDSegLayerId(unsigned id);

// returns # of SegId(Segment ID) of SVD, that is, 0, 1, or 2.
  unsigned
  SVDSegId(double x,             // x of SVD Hit Point at Global Frame
           double y,             // y of SVD Hit Point at Global Frame
           unsigned& segId0,     // obtained SegId of SVD(1st)
           unsigned& segId1);    // obtained SegId of SVD(2nd)

// returns moved Seg Id of SVD Segment w.r.t refSegId.
  int
  SVDMovedSegId(unsigned refSegId,    // reference SVD Segment Id
                unsigned segId,       // self SVD Segment Id
                unsigned segLayerId,  // self SVD Segment Layer Id
                unsigned type);       // type of "move" method

// returns moved Seg Id of SVD Segment w.r.t refSegId.
// wide region version
  int
  SVDMovedSegId2(unsigned refSegId,    // reference SVD Segment Id
                 unsigned segId,       // self SVD Segment Id
                 unsigned segLayerId,  // self SVD Segment Layer Id
                 unsigned type);       // type of "move" method

  class Gen_hepevt;
// return hep-info from Reccdc_wirhit.
// if no-hep, return NULL.
  Gen_hepevt*
  wirhit2hep(const struct reccdc_wirhit* wirhit);

// return hep-info from Recsvd_cluster.
// if no-hep, return NULL.
  class Recsvd_cluster;
  Gen_hepevt*
  cluster2hep(const Recsvd_cluster* clus);

#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif
#endif /* TSEGMENTUTIL_FLAG */
