#ifndef TCURLFINDERPARAMETERS_FLAG_
#define TCURLFINDERPARAMETERS_FLAG_


#include <iostream>

namespace Belle {

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif

  class TCurlFinderParameter {
  public:
    TCurlFinderParameter(void):
      ALPHA_SAME_WITH_HELIX(222.376063),
      MIN_RADIUS_OF_STRANGE_TRACK(65.0) {};

    ~TCurlFinderParameter(void) {}

    void now(void) const;


  public :
    //
    // TCurlFinder.cc
    //

    // constant factor
    const double ALPHA_SAME_WITH_HELIX;

    // minimum size of segment
    unsigned MIN_SEGMENT;

    // Salvage Section
    unsigned MIN_SALVAGE;
    double   BAD_DISTANCE_FOR_SALVAGE;
    double   GOOD_DISTANCE_FOR_SALVAGE;

    // If size of the segment is equal to or larger than MIN_SEQUENCE,
    // it may be divided. --> exceptional type 1
    unsigned MIN_SEQUENCE;

    // exceptional type 3
    unsigned MAX_FULLWIRE;

    // range for appending axial wires to the segment. unit = cm.
    double   RANGE_FOR_AXIAL_SEARCH;
    double   RANGE_FOR_STEREO_SEARCH;
    unsigned SUPERLAYER_FOR_STEREO_SEARCH;
    double   RANGE_FOR_AXIAL_LAST2D_SEARCH;
    double   RANGE_FOR_STEREO_LAST2D_SEARCH;

    // Trace Section
    double TRACE2D_DISTANCE;
    double TRACE2D_FIRST_SUPERLAYER;
    double TRACE3D_DISTANCE;

    // determine one track (curler) by dz
    unsigned DETERMINE_ONE_TRACK;

    // stereo find at 2D section
    unsigned STEREO_2DFIND;

    // merge
    unsigned MERGE_EXE;
    double   MERGE_RATIO;
    double   MERGE_Z_DIFF;

    // mask
    double MASK_DISTANCE;

    // # of wire which should be used
    double RATIO_USED_WIRE;

    // strange tracks or upper limit of radius -- Pt < ~300MeV
    const double MIN_RADIUS_OF_STRANGE_TRACK;//65=..300MeV(FIX)

    // range for appending stereo wires of each super layer
    double RANGE_FOR_STEREO_FIRST;
    double RANGE_FOR_STEREO_SECOND;
    double RANGE_FOR_STEREO_THIRD;
    double RANGE_FOR_STEREO_FORTH;
    double RANGE_FOR_STEREO_FIFTH;

    // Output 2D tracks
    unsigned OUTPUT_2DTRACKS;

    // CurlFinder Version ... for development
    // 0 is default
    // others ... depend on development process
    unsigned CURL_VERSION;

    //
    // TBuilderCurl.cc
    //
    // Global Fit
    unsigned ON_CORRECTION;

    // makes lines
    double Z_CUT;

    // appends stereo wires
    double Z_DIFF_FOR_LAST_ATTEND;

    // Svd Reconstruction Flag
    unsigned SVD_RECONSTRUCTION;
    double MIN_SVD_ELECTRONS;

    // Track Selector Class
    double SELECTOR_MAX_IMPACT;
    double SELECTOR_MAX_SIGMA;
    double SELECTOR_STRANGE_PZ;
    double SELECTOR_REPLACE_DZ;
  };

//
// TSegmentCurl.cc
//
  const unsigned TCURL_LARGE_SEQUENCE = 4; // 4

} // namespace Belle

#endif // TCURLFINDERPARAMETERS_FLAG_
