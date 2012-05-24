//-----------------------------------------------------------------------------
// $Id: TCurlFinder.cc 11152 2010-04-28 01:24:38Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TCurlFinder.cc
// Section  : Tracking
// Owner    : J. Tanaka
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.61  2005/01/14 00:55:39  katayama
// uninitialized variable
//
// Revision 1.60  2004/04/15 05:34:09  yiwasaki
// Trasan 3.11 : trkmgr supports tracks found by other than trasan, Hough finder updates
//
// Revision 1.59  2004/03/26 06:07:03  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.58  2004/02/18 04:07:26  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
// Revision 1.57  2003/12/25 12:03:33  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.56  2003/12/19 07:36:02  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
//-----------------------------------------------------------------------------

//
// ... Section of Include Files
//
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdlib.h>



#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/TrackMC.h"

//#include "panther/panther.h"
#include "tracking/modules/trasan/Strings.h"
#include "tracking/modules/trasan/TCurlFinder.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TCircle.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TSegmentCurl.h"
//cnv #include "tracking/modules/trasan/TSvdFinder.h"
//cnv #include "tracking/modules/trasan/TSvdHit.h"
#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif
#ifndef PANTHER_RECCDC_WIRHIT_
#define PANTHER_RECCDC_WIRHIT_
struct reccdc_wirhit {
  int m_panther_dummy_;
  int m_ID;
  float m_ddl;
  float m_ddr;
  float m_erddl;
  float m_erddr;
  float m_pChiSq;
  int m_lr;
  int m_stat;
  int m_geo;
  int m_dat;
  int m_trk;
  float m_tdc;
  float m_adc;
};
#else
struct reccdc_wirhit;
#endif
#ifndef PANTHER_GEOCDC_LAYER_
#define PANTHER_GEOCDC_LAYER_
struct geocdc_layer {
  int m_panther_dummy_;
  int m_ID;
  float m_slant;
  float m_r;
  float m_zf;
  float m_zb;
  float m_rcsiz1;
  float m_rcsiz2;
  float m_pcsiz;
  float m_div;
  float m_offset;
  float m_shift;
  int m_wirst;
  int m_sup;
  float m_fdist;
  float m_bdist;
  float m_ft;
  float m_bt;
  float m_fpin;
  float m_bpin;
  float m_ftwist;
  float m_btwist;
  float m_fxoff;
  float m_fyoff;
  float m_fzoff;
  float m_bxoff;
  float m_byoff;
  float m_bzoff;
};
#endif
#ifndef PANTHER_GEOCDC_WIRE_
#define PANTHER_GEOCDC_WIRE_
struct geocdc_wire {
  int m_panther_dummy_;
  int m_ID;
  float m_xwb;
  float m_ywb;
  float m_zwb;
  float m_xwf;
  float m_ywf;
  float m_zwf;
  float m_slant;
  int m_cell;
  int m_layer;
  int m_stat;
  int m_lyr;
};
#endif
#ifndef PANTHER_RECCDC_WIRHIT_
#define PANTHER_RECCDC_WIRHIT_
struct reccdc_wirhit {
  int m_panther_dummy_;
  int m_ID;
  float m_ddl;
  float m_ddr;
  float m_erddl;
  float m_erddr;
  float m_pChiSq;
  int m_lr;
  int m_stat;
  int m_geo;
  int m_dat;
  int m_trk;
  float m_tdc;
  float m_adc;
};
#endif
#ifndef PANTHER_DATRGCDC_MCWIRHIT_
#define PANTHER_DATRGCDC_MCWIRHIT_
struct datcdc_mcwirhit {
  int m_panther_dummy_;
  int m_ID;
  float m_xw;
  float m_yw;
  float m_zw;
  float m_dist;
  float m_chrg;
  float m_xin;
  float m_yin;
  float m_zin;
  float m_xout;
  float m_yout;
  float m_zout;
  int m_lr;
  int m_stat;
  int m_geo;
  int m_dat;
  int m_hep;
  int m_trk;
  float m_px;
  float m_py;
  float m_pz;
  float m_pid;
  float m_tof;
  float m_tdc;
};
#endif
#ifndef PANTHER_RECCDC_TRK_
#define PANTHER_RECCDC_TRK_
struct reccdc_trk {
  int m_panther_dummy_;
  int m_ID;
  float m_helix[5];
  float m_pivot[3];
  float m_error[15];
  float m_chiSq;
  float m_ndf;
  float m_fiTerm;
  int m_nhits;
  int m_nster;
  int m_nclus;
  int m_stat;
  float m_mass;
};
#endif
#ifndef PANTHER_RECCDC_TRK_ADD_
#define PANTHER_RECCDC_TRK_ADD_
struct reccdc_trk_add {
  int m_panther_dummy_;
  int m_ID;
  int m_quality;
  int m_kind;
  int m_mother;
  int m_daughter;
  int m_decision;
  float m_likelihood[3];
  int m_stat;
  int m_rectrk;
};
#endif
#ifndef PANTHER_RECCDC_MCTRK2HEP_
#define PANTHER_RECCDC_MCTRK2HEP_
struct reccdc_mctrk2hep {
  int m_panther_dummy_;
  int m_ID;
  int m_wir;
  int m_clust;
  int m_trk;
  int m_hep;
};
#endif
#ifndef PANTHER_RECCDC_MCTRK_
#define PANTHER_RECCDC_MCTRK_
struct reccdc_mctrk {
  int m_panther_dummy_;
  int m_ID;
  int m_hep;
  float m_wirFrac;
  float m_wirFracHep;
  int m_charge;
  float m_ptFrac;
  float m_pzFrac;
  int m_quality;
};
#endif
#ifndef PANTHER_RECCDC_TIMING_
#define PANTHER_RECCDC_TIMING_
struct reccdc_timing {
  int m_panther_dummy_;
  int m_ID;
  float m_time;
  int m_quality;
};
#endif


// Following 3 parameters : (0,0,0,0) is best!
// Other cases are for the development.
#define DEBUG_CURL_DUMP 0
#define DEBUG_CURL_SEGMENT 0
#define DEBUG_CURL_GNUPLOT 0
#define DEBUG_CURL_MC 0

#if (TRASAN_DEBUG_DETAIL+DEBUG_CURL_GNUPLOT+DEBUG_CURL_MC)
//cnv #include "trg/cdc/WireHitMC.h"
// #include "tracking/modules/trasan/TTrackHEP.h"
#include <set>
// #include <algorithm>
// #include "tables/belletdf.h"
#endif


namespace Belle {

  extern const HepGeom::Point3D<double>  ORIGIN;

//cnv  static int debugMcFlag = 1;
  bool TCurlFinder::ms_smallcell(false);
  bool TCurlFinder::ms_superb(false);

  void TCurlFinderParameter::now(void) const
  {
    std::cout << std::endl;
    std::cout << "===== CURLFINDER PARAMETERS =====" << std::endl;
    std::cout << "ALHPA : " << ALPHA_SAME_WITH_HELIX << std::endl
              << std::endl
              << "MIN_SEGMENT : " << MIN_SEGMENT << std::endl
              << std::endl
              << "MIN_SALVAGE               : " << MIN_SALVAGE << std::endl
              << "BAD_DISTANCE_FOR_SALVAGE  : " << BAD_DISTANCE_FOR_SALVAGE << std::endl
              << "GOOD_DISTANCE_FOR_SALVAGE : " << GOOD_DISTANCE_FOR_SALVAGE << std::endl
              << std::endl
              << "MIN_SEQUENCE : " << MIN_SEQUENCE << std::endl
              << "MAX_FULLWIRE : " << MAX_FULLWIRE << std::endl
              << std::endl
              << "RANGE_FOR_AXIAL_SEARCH          : " << RANGE_FOR_AXIAL_SEARCH << std::endl
              << "RANGE_FOR_AXIAL_LAST2D_SEARCH   : " << RANGE_FOR_AXIAL_LAST2D_SEARCH << std::endl
              << "RANGE_FOR_STEREO_LAST2D_SEARCH  : " << RANGE_FOR_STEREO_LAST2D_SEARCH << std::endl
              << "STEREO_2DFIND                   : " << STEREO_2DFIND << std::endl
              << "   SUPERLAYER_FOR_STEREO_SEARCH : " << SUPERLAYER_FOR_STEREO_SEARCH << std::endl
              << "   RANGE_FOR_STEREO_SEARCH      : " << RANGE_FOR_STEREO_SEARCH << std::endl
              << std::endl
              << "TRACE2D_DISTANCE         : " << TRACE2D_DISTANCE << std::endl
              << "TRACE2D_FIRST_SUPERLAYER : " << TRACE2D_FIRST_SUPERLAYER << std::endl
              << std::endl
              << "TRACE3D_DISTANCE : " << TRACE3D_DISTANCE << std::endl
              << std::endl
              << "RATIO_USED_WIRE : " << RATIO_USED_WIRE << std::endl
              << std::endl
              << "DETERMINE_ONE_TRACK : " << DETERMINE_ONE_TRACK << std::endl
              << std::endl
              << "MERGE_EXE       : " << MERGE_EXE << std::endl
              << "   MERGE_RATIO  : " << MERGE_RATIO << std::endl
              << "   MERGE_Z_DIFF : " << MERGE_Z_DIFF << std::endl
              << std::endl
              << "MASK_DISTANCE : " << MASK_DISTANCE << std::endl
              << std::endl
              << "MIN_RADIUS_OF_STRANGE_TRACK : " << MIN_RADIUS_OF_STRANGE_TRACK << std::endl
              << std::endl
              << "RANGE_FOR_STEREO_FIRST  : " << RANGE_FOR_STEREO_FIRST << std::endl
              << "RANGE_FOR_STEREO_SECOND : " << RANGE_FOR_STEREO_SECOND << std::endl
              << "RANGE_FOR_STEREO_THIRD  : " << RANGE_FOR_STEREO_THIRD << std::endl
              << "RANGE_FOR_STEREO_FORTH  : " << RANGE_FOR_STEREO_FORTH << std::endl
              << "RANGE_FOR_STEREO_FIFTH  : " << RANGE_FOR_STEREO_FIFTH << std::endl
              << std::endl
              << "SVD_RECONSTRUCTION   : " << SVD_RECONSTRUCTION  << std::endl
              << "   MIN_SVD_ELECTRONS : " << MIN_SVD_ELECTRONS   << std::endl
              << "SELECTOR_MAX_IMPACT  : " << SELECTOR_MAX_IMPACT << std::endl
              << "SELECTOR_MAX_SIGMA   : " << SELECTOR_MAX_SIGMA  << std::endl
              << "SELECTOR_STRANGE_PZ  : " << SELECTOR_STRANGE_PZ << std::endl
              << "SELECTOR_REPLACE_DZ  : " << SELECTOR_REPLACE_DZ << std::endl
              << std::endl
              << "Z_CUT : " << Z_CUT << std::endl
              << "Z_DIFF_FOR_LAST_ATTEND : " << Z_DIFF_FOR_LAST_ATTEND << std::endl
              << std::endl
              << "ON_CORRECTION : " << ON_CORRECTION << std::endl
              << "OUTPUT_2DTRACKS : " << OUTPUT_2DTRACKS << std::endl
              << std::endl
              << "CURL_VERSION : " << CURL_VERSION << std::endl
              << "             : 0 default, 1 b20010409_2122" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << std::endl;
  }

//
// ... Constructor and Destructor Section ...
//
  TCurlFinder::TCurlFinder(void)
    :
    m_unusedAxialHitsOnEachLayer(NULL),
    m_unusedStereoHitsOnEachLayer(NULL),
    m_unusedAxialHitsOnEachSuperLayer(NULL),
    m_unusedStereoHitsOnEachSuperLayer(NULL),
    m_builder("CurlBuilder"),
    m_fitter("TCurlFinder Fitter"),
    m_debugCdcFrame(false),
    m_debugPlotFlag(0),
    m_debugFileNumber(0),
    _turnOffInnermost3Layers(0)
#ifdef TRASAN_WINDOW
    , _cWindow("curl window")
#endif
  {
    // *****NOTE*****
    // Do not use this!!!!!
    // Because parameters can not be set correctly.
  }

  TCurlFinder::TCurlFinder(const unsigned min_segment,
                           const unsigned min_salvage,
                           const double bad_distance_for_salvage,
                           const double good_distance_for_salvage,
                           const unsigned min_sequence,
                           const unsigned min_fullwire,
                           const double range_for_axial_search,
                           const double range_for_stereo_search,
                           const unsigned superlayer_for_stereo_search,
                           const double range_for_axial_last2d_search,
                           const double range_for_stereo_last2d_search,
                           const double trace2d_distance,
                           const double trace2d_first_distance,
                           const double trace3d_distance,
                           const unsigned determine_one_track,
                           //
                           const double selector_max_impact,
                           const double selector_max_sigma,
                           const double selector_strange_pz,
                           const double selector_replace_dz,
                           //
                           const unsigned stereo_2dfind,
                           const unsigned merge_exe,
                           const double merge_ratio,
                           const double merge_z_diff,
                           const double mask_distance,
                           const double ratio_used_wire,
                           const double range_for_stereo1,
                           const double range_for_stereo2,
                           const double range_for_stereo3,
                           const double range_for_stereo4,
                           const double range_for_stereo5,
                           //
                           const double z_cut,
                           const double z_diff_for_last_attend,
                           const unsigned svd_reconstruction,
                           const double min_svd_electrons,
                           const unsigned on_correction,
                           const unsigned output_2dtracks,
                           const unsigned curl_version,
                           int turnOffInnermost3Layers)
    :
    m_unusedAxialHitsOnEachLayer(NULL),
    m_unusedStereoHitsOnEachLayer(NULL),
    m_unusedAxialHitsOnEachSuperLayer(NULL),
    m_unusedStereoHitsOnEachSuperLayer(NULL),
    m_builder("CurlBuilder"),
    m_fitter("TCurlFinder Fitter"),
    m_debugCdcFrame(false),
    m_debugPlotFlag(0),
    m_debugFileNumber(0),
    _turnOffInnermost3Layers(turnOffInnermost3Layers)
#ifdef TRASAN_WINDOW
    , _cWindow("curl window")
#endif
  {
    //...Set Parameter
    m_param.MIN_SEGMENT = min_segment;
    m_param.MIN_SALVAGE = min_salvage;
    m_param.BAD_DISTANCE_FOR_SALVAGE  = bad_distance_for_salvage;
    m_param.GOOD_DISTANCE_FOR_SALVAGE = good_distance_for_salvage;
    m_param.MIN_SEQUENCE = min_sequence;
    m_param.MAX_FULLWIRE = min_fullwire;
    m_param.RANGE_FOR_AXIAL_SEARCH  = range_for_axial_search;
    m_param.RANGE_FOR_STEREO_SEARCH = range_for_stereo_search;
    m_param.SUPERLAYER_FOR_STEREO_SEARCH = superlayer_for_stereo_search;
    m_param.RANGE_FOR_AXIAL_LAST2D_SEARCH  = range_for_axial_last2d_search;
    m_param.RANGE_FOR_STEREO_LAST2D_SEARCH = range_for_stereo_last2d_search;
    m_param.TRACE2D_DISTANCE = trace2d_distance;
    m_param.TRACE2D_FIRST_SUPERLAYER = trace2d_first_distance;
    m_param.TRACE3D_DISTANCE = trace3d_distance;
    m_param.DETERMINE_ONE_TRACK = determine_one_track;
    //
    m_param.SELECTOR_MAX_IMPACT = selector_max_impact;
    m_param.SELECTOR_MAX_SIGMA  = selector_max_sigma;
    m_param.SELECTOR_STRANGE_PZ = selector_strange_pz;
    m_param.SELECTOR_REPLACE_DZ = selector_replace_dz;
    //
    m_param.STEREO_2DFIND = stereo_2dfind;
    m_param.MERGE_EXE    = merge_exe;
    m_param.MERGE_RATIO  = merge_ratio;
    m_param.MERGE_Z_DIFF = merge_z_diff;
    m_param.MASK_DISTANCE = mask_distance;
    m_param.RATIO_USED_WIRE = ratio_used_wire;
    m_param.RANGE_FOR_STEREO_FIRST  = range_for_stereo1;
    m_param.RANGE_FOR_STEREO_SECOND = range_for_stereo2;
    m_param.RANGE_FOR_STEREO_THIRD  = range_for_stereo3;
    m_param.RANGE_FOR_STEREO_FORTH  = range_for_stereo4;
    m_param.RANGE_FOR_STEREO_FIFTH  = range_for_stereo5;
    //
    m_param.Z_CUT = z_cut;
    m_param.Z_DIFF_FOR_LAST_ATTEND = z_diff_for_last_attend;
    m_param.SVD_RECONSTRUCTION = svd_reconstruction;
    m_param.MIN_SVD_ELECTRONS = min_svd_electrons;
    m_param.ON_CORRECTION = on_correction;
    m_param.OUTPUT_2DTRACKS = output_2dtracks;
    m_param.CURL_VERSION = curl_version;
    m_param.now();

    //...Set up TBuilder...
    m_builder.setParam(m_param);
  }

  TCurlFinder::~TCurlFinder(void)
  {
    if (m_unusedAxialHitsOnEachLayer != NULL) delete [] m_unusedAxialHitsOnEachLayer;
    if (m_unusedStereoHitsOnEachLayer != NULL) delete [] m_unusedStereoHitsOnEachLayer;
    if (m_unusedAxialHitsOnEachSuperLayer != NULL) delete [] m_unusedAxialHitsOnEachSuperLayer;
    if (m_unusedStereoHitsOnEachSuperLayer != NULL) delete []  m_unusedStereoHitsOnEachSuperLayer;
  }

//
// ... Version and Name Section ...
//
  std::string
  TCurlFinder::name(void) const
  {
    return std::string("Curling Track Finder");
  }

  std::string
  TCurlFinder::version(void) const
  {
    return std::string("3.24");
  }

//
// ... AList Sorter Section...
//
#if defined(__GNUG__)
  int
  sortBySequentialLength(const TSegmentCurl** a, const TSegmentCurl** b)
  {
    if ((*a)->maxSeq() < (*b)->maxSeq()) {
      return 1;
    } else if ((*a)->maxSeq() == (*b)->maxSeq()) {
      return 0;
    } else {
      return -1;
    }
  }
#else
  extern "C" int
  sortBySequentialLength(const void* av, const void* bv)
  {
    const TSegmentCurl** a((const TSegmentCurl**)av);
    const TSegmentCurl** b((const TSegmentCurl**)bv);
    if ((*a)->maxSeq() < (*b)->maxSeq()) {
      return 1;
    } else if ((*a)->maxSeq() == (*b)->maxSeq()) {
      return 0;
    } else {
      return -1;
    }
  }
#endif

#if defined(__GNUG__)
  int
  sortByArcLength(const TLink** a, const TLink** b)
  {
    if ((*a)->position().x() > (*b)->position().x()) {
      return 1;
    } else if ((*a)->position().x() == (*b)->position().x()) {
      return 0;
    } else {
      return -1;
    }
  }
#else
  int
  sortByArcLength(const void* av, const void* bv)
  {
    const TLink** a((const TLink**)av);
    const TLink** b((const TLink**)bv);
    if ((*a)->position().x() > (*b)->position().x()) {
      return 1;
    } else if ((*a)->position().x() == (*b)->position().x()) {
      return 0;
    } else {
      return -1;
    }
  }
#endif
//
// ... Utility Section ...
//
  double
  TCurlFinder::distance(const double x, const double y) const
  {
    return sqrt(x * x + y * y);
  }

  unsigned
  TCurlFinder::offset(const unsigned layerId) const
  {
//cnv

//   const Belle2::TRGCDC &cdc = *Belle2::TRGCDC::getTRGCDC();
//   const Belle2::TRGCDCLayer &l=*cdc.layer(layerId);
//   //  if(ms_superb) {
//     if(l.offset()==0.0) return 0;
//     return 1;
//     //  }
//     // old way
// #if 0
//   // input   - layer id#(0-49)
//   // output  - offset of its layer
//   if( layerId ==  0 || layerId ==  2 || layerId ==  4 ||
//       layerId ==  6 || layerId ==  8 || layerId ==  9 ||
//       layerId == 11 || layerId == 13 || layerId == 15 ||
//       layerId == 17 || layerId == 18 || layerId == 20 ||
//       layerId == 22 || layerId == 24 || layerId == 26 ||
//       layerId == 27 || layerId == 29 || layerId == 31 ||
//       layerId == 33 || layerId == 35 || layerId == 36 ||
//       layerId == 38 || layerId == 40 || layerId == 42 ||
//       layerId == 44 || layerId == 45 || layerId == 47 ||
//       layerId == 49 ) return 1; // off set is 0.5
//   return 0; // off set is 0
// #endif

    return 0;
  }

  unsigned
  TCurlFinder::layerId(const double& R) const
  {
    // R is radius for CDC but is 2*radius for track
    double r = R * 10.; // cm -> mm
//cnv  const Belle2::TRGCDC &cdc = *Belle2::TRGCDC::getTRGCDC();
//   if(ms_superb) {
//     static bool init(true);
//     static std::vector<unsigned>rtol;
//     static unsigned int nl;
//     if(init) {
//       init = false;
//       rtol.reserve(1500);
//       Geocdc_layer_Manager &layer_mgr = Geocdc_layer_Manager::get_manager();
//       Geocdc_misc_Manager &misc_mgr = Geocdc_misc_Manager::get_manager();
//       const Geocdc_misc &misc = misc_mgr[0];
//       nl = layer_mgr.size();
//       double rw = misc.rcdinn() * 10.0;
//       double rr(0.0);
//       while (rr<rw) {
//  rtol.push_back(nl);
//  rr += 1.0;
//       }
//       for(int i=0;i<nl; ++i) {
//  double rw = (layer_mgr[i].r() + layer_mgr[i].rcsiz2()) * 10.0;
//  while (rr<rw) {
//    rtol.push_back(i);
//    rr += 1.0;
//  }
//       }
//     }
//     int ir = (int)r;
//     if (ir<0 || ir>=rtol.size()) return nl;
//     return rtol[ir];
//   }
    // old way
    if (r < 83.0 || r > 874.)return 50;
    if (r <=  93.0)return  0; if (r <= 103.25)return 1; if (r <= 113.5)return  2;
    if (r <= 136.0)return  3; if (r <= 152.0)return  4; if (r <= 169.0)return  5;
    if (r <= 186.0)return  6; if (r <= 202.0)return  7; if (r <= 217.0)return  8;
    if (r <= 232.0)return  9; if (r <= 248.0)return 10; if (r <= 264.0)return 11;
    if (r <= 280.0)return 12; if (r <= 297.0)return 13; if (r <= 313.0)return 14;
    if (r <= 330.0)return 15; if (r <= 346.0)return 16; if (r <= 361.0)return 17;
    if (r <= 376.0)return 18; if (r <= 392.0)return 19; if (r <= 408.0)return 20;
    if (r <= 424.0)return 21; if (r <= 441.0)return 22; if (r <= 458.0)return 23;
    if (r <= 474.0)return 24; if (r <= 490.0)return 25; if (r <= 505.0)return 26;
    if (r <= 520.0)return 27; if (r <= 536.0)return 28; if (r <= 552.0)return 29;
    if (r <= 568.0)return 30; if (r <= 585.0)return 31; if (r <= 602.0)return 32;
    if (r <= 618.0)return 33; if (r <= 634.0)return 34; if (r <= 649.0)return 35;
    if (r <= 664.0)return 36; if (r <= 680.0)return 37; if (r <= 696.0)return 38;
    if (r <= 712.0)return 39; if (r <= 729.0)return 40; if (r <= 746.0)return 41;
    if (r <= 762.0)return 42; if (r <= 778.0)return 43; if (r <= 793.0)return 44;
    if (r <= 808.0)return 45; if (r <= 824.0)return 46; if (r <= 840.0)return 47;
    if (r <= 856.0)return 48; if (r <= 874.0)return 49;
    return 99;
  }

  unsigned
  TCurlFinder::maxLocalLayerId(const unsigned superLayerId) const
  {
//cnv

//   const Belle2::TRGCDC &cdc = *Belle2::TRGCDC::getTRGCDC();
//   const AList<Belle2::TRGCDCLayer> &sl=*cdc.superLayer(superLayerId);
// //   if(ms_superb) {
// //     return sl.length()-1;
// //   }
//   // input   - superlayer id#(0-10)
//   // output  - max# id of locallayer in its superlayer.
//   if(superLayerId == 1 || superLayerId == 3)return 2;
//   if(superLayerId == 5 || superLayerId == 7 ||
//      superLayerId == 9)return 3;
//   if(superLayerId == 4 || superLayerId == 6 ||
//      superLayerId == 8 || superLayerId == 10)return 4;
//   if(superLayerId == 0 || superLayerId ==  2)return 5;

//   std::cout << "Error in the CurlFinder(maxLocalLayerId). superLayerId = "
//        << superLayerId  << std::endl;
//   return 0;
    return 0;
  }

  int
  TCurlFinder::nextSuperAxialLayerId(const unsigned superLayerId, const int in) const
  {

//cnv
//   // This function is to find next axial superlayer!
//   // input   - superLayerId = superlayer id#(0-10)
//   //         - in = find inner axial superlayer from "superLayerID"
//   //                This is depth of it.
//   //                ex1. If superLayerID = 2, in =  1, return 0
//   //             ex2. If superLayerID = 2, in = -1, return 4
//   // output  - return 0, 2, 4, 6, 8, 10 = no error
//   //                                 -1 = error
//   const Belle2::TRGCDC &cdc = *Belle2::TRGCDC::getTRGCDC();
//   const AList<Belle2::TRGCDCLayer> &sl=*cdc.superLayer(superLayerId);
//   const int ns = cdc.nSuperLayers();

// //   if(ms_superb) {
// //     if(superLayerId<0 || superLayerId>=ns) return -1;
// //     if(0==in) {
// //       if (0==superLayerId%2) return superLayerId;
// //       return -1;
// //     } else if (1==in) {
// //       int iret = superLayerId - 1;
// //       if(iret>0 && 1==iret%2) iret -= 1;
// //       return iret;
// //     } else if (-1==in) {
// //       int iret = superLayerId + 1;
// //       if (iret>=ns) return -1;
// //       if (1==iret%2) iret += 1;
// //       return iret;
// //     }
// //   }

//   if(superLayerId > 10) {
//     std::cout << "Error in the CurlFinder(nextSuperAxialLayerId)." << std::endl;
//     return -1;
//   }

//   if(in == 0){
//     if(superLayerId == 0 || superLayerId ==  2 ||
//        superLayerId == 4 || superLayerId ==  6 ||
//        superLayerId == 8 || superLayerId == 10){
//       return superLayerId;
//     }else{
//       //return superLayerId - 1;
//       return -1;
//     }
//   }
//   // almost case --> inner type
//   if((superLayerId == 1 || superLayerId == 2) && in == 1)return 0;
//   if(superLayerId == 3 || superLayerId == 4){
//     if(in == 1)return 2; if(in == 2)return 0;
//   }
//   if(superLayerId == 5 || superLayerId == 6){
//     if(in == 1)return 4; if(in == 2)return 2;
//     if(in == 3)return 0;
//   }
//   if(superLayerId == 7 || superLayerId == 8){
//     if(in == 1)return 6; if(in == 2)return 4;
//     if(in == 3)return 2; if(in == 4)return 0;
//   }
//   if(superLayerId ==  9 || superLayerId == 10){
//     if(in == 1)return 8; if(in == 2)return 6;
//     if(in == 3)return 4; if(in == 4)return 2;
//     if(in == 5)return 0;
//   }
//   // rare case --> outer type
//   if(superLayerId == 0 || superLayerId == 1){
//     if(in == -1)return  2; if(in == -2)return 4;
//     if(in == -3)return  6; if(in == -4)return 8;
//     if(in == -5)return 10;
//   }
//   if(superLayerId == 2 || superLayerId == 3){
//     if(in == -1)return 4; if(in == -2)return 6;
//     if(in == -3)return 8; if(in == -4)return 10;
//   }
//   if(superLayerId == 4 || superLayerId == 5){
//     if(in == -1)return  6; if(in == -2)return 8;
//     if(in == -3)return 10;
//   }
//   if(superLayerId == 6 || superLayerId == 7){
//     if(in == -1)return 8; if(in == -2)return 10;
//   }
//   if(superLayerId == 8 || superLayerId == 9){
//     if(in == -1)return 10;
//   }

    return -1;
  }

  int
  TCurlFinder::nextSuperStereoLayerId(const unsigned superLayerId, const int in) const
  {

//cnv
//   // This function is to find next stereo superlayer!
//   // input   - superLayerId = superlayer id#(0-10)
//   //         - in = find inner stereo superlayer from "superLayerID"
//   //                This is depth of it.
//   //                ex1. If superLayerID = 2, in =  1, return 1
//   //             ex2. If superLayerID = 2, in = -1, return 3
//   // output  - return 1, 3, 5, 7, 9 = no error
//   //                             -1 = error
//   const Belle2::TRGCDC &cdc = *Belle2::TRGCDC::getTRGCDC();
//   const AList<Belle2::TRGCDCLayer> &sl=*cdc.superLayer(superLayerId);
//   const int ns = cdc.nSuperLayers();

// //   if(ms_superb) {
// //     if(superLayerId<0 || superLayerId>=ns) return -1;
// //     if(0==in) {
// //       // 1,3,5,7,,,-> 1,3,5,7
// //       if (1==superLayerId%2) return superLayerId;
// //       return -1;
// //     } else if (1==in) {
// //       //  0,1->-1, 2,3->1, 4,5->3
// //       int iret = superLayerId - 1;
// //       if(iret<=0) return -1;
// //       if(0==iret%2) iret -= 1;
// //       return iret;
// //     } else if (-1==in) {
// //       // 0->1, 1,2->3, 3,4->5,,, 7,8->-1 (ns=9)
// //       int iret = superLayerId + 2;
// //       if (iret>=ns) return -1;
// //       if (0==iret%2) iret -= 1;
// //       return iret;
// //     }
// //   }

//   if(superLayerId > 10) {
//     std::cout << "Error in the CurlFinder(nextSuperStereoLayerId)." << std::endl;
//     return -1;
//   }

//   if(in == 0){
//     if(superLayerId == 1 || superLayerId ==  3 ||
//        superLayerId == 5 || superLayerId ==  7 ||
//        superLayerId == 9){
//       return superLayerId;
//     }else{
//       return -1;
//     }
//   }
//   // almost case --> inner type
//   if((superLayerId == 2 || superLayerId == 3) && in == 1)return 1;
//   if(superLayerId == 4 || superLayerId == 5){
//     if(in == 1)return 3; if(in == 2)return 1;
//   }
//   if(superLayerId == 6 || superLayerId == 7){
//     if(in == 1)return 5; if(in == 2)return 3;
//     if(in == 3)return 1;
//   }
//   if(superLayerId == 8 || superLayerId == 9){
//     if(in == 1)return 7; if(in == 2)return 5;
//     if(in == 3)return 3; if(in == 4)return 1;
//   }
//   if(superLayerId ==  10){
//     if(in == 1)return 9; if(in == 2)return 7;
//     if(in == 3)return 5; if(in == 4)return 3;
//     if(in == 5)return 1;
//   }
//   // rare case --> outer type
//   if(superLayerId == 0){
//     if(in == -1)return 1; if(in == -2)return 3;
//     if(in == -3)return 5; if(in == -4)return 7;
//     if(in == -5)return 9;
//   }
//   if(superLayerId == 1 || superLayerId == 2){
//     if(in == -1)return 3; if(in == -2)return 5;
//     if(in == -3)return 7; if(in == -4)return 9;
//   }
//   if(superLayerId == 3 || superLayerId == 4){
//     if(in == -1)return 5; if(in == -2)return 7;
//     if(in == -3)return 9;
//   }
//   if(superLayerId == 5 || superLayerId == 6){
//     if(in == -1)return 7; if(in == -2)return 9;
//   }
//   if(superLayerId == 7 || superLayerId == 8){
//     if(in == -1)return 9;
//   }

    return -1;
  }

  unsigned
  TCurlFinder::nAxialHits(const double& r) const
  {
    // r is radius for CDC but is 2*radius for track
    const double eps = 0.2;
    if (r < 8.8 - eps)        return 0;
    else if (r < 9.8 - eps)   return 1;
    else if (r < 10.85 - eps) return 2;
    else if (r < 12.8 - eps)  return 3;
    else if (r < 14.4 - eps)  return 4;
    else if (r < 15.95 - eps) return 5;
    else if (r < 22.45 - eps) return 6;
    else if (r < 24.0 - eps)  return 7;
    else if (r < 25.6 - eps)  return 8;
    else if (r < 27.2 - eps)  return 9;
    else if (r < 28.8 - eps)  return 10;
    else if (r < 30.4 - eps)  return 11;
    else if (r < 36.85 - eps) return 12;
    else if (r < 38.4 - eps)  return 13;
    else if (r < 40.0 - eps)  return 14;
    else if (r < 41.6 - eps)  return 15;
    else if (r < 43.15 - eps) return 16;
    else if (r < 51.25 - eps) return 17;
    else if (r < 52.8 - eps)  return 18;
    else if (r < 54.4 - eps)  return 19;
    else if (r < 56.0 - eps)  return 20;
    else if (r < 57.55 - eps) return 21;
    else if (r < 65.65 - eps) return 22;
    else if (r < 67.2 - eps)  return 23;
    else if (r < 68.8 - eps)  return 24;
    else if (r < 70.4 - eps)  return 25;
    else if (r < 71.9 - eps)  return 26;
    else if (r < 80.05 - eps) return 27;
    else if (r < 81.6 - eps)  return 28;
    else if (r < 83.2 - eps)  return 29;
    else if (r < 84.8 - eps)  return 30;
    else if (r < 86.3 - eps)  return 31;
    else return 32;
  }

//
// ... Clean or Remove Section ...
//
  void
  TCurlFinder::makeList(AList<TLink> &madeList,
                        const AList<TSegmentCurl> &originalList,
                        const AList<TLink> &removeList)
  {
    // This is to make "madeList" from "originalList",
    // but remove "removeList" from this "madeList", that is,
    // madeList = originalList - removeList.
    madeList.removeAll();
    for (unsigned i = 0, size = originalList.length(); i < size; ++i)
      madeList.append(originalList[i]->list());
    madeList.remove(removeList);
  }

  void
  TCurlFinder::makeList(AList<TLink> &madeList,
                        const AList<TLink> &originalList,
                        const AList<TLink> &removeList)
  {
    // This is to make "madeList" from "originalList",
    // but remove "removeList" from this "madeList", that is,
    // madeList = originalList - removeList.
    madeList.removeAll();
    madeList.append(originalList);
    madeList.remove(removeList);
  }

  void
  TCurlFinder::clear(void)
  {
//cnv
// //  const Belle2::TRGCDC *pcdc = Belle2::TRGCDC::getTRGCDC0();
//   const Belle2::TRGCDC *pcdc = Belle2::TRGCDC::getTRGCDC();
//   if(NULL==pcdc) return;
//   const Belle2::TRGCDC &cdc = *pcdc;
//   // This is to clear this Class(TCurlFinder) in Trasan.cc .
//   // Private members are cleaned.
//   HepAListDeleteAll(m_allAxialHitsOriginal);
//   HepAListDeleteAll(m_allStereoHitsOriginal);
//   HepAListDeleteAll(m_segmentList);
//   HepAListDeleteAll(m_allCircles);
//   HepAListDeleteAll(m_allTracks);

//   m_unusedAxialHitsOriginal.removeAll();
//   m_unusedStereoHitsOriginal.removeAll();
//   m_unusedAxialHits.removeAll();
//   m_unusedStereoHits.removeAll();
//   m_removedHits.removeAll();
//   m_circles.removeAll();
//   m_tracks.removeAll();
//   m_2dTracks.removeAll();
//   if(m_unusedAxialHitsOnEachLayer!=NULL) {
//     for(int i=0;i<cdc.nAxialLayers();++i)
//       m_unusedAxialHitsOnEachLayer[i].removeAll();
//     for(int i=0;i<cdc.nStereoLayers();++i)
//       m_unusedStereoHitsOnEachLayer[i].removeAll();
//     for(int i=0;i<cdc.nAxialSuperLayers();++i)
//       m_unusedAxialHitsOnEachSuperLayer[i].removeAll();
//     for(int i=0;i<cdc.nStereoSuperLayers();++i)
//       m_unusedStereoHitsOnEachSuperLayer[i].removeAll();
//   }
//   m_hitsOnInnerSuperLayer.removeAll();
  }

//
// ... doit .. Section ... This is a main section.
//
  int
  TCurlFinder::doit(const CAList<Belle2::TRGCDCWireHit> & axialHits,
                    const CAList<Belle2::TRGCDCWireHit> & stereoHits,
                    AList<TTrack> & tracks,
                    AList<TTrack> & tracks2D)
  {

    //...For debug...
    if (debugLevel() > 1)
      std::cout << name() << " ... processing"
                << " axial=" << axialHits.length()
                << ",stereo=" << stereoHits.length()
                << ",tracks=" << tracks.length()
                << std::endl;

#ifdef TRASAN_DEBUG_DETAIL
    const std::string stage = "Curl";
    EnterStage(stage);
#endif
#ifdef TRASAN_WINDOW
    _cWindow.clear();
    _cWindow.skip(false);
    _cWindow.skipAllWindow(false);
#endif
    if (m_unusedAxialHitsOnEachLayer == NULL) {
      const Belle2::TRGCDC& cdc = *Belle2::TRGCDC::getTRGCDC();
      m_unusedAxialHitsOnEachLayer = new AList<TLink> [cdc.nAxialLayers()];
      m_unusedStereoHitsOnEachLayer = new AList<TLink> [cdc.nStereoLayers()];
      m_unusedAxialHitsOnEachSuperLayer = new AList<TLink> [cdc.nAxialSuperLayers()];
      m_unusedStereoHitsOnEachSuperLayer = new AList<TLink> [cdc.nStereoSuperLayers()];;
    }

#if (TRASAN_DEBUG_DETAIL+DEBUG_CURL_GNUPLOT+DEBUG_CURL_MC)
//cnv     Belle_event_Manager &evtMgr = Belle_event_Manager::get_manager();
//     debugMcFlag = 1;
//     if(evtMgr.count() != 0 &&
//        evtMgr[0].ExpMC() != 2)debugMcFlag = 0;// not MC
//     m_debugCdcFrame = false;
// #endif
// #if !(DEBUG_CURL_MC)
// #if DEBUG_CURL_GNUPLOT
//     std::cout
//  << "(TCurlFinder)Plot Menu : All Off = 0, Interactive = 1, All On = 2"
//  << std::endl;
//     cin >> m_debugPlotFlag;
#endif

    // sub main functions #1, #2, #3
    //...#1
    makeWireHitsListsSegments(axialHits, stereoHits);
#if DEBUG_CURL_SEGMENT
    std::cout
        << "(TCurlFinder)# of segment = " << m_segmentList.length()
        << std::endl;
    debugCheckSegments0();
    debugCheckSegments1();
    debugCheckSegments2();
#endif

    //...#2
    if (checkSortSegments() == 0) {
#ifdef TRASAN_DEBUG_DETAIL
      LeaveStage(stage);
#endif
      return 0;
    }
#if DEBUG_CURL_GNUPLOT
    if (m_debugPlotFlag) {
      int noPlot = 1;
      if (m_debugPlotFlag == 1) {
        std::cout
            << "(TCurlFinder) Do you want to see Segment Plot?",
            << ": yes = 1, no = other #" << std::endl;
        cin >> noPlot;
      }
      if (noPlot == 1) {
        for (int i = 0; i < m_segmentList.length(); ++i)
          plotSegment(m_segmentList[i]->list(), 0);
      }
    }
#endif

    //...#3
    makeCurlTracks(tracks, tracks2D);
//#else
//cnv    makeWithMC(axialHits, stereoHits, tracks);
//#endif

    //...iw 2007/02/27...
    for (unsigned i = 0; i < (unsigned) tracks.length(); i++)
      if (tracks[i]->name() == "none")
        tracks[i]->name("c" + itostring(i) + "-3D");
    //...iw end...

    //...iw 2001/01/26...
    unsigned n = tracks2D.length();
    for (unsigned i = 0; i < n; i++) {
      if (tracks2D[i]->name() == "none")
        tracks2D[i]->name("c" + itostring(i + tracks.length()));
      tracks2D[i]->quality(TrackQuality2D);
    }
    //...iw end...

    //...For debug...
    if (debugLevel() > 1)
      std::cout << name() << " ... processed"
                << " axial=" << axialHits.length()
                << ",stereo=" << stereoHits.length()
                << ",tracks=" << tracks.length()
                << std::endl;

#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif

    return 0;
  }

//
// ... Sub Section #1 -- makes segments --
// ... fuction name = "TMakeUnusedWireHitsList"
//
  void
  TCurlFinder::makeWireHitsListsSegments(const CAList<Belle2::TRGCDCWireHit> &axialList,
                                         const CAList<Belle2::TRGCDCWireHit> &stereoList)
  {
    const Belle2::TRGCDC& cdc = *Belle2::TRGCDC::getTRGCDC();
    // A sub main function ... called by "doit".
    // #0 makes lists.
    // #1 makes segments.

    //...makes original lists(axial and stereo)
    //
    //......axial
    unsigned size = axialList.length();
    for (unsigned i = 0; i < size; ++i) {
      if (axialList[i]->state() & CellHitFindingValid) {
        m_allAxialHitsOriginal.append(new TLink(0, axialList[i]));
        if (axialList[i]->wire().superLayerId() <= 2)
          m_hitsOnInnerSuperLayer.append(axialList[i]);

      }
    }
    size = m_allAxialHitsOriginal.length();
    for (unsigned i = 0; i < size; ++i) {
      if (!(m_allAxialHitsOriginal[i]->hit()->state() & CellHitUsed)) {
        if (m_allAxialHitsOriginal[i]->hit()->state() & CellHitInvalidForFit) {
          unsigned newState = m_allAxialHitsOriginal[i]->hit()->state() & (~CellHitInvalidForFit);
          m_allAxialHitsOriginal[i]->hit()->state(newState);
        }
        m_unusedAxialHitsOriginal.append(m_allAxialHitsOriginal[i]);
      }
    }
    m_unusedAxialHits = m_unusedAxialHitsOriginal;

    //......stereo
    size = stereoList.length();
    for (unsigned i = 0; i < size; ++i) {
      if (stereoList[i]->state() & CellHitFindingValid) {
        m_allStereoHitsOriginal.append(new TLink(0, stereoList[i]));
        if (stereoList[i]->wire().superLayerId() <= 2)
          m_hitsOnInnerSuperLayer.append(stereoList[i]);
      }
    }
    size = m_allStereoHitsOriginal.length();
    for (unsigned i = 0; i < size; ++i) {
      if (!(m_allStereoHitsOriginal[i]->hit()->state() & CellHitUsed)) {
        if (m_allStereoHitsOriginal[i]->hit()->state() & CellHitInvalidForFit) {
          unsigned newState = m_allStereoHitsOriginal[i]->hit()->state() & (~CellHitInvalidForFit);
          m_allStereoHitsOriginal[i]->hit()->state(newState);
        }
        m_unusedStereoHitsOriginal.append(m_allStereoHitsOriginal[i]);
      }
    }
    m_unusedStereoHits = m_unusedStereoHitsOriginal;

    //...shares unsed hit wires to each layer.
    size = m_unusedAxialHitsOriginal.length();
    for (unsigned i = 0; i < size; ++i) {
      m_unusedAxialHitsOnEachLayer[m_unusedAxialHitsOriginal[i]->hit()->wire().
                                   axialStereoLayerId()].append(m_unusedAxialHitsOriginal[i]);
    }
    size = m_unusedStereoHitsOriginal.length();
    for (unsigned i = 0; i < size; ++i) {
      m_unusedStereoHitsOnEachLayer[m_unusedStereoHitsOriginal[i]->hit()->wire().
                                    axialStereoLayerId()].append(m_unusedStereoHitsOriginal[i]);
    }

    //...For small cell (iw)...
    bool newcdc = false;
    if (Belle2::TRGCDC::getTRGCDC()->versionCDC() == "small cell")
      newcdc = true;

    //...sets pointers to neighboring hit wires of each TLink
    if ((newcdc) && (m_param.CURL_VERSION != 0)) {
      linkNeighboringWiresSmallCell(m_unusedAxialHitsOnEachLayer, cdc.nAxialLayers(), false);
      linkNeighboringWiresSmallCell(m_unusedStereoHitsOnEachLayer, cdc.nStereoLayers(), true);
    } else {
      linkNeighboringWires(m_unusedAxialHitsOnEachLayer, cdc.nAxialLayers(), false);
      linkNeighboringWires(m_unusedStereoHitsOnEachLayer, cdc.nStereoLayers(), true);
    }

    //...makes _unusedSuperAxialHits and _unusedSuperStereoHits
    createSuperLayer();

    //...makes segments by linking neighboring hit wires in the same super layer
    m_segmentList.removeAll();
    for (unsigned i = 0; i < cdc.nAxialSuperLayers(); ++i)
      if (m_unusedAxialHitsOnEachSuperLayer[i].length() > 0)
        createSegments(m_unusedAxialHitsOnEachSuperLayer[i]);
    for (unsigned i = 0; i < cdc.nStereoSuperLayers(); ++i)
      if (m_unusedStereoHitsOnEachSuperLayer[i].length() > 0)
        createSegments(m_unusedStereoHitsOnEachSuperLayer[i]);

#ifdef TRASAN_WINDOW
    displayStatus("Curl::makeWireHitsListsSegments ... all hits and segments");
    _cWindow.wait();
#endif
  }

  void
  TCurlFinder::linkNeighboringWires(AList<TLink> *list, const unsigned num, bool stereo)
  {
//  const Belle2::TRGCDC &cdc = *Belle2::TRGCDC::getTRGCDC();

//cnv
// #ifdef TRASAN_DEBUG_DETAIL
//     const std::string stage = "LinkNeighbors";
//     EnterStage(stage);
//     std::cout << Tab() << "... #links in each layer" << std::endl;
//     for (unsigned i = 0; i < num; i++) {
//  std::cout << Tab() << "    i=" << i << ",length=" << list[i].length()
//      << std::endl;
//     }
// #endif

//   // Axial(num == 32) and Stereo(num == 18).
//   // ...sets pointers to neighboring wires
//   // in "neighbor" of "AList<TLink> *list" element

//   //...For small cell (iw)...
//   bool newcdc = false;
//   if (Belle2::TRGCDC::getTRGCDC()->versionCDC() == "small cell")
//       newcdc = true;

//   for(int i=0;i< (int) num;++i){
//     const unsigned layerid = cdc.layerId(stereo?1:0, i);
//     const Belle2::TRGCDCWire &w0 = *cdc.wire(layerid, 0);
//     const unsigned superlayerid = w0.superLayerId();
//     const unsigned locallayerid = w0.localLayerId();
//     const unsigned nlocallayer = cdc.nLocalLayer(superlayerid);

//     //...For small cell (iw)...
//     if (newcdc && (m_param.CURL_VERSION == 0))
//  if (i < 3)
//      continue;

//     //...Simulate small cell with normal cell(curl version = 0)...
//     if (_turnOffInnermost3Layers)
//  if (i < 3)
//      continue;

//     if(list[i].length() == 0)continue;
//     for(int j=0;j<list[i].length();++j){
//  //find two links of list[i][j]
// //       if(ms_superb) {
// //   if(0==locallayerid) goto outer;
// //       }
// //       else
//       {
//  //...Inner layers...
//  if (! newcdc) {
//      if (num == 32) {
//    if (i == 0  || i == 6  || i == 12 ||
//        i == 17 || i == 22 || i == 27) goto outer;
//      }
//      else if (num == 18) {
//    if (i == 0 || i == 3 ||
//        i == 6 || i == 10 || i == 14 ) goto outer;
//      }
//  }
//  else {
//      if (num == 32) {
//    if (i == 1  || i == 6  || i == 12 ||
//        i == 17 || i == 22 || i == 27) goto outer;
//    if ((m_param.CURL_VERSION == 0) && (i == 3))
//        goto outer;
//      }
//      else if (num == 18) {
//    if (i == 0 || i == 3 ||
//        i == 6 || i == 10 || i == 14 ) goto outer;
//      }
//  }
//       }

//       for(int k=0;k<list[i-1].length();++k){
//         if(list[i-1][k]->hit()->wire()->localId() ==
//            list[i][j]->hit()->wire()->localId()){
//    setNeighboringWires(list[i][j], list[i-1][k]);
//         }else if(offset(list[i][j]->hit()->wire()->layerId()) == 1){
//    if((int) list[i-1][k]->hit()->wire()->localId() ==
//       list[i][j]->hit()->wire()->localIdForPlus()+1){
//      setNeighboringWires(list[i][j], list[i-1][k]);
//    }
//  }else{
//    if((int) list[i-1][k]->hit()->wire()->localId() ==
//       list[i][j]->hit()->wire()->localIdForMinus()-1){
//      setNeighboringWires(list[i][j], list[i-1][k]);
//    }
//  }
//       }//k

//     outer:
//       //...outer layer
// //       if(ms_superb) {
// //   if(locallayerid+1==nlocallayer) goto same;
// //       } else
//       {
//  if(num == 32){
//    if( i ==  5 || i == 11 || i == 16 ||
//        i == 21 || i == 26 || i == 31 )goto same;
//  }else if(num == 18){
//    if( i == 2 || i ==  5 ||
//        i == 9 || i == 13 || i == 17 )goto same;
//  }
//       }
//       for(int k=0;k<list[i+1].length();++k){
//         if(list[i+1][k]->hit()->wire()->localId() ==
//            list[i][j]->hit()->wire()->localId()){
//    setNeighboringWires(list[i][j], list[i+1][k]);
//  }else if(offset(list[i][j]->hit()->wire()->layerId()) == 1){
//    if((int) list[i+1][k]->hit()->wire()->localId() ==
//       list[i][j]->hit()->wire()->localIdForPlus()+1){
//      setNeighboringWires(list[i][j], list[i+1][k]);
//    }
//  }else{
//    if((int) list[i+1][k]->hit()->wire()->localId() ==
//       list[i][j]->hit()->wire()->localIdForMinus()-1){
//      setNeighboringWires(list[i][j], list[i+1][k]);
//    }
//  }
//       }//k
//     same:
//       //...same layer
//       for(int k=0;k<list[i].length();++k){
//         if((int) list[i][k]->hit()->wire()->localId() ==
//            list[i][j]->hit()->wire()->localIdForPlus()+1){
//    setNeighboringWires(list[i][j], list[i][k]);
//         }else if((int) list[i][k]->hit()->wire()->localId() ==
//     list[i][j]->hit()->wire()->localIdForMinus()-1){
//    setNeighboringWires(list[i][j], list[i][k]);
//         }
//       }//k
//     }//j
//   }//i

// #ifdef TRASAN_DEBUG_DETAIL
//   LeaveStage(stage);
// #endif
  }

  void
  TCurlFinder::linkNeighboringWiresSmallCell(AList<TLink> *list,
                                             const unsigned num, bool stereo)
  {
#ifdef TRASAN_DEBUG_DETAIL
    const std::string stage = "LinkNeighborsSmallCell";
    EnterStage(stage);
    std::cout << Tab() << "... #links in each layer" << std::endl;
    for (unsigned i = 0; i < num; i++) {
      std::cout << Tab() << "    i=" << i << ",length=" << list[i].length()
                << std::endl;
    }
#endif

    const Belle2::TRGCDC* const CDC = Belle2::TRGCDC::getTRGCDC();
    for (unsigned i = 0; i < num; ++i) {
      if (list[i].length() == 0) continue;
      for (unsigned j = 0; j < (unsigned) list[i].length(); ++j) {
        if (i) {
          for (unsigned k = 0; k < (unsigned) list[i - 1].length(); k++)
            if (CDC->neighbor(list[i][j]->hit()->wire(),
                              list[i - 1][k]->hit()->wire()))
              setNeighboringWires(list[i][j], list[i - 1][k]);
        }
        if ((i + 1) < num) {
          for (unsigned k = 0; k < (unsigned) list[i + 1].length(); k++)
            if (CDC->neighbor(list[i][j]->hit()->wire(),
                              list[i + 1][k]->hit()->wire()))
              setNeighboringWires(list[i][j], list[i + 1][k]);
        }
        for (unsigned k = 0; k < (unsigned) list[i].length(); k++) {
          if (j == k) continue;
          if (CDC->neighbor(list[i][j]->hit()->wire(),
                            list[i][k]->hit()->wire()))
            setNeighboringWires(list[i][j], list[i][k]);
        }
      }
    }
#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif
  }

  void
  TCurlFinder::setNeighboringWires(TLink* list, const TLink* next)
  {
    // ...sets a neighboring wire of "list".
    // Its candidate is "next".
    for (int i = 0; i < 7; ++i) {
      if (!list->neighbor(i)) {
        list->neighbor(i, const_cast<TLink*>(next));
        break;
      }
    }
  }

  void
  TCurlFinder::createSuperLayer(void)
  {
    const Belle2::TRGCDC& cdc = *Belle2::TRGCDC::getTRGCDC();
    //  if(ms_superb) {
    for (int i = 0; i < (int) cdc.nAxialLayers(); ++i) {
      if (m_unusedAxialHitsOnEachLayer[i].length() > 0) {
//  int is = cdc.AxialSuperLayer(i);
        int is = cdc.axialStereoSuperLayerId(0, i);
        m_unusedAxialHitsOnEachSuperLayer[is].append(m_unusedAxialHitsOnEachLayer[i]);
      }
    }
    for (int i = 0; i < (int) cdc.nStereoLayers(); ++i) {
      if (m_unusedStereoHitsOnEachLayer[i].length() > 0) {
//  int is = cdc.StereoSuperLayer(i);
        int is = cdc.axialStereoSuperLayerId(1, i);
        m_unusedStereoHitsOnEachSuperLayer[is].append(m_unusedStereoHitsOnEachLayer[i]);
      }
    }
    //  } else {
    // old way
#if 0
    for (int i = 0; i < 6; ++i) {
      if (m_unusedAxialHitsOnEachLayer[i].length() > 0)
        m_unusedAxialHitsOnEachSuperLayer[0].append(m_unusedAxialHitsOnEachLayer[i]);
      if (m_unusedAxialHitsOnEachLayer[i + 6].length() > 0)
        m_unusedAxialHitsOnEachSuperLayer[1].append(m_unusedAxialHitsOnEachLayer[i + 6]);
    }
    for (int i = 0; i < 5; ++i) {
      if (m_unusedAxialHitsOnEachLayer[i + 12].length() > 0)
        m_unusedAxialHitsOnEachSuperLayer[2].append(m_unusedAxialHitsOnEachLayer[i + 12]);
      if (m_unusedAxialHitsOnEachLayer[i + 17].length() > 0)
        m_unusedAxialHitsOnEachSuperLayer[3].append(m_unusedAxialHitsOnEachLayer[i + 17]);
      if (m_unusedAxialHitsOnEachLayer[i + 22].length() > 0)
        m_unusedAxialHitsOnEachSuperLayer[4].append(m_unusedAxialHitsOnEachLayer[i + 22]);
      if (m_unusedAxialHitsOnEachLayer[i + 27].length() > 0)
        m_unusedAxialHitsOnEachSuperLayer[5].append(m_unusedAxialHitsOnEachLayer[i + 27]);
    }
    for (int i = 0; i < 3; ++i) {
      if (m_unusedStereoHitsOnEachLayer[i].length() > 0)
        m_unusedStereoHitsOnEachSuperLayer[0].append(m_unusedStereoHitsOnEachLayer[i]);
      if (m_unusedStereoHitsOnEachLayer[i + 3].length() > 0)
        m_unusedStereoHitsOnEachSuperLayer[1].append(m_unusedStereoHitsOnEachLayer[i + 3]);
    }
    for (int i = 0; i < 4; ++i) {
      if (m_unusedStereoHitsOnEachLayer[i + 6].length() > 0)
        m_unusedStereoHitsOnEachSuperLayer[2].append(m_unusedStereoHitsOnEachLayer[i + 6]);
      if (m_unusedStereoHitsOnEachLayer[i + 10].length() > 0)
        m_unusedStereoHitsOnEachSuperLayer[3].append(m_unusedStereoHitsOnEachLayer[i + 10]);
      if (m_unusedStereoHitsOnEachLayer[i + 14].length() > 0)
        m_unusedStereoHitsOnEachSuperLayer[4].append(m_unusedStereoHitsOnEachLayer[i + 14]);
    }
  }
#endif
}

void
TCurlFinder::createSegments(AList<TLink> &list)
{
  // ...makes segments from AList<TLink> &list
  // These segments are add to _segmentList.(# of segments >= MIN_SEGMENT)

#ifdef TRASAN_DEBUG_DETAIL
  const std::string stage = "CreateSegments";
  EnterStage(stage);
#endif

  AList<TLink> seedStock;
  do {
    TSegmentCurl* segment = new TSegmentCurl(list[0]->hit()->wire().superLayerId(),
                                             maxLocalLayerId(list[0]->hit()->wire()
                                                             .superLayerId()));

    segment->append(list[0]);
    TLink* seed = list[0];
    list.remove(seed);

next:
    searchSegment(seed, list, seedStock, segment);
    if (seedStock.length() > 0) {
      seed = seedStock[0];
      seedStock.remove(seed);
      goto next;
    } else if (segment->size() >= m_param.MIN_SEGMENT) {
      segment->update();
      m_segmentList.append(segment);
#if TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "Segment # = " << m_segmentList.length()
                << std::endl;
      segment->dump();
#endif
    } else {
      delete segment;
    }
  } while (list.length() > 0);

#ifdef TRASAN_DEBUG_DETAIL
  LeaveStage(stage);
#endif
}

void
TCurlFinder::searchSegment(TLink* seed, AList<TLink> &list,
                           AList<TLink> &seedStock, TSegmentCurl* segment)
{
  for (int i = 0; i < 7; ++i) {
    if (seed->neighbor(i)) {
      if (!findLink(seed->neighbor(i), list))continue;
      segment->append(seed->neighbor(i));
      seedStock.append(seed->neighbor(i));
      list.remove(seed->neighbor(i));
    } else {
      break;
    }
  }
}

TLink*
TCurlFinder::findLink(const TLink* seed, const AList<TLink> &list)
{
  // This is to search "TLink *seed" in "AList<TLink> &list".
  // Return is when found, the "TLink *list[i]"
  // when not found, NULL.
  unsigned size = list.length();
  if (size == 0)return NULL;
  for (unsigned i = 0; i < size; ++i) {
    if (seed == list[i])return list[i];
  }
  return NULL;
}

//
// ... Sub Section #2 -- checks and sorts segments --
// ... fuction name = "checkSortSegments"
//
int
TCurlFinder::checkSortSegments(void)
{
  // A sub main function ...called by "doit".
#if TRASAN_DEBUG_DETAIL
  std::cout << Tab() << "... checking and sorting segments..." << std::endl;
#endif
  unsigned length = m_segmentList.length();
  if (length == 0)return 0;
  checkExceptionalSegmentsType03();//...exception #3
  //checkExceptionalSegmentsType02();//...exception #2
  checkExceptionalSegmentsType01();//...exception #1
  m_segmentList.sort(sortBySequentialLength);
#if TRASAN_DEBUG_DETAIL
  std::cout << Tab() << "... done check and sort of segments." << std::endl;
#endif
#ifdef TRASAN_WINDOW
  displayStatus("Curl::checkSortSegments ... after segment selection");
  _cWindow.wait();
#endif
  return 1;
}

void
TCurlFinder::checkExceptionalSegmentsType03(void)
{
  int max = m_param.MAX_FULLWIRE;
  int nMinWires;
  if (max == 7)nMinWires = 21;
  else if (max == 6)nMinWires = 19;
  else if (max == 5)nMinWires = 18;
  else if (max == 4)nMinWires = 16;
  else if (max == 3)nMinWires = 14;
  else if (max == 2)nMinWires = 12;
  else if (max == 1)nMinWires = 10;
  else if (max == 0)nMinWires = 7;

  AList<TSegmentCurl> removeList;
  for (unsigned i = 0, length = m_segmentList.length(); i < length; ++i) {
    if ((int) m_segmentList[i]->size() >= nMinWires) {
      unsigned nWires = m_segmentList[i]->size();
      unsigned n6Wires = 0;
      for (unsigned j = 0; j < nWires; ++j) {
        if (((m_segmentList[i]->list())[j])->neighbor(5))++n6Wires;
        if ((int) n6Wires > max)break;
      }
      if ((int) n6Wires <= max)continue;
      removeList.append(m_segmentList[i]);
#if DEBUG_CURL_SEGMENT
      writeSegment(m_segmentList[i]->list(), 3);
#endif
    }
  }
  if (removeList.length() >= 1) {
#if TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "... removing large segments: # = "
              << removeList.length() << std::endl;
#endif
    m_segmentList.remove(removeList);
    HepAListDeleteAll(removeList);
  }
}

void
TCurlFinder::checkExceptionalSegmentsType02(void)
{
  int max  = 10;
  int hmax = 5;
  AList<TSegmentCurl> removeList;
  for (unsigned i = 0, length = m_segmentList.length(); i < length; ++i) {
    int lSize = max * 3 + hmax * 3;
    int lNum  = 3;
    if (m_segmentList[i]->superLayerId() == 1 ||
        m_segmentList[i]->superLayerId() == 3) {
      lSize = max * 2 + hmax;
      lNum = 2;
    }
    if (m_segmentList[i]->superLayerId() == 5 ||
        m_segmentList[i]->superLayerId() == 7 ||
        m_segmentList[i]->superLayerId() == 9) {
      lSize = max * 2 + hmax * 2;
      lNum  = 2;
    }
    if (m_segmentList[i]->superLayerId() == 4 ||
        m_segmentList[i]->superLayerId() == 6 ||
        m_segmentList[i]->superLayerId() == 8 ||
        m_segmentList[i]->superLayerId() == 10)lSize = max * 3 + hmax * 2;
    if ((int) m_segmentList[i]->size() < lSize)continue;
    int nL = 0;
    for (unsigned j = 0, size = m_segmentList[i]->maxLocalLayerId(); j < size; ++j) {
      if ((int) m_segmentList[i]->sizeOfLayer(j) >= max)++nL;
    }
    if (nL < lNum)continue;
    removeList.append(m_segmentList[i]);
    //plotSegment(m_segmentList[i]->list(),0);
#if DEBUG_CURL_SEGMENT
    //writeSegment(m_segmentList[i]->list(),2);
#endif
  }
  if (removeList.length() >= 1) {
#if TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "... removing large segments: # = "
              << removeList.length() << std::endl;
#endif
    m_segmentList.remove(removeList);
    HepAListDeleteAll(removeList);
  }
}

void
TCurlFinder::checkExceptionalSegmentsType01(void)
{
  for (unsigned i = 0, length = m_segmentList.length(); i < length; ++i) {
    if (m_segmentList[i]->maxLocalLayerId() != m_segmentList[i]->layerIdOfMaxSeq() &&
        m_segmentList[i]->maxSeq() >= m_param.MIN_SEQUENCE) {
      unsigned innerHits = 0;
      if (m_segmentList[i]->layerIdOfMaxSeq() == 0)continue;
      TSegmentCurl* outer = new TSegmentCurl(m_segmentList[i]->superLayerId(),
                                             m_segmentList[i]->maxLocalLayerId());
      for (unsigned j = 0, size = m_segmentList[i]->size(); j < size; ++j) {
        if (m_segmentList[i]->layerIdOfMaxSeq() + 1 <=
            (m_segmentList[i]->list())[j]->hit()->wire().localLayerId() &&
            (m_segmentList[i]->list())[j]->hit()->wire().localLayerId() <=
            m_segmentList[i]->maxLocalLayerId()) {
          outer->append((m_segmentList[i]->list())[j]);
        } else if (m_segmentList[i]->layerIdOfMaxSeq() - 1 >=
                   (m_segmentList[i]->list())[j]->hit()->wire().localLayerId()) {
          ++innerHits;
        }
      }
      if (innerHits != 0 && outer->size() != 0) {
#if TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "... removing some wires in the segment."
                  << std::endl;
#endif
#if DEBUG_CURL_SEGMENT
        //writeSegment(m_segmentList[i]->list(),1);
#endif
        m_segmentList[i]->remove(const_cast< AList<TLink>& >(outer->list()));
        outer->removeAll();
        delete outer;
      } else {
        outer->removeAll();
        delete outer;
      }
    }
  }
}

//
// ... Sub Section #3 -- makes curl tracks --
// ... fuction name = "makeCurlTracks"
//

//............................................
//..............2D+3D Manage Parts............
//............................................

void
TCurlFinder::makeCurlTracks(AList<TTrack> &tracks,
                            AList<TTrack> &tracks2D)
{
#ifdef TRASAN_DEBUG
  const std::string stage = "MakeCurl";
  EnterStage(stage);
#endif

  AList<TSegmentCurl> segmentList = m_segmentList;

  if (m_param.SVD_RECONSTRUCTION)m_builder.setSvdClusters();

  for (unsigned i = 0, size = m_segmentList.length(); i < size; ++i) {
    TCircle* circle = make2DTrack(segmentList[i]->list(), segmentList, 1);

#if TRASAN_WINDOW
    _cWindow.skip(false);
    if (! circle) {
      _cWindow.text("Fail to create a circle");
      _cWindow.wait();
    } else {
      _cWindow.text("A circle is made");
      _cWindow.oneShot(* circle, leda_green);
    }
#endif
    if (circle) {

#if TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... 2D:Created Circle!!!" << std::endl;
#endif

      AList<TLink> tmp;

      //...~(Tagir)...
      if (m_param.CURL_VERSION < 2)
        tmp.append(circle->links());

#if DEBUG_CURL_GNUPLOT
      if (m_debugPlotFlag) {
        int noPlot = 1;
        if (m_debugPlotFlag == 1) {
          std::cout << "(TCurlFinder)   Do you want to see "
                    << "Circle Plot(2D)? : yes = 1, no = other #"
                    << std::endl;
          cin >> noPlot;
        }
        if (noPlot == 1)plotCircle(*circle, 0);
      }
#endif

      if (TCircle* dividedCircle = dividing2DTrack(circle)) {
#if TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "... 2D:dividing good 2 Circles!!!"
                  << std::endl;
#endif
#ifdef TRASAN_WINDOW
        displayStatus("Dividing a circle into two tracks");
        AList<TLink> c0 = circle->links();
        AList<TLink> c1 = dividedCircle->links();
        _cWindow.append(c0, leda_green);
        _cWindow.append(c1, leda_blue);
        _cWindow.oneShot(* circle, leda_green);
#endif

        TTrack* track1(NULL), *track2(NULL);
        int ok2d[2] = { 0, 0 };
        int ok3d[2] = { 0, 0 };
        if (trace2DTrack(circle) &&
            check2DCircle(circle) &&
            circle->fitForCurl(1) != -1) {
          ok2d[0] = 1;
#if TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "... 2D:Success Circle Fit!!!"
                    << std::endl;
#endif
#ifdef TRASAN_WINDOW
          displayStatus("First 2D circle is OK");
          _cWindow.oneShot(* circle, leda_green);
#endif
          track1 = make3DTrack(circle, segmentList);
#ifdef TRASAN_WINDOW
          if (track1) {
            displayStatus("3D track is made from 1st 2D circle");
            _cWindow.oneShot(* track1, leda_green);
          } else {
            displayStatus("3D track failed from 1st 2D circle");
            _cWindow.oneShot(* circle, leda_red);
          }
#endif
        }
        if (trace2DTrack(dividedCircle) &&
            check2DCircle(dividedCircle) &&
            dividedCircle->fitForCurl(1) != -1) {
          ok2d[1] = 1;
#if TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "... 2D:Success Circle Fit!!!"
                    << std::endl;
#endif
#ifdef TRASAN_WINDOW
          displayStatus("Second 2D circle is OK");
          _cWindow.oneShot(* dividedCircle, leda_green);
#endif
          track2 = make3DTrack(dividedCircle, segmentList);
#ifdef TRASAN_WINDOW
          if (track2) {
            displayStatus("3D track is made from 1st 2D circle");
            _cWindow.oneShot(* track2, leda_green);
          } else {
            displayStatus("3D track failed from 1st 2D circle");
            _cWindow.oneShot(* dividedCircle, leda_red);
          }
#endif
        }
        if (track1 && track2) {
#if TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "... 3D:Create Track!!!" << std::endl;
#endif
          salvage3DTrack(track1, true);
          salvage3DTrack(track2, true);
#if TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "... 1:dz = "
                    << track1->helix().dz() << ", 2:dz = "
                    << track2->helix().dz() << std::endl;
#endif
          if (m_param.DETERMINE_ONE_TRACK) {
            if (fabs(track1->helix().dz()) < fabs(track2->helix().dz())) {
              m_tracks.remove(track2);
              if (merge3DTrack(track1, tracks))
                if (check3DTrack(track1) &&
                    trace3DTrack(track1)) {
                  ok3d[0] = 1;
                  ok3d[1] = 1;
                  mask3DTrack(track1, tmp);
                  tmp.append(track1->links());
                }
            } else {
              m_tracks.remove(track1);
              if (merge3DTrack(track2, tracks))
                if (check3DTrack(track2) &&
                    trace3DTrack(track2)) {
                  ok3d[0] = 1;
                  ok3d[1] = 1;
                  mask3DTrack(track2, tmp);
                  tmp.append(track2->links());
                }
            }
          } else {
            int isSaved[2] = { 0, 0 };
            if (merge3DTrack(track1, tracks)) {
              if (check3DTrack(track1) &&
                  trace3DTrack(track1)) {
                ok3d[0] = 1;
                mask3DTrack(track1, tmp);
                tmp.append(track1->links());
                isSaved[0] = 1;
              }
            }
            if (merge3DTrack(track2, tracks)) {
              if (check3DTrack(track2) &&
                  trace3DTrack(track2)) {
                ok3d[1] = 1;
                mask3DTrack(track2, tmp);
                tmp.append(track2->links());
                isSaved[1] = 1;
              }
            }
            if (isSaved[0] == 1 && isSaved[1] == 1) {
              track1->daughter(track2);
              track2->daughter(track1);
            }
          }
        } else if (track1) {
#if TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "... 3D:Create Track!!!"
                    << std::endl;
#endif
          salvage3DTrack(track1, true);
          if (merge3DTrack(track1, tracks))
            if (check3DTrack(track1) &&
                trace3DTrack(track1)) {
              ok3d[0] = 1;
              mask3DTrack(track1, tmp);
              tmp.append(track1->links());
            }
        } else if (track2) {
#if TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "... 3D:Create Track!!!"
                    << std::endl;
#endif
          salvage3DTrack(track2, true);
          if (merge3DTrack(track2, tracks))
            if (check3DTrack(track2) &&
                trace3DTrack(track2)) {
              ok3d[1] = 1;
              mask3DTrack(track2, tmp);
              tmp.append(track2->links());
            }
        }

        if (m_param.OUTPUT_2DTRACKS) {
          // When 2d is OK but 3d is BAD, a 2dtrk is saved.
          if (ok2d[0] == 1 && ok3d[0] == 0) {
            removeStereo(*circle);
            double chi2_2d;
            int    ndf_2d;
            if (fitWDD(*circle, chi2_2d, ndf_2d)) {
              TTrack* trk2d;
#if defined(BELLE_DEBUG)
              try {
#endif
                trk2d = new TTrack(*circle);
                trk2d->_ndf   = ndf_2d;
                trk2d->_chi2  = chi2_2d;
                m_2dTracks.append(trk2d);
                m_allTracks.append(trk2d);
#if defined(BELLE_DEBUG)
              } catch (std::string& e) {
                std::cout << "TcurlFinder::makeCurlTracks->helix is invalid" << std::endl;
                //            continue;
                //      dump("Badhelix","Trasan::TTrack");
              }
#endif
            }
#if TRASAN_DEBUG_DETAIL
            else {
              std::cout << Tab()
                        << "... 2D:fit with drift information!!!" << std::endl;
            }
#endif
          }
          if (ok2d[1] == 1 && ok3d[1] == 0) {
            removeStereo(*dividedCircle);
            double chi2_2d;
            int    ndf_2d;
            if (fitWDD(*dividedCircle, chi2_2d, ndf_2d)) {
              TTrack* trk2d;
#if defined(BELLE_DEBUG)
              try {
#endif
                trk2d = new TTrack(*dividedCircle);
                trk2d->_ndf   = ndf_2d;
                trk2d->_chi2  = chi2_2d;
                m_2dTracks.append(trk2d);
                m_allTracks.append(trk2d);
#if defined(BELLE_DEBUG)
              } catch (std::string& e) {
                std::cout << "TcurlFinder::makeCurlTracks->helix is invalid" << std::endl;
                //            continue;
                //      dump("Badhelix","Trasan::TTrack");
              }
#endif
            }
#if TRASAN_DEBUG_DETAIL
            else {
              std::cout
                  << Tab()
                  << "... 2D:fit with drift information!!!"
                  << std::endl;
            }
#endif
          }
        }

      } else {
#if TRASAN_DEBUG_DETAIL
        std::cout << Tab()
                  << "... 2D:dividing...no good...1 Circles!!!"
                  << std::endl;
#endif
        int ok2d = 0;
        int ok3d = 0;
        if (trace2DTrack(circle) &&
            check2DCircle(circle) &&
            circle->fitForCurl(1) != -1) {
#if TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "... 2D:Success Circle Fit!!!"
                    << std::endl;
#endif
          ok2d = 1;
          TTrack* track3 = make3DTrack(circle, segmentList);
          if (track3) {
#if TRASAN_DEBUG_DETAIL
            std::cout << Tab() << "... 3D:Create Track!!!"
                      << std::endl;
#endif
            salvage3DTrack(track3, true);
#if defined(BELLE_DEBUG)
            try {
#endif
              if (merge3DTrack(track3, tracks))
                if (check3DTrack(track3) &&
                    trace3DTrack(track3)) {
                  ok3d = 1;
                  mask3DTrack(track3, tmp);
                  tmp.append(track3->links());
                }
#if defined(BELLE_DEBUG)
            } catch (std::string& e) {
              std::cout << "TcurlFinder::makeCurlTracks->helix is invalid" << std::endl;
              //            continue;
              //      dump("Badhelix","Trasan::TTrack");
            }
#endif
          }

          if (m_param.OUTPUT_2DTRACKS) {
            // When 2d is OK but 3d is BAD, a 2dtrk is saved.
            if (ok2d == 1 && ok3d == 0) {
              removeStereo(*circle);
              double chi2_2d;
              int    ndf_2d;
              if (fitWDD(*circle, chi2_2d, ndf_2d)) {
                TTrack* trk2d;
#if defined(BELLE_DEBUG)
                try {
#endif
                  trk2d = new TTrack(*circle);
                  trk2d->_ndf   = ndf_2d;
                  trk2d->_chi2  = chi2_2d;
                  m_2dTracks.append(trk2d);
                  m_allTracks.append(trk2d);
#if defined(BELLE_DEBUG)
                } catch (std::string& e) {
                  std::cout << "TcurlFinder::makeCurlTracks->helix is invalid" << std::endl;
                  //            continue;
                  //      dump("Badhelix","Trasan::TTrack");
                }
#endif
              }
#if TRASAN_DEBUG_DETAIL
              else {
                std::cout
                    << Tab()
                    << "... 2D:fit with drift information!!!"
                    << std::endl;
              }
#endif
            }
          }
        }
      }
      m_unusedAxialHits.remove(tmp);
      m_unusedStereoHits.remove(tmp);
      for (unsigned ii = 0, nsize = m_segmentList.length(); ii < nsize; ++ii) {
        m_segmentList[ii]->remove(tmp);
        if ((unsigned) m_segmentList[ii]->list().length() < m_param.MIN_SEGMENT)
          m_segmentList[ii]->removeAll();
      }
    }
    segmentList[i]->removeAll();
  }

#if TRASAN_DEBUG_DETAIL
  const std::string stage1 = "SalvageConf2D";
  EnterStage(stage1);
  std::cout << Tab() << "... tracks2D(from Conformal).length="
            << tracks2D.length() << std::endl;
#endif

  //...Tagir...
  if (m_param.CURL_VERSION >= 2) {
    for (int i = 0; i < tracks2D.length(); ++i) {
      TTrack* track = new TTrack(*tracks2D[i]);
      m_allTracks.append(track);
      AList<TLink> allStereoHits(m_unusedStereoHits);
      allStereoHits.remove(track->links());
      AList<TLink> closeHits;
      findCloseHits(allStereoHits, *track, closeHits);
      if (! m_builder.buildStereo(*track,
                                  closeHits,
                                  m_allStereoHitsOriginal)) {
        continue;
      }

      salvage3DTrack(track, true);

#if TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... 3D track made : Missing SL="
                << track->nMissingSuperLayers() << std::endl;
#endif

      AList<TLink> tmp;
      if (merge3DTrack(track, tracks)) {
        if (check3DTrack(track) && trace3DTrack(track)) {
//        std::cout << "Found 2D! "<<tracks2D.length();
          mask3DTrack(track, tmp);
          m_tracks.append(track);
          tracks2D.remove(tracks2D[i--]);
//        std::cout << " "<<tracks2D.length()<<std::endl;
          tmp.append(track->links());
          m_unusedAxialHits.remove(tmp);
          m_unusedStereoHits.remove(tmp);
        }
      }
    }
  }

#if TRASAN_DEBUG_DETAIL
  LeaveStage(stage1);
#endif

  // Check 2D trk's wires
  //check2DTracks(); // ... not need because of the current algorithm

  // 3D & 2D trks information
  assignTracks();

#if TRASAN_DEBUG_DETAIL
  std::cout << Tab() << "CDC Rec Track # 3D = " << m_tracks.length()
            << ", 2D = " << m_2dTracks.length() << std::endl;
  std::cout << Tab() << "3D Track List" << std::endl;
  for (int j = 0; j < m_tracks.length(); ++j) {
    unsigned nA = 0, nS = 0;
    unsigned nAOK = 0, nSOK = 0;
    for (unsigned i = 0, size = m_tracks[j]->nLinks(); i < size; ++i) {
      if (m_tracks[j]->links()[i]->wire()->stereo())++nS;
      else ++nA;
      if ( /*!(m_tracks[j]->links()[i]->hit()->state() & CellHitInvalidForFit) &&*/
        (m_tracks[j]->links()[i]->hit()->state() & CellHitFittingValid)) {
        if (m_tracks[j]->links()[i]->wire()->stereo())++nSOK;
        else ++nAOK;
      }
    }
    std::cout << Tab() << "#" << j << ": wire info...A+S: "
              << m_tracks[j]->nLinks()
              << ", A: " << nAOK << "/" << nA
              << ", S: " << nSOK << "/" << nS << std::endl;
    if (m_tracks[j]->daughter())
      std::cout << Tab() << "    Relation = EXIST" << std::endl;
    else
      std::cout << Tab() << "    Relation = NO EXIST" << std::endl;
  }
  std::cout << "2D Track List" << std::endl;
  for (int j = 0; j < m_2dTracks.length(); ++j) {
    unsigned nA = 0, nS = 0;
    unsigned nAOK = 0, nSOK = 0;
    for (unsigned i = 0, size = m_2dTracks[j]->nLinks(); i < size; ++i) {
      if (m_2dTracks[j]->links()[i]->wire()->stereo())++nS;
      else ++nA;
      if ( /*!(m_2dTracks[j]->links()[i]->hit()->state() & CellHitInvalidForFit) &&*/
        (m_2dTracks[j]->links()[i]->hit()->state() & CellHitFittingValid)) {
        if (m_2dTracks[j]->links()[i]->wire()->stereo())++nSOK;
        else ++nAOK;
      }
    }
    std::cout << Tab() << "#" << j << ": wire info...A+S: "
              << m_2dTracks[j]->nLinks()
              << ", A: " << nAOK << "/" << nA
              << ", S: " << nSOK << "/" << nS
              << ", Chi2: " << m_2dTracks[j]->chi2()
              << ", Ndf: " << m_2dTracks[j]->ndf() << std::endl;
    if (m_2dTracks[j]->daughter())
      std::cout << Tab() << "    Relation = EXIST" << std::endl;
    else
      std::cout << Tab() << "    Relation = NO EXIST" << std::endl;
  }
#endif

  //...3D trks...
  m_allTracks.remove(m_tracks);
  checkRelation(m_tracks);
  if (m_param.CURL_VERSION > 2) {
    unsigned nMaxMissing = 1;
    if (m_param.CURL_VERSION == 4) nMaxMissing = 2;

    const unsigned n = m_tracks.length();
    for (unsigned i = 0; i < n; i++) {
      if (m_tracks[i]->nMissingSuperLayers() >= nMaxMissing)
        continue;
      tracks.append(m_tracks[i]);
    }
  } else {
    tracks.append(m_tracks);
  }

  if (m_param.OUTPUT_2DTRACKS) {
    // 2D trks
    m_allTracks.remove(m_2dTracks);
    tracks2D.append(m_2dTracks);
  }

#ifdef TRASAN_DEBUG
  LeaveStage(stage);
#endif
}

void
TCurlFinder::check2DTracks(void)
{
  if (m_2dTracks.length() == 0)return;
  AList<TLink> allWires_3Dtrks;
  for (int i = 0; i < m_tracks.length(); ++i) {
    allWires_3Dtrks.append(m_tracks[i]->links());
  }
  //std::cout << "ALL Wire(3D) " << allWires_3Dtrks.length() << std::endl;

  for (int i = 0; i < m_2dTracks.length(); ++i) {
    AList<TLink> usedWires;
    for (int j = 0; j < (int)m_2dTracks[i]->nLinks(); ++j) {
      int ok = 1;
      for (int k = 0; k < allWires_3Dtrks.length(); ++k) {
        if (m_2dTracks[i]->links()[j]->wire()->id() ==
            allWires_3Dtrks[k]->wire()->id()) {
          ok = 0;
          break;
        }
      }
      if (ok == 0) {
        usedWires.append(m_2dTracks[i]->links()[j]);
      }
    }
    //std::cout << i << " : used # " << usedWires.length()
    // << ", all # " << m_2dTracks[i]->nLinks() << std::endl;
    m_2dTracks[i]->remove(usedWires);
  }
}

void
TCurlFinder::checkRelation(AList<TTrack> &list)
{
  unsigned nT = list.length();

  //...~(Tagir)...
  if (m_param.CURL_VERSION < 2)
    if (nT <= 1) return;

  for (unsigned i = 0; i < nT; ++i) {
    if (list[i]->daughter()) {
      int isHere = 0;
      for (unsigned j = 0; j < nT; ++j) {
        if (i != j &&
            list[i]->daughter() == list[j]) {
          isHere = 1;
          break;
        }
      }
      if (isHere == 0) {
        list[i]->daughter(NULL);
      }
    }
  }
}

TCircle*
TCurlFinder::dividing2DTrack(TCircle* circle)
{
#ifdef TRASAN_DEBUG_DETAIL
  const std::string stage = "DivideCircle";
  EnterStage(stage);
#endif

  AList<TLink> positive, negative;
  for (unsigned i = 0, size = circle->nLinks(); i < size; ++i) {
    if (circle->center().x()*circle->links()[i]->hit()->wire().xyPosition().y() -
        circle->center().y()*circle->links()[i]->hit()->wire().xyPosition().x() > 0.) {
      positive.append(circle->links()[i]);
    } else {
      negative.append(circle->links()[i]);
    }
  }
  if (positive.length() > negative.length()) {
    circle->remove(negative);
    circle->property(1., fabs(circle->radius()), circle->center());
    if (negative.length() >= 3) {
      TCircle* new_circle = new TCircle(negative);
      m_allCircles.append(new_circle);
      new_circle->property(-1., -1.*fabs(circle->radius()), circle->center());
#ifdef TRASAN_DEBUG_DETAIL
      LeaveStage(stage);
#endif
      return new_circle;
    } else {
#ifdef TRASAN_DEBUG_DETAIL
      LeaveStage(stage);
#endif
      return NULL;
    }
  } else {
    circle->remove(positive);
    circle->property(-1., -1.*fabs(circle->radius()), circle->center());
    if (positive.length() >= 3) {
      TCircle* new_circle = new TCircle(positive);
      m_allCircles.append(new_circle);
      new_circle->property(1., fabs(circle->radius()), circle->center());
#ifdef TRASAN_DEBUG_DETAIL
      LeaveStage(stage);
#endif
      return new_circle;
    } else {
#ifdef TRASAN_DEBUG_DETAIL
      LeaveStage(stage);
#endif
      return NULL;
    }
  }

#ifdef TRASAN_DEBUG_DETAIL
  LeaveStage(stage);
#endif
  return 0;
}

void
TCurlFinder::assignTracks(void)
{
  // 3D trks
  for (int i = 0, size = m_tracks.length(); i < size; ++i) {
    m_tracks[i]->assign(CellHitCurlFinder);
    m_tracks[i]->finder(TrackCurlFinder);
//    m_tracks[i]->assign(CellHitCurlFinder, TrackCurlFinder | TrackValid | Track3D);
  }

  // 2D trks
  for (int i = 0, size = m_2dTracks.length(); i < size; ++i) {
    m_2dTracks[i]->assign(CellHitCurlFinder);
    m_2dTracks[i]->finder(TrackCurlFinder);
  }
}

extern "C" int
TCurlFinder_doubleCompare(const void* i, const void* j)
{
  if (*(static_cast<const double*>(i)) > *(static_cast<const double*>(j)))return 1;
  if (*(static_cast<const double*>(i)) < * (static_cast<const double*>(j)))return -1;
  return 0;
}

int
TCurlFinder::trace2DTrack(TCircle* circle)
{
  // 0 is bad, 1 is good
  unsigned nSize = circle->links().length();
  if (nSize == 0)return 0;
  double r = fabs(circle->radius());
  if (r < 0.01)return 0; // to reject "r=0cm" circles.
  double cx = circle->center().x();
  double cy = circle->center().y();
  double th = atan2(-cy, -cx);
  if (th < 0.)th += 2.*M_PI;

  unsigned innerOK = 0;
  double* angle = new double [circle->links().length()];
  for (unsigned i = 0, size = nSize; i < size; ++i) {
    double th_r = atan2(circle->links()[i]->wire()->xyPosition().y() - cy,
                        circle->links()[i]->wire()->xyPosition().x() - cx);
    if (th_r < 0.)th_r += 2.*M_PI;
    double diff = th_r - th + 2.*M_PI;
    if (th_r > th)diff = th_r - th;
    if (circle->links()[i]->wire()->superLayerId() <= 2)innerOK = 1;
    angle[i] = diff;
  }
  qsort(angle, nSize, sizeof(double), TCurlFinder_doubleCompare); //chiisai-jun
  double maxDiffAngle = 0.;
  unsigned maxIndex = 0;
  for (unsigned i = 0, size = nSize; i < size - 1; ++i) {
    if (angle[i + 1] - angle[i] > maxDiffAngle) {
      maxDiffAngle = angle[i + 1] - angle[i];
      maxIndex = i;
    }
  }
  delete [] angle;
  // std::cout << "2D TRACE : maxDifAngle = " << maxDiffAngle
  //   << ", r = " << r << ", dist = " << r*maxDiffAngle << std::endl;
  if (r * maxDiffAngle > m_param.TRACE2D_DISTANCE)return 0;
  if (innerOK == 1)return 1;

  double q = circle->radius() > 0. ? 1. : -1;
  for (unsigned i = 0, size = m_hitsOnInnerSuperLayer.length(); i < size; ++i) {
    double mag = distance(m_hitsOnInnerSuperLayer[i]->wire().xyPosition().x() - cx,
                          m_hitsOnInnerSuperLayer[i]->wire().xyPosition().y() - cy);
    if (fabs(mag - r) < m_param.TRACE2D_FIRST_SUPERLAYER &&
        q * (cx * m_hitsOnInnerSuperLayer[i]->wire().xyPosition().y() -
             cy * m_hitsOnInnerSuperLayer[i]->wire().xyPosition().x()) > 0.) {
      return 1;
    }
  }
  return 0;
}

bool
TCurlFinder::check2DCircle(TCircle* circle)
{
  unsigned nA(nAxialHits(fabs(circle->radius()) * 2.0));

  unsigned nMA = static_cast<unsigned>(floor(m_param.RATIO_USED_WIRE * static_cast<double>(nA)));
  if (nMA < 3)nMA = 3;

  unsigned nAhits(0), nShits(0);
  for (unsigned i = 0, size = circle->nLinks(); i < size; ++i) {
    if ((circle->links()[i])->wire()->axial())++nAhits;
    else ++nShits;
  }
#if TRASAN_DEBUG_DETAIL
  if (nAhits < nMA) {
    std::cout << Tab() << "2D:Fail...checking axial wires # = "
              << nAhits << " < " << nMA << std::endl;
  }
#endif
  if (nAhits >= nMA)return true;
  return false;
}

bool
TCurlFinder::check3DTrack(TTrack* track)
{
  trace3DTrack(track);
  unsigned nA = 0, nS = 0;
  for (unsigned i = 0, size = track->nLinks(); i < size; ++i) {
    if (!(track->links()[i]->hit()->state() & CellHitFittingValid))continue;
    if (track->links()[i]->wire()->stereo())++nS;
    else ++nA;
    if (nA >= 3 && nS >= 2)return true;
  }
  m_tracks.remove(track);
#if TRASAN_DEBUG_DETAIL
  std::cout << Tab() << "3D:Checked...Fail...removing this track."
            << "Valid Axial # = " << nA << ", Stereo # = " << nS << std::endl;
#endif
  return false;
}

int
TCurlFinder::trace3DTrack(TTrack* track)
{
  // 0 is bad, 1 is good
  unsigned nSize = track->links().length();
  if (nSize == 0) {
    m_tracks.remove(track);
    return 0;
  }
  double r;
#if defined(BELLE_DEBUG)
  try {
#endif
    r = fabs(track->helix().radius());
    if (r < 0.01) {
      m_tracks.remove(track);
      return 0; // to reject "r=0cm" circles.
    }
#if defined(BELLE_DEBUG)
  } catch (std::string& e) {
    std::cout << "TcurlFinder::makeCurlTracks->helix is invalid" << std::endl;
    //            continue;
    //      dump("Badhelix","Trasan::TTrack");
    m_tracks.remove(track);
    return 0; // to reject "r=0cm" circles.
  }
#endif

  double cx = track->helix().center().x();
  double cy = track->helix().center().y();
  double th = atan2(-cy, -cx);
  if (th < 0.)th += 2.*M_PI;

  double* angle = new double [track->links().length()];
  for (unsigned i = 0, size = nSize; i < size; ++i) {
    double th_r = atan2(track->links()[i]->positionOnTrack().y() - cy,
                        track->links()[i]->positionOnTrack().x() - cx);
    if (th_r < 0.)th_r += 2.*M_PI;
    double diff = th_r - th + 2.*M_PI;
    if (th_r > th)diff = th_r - th;
    angle[i] = diff;
  }
  qsort(angle, nSize, sizeof(double), TCurlFinder_doubleCompare); //chiisai-jun
  double maxDiffAngle = 0.;
  unsigned maxIndex = 0;
  for (unsigned i = 0, size = nSize; i < size - 1; ++i) {
    if (angle[i + 1] - angle[i] > maxDiffAngle) {
      maxDiffAngle = angle[i + 1] - angle[i];
      maxIndex = i;
    }
  }
  delete [] angle;
  /* std::cout << "3D TRACE : maxDifAngle = " << maxDiffAngle
     << ", r = " << r << ", dist = " << r*maxDiffAngle << std::endl; */
  if (r * maxDiffAngle > m_param.TRACE3D_DISTANCE) {
    m_tracks.remove(track);
    return 0;
  } else {
    return 1;
  }
}

void
TCurlFinder::mask3DTrack(TTrack* track,
                         AList<TLink> &maskList)
{
//cnv  double r(fabs(track->helix().radius()));
//   const Belle2::TRGCDC &cdc = *Belle2::TRGCDC::getTRGCDC();
// //   double cx(track->helix().center().x());
// //   double cy(track->helix().center().y());

//   AList<TLink> list(m_unusedAxialHits);
//   list.append(m_unusedStereoHits);
//   list.remove(track->links());
//   list.sort(TLink::sortByWireId);

//   AList<TLink> removeList;
//   for(unsigned i=0,size=list.length();i<size;++i){
//     double d = distance(*track, *(list[i]));
//     if(d < m_param.MASK_DISTANCE){
//       HepGeom::Point3D<double> tmp(d, 0., 0.);
//       list[i]->position(tmp);
//       removeList.append(list[i]);
//     }
//   }

//   int pLayerId1 = static_cast<int>(layerId(2.*r));
// //   if(ms_superb) {
// //     if(pLayerId1!=cdc.nLayers()) pLayerId1 -= 1;
// //   } else
//  {
//     if(pLayerId1 != 50)pLayerId1 -= 1;//hard coding parameter
//   }

//   int pLayerId2 = pLayerId1+2;      //hard coding parameter

//   AList<TLink> preCand, cand;
//   while(removeList.length()){
//     preCand.removeAll();
//     preCand.append(removeList[0]);
//     if(removeList.length() >= 2){
//       for(unsigned j=1,size=removeList.length();j<size;++j){
//  if(removeList[0]->wire()->layerId() == removeList[j]->wire()->layerId()){
//    for(unsigned k=0,num=preCand.length();k<num;++k){
//      if(preCand[k]->wire()->localIdForPlus()+1 == (int) removeList[j]->wire()->localId()){
//        preCand.append(removeList[j]);
//        break;
//      }
//    }
//  }
//       }
// #if 1
//       // new
//       if((int) preCand[0]->wire()->layerId() >= pLayerId1 &&
//   (int) preCand[0]->wire()->layerId() <= pLayerId2){
//  cand.append(preCand);
//       }else if(preCand.length() == 2){//hard coding parameter
//  cand.append(preCand);
//       }else if(preCand.length() == 1){
//  cand.append(preCand[0]);
//       }
// #else
//       if(preCand.length() == 1){
//  if(preCand[0]->position().x() < MASK_DISTANCE)cand.append(preCand[0]);
//       }else{
//  if(preCand[0]->wire()->layerId() >= pLayerId1 &&
//     preCand[0]->wire()->layerId() <= pLayerId2){
//    cand.append(preCand);
//  }else if(preCand.length() == 2){//hard coding parameter
//    cand.append(preCand);
//  }
//       }
// #endif
//     }else{
//       cand.append(removeList[0]);
//     }
//     removeList.remove(removeList[0]);
//     removeList.remove(cand);
//   }
//   maskList.append(cand);
}

TTrack*
TCurlFinder::merge3DTrack(TTrack* track, AList<TTrack> &confTracks)
{
  if (!m_param.MERGE_EXE)return track;

  AList<TTrack> tracks(confTracks);
  tracks.append(m_tracks);
  tracks.remove(track);
  if (tracks.length() == 0)return track;
  double r, cy, cx;

  unsigned bestIndex = 0;
  double   bestDiff = 1.0e+20;
  double   R, cX, cY;

#if defined(BELLE_DEBUG)
  try {
#endif
    r  = track->helix().radius();
    cx = track->helix().center().x();
    cy = track->helix().center().y();
    for (unsigned i = 0, size = tracks.length(); i < size; ++i) {
      R  = fabs(tracks[i]->helix().radius());
      cX = tracks[i]->helix().center().x();
      cY = tracks[i]->helix().center().y();
      if (fabs(r) * (1. - m_param.MERGE_RATIO) <= R && R <= fabs(r) * (1. + m_param.MERGE_RATIO)) {
        if (fabs(cx - cX) <= fabs(r)*m_param.MERGE_RATIO && fabs(cy - cY) <= fabs(r)*m_param.MERGE_RATIO) {
          double diff = fabs((fabs(r) - fabs(R)) * (cx - cX) * (cy - cY));
          if (diff < bestDiff) {
            bestDiff = diff;
            bestIndex = i;
          }
        }
      }
    }
#if defined(BELLE_DEBUG)
  } catch (std::string& e) {
    std::cout << "TcurlFinder::makeCurlTracks->helix is invalid" << std::endl;
    //            continue;
    //      dump("Badhelix","Trasan::TTrack");
    return track;
  }
#endif

  if (bestDiff == 1.0e20)return track;
  R  = tracks[bestIndex]->helix().radius();
  cX = tracks[bestIndex]->helix().center().x();
  cY = tracks[bestIndex]->helix().center().y();
  if (r* R >= 0.) {
    if (fabs(track->helix().dz() - tracks[bestIndex]->helix().dz()) < m_param.MERGE_Z_DIFF) {
      if (track->nLinks() > tracks[bestIndex]->nLinks()) {
        m_tracks.remove(tracks[bestIndex]);
        return track;
      } else {
        m_tracks.remove(track);
#if TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "... 3D:Merged...removing this track.(type1)"
                  << std::endl;
#endif
        return NULL;
      }
    }
  } else {
    bool newTrack(false), oldTrack(false);
    unsigned newCounter(0), oldCounter(0);
    for (unsigned i = 0, size = m_hitsOnInnerSuperLayer.length();
         i < size; ++i) {
      if (!oldTrack) {
        if ((R > 0. &&
             cX * (m_hitsOnInnerSuperLayer[i]->wire().xyPosition().y() - cY) -
             cY * (m_hitsOnInnerSuperLayer[i]->wire().xyPosition().x() - cX) > 0.) ||
            (R < 0. &&
             cX * (m_hitsOnInnerSuperLayer[i]->wire().xyPosition().y() - cY) -
             cY * (m_hitsOnInnerSuperLayer[i]->wire().xyPosition().x() - cX) < 0.)) {
          double dist = distance(m_hitsOnInnerSuperLayer[i]->wire().xyPosition().x() - cX,
                                 m_hitsOnInnerSuperLayer[i]->wire().xyPosition().y() - cY);
          if (dist < fabs(R)) {
            if (fabs(fabs(R) - dist - m_hitsOnInnerSuperLayer[i]->drift()) < 0.5) {
              ++oldCounter;
              if (oldCounter >= 3)oldTrack = true;
            }
          } else {
            if (fabs(dist - fabs(R) - m_hitsOnInnerSuperLayer[i]->drift()) < 0.5) {
              ++oldCounter;
              if (oldCounter >= 3)oldTrack = true;
            }
          }
        }
      }
      if (!newTrack) {
        if ((r > 0. &&
             cx * (m_hitsOnInnerSuperLayer[i]->wire().xyPosition().y() - cy) -
             cy * (m_hitsOnInnerSuperLayer[i]->wire().xyPosition().x() - cx) > 0.) ||
            (r < 0. &&
             cx * (m_hitsOnInnerSuperLayer[i]->wire().xyPosition().y() - cy) -
             cy * (m_hitsOnInnerSuperLayer[i]->wire().xyPosition().x() - cx) < 0.)) {
          double dist = distance(m_hitsOnInnerSuperLayer[i]->wire().xyPosition().x() - cx,
                                 m_hitsOnInnerSuperLayer[i]->wire().xyPosition().y() - cy);
          if (dist < fabs(r)) {
            if (fabs(fabs(r) - dist - m_hitsOnInnerSuperLayer[i]->drift()) < 0.5) {
              ++newCounter;
              if (newCounter >= 3)newTrack = true;
            }
          } else {
            if (fabs(dist - fabs(r) - m_hitsOnInnerSuperLayer[i]->drift()) < 0.5) {
              ++newCounter;
              if (newCounter >= 3)newTrack = true;
            }
          }
        }
      }
      if (oldTrack && newTrack)break;
    }
    if (oldTrack && !newTrack) {
      m_tracks.remove(track);
#if TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... 3D:Merged...removing this track.(type2)"
                << std::endl;
#endif
      return NULL;
    } else if (!oldTrack && newTrack) {
      m_tracks.remove(tracks[bestIndex]);
      return track;
    } else if (!oldTrack && !newTrack) {
      m_tracks.remove(track);
      m_tracks.remove(tracks[bestIndex]);
#if TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... 3D:Merged...removing this track.(type3)"
                << std::endl;
#endif
      return NULL;
    } else if (oldTrack && newTrack) {
      if (fabs(track->helix().dz()) > fabs(tracks[bestIndex]->helix().dz()) &&
          fabs(track->helix().dz()) > fabs(tracks[bestIndex]->helix().dz()) + m_param.MERGE_Z_DIFF) {
        m_tracks.remove(track);
#if TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "... 3D:Merged...removing this track.(type4)"
                  << std::endl;
#endif
        return NULL;
      } else if (fabs(tracks[bestIndex]->helix().dz()) > fabs(track->helix().dz()) &&
                 fabs(tracks[bestIndex]->helix().dz()) > fabs(track->helix().dz()) + m_param.MERGE_Z_DIFF) {
        m_tracks.remove(tracks[bestIndex]);
        return track;
      }
    }
  }
  return NULL;
}

void
TCurlFinder::salvage3DTrack(TTrack* track, bool half)
{
#ifdef TRASAN_DEBUG_DETAIL
  const std::string stage = "Salvage3D";
  EnterStage(stage);
  std::cout << Tab() << "track links:";
  TLink::dump(track->links(), "breif");
#endif

  if (track->nLinks() >= m_param.MIN_SALVAGE) {
    AList<TLink> list = m_unusedAxialHits;
    list.append(m_unusedStereoHits);
//  list.remove(track->links());
    TLink::remove(list, track->links());   // iw 2007/03/02

    if (half) {
      double q = track->charge();
      double x = track->helix().center().x();
      double y = track->helix().center().y();
      AList<TLink> removeList;
      for (unsigned i = 0, size = list.length(); i < size; ++i)
        if (q * (x * list[i]->wire()->xyPosition().y() -
                 y * list[i]->wire()->xyPosition().x()) < 0.)
          removeList.append(list[i]);
      list.remove(removeList);
    }

    AList<TLink> badCand, goodCand;
    double dist;
    for (unsigned j = 0, nLinks = track->nLinks(); j < nLinks; ++j) {
      dist = distance(*track, *(track->links()[j]));
      if (dist > m_param.BAD_DISTANCE_FOR_SALVAGE)
        badCand.append(track->links()[j]);
    }
    for (unsigned j = 0, nList = list.length(); j < nList; ++j) {
      dist = distance(*track, *(list[j]));
      if (dist < m_param.GOOD_DISTANCE_FOR_SALVAGE)
        goodCand.append(list[j]);
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "removed:";
    TLink::dump(badCand, "breif");
    std::cout << Tab() << "appended:";
    TLink::dump(goodCand, "breif");
#endif

    track->TTrackBase::remove(badCand);
    track->TTrackBase::append(goodCand);
    if (m_fitter.fit(*track) < 0) {
      track->TTrackBase::remove(goodCand);
      track->TTrackBase::append(badCand);
      m_fitter.fit(*track);
    }
  }

#ifdef TRASAN_DEBUG_DETAIL
  LeaveStage(stage);
#endif

  return;
}

double
TCurlFinder::distance(const TTrack& track, const TLink& link) const
{
  if (link.wire()->axial()) {
    //...axial
    double d = distance(track.helix().center().x() - link.xyPosition().x(),
                        track.helix().center().y() - link.xyPosition().y());
    double diff = fabs(d - fabs(track.helix().radius()));
    return fabs(link.hit()->drift() - diff);
  }
  //...stereo
  HepGeom::Point3D<double> xc(track.helix().center());
  HepGeom::Point3D<double> xw(link.xyPosition());
  HepGeom::Point3D<double> xt(track.helix().x());
  Vector3D v0(xt - xc), v1(xw - xc);
  double vCrs(v0.x() * v1.y() - v0.y() * v1.x());
  double vDot(v0.x() * v1.x() + v0.y() * v1.y());
  double dPhi = atan2(vCrs, vDot);
  CLHEP::HepVector a(track.helix().a());
  double kappa = a[2];
  double phi0  = a[1];

  double rho = m_param.ALPHA_SAME_WITH_HELIX / kappa;
  double tanLambda = a[4];
  Vector3D v = link.wire()->direction();
  CLHEP::HepVector c(3);
  c = Point3D(link.wire()->backwardPosition() - (v * link.wire()->backwardPosition()) * v);

  CLHEP::HepDiagMatrix e(3, 1);
  CLHEP::HepMatrix t(3, 3);
  t[0][0] = v.x() * v.x();
  t[0][1] = v.x() * v.y();
  t[0][2] = v.x() * v.z();
  t[1][0] = t[0][1];
  t[1][1] = v.y() * v.y();
  t[1][2] = v.y() * v.z();
  t[2][0] = t[0][2];
  t[2][1] = t[1][2];
  t[2][2] = v.z() * v.z();
  t -= e;

  double factor = 1.;
  unsigned nTrial = 0;

  //...Cal. closest point(Newton method)...
  CLHEP::HepVector x(3);
  CLHEP::HepVector dXdPhi(3);
  CLHEP::HepVector d2Xd2Phi(3);
  double fOld = 0.; // The initialization is not needed.
  const double convergence = 1.0e-5;
  while (nTrial < 100) {
    x = track.helix().x(dPhi);
    double cosPhi = cos(phi0 + dPhi);
    double sinPhi = sin(phi0 + dPhi);
    dXdPhi[0] =   rho * sinPhi;
    dXdPhi[1] = - rho * cosPhi;
    dXdPhi[2] = - rho * tanLambda;

    //...f = d(Distance) / d phi...
    double f = dot(c, dXdPhi) + dot(x, (t * dXdPhi));

    if (fabs(f) < convergence)break;
    if (nTrial > 0) {
      double eval = (1. - 0.25 * factor) * fabs(fOld) - fabs(f);
      if (eval <= 0.)factor *= 0.5;
    }
    //...Cal. next phi...
    d2Xd2Phi[0] = rho * cosPhi;
    d2Xd2Phi[1] = rho * sinPhi;
    d2Xd2Phi[2] = 0.;
    double df = dot(c, d2Xd2Phi) +
                dot(dXdPhi, (t * dXdPhi)) +
                dot(x, (t * d2Xd2Phi));
    dPhi -= factor * f / df;

    fOld = f;
    ++nTrial;
  }

  double beta = v * (track.helix().x(dPhi) - link.wire()->backwardPosition());
  return fabs((link.wire()->backwardPosition() + beta * v - track.helix().x(dPhi)).mag() -
              link.hit()->drift());
}

//............................................
//.................2D Parts...................
//............................................

TCircle*
TCurlFinder::make2DTrack(const AList<TLink> &seed,
                         const AList<TSegmentCurl> &segmentList,
                         const unsigned ip)
{
  if (seed.length() < 3)return NULL;

#ifdef TRASAN_WINDOW
  displayStatus("Curl::makeCurlTracks ... seed segment");
  _cWindow.oneShot(seed, leda_green);
#endif

  TCircle* circle = new TCircle(seed);
  m_allCircles.append(circle);
  int errorFlag = circle->fitForCurl(ip);

#ifdef TRASAN_WINDOW
  displayStatus("Curl::makeCurlTracks ... circle");
  _cWindow.append(seed, leda_green);
  _cWindow.oneShot(* circle, leda_green);
#endif

  if (fabs(circle->radius()) > m_param.MIN_RADIUS_OF_STRANGE_TRACK)return NULL;
  int searchDirection = 1;//[ 1 : inner ], [ -1 : outer ]
  int searchPath(searchDirection);
  bool searchZero(false);
  bool changeDirection(false);
  unsigned superLayerId = seed[0]->hit()->wire().superLayerId();

  AList<TLink> cand, tmpList;
  AList<TLink> preAxialCand, preStereoCand;
  makeList(tmpList, segmentList, seed);
  for (unsigned i = 0, size = tmpList.length(); i < size; ++i) {
    if (tmpList[i]->wire()) {
      if (tmpList[i]->wire()->axial())preAxialCand.append(tmpList[i]);
      else preStereoCand.append(tmpList[i]);
    }
  }
#if DEBUG_DEBUG_DETAIL
  std::cout << "(TCurlFinder)  2D: Superlayer of seed = " << superLayerId << std::endl;
#endif

  bool appendFlag = false;
nextStep:
#if DEBUG_DEBUG_DETAIL
  std::cout << "(TCurlFinder)  2D: SearchPath = " << searchPath
            << " Search SelfSuperlayer = " << (int)(searchZero)
            << " Change Direction of Search = " << (int)(changeDirection) << std::endl;
#endif
  if (preAxialCand.length() == 0 && preStereoCand.length() == 0) {
    if (circle->links().length() >= 3) {
      if (m_unusedAxialHits.length() == 0)
        if (errorFlag == -1)return NULL;
        else return circle;
      else goto salvage;
    } else {
      if (m_unusedAxialHits.length() == 0)return NULL;
      else goto salvage;
    }
  }
  searchAxialCand(cand, preAxialCand, circle,
                  searchPath, superLayerId, m_param.RANGE_FOR_AXIAL_SEARCH);
  if (cand.length() > 0) {
#ifdef TRASAN_WINDOW
    displayStatus("Appending axial hits");
    _cWindow.append(cand, leda_blue);
    _cWindow.oneShot(* circle, leda_green);
#endif

    appendFlag = true;
    for (unsigned i = 0, size = cand.length(); i < size; ++i)
      circle->append(*cand[i]);
    errorFlag = circle->fitForCurl(ip);
    preAxialCand.remove(circle->links());

    if (m_param.STEREO_2DFIND) {
      searchStereoCand(cand, preStereoCand, circle,
                       searchPath, superLayerId,
                       m_param.RANGE_FOR_STEREO_SEARCH);
      if (cand.length() > 0) {
        appendFlag = true;
        for (unsigned i = 0, size = cand.length(); i < size; ++i)
          circle->append(*cand[i]);
        errorFlag = circle->fitForCurl(ip);
        preStereoCand.remove(circle->links());

#ifdef TRASAN_WINDOW
        displayStatus("Appending stereo hits");
        _cWindow.append(cand, leda_blue);
        _cWindow.oneShot(* circle, leda_green);
#endif
      }
    }

    if (searchDirection == 1)++searchPath;
    else --searchPath;
    goto nextStep;
  } else {
    if (m_param.STEREO_2DFIND) {
      searchStereoCand(cand, preStereoCand, circle,
                       searchPath, superLayerId, m_param.RANGE_FOR_STEREO_SEARCH);
      if (cand.length() > 0) {
        appendFlag = true;
        for (unsigned i = 0, size = cand.length(); i < size; ++i)circle->append(*cand[i]);
        errorFlag = circle->fitForCurl(ip);
        preStereoCand.remove(circle->links());
        if (searchDirection == 1)++searchPath;
        else --searchPath;
        goto nextStep;
      } else if ((searchPath == 1 || searchPath == -1) && !searchZero) {
        searchPath = 0;
        searchZero = true;
        goto nextStep;
      } else if ((searchPath == 1 || searchPath == -1) && searchZero && !changeDirection) {
        searchPath *= -1;
        searchDirection *= -1;
        changeDirection = true;
        goto nextStep;
      } else {
        if (circle->links().length() >= 3) {
          if (m_unusedAxialHits.length() == 0)
            if (errorFlag == -1)return NULL;
            else return circle;
          else goto salvage;
        } else {
          if (m_unusedAxialHits.length() == 0)return NULL;
          else goto salvage;
        }
      }
    } else {
      if ((searchPath == 1 || searchPath == -1) && !searchZero) {
        searchPath = 0;
        searchZero = true;
        goto nextStep;
      } else if ((searchPath == 1 || searchPath == -1) && searchZero && !changeDirection) {
        searchPath *= -1;
        searchDirection *= -1;
        changeDirection = true;
        goto nextStep;
      } else {
        if (circle->links().length() >= 3) {
          if (m_unusedAxialHits.length() == 0)
            if (errorFlag == -1)return NULL;
            else return circle;
          else goto salvage;
        } else {
          if (m_unusedAxialHits.length() == 0)return NULL;
          else goto salvage;
        }
      }
    }
  }

salvage:
  cand.removeAll();
  searchHits(cand, m_unusedAxialHits,  circle, m_param.RANGE_FOR_AXIAL_LAST2D_SEARCH);
  if (m_param.STEREO_2DFIND) {
    searchHits(cand, m_unusedStereoHits, circle, m_param.RANGE_FOR_STEREO_LAST2D_SEARCH);
  }
  if (checkAppendHits(circle->links(), cand)) {

#ifdef TRASAN_WINDOW
    if (cand.length()) {
      displayStatus("Salvaging hits");
      _cWindow.append(cand, leda_blue);
      _cWindow.oneShot(* circle, leda_green);
    }
#endif
    circle->append(cand);

    if (circle->nLinks() >= 3)
      if (circle->fitForCurl(ip) == -1)return NULL;
      else return circle;
    else return NULL;
  } else if (circle->nLinks() >= 3) {
    return circle;
  } else return NULL;
}

TLink*
findIsolatedCloseHits(TLink* link);

void
TCurlFinder::searchAxialCand(AList<TLink> &cand,
                             const AList<TLink> &preCand,
                             const TCircle* circle,
                             const int depth,
                             const unsigned superLayerID,
                             const double searchError)
{
  cand.removeAll();
  int innerSuperLayerId = nextSuperAxialLayerId(superLayerID, depth);
  if (innerSuperLayerId < 0)return;
  for (unsigned i = 0, size = preCand.length(); i < size; ++i) {
    if (preCand[i]->hit()->wire().superLayerId() ==
        (static_cast<unsigned>(innerSuperLayerId))) {
#if 0
      if (searchHits(preCand[i], circle, searchError))cand.append(preCand[i]);
#else
      if (searchHits(preCand[i], circle, searchError)) {
        cand.remove(preCand[i]);
        cand.append(preCand[i]);
        TLink* cand2 = findIsolatedCloseHits(preCand[i]);
        if (cand2) {
          for (unsigned j = 0; j < size; ++j) {
            if (preCand[j]->wire()->id() == cand2->wire()->id()) {
              cand.remove(cand2);
              cand.append(cand2);
#if 0
              std::cout << "Axial Appending....";
              std::cout << " layerID = " << cand2->wire()->layerId();
              std::cout << " localID = " << cand2->wire()->localId() << std::endl;
              if (searchHits(cand2, circle, searchError)) {
                std::cout << " But this can be added by default!" << std::endl;
              } else {
                std::cout << " Good!! this cannot be added by default!" << std::endl;
              }
#endif
              break;
            }
          }
        }
      }
#endif
    }
  }
}

void
TCurlFinder::searchStereoCand(AList<TLink> &cand,
                              const AList<TLink> &preCand,
                              const TCircle* circle,
                              const int depth,
                              const unsigned superLayerID,
                              const double searchError)
{
  cand.removeAll();
  int innerSuperLayerId = nextSuperStereoLayerId(superLayerID, depth);
  if (innerSuperLayerId < 0 || innerSuperLayerId > (int) m_param.SUPERLAYER_FOR_STEREO_SEARCH)return;
  for (unsigned i = 0, size = preCand.length(); i < size; ++i) {
    if (preCand[i]->hit()->wire().superLayerId() ==
        (static_cast<unsigned>(innerSuperLayerId))) {
      if (searchHits(preCand[i], circle, searchError))cand.append(preCand[i]);
    }
  }
}

unsigned
TCurlFinder::searchHits(const TLink* link, const TCircle* circle,
                        const double searchError) const
{
  // ...checks whether "link" can be added to circle.
  // ..."searchError" is length for checking.
  // ...returns 0 = error
  //            1 = no error
  double dist = distance(link->hit()->wire().xyPosition().x() - circle->center().x(),
                         link->hit()->wire().xyPosition().y() - circle->center().y());
  double radius = fabs(circle->radius());
  // std::cout << link->wire()->localId() << " " << link->wire()->layerId() << " r="
  //   << radius << " e=" << searchError << " d=" << dist << std::endl;
  if (radius - searchError < dist &&
      radius + searchError > dist)return 1;
  return 0;
}

unsigned
TCurlFinder::searchHits(AList<TLink> &cand,
                        const AList<TLink> &preCand,
                        const TCircle* circle,
                        const double searchError) const
{
  unsigned numBefore = cand.length();
  for (unsigned i = 0, size = preCand.length(); i < size; ++i) {
    if (searchHits(preCand[i], circle, searchError)) {
#if 0
      cand.append(preCand[i]);
#else
      cand.remove(preCand[i]);
      cand.append(preCand[i]);
      TLink* cand2 = findIsolatedCloseHits(preCand[i]);
      if (cand2) {
        for (unsigned j = 0; j < size; ++j) {
          if (preCand[j]->wire()->id() == cand2->wire()->id()) {
            cand.remove(cand2);
            cand.append(cand2);
            break;
          }
        }
      }
#endif
    }
  }
  if (numBefore == (unsigned) cand.length())return 0;
  return 1;
}

unsigned
TCurlFinder::checkAppendHits(const AList<TLink> &link,
                             AList<TLink> &cand) const
{
  if (cand.length() == 0)return 0;
  AList<TLink> tmp;
  for (unsigned i = 0, size1 = cand.length(), size2 = link.length(); i < size1; ++i) {
    for (unsigned j = 0; j < size2; ++j) {
      if ((cand[i])->wire()->id() == (link[j])->wire()->id()) {
        tmp.append(cand[i]);
        break;
      }
    }
  }
  cand.remove(tmp);
  if (cand.length() > 0)return 1;
  return 0;
}

void
TCurlFinder::removeStereo(TCircle& c) const
{
  AList<TLink> stereoList;
  for (int i = 0; i < c.links().length(); ++i) {
    if (c.links()[i]->wire()->stereo()) {
      stereoList.append(c.links()[i]);
    }
  }
  if (stereoList.length() > 0)c.remove(stereoList);
}

bool
TCurlFinder::fitWDD(TCircle& c,
                    double& chi2,
                    int& ndf) const
{
  if (c.links().length() <= 3)return false;
  Lpav circle;
  // CDC
  for (int i = 0; i < c.links().length(); ++i) {
    circle.add_point((c.links()[i])->wire()->xyPosition().x(),
                     (c.links()[i])->wire()->xyPosition().y(), 1.0);
  }
  circle.add_point(0., 0., 1.0); // IP Constraint
  if (circle.fit() < 0.0 || circle.kappa() == 0.0) return false;
  double xc = circle.center()[0];
  double yc = circle.center()[1];
  double r  = circle.radius();
  const int maxIte = 2;
  for (int ite = 0; ite < maxIte; ++ite) {
    Lpav circle2;
    circle2.clear();
    // CDC
    for (int i = 0; i < c.links().length(); ++i) {
      if (!((c.links()[i])->hit()->state() & CellHitFittingValid))continue;
      double R = sqrt(((c.links()[i])->wire()->xyPosition().x() - xc) * ((c.links()[i])->wire()->xyPosition().x() - xc) +
                      ((c.links()[i])->wire()->xyPosition().y() - yc) * ((c.links()[i])->wire()->xyPosition().y() - yc));
      if (R == 0.)continue;
      double U = 1. / R;
      double dir = R > r ? -1. : 1.;
      double X = xc + ((c.links()[i])->wire()->xyPosition().x() - xc) * U * (R + dir * (c.links()[i])->hit()->drift());
      double Y = yc + ((c.links()[i])->wire()->xyPosition().y() - yc) * U * (R + dir * (c.links()[i])->hit()->drift());
      circle2.add_point(X, Y, 1.0);
    }
    circle2.add_point(0., 0., 1.0); // IP Constraint
    if (circle2.fit() < 0.0 || circle2.kappa() == 0.0) return false;
    xc = circle2.center()[0];
    yc = circle2.center()[1];
    r  = circle2.radius();
    //std::cout << xc << ", " << yc << " : " << r << std::endl;
  }

  // update of point information
  double totalChi2 = 0.;
  int totalNHit = 0;
  for (int i = 0; i < c.links().length(); ++i) {
    if (!((c.links()[i])->hit()->state() & CellHitFittingValid))continue;
    double xw = (c.links()[i])->wire()->xyPosition().x();
    double yw = (c.links()[i])->wire()->xyPosition().y();
    double R = sqrt((xw - xc) * (xw - xc) + (yw - yc) * (yw - yc));
    if (R == 0.)continue;
    double U = 1. / R;
    double X = xc + (xw - xc) * U * r;
    double Y = yc + (yw - yc) * U * r;
    double zlr = xw * Y - yw * X;
    unsigned leftRight = zlr > 0. ? CellHitRight : CellHitLeft;
    double pChi2 = sqrt((X - xw) * (X - xw) + (Y - yw) * (Y - yw)) - (c.links()[i])->hit()->drift();
    //std::cout << sqrt((X-xw)*(X-xw)+(Y-yw)*(Y-yw)) << " - " << (c.links()[i])->hit()->drift() << std::endl;
    //std::cout << i << ": " << pChi2 << std::endl;
    if ((c.links()[i])->hit()->dDrift() != 0.) {
      pChi2 *= pChi2 / ((c.links()[i])->hit()->dDrift() * (c.links()[i])->hit()->dDrift());
      totalChi2 += pChi2;
      //std::cout << pChi2 << ", " << c.links()[i]->hit()->dDrift() << std::endl;
      ++totalNHit;
    } else pChi2 = 1.0e+10;
    (c.links()[i])->update(HepGeom::Point3D<double> (X, Y, 0.), HepGeom::Point3D<double> (xw, yw, 0.), leftRight, pChi2);
    //std::cout << i << ": trk(" << X << "," << Y << "), wir(" << xw << "," << yw << ")" << std::endl;
  }
  chi2 = totalChi2;
  if (totalNHit <= 3)return false;
  ndf = totalNHit - 3;

  HepGeom::Point3D<double>  center(xc, yc, 0.);
  double charge = 0.;
  //...Determine charge...Better way???
  int qSum = 0;
  for (int i = 0; i < c.links().length(); ++i) {
    TLink* l = c.links()[i];
    if (l == 0)continue;
    const Belle2::TRGCDCWireHit* h = l->hit();
    if (h == 0)continue;
    double q = (center.cross(h->xyPosition())).z();
    if (q > 0.)qSum += 1;
    else      qSum -= 1;
  }
  if (qSum >= 0)charge = +1.;
  else         charge = -1.;
  r *= charge;
  //std::cout << "B q = " << c.charge() << ", r = " << c.radius() << ", center = " << c.center() << std::endl;
  c.property(charge, r, center);
  //std::cout << "A q = " << c.charge() << ", r = " << c.radius() << ", center = " << c.center() << std::endl;
  return true;
}

//............................................
//.................3D Parts...................
//............................................

TTrack*
TCurlFinder::make3DTrack(const TCircle* circle, AList<TSegmentCurl> &)
{
  if (TTrack* track = make3DTrack(circle)) {
    m_tracks.append(track);
#if 0
    std::cout << "CDC THelix+Pt: " << track->helix().dr() << ", "
              << track->helix().phi0() << ", "
              << track->helix().kappa() << ", "
              << track->helix().dz() << ", "
              << track->helix().tanl()
              << ": " << 10000. / 2.9979258 / 15. / track->helix().kappa()
              << std::endl;
#endif
    return track;
  }
  return NULL;
}

TTrack*
TCurlFinder::make3DTrack(const TCircle* circle)
{
#ifdef TRASAN_DEBUG_DETAIL
  const std::string stage = "Make3D";
  EnterStage(stage);
#endif

  TTrack* track;
#if defined(BELLE_DEBUG)
  try {
#endif
    track = new TTrack(*circle);
#if defined(BELLE_DEBUG)
  } catch (std::string& e) {
    std::cout << "TcurlFinder::make3DTrack->helix is invalid" << std::endl;
#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif
    return NULL;
    //      dump("Badhelix","Trasan::TTrack");
  }
#endif
  m_allTracks.append(track);
  if (track->links().length() < 3) {
#if TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "3D:Fail...inital hit wire # < 3."
              << std::endl;
    LeaveStage(stage);
#endif
#ifdef TRASAN_WINDOW
    _cWindow.text("Fail to create 3D track : #hits<3");
    _cWindow.oneShot(* track, leda_red);
#endif
    return NULL;
  }

  AList<TLink> allStereoHits(m_unusedStereoHits);
  allStereoHits.remove(track->links());
  AList<TLink> closeHits;
  findCloseHits(allStereoHits, *track, closeHits);
  //if(!m_builder.buildStereo(*track, closeHits)){
  if (!m_builder.buildStereo(*track, closeHits, m_allStereoHitsOriginal)) {
#if TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "3D:Fail...can not build stereo." << std::endl;
    LeaveStage(stage);
#endif
    return NULL;
  }
  if (fabs(track->helix().radius()) > m_param.MIN_RADIUS_OF_STRANGE_TRACK) {
#if TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "3D:Fail...success 3D fit, but large radius > "
              << m_param.MIN_RADIUS_OF_STRANGE_TRACK << "." << std::endl;
    LeaveStage(stage);
#endif
    return NULL;
  }
  if (track->links().length() >= 5) {
#if TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "... 3D:Success...can build stereo!!!"
              << std::endl;
    LeaveStage(stage);
#endif
    return track;
  } else {
#if TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "... 3D:Fail...success 3D fit,"
              << "but final hit wire # < 3." << std::endl;
    LeaveStage(stage);
#endif
    return NULL;
  }
#ifdef TRASAN_DEBUG_DETAIL
  LeaveStage(stage);
#endif

  return 0;
}

TLink*
findIsolatedCloseHits(TLink* link)
{
  int nNeighbor = 0;
  int nIsolated = 0;
  TLink* isolatedLink[2] = {NULL, NULL};
  unsigned layerID = link->wire()->layerId();
  unsigned localID = link->wire()->localId();
  for (int i = 0; i < 7; ++i) {
    if (link->neighbor(i)) {
      if (link->neighbor(i)->wire()->layerId() == layerID) {
        ++nNeighbor;
        int isolated = 1;
        int testEach = 0; // for test
        for (int j = 0; j < 7; ++j) {
          if (link->neighbor(i)->neighbor(j)) {
            if (link->neighbor(i)->neighbor(j)->wire()->layerId() == layerID &&
                link->neighbor(i)->neighbor(j)->wire()->localId() != localID) {
              isolated = 0;
            }
#if 1
            else if (link->neighbor(i)->neighbor(j)->wire()->layerId() == layerID &&
                     link->neighbor(i)->neighbor(j)->wire()->localId() == localID) {
              testEach = 1;
            }
#endif
          } else break;
        }
        if (isolated == 1) {
          if (nIsolated < 2)
            isolatedLink[nIsolated] = link->neighbor(i);
          ++nIsolated;
        }
#if 1
//  if(testEach == 0){
//    std::cout << "Why?? Neighborhood info. dose not exist!! layerID="
//        << layerID << std::endl;
//  }
#endif
      }
    } else break;
  }
#if 0
  std::cout << "isolated/neighbor # = " << nIsolated << "/" << nNeighbor << std::endl;
  std::cout << "layer ID = " << layerID << " ";
  std::cout << "local ID = " << localID << " --> ";
  if (isolatedLink[0])std::cout << isolatedLink[0]->wire()->localId() << " ";
  if (isolatedLink[1])std::cout << isolatedLink[1]->wire()->localId() << " ";
  std::cout << std::endl;
#endif
  if (nIsolated == 1 &&
      nNeighbor == 1 && isolatedLink[0])return isolatedLink[0];
  else return NULL;
}

void
TCurlFinder::findCloseHits(AList<TLink> &links,
                           TTrack& track, AList<TLink> &list)
{
  // ...finds candidates in the "links".
  // ...Candidates mean |"track" - wire(from "links" elements)| < dRcut
  // ...returns these candidates("list").
  double dRcut[11] = {0., m_param.RANGE_FOR_STEREO_FIRST,
                      0., m_param.RANGE_FOR_STEREO_SECOND,
                      0., m_param.RANGE_FOR_STEREO_THIRD,
                      0., m_param.RANGE_FOR_STEREO_FORTH,
                      0., m_param.RANGE_FOR_STEREO_FIFTH,
                      0.
                     };
#if defined(BELLE_DEBUG)
  try {
#endif

    double r = fabs(track.helix().curv());
    double q = track.charge();
    double x = track.helix().center().x();
    double y = track.helix().center().y();
    for (unsigned i = 0, size = links.length(); i < size; ++i) {
      if (fabs((links[i]->wire()->xyPosition() - track.helix().center()).mag() - r) <
          dRcut[links[i]->wire()->superLayerId()]) {
        if (q * (x * links[i]->wire()->xyPosition().y() - y * links[i]->wire()->xyPosition().x()) > 0.) {
          list.remove(links[i]);
          list.append(links[i]);
          TLink* cand = findIsolatedCloseHits(links[i]);
          if (cand) {
            list.remove(cand);
            list.append(cand);
          }
        }
      }
    }
    return;
#if defined(BELLE_DEBUG)
  } catch (std::string& e) {
    std::cout << "TConformalFinder::stereoSegments::helix is invalid" << std::endl;
    return;
  }

#endif
}

//..................................................
//..................................................
//..................................................
//..................................................
//..................................................

#if DEBUG_CURL_MC
int
TCurlFinder::makeWithMC(const AList<Belle2::TRGCDCWireHit> & axialHits,
                        const AList<Belle2::TRGCDCWireHit> & stereoHits,
                        AList<TTrack> & tracks)
{
#define MAX_INDEX_MAKEMC 100
#if TRASAN_DEBUG_DETAIL
  std::cout << "(TCurlFinder)Now making tracks using MC info..." << std::endl;
#endif
  int index[MAX_INDEX_MAKEMC];
  for (unsigned i = 0; i < MAX_INDEX_MAKEMC; ++i)index[i] = 9999;

  int counter(0);
  bool first(true);

  for (unsigned i = 0, size = axialHits.length(); i < size; ++i) {
    if (axialHits[i]->mc() &&
        axialHits[i]->mc()->hep() &&
        !(axialHits[i]->state() & CellHitUsed)) {
      int flag(1);
      for (unsigned j = 0; j < MAX_INDEX_MAKEMC; ++j) {
        if (index[j] != 9999 && index[j] == axialHits[i]->mc()->hep()->id()) {
          flag = 0;
          break;
        }
      }
      if (flag) {
        index[counter] = axialHits[i]->mc()->hep()->id();
        ++counter;
      }
    }
  }
#if TRASAN_DEBUG_DETAIL
  std::cout << "(TCurlFinder)Found " << counter
            << " tracks with MC information." << std::endl;
#endif
  for (unsigned j = 0; j < counter; ++j) {
    AList<TLink> axialList;
    AList<TLink> stereoList;
    int axialCounter(0);
    int stereoCounter(0);
    //...axial
    for (unsigned i = 0, size = axialHits.length(); i < size; ++i) {
      if (index[j] == axialHits[i]->mc()->hep()->id() &&
          !(axialHits[i]->state() & CellHitUsed)) {
        axialList.append(new TLink(0, axialHits[i]));
        ++axialCounter;
      }
    }
    if (axialCounter < 3) {
      HepAListDeleteAll(axialList);
      continue;
    }
    //...stereo
    for (unsigned i = 0, size = stereoHits.length(); i < size; ++i) {
      if (index[j] == stereoHits[i]->mc()->hep()->id() &&
          !(stereoHits[i]->state() & CellHitUsed)) {
        stereoList.append(new TLink(0, stereoHits[i]));
        ++stereoCounter;
      }
    }
    if (stereoCounter < 2) {
      HepAListDeleteAll(axialList);
      HepAListDeleteAll(stereoList);
      continue;
    }
#if TRASAN_DEBUG_DETAIL
    std::cout << "(TCurlFinder)#" << j << " : Use "
              << axialCounter << " axial hit wires and "
              << stereoCounter << " stereo hit wires" << std::endl;
    std::cout << "(TCurlFinder)Particle Type(LUND) = "
              << axialList[0]->hit()->mc()->hep()->pType() << std::endl;
#endif

    m_unusedAxialHitsOriginal.append(axialList);
    m_unusedAxialHitsOriginal.append(stereoList);
    TCircle* circle = new TCircle(axialList);
    m_allCircles.append(circle);
    circle->fitForCurl();
    double charge = 1.;
    if (axialList[0]->hit()->mc()->hep()->pType() < 0)charge = -1.;
    if (fabs(axialList[0]->hit()->mc()->hep()->pType()) == 11 ||
        fabs(axialList[0]->hit()->mc()->hep()->pType()) == 13 ||
        fabs(axialList[0]->hit()->mc()->hep()->pType()) == 15)charge *= -1.;
    circle->property(charge, charge * fabs(circle->radius()), circle->center());

    AList<TLink> removeList;
    double x = circle->center().x();
    double y = circle->center().y();
    //...axial
    for (unsigned i = 0, size = axialList.length();
         i < size; ++i) {
      if (charge * (x * axialList[i]->xyPosition().y() -
                    y * axialList[i]->xyPosition().x()) < 0.) {
        removeList.append(axialList[i]);

      }
    }
    circle->remove(removeList);
    if (circle->nLinks() < 3)continue;
    //...refits
    circle->fitForCurl(1);
    x = circle->center().x();
    y = circle->center().y();
    removeList.removeAll();
    ///...stereo
    for (unsigned i = 0, size = stereoList.length();
         i < size; ++i) {
      if (charge * (x * stereoList[i]->xyPosition().y() -
                    y * stereoList[i]->xyPosition().x()) < 0.) {
        removeList.append(stereoList[i]);
      }
    }
    stereoList.remove(removeList);
    if (stereoList.length() < 2)continue;

    TTrack* track = new TTrack(*circle);
    m_allTracks.append(track);
    if (m_builder.buildStereoMC(*track, stereoList)) {
      if (track->links().length() >= 5) {
        track->assign(CellHitCurlFinder, TrackCurlFinder | TrackValid | Track3D);
        tracks.append(track);
        m_allTracks.remove(track);
      } else {
        std::cout << "Can not reconstruct with MC information!" << std::endl;
      }
    } else {
      std::cout << "Can not reconstruct with MC information!" << std::endl;
    }
  }
  return 0;
}
#endif

#if DEBUG_CURL_GNUPLOT+DEBUG_CURL_SEGMENT
void
TCurlFinder::makeCdcFrame(void)
{
  //#if 1
  double X = 0.;
  double Y = 0.;
  double R[12] = {8.3,  16.9, 21.7, 31.3, 36.1, 44.1,
                  50.5, 58.5, 64.9, 72.9, 79.3, 87.4
                 };
  double step = 300.;
  double dStep = 2.*M_PI / step;
  FILE* data;
  std::string nameHead = "tmp.cdc_";
  for (int j = 0; j < 12; ++j) {
    std::string nameFile = nameHead + "0" + itostring(j);
    if (j >= 10)nameFile = nameHead + itostring(j);
    if ((data = fopen(nameFile, "w")) != NULL) {
      for (int i = 0; i < step; ++i) {
        double x = X + R[j] * cos(dStep * static_cast<double>(i));
        double y = Y + R[j] * sin(dStep * static_cast<double>(i));
        std::fprintf(data, "%lf, %lf\n", x, y);
      }
      fclose(data);
    }
  }

  if ((data = fopen("tmp_wires.dat", "w")) != NULL) {
    AList<TLink> list = m_unusedAxialHitsOriginal;
    list.append(m_unusedStereoHitsOriginal);
    for (int i = 0; i < list.length(); i++) {
      double x = list[i]->hit()->wire().xyPosition().x();
      double y = list[i]->hit()->wire().xyPosition().y();
      std::fprintf(data, "%lf, %lf\n", x, y);
    }
    fclose(data);
  }
  return;
}
#endif

#if DEBUG_CURL_GNUPLOT
void
TCurlFinder::plotSegment(const AList<TLink>& list, const int flag)
{
  if (!m_debugCdcFrame) {
    makeCdcFrame();
    m_debugCdcFrame = true;
  }
  double gmaxX = 90. , gminX = -90.;
  double gmaxY = 90. , gminY = -90.;
  FILE* gnuplot, *data;
  if ((data = fopen("tmp.dat", "w")) != NULL) {
    if (flag)std::cout << "Wire ID = ";
    for (int i = 0; i < list.length(); i++) {
      double x = list[i]->hit()->wire().xyPosition().x();
      double y = list[i]->hit()->wire().xyPosition().y();
      std::fprintf(data, "%lf, %lf\n", x, y);
      if (flag)std::cout << list[i]->hit()->wire().id() << ", ";
    }
    if (flag)std::cout << std::endl;
    fclose(data);
  }
  if ((gnuplot = popen("gnuplot", "w")) != NULL) {
    std::fprintf(gnuplot, "set nokey \n");
    std::fprintf(gnuplot, "set size 0.721,1.0 \n");
    std::fprintf(gnuplot, "set xrange [%f:%f] \n", gminX, gmaxX);
    std::fprintf(gnuplot, "set yrange [%f:%f] \n", gminY, gmaxY);
    std::string longName = "plot \"tmp_wires.dat\", \"tmp.dat\"";
    std::string nameHead = ",\"tmp.cdc_";
    for (int j = 0; j < 12; ++j) {
      std::string nameFile = nameHead + "0" + itostring(j) + "\"w l 0";
      if (j >= 10)nameFile = nameHead + itostring(j) + "\"w l 0";
      longName += nameFile;
    }
    longName += " \n";
    std::fprintf(gnuplot, longName);
    fflush(gnuplot);
    char tmp[8];
    gets(tmp);
    pclose(gnuplot);
  }
  return;
}

void
TCurlFinder::plotCircle(const TCircle& circle, const int flag)
{
  //#if 1
  if (!m_debugCdcFrame) {
    makeCdcFrame();
    m_debugCdcFrame = true;
  }
  double gmaxX = 90. , gminX = -90.;
  double gmaxY = 90. , gminY = -90.;
  FILE* gnuplot, *data;
  if ((data = fopen("tmp.dat1", "w")) != NULL) {
    if (flag)std::cout << "Axial  Wire ID ==> " << std::endl;
    for (int i = 0; i < circle.nLinks(); ++i) {
      if (circle.links()[i]->hit()->wire().axial()) {
        double x = circle.links()[i]->hit()->wire().xyPosition().x();
        double y = circle.links()[i]->hit()->wire().xyPosition().y();
        std::fprintf(data, "%lf, %lf\n", x, y);
        if (flag) {
          /*if(debugMcFlag){
            std::cout << " A:" << circle.links()[i]->hit()->wire().id() << ", ";
            std::cout << ", HepTrackID = " << circle.links()[i]->hit()->mc()->hep()->id();
            std::cout << ", HepLundID = "  << circle.links()[i]->hit()->mc()->hep()->pType() << std::endl;
            }else std::cout << " A:" << circle.links()[i]->hit()->wire().id() << std::endl;*/
        }
      }
    }
    if (flag)std::cout << std::endl;
    fclose(data);
  }
  if ((data = fopen("tmp.dat2", "w")) != NULL) {
    if (flag)std::cout << "Stereo Wire ID ==> " << std::endl;
    for (int i = 0; i < circle.nLinks(); ++i) {
      if (circle.links()[i]->hit()->wire().stereo()) {
        double x = circle.links()[i]->hit()->wire().xyPosition().x();
        double y = circle.links()[i]->hit()->wire().xyPosition().y();
        std::fprintf(data, "%lf, %lf\n", x, y);
        if (flag) {
          /*if(debugMcFlag){
            std::cout << " S:" << circle.links()[i]->hit()->wire().id() << ", ";
            std::cout << ", HepTrackID = " << circle.links()[i]->hit()->mc()->hep()->id();
            std::cout << ", HepLundID = "  << circle.links()[i]->hit()->mc()->hep()->pType() << std::endl;
            }else std::cout << " S:" << circle.links()[i]->hit()->wire().id() << std::endl;*/
        }
      }
    }
    if (flag)std::cout << std::endl;
    fclose(data);
  }
  double X = circle.center().x();
  double Y = circle.center().y();
  double R = fabs(circle.radius());
  double step = 300.;
  double dStep = 2.*M_PI / step;
  if ((data = fopen("tmp.dat3", "w")) != NULL) {
    for (int i = 0; i < step; ++i) {
      double x = X + R * cos(dStep * static_cast<double>(i));
      double y = Y + R * sin(dStep * static_cast<double>(i));
      std::fprintf(data, "%lf, %lf\n", x, y);
    }
    fclose(data);
  }
  if ((gnuplot = popen("gnuplot", "w")) != NULL) {
    std::fprintf(gnuplot, "set nokey \n");
    std::fprintf(gnuplot, "set size 0.721,1.0 \n");
    std::fprintf(gnuplot, "set xrange [%f:%f] \n", gminX, gmaxX);
    std::fprintf(gnuplot, "set yrange [%f:%f] \n", gminY, gmaxY);
    std::string longName = "plot \"tmp_wires.dat\", \"tmp.dat1\", \"tmp.dat3\" w l, \"tmp.dat2\"";
    std::string nameHead = ",\"tmp.cdc_";
    for (int j = 0; j < 12; ++j) {
      std::string nameFile = nameHead + "0" + std::string(j) + "\"w l 0";
      if (j >= 10)nameFile = nameHead + std::string(j) + "\"w l 0";
      longName += nameFile;
    }
    longName += " \n";
    std::fprintf(gnuplot, longName);
    fflush(gnuplot);
    char tmp[8];
    gets(tmp);
    pclose(gnuplot);
  }
  return;
}

void
TCurlFinder::plotTrack(const TTrack& track, const int flag)
{
  if (!m_debugCdcFrame) {
    makeCdcFrame();
    m_debugCdcFrame = true;
  }
  double gmaxX = 90. , gminX = -90.;
  double gmaxY = 90. , gminY = -90.;
  FILE* gnuplot, *data;
  if ((data = fopen("tmp.dat1", "w")) != NULL) {
    if (flag)std::cout << "Axial  Wire ID ==> " << std::endl;
    for (int i = 0; i < track.nLinks(); ++i) {
      if (track.links()[i]->hit()->wire().axial()) {
        double x = track.links()[i]->hit()->wire().xyPosition().x();
        double y = track.links()[i]->hit()->wire().xyPosition().y();
        std::fprintf(data, "%lf, %lf\n", x, y);
        if (flag) {
          if (debugMcFlag) {
            std::cout << " A:" << track.links()[i]->hit()->wire().id() << ", ";
            std::cout << ", HepTrackID = " << track.links()[i]->hit()->mc()->hep()->id();
            std::cout << ", HepLundID = "  << track.links()[i]->hit()->mc()->hep()->pType() << std::endl;
          } else std::cout << " A:" << track.links()[i]->hit()->wire().id() << std::endl;
        }
      }
    }
    if (flag)std::cout << std::endl;
    fclose(data);
  }
  if ((data = fopen("tmp.dat2", "w")) != NULL) {
    if (flag)std::cout << "Stereo Wire ID ==> " << std::endl;
    for (int i = 0; i < track.nLinks(); ++i) {
      if (track.links()[i]->hit()->wire().stereo()) {
        double x = track.links()[i]->hit()->wire().xyPosition().x();
        double y = track.links()[i]->hit()->wire().xyPosition().y();
        std::fprintf(data, "%lf, %lf\n", x, y);
        if (flag) {
          if (debugMcFlag) {
            std::cout << " S:" << track.links()[i]->hit()->wire().id() << ", ";
            std::cout << ", HepTrackID = " << track.links()[i]->hit()->mc()->hep()->id();
            std::cout << ", HepLundID = "  << track.links()[i]->hit()->mc()->hep()->pType() << std::endl;
          } else std::cout << " S:" << track.links()[i]->hit()->wire().id() << std::endl;
        }
      }
    }
    if (flag)std::cout << std::endl;
    fclose(data);
  }
  double X = track.helix().center().x();
  double Y = track.helix().center().y();
  double R = fabs(track.helix().radius());
  double step = 300.;
  double dStep = 2.*M_PI / step;
  if ((data = fopen("tmp.dat3", "w")) != NULL) {
    for (int i = 0; i < step; ++i) {
      double x = X + R * cos(dStep * static_cast<double>(i));
      double y = Y + R * sin(dStep * static_cast<double>(i));
      std::fprintf(data, "%lf, %lf\n", x, y);
    }
    fclose(data);
  }
  if ((gnuplot = popen("gnuplot", "w")) != NULL) {
    std::fprintf(gnuplot, "set nokey \n");
    std::fprintf(gnuplot, "set size 0.721,1.0 \n");
    std::fprintf(gnuplot, "set xrange [%f:%f] \n", gminX, gmaxX);
    std::fprintf(gnuplot, "set yrange [%f:%f] \n", gminY, gmaxY);
    std::string longName = "plot \"tmp_wires.dat\", \"tmp.dat1\", \"tmp.dat3\" w l, \"tmp.dat2\"";
    std::string nameHead = ",\"tmp.cdc_";
    for (int j = 0; j < 12; ++j) {
      std::string nameFile = nameHead + "0" + itostring(j) + "\"w l 0";
      if (j >= 10)nameFile = nameHead + itostring(j) + "\"w l 0";
      longName += nameFile;
    }
    longName += " \n";
    std::fprintf(gnuplot, longName);
    fflush(gnuplot);
    char tmp[8];
    gets(tmp);
    pclose(gnuplot);
  }
  return;
}
#endif

#if DEBUG_CURL_SEGMENT
void
TCurlFinder::writeSegment(const AList<TLink>& list, const int type)
{
  if (!m_debugCdcFrame) {
    makeCdcFrame();
    m_debugCdcFrame = true;
  }
  double gmaxX = 90. , gminX = -90.;
  double gmaxY = 90. , gminY = -90.;

  FILE* data;
  std::string nameHead = "tmp.segment_";
  std::string nameFile = nameHead + itostring(type) + "_" + itostring(m_debugFileNumber);
  ++m_debugFileNumber;
  if ((data = fopen(nameFile, "w")) != NULL) {
    for (int i = 0; i < list.length(); i++) {
      double x = list[i]->hit()->wire().xyPosition().x();
      double y = list[i]->hit()->wire().xyPosition().y();
      std::fprintf(data, "%lf, %lf\n", x, y);
    }
    fclose(data);
  }
  return;
}
#endif

#if TRASAN_DEBUG_DETAIL
void
TCurlFinder::dumpType1(TTrack* track)
{
  for (int j = 0; j < (int) track->nLinks(); ++j) {
    std::cout << "Used Wire Info...";
    if (track->links()[j]->hit()->wire().axial()) {
      std::cout << "A:" << track->links()[j]->hit()->wire().id() << ", ";
    } else {
      std::cout << "S:" << track->links()[j]->hit()->wire().id() << ", ";
    }
//     if(debugMcFlag){
//       std::cout << ", HepTrackID = " << track->links()[j]->hit()->mc()->hep()->id();
//       std::cout << ", HepLundID = "  << track->links()[j]->hit()->mc()->hep()->pType();
//     }
    double dist = distance(*track, *(track->links()[j]));
    if (dist > 2.)std::cout << ": Large Distance( >2cm ) = " << dist;
    std::cout << std::endl;
  }
  AList<TLink> list = m_unusedAxialHits;
  list.append(m_unusedStereoHits);
  for (unsigned j = 0, nList = list.length(); j < nList; ++j) {
    double dist = distance(*track, *(list[j]));
    std::cout << "Close Wire Info in ALL( <0.5cm )...";
    if (dist < 0.5) {
      if (list[j]->hit()->wire().axial())
        std::cout << "CA:" << list[j]->hit()->wire().id() << ", ";
      else
        std::cout << "CS:" << list[j]->hit()->wire().id() << ", ";
//       if(debugMcFlag){
//  std::cout << ", HepTrackID = " << list[j]->hit()->mc()->hep()->id();
//  std::cout << ", HepLundID = "  << list[j]->hit()->mc()->hep()->pType();
//       }
      std::cout << ", Distance = " << dist << std::endl;
    }
  }
  return;
}

void
TCurlFinder::dumpType2(TTrack* track)
{
  unsigned size = track->nLinks();
  if (size == 0)return;

  std::set< int, std::less<int> > uniqueHepID;
  std::vector<int> hepID;
  std::vector<double> ratio;
  for (int i = 0; i < (int)size; ++i) {
    uniqueHepID.insert(track->links()[i]->hit()->mc()->hep()->id());
    hepID.push_back(track->links()[i]->hit()->mc()->hep()->id());
    // std::cout << i << " : " << track->links()[i]->hit()->mc()->hep()->id() << std::endl;
  }

  std::set< int, std::less<int> >::iterator u = uniqueHepID.begin();
  std::vector<int>::size_type sizeInt;
  for (unsigned i = 0; i < uniqueHepID.size(); ++i) {
    sizeInt = std::count(hepID.begin(), hepID.end(), *u);
    ratio.push_back((static_cast<double>(sizeInt) / static_cast<double>(size)));
    // std::cout << "HepID = " << *u << ", Ratio = " << ratio[i] << " = " << sizeInt << "/" << size << std::endl;
    ++u;
  }

  std::vector<double>::iterator m = max_element(ratio.begin(), ratio.end());
  int maxIndex = std::distance(ratio.begin(), m);
  u = uniqueHepID.begin();
  advance(u, maxIndex);
  // std::cout << "MAX HepID = " << *u << ", Ratio = " << ratio[maxIndex] << std::endl;
  std::cout << "Ratio " << ratio[maxIndex] << std::endl;
  for (int i = 0; i < (int)size; ++i) {
    if (track->links()[i]->hit()->wire().axial())std::cout << "A ";
    else std::cout << "S ";

    double dist = distance(*track, *(track->links()[i]));
    if (*u != (int) track->links()[i]->hit()->mc()->hep()->id()) {
      std::cout << "Bad " << dist << std::endl;
    } else {
      std::cout << "Good " << dist << std::endl;
    }
  }
  return;
}
#endif

//#if DEBUG_CURL_MC
#if 0
// --> TSegmentUtil.h
sortBySvdRLA(const Datsvd_hit** a, const Datsvd_hit** b)
{
  if ((*a)->rla() < (*b)->rla()) {
    return -1;
  } else if ((*a)->rla() == (*b)->rla()) {
    return 0;
  } else {
    return 1;
  }
}

Gen_hepevt*
cluster2hep(Recsvd_cluster* clus)
{
  AList<Datsvd_hit> m_datsvd_hit;
  Datsvd_hit_Manager& svdHitMgr = Datsvd_hit_Manager::get_manager();
  for (Datsvd_hit_Manager::iterator
       it  = svdHitMgr.begin(),
       end = svdHitMgr.end();
       it != end; ++it) {
    m_datsvd_hit.append(it);
  }
  m_datsvd_hit.sort(sortBySvdRLA);

  unsigned size = m_datsvd_hit.length();
  int startPosition = -1;
  int direction = 1;
#if 0
  for (unsigned i = 0; i < size; ++i) {
    //if(clus->width() == 1)
    std::cout << i << ": " << clus->hit().get_ID() << " <--> " << m_datsvd_hit[i]->get_ID()
              << ", RLA = " << m_datsvd_hit[i]->rla() << ", LSA = " << m_datsvd_hit[i]->amp()
              << ", Width = " << clus->width()
              << ", Cluster LSA = " << clus->lsa() << std::endl;
  }
#endif
  for (unsigned i = 0; i < size; ++i) {
    if (m_datsvd_hit[i]->amp() == 0)std::cout << "DatSVD_Hit:amp == 0" << std::endl;
    //if(m_datsvd_hit[i]->amp() == 640)std::cout << "DatSVD_Hit:amp == 640" << std::endl;
    if (m_datsvd_hit[i]->rla() == 0)std::cout << "DatSVD_Hit:rla == 0" << std::endl;
    if (m_datsvd_hit[i]->rla() == 81920)std::cout << "DatSVD_Hit:rla == 81920" << std::endl;
    if (clus->hit().get_ID() == m_datsvd_hit[i]->get_ID()) {
      startPosition = i;
      if (static_cast<double>(m_datsvd_hit[i]->amp() - 1) > clus->lsa())direction = -1;
      break;
    }
  }
  if (startPosition == -1)return NULL;
  int width = clus->width();
#if 0
  std::cout << "Start # = " << startPosition
            << ", Width = " << width
            << ", Direction = " << direction << std::endl;
#endif

  int* hepID = new int[width];
  set< int, less<int> > uniqueHepID;

  for (int i = startPosition; i < startPosition + width; ++i)hepID[i - startPosition] = 0;
  //Datsvd_mchit_Manager& svdMcHitMgr = Datsvd_mchit_Manager::get_manager();
  Datsvd_mcdata_Manager& svdMcHitMgr = Datsvd_mcdata_Manager::get_manager();
  // std::cout << "SVD MC# = " << svdMcHitMgr.count() << std::endl;
  if (direction == 1) {
    for (int i = startPosition; i < startPosition + width; ++i) {
      for (Datsvd_mcdata_Manager::iterator
           it  = svdMcHitMgr.begin(),
           end = svdMcHitMgr.end();
           it != end; ++it) {
        if (it->Hep()) {
          if (m_datsvd_hit[i]->rla() == it->rla() &&
              it->Hep().get_ID() != 0) {
            hepID[i - startPosition] = it->Hep().get_ID();
            uniqueHepID.insert(it->Hep().get_ID());
            break;
          }
        }
      }
    }
  } else {
    int reverse = 0;
    for (int i = startPosition; i < startPosition + width; ++i) {
      ++reverse;
      // std::cout << startPosition+1-reverse << " --- "
      //   << i << std::endl;
      for (Datsvd_mcdata_Manager::iterator
           it  = svdMcHitMgr.begin(),
           end = svdMcHitMgr.end();
           it != end; ++it) {
        if (it->Hep()) {
          if (m_datsvd_hit[startPosition + 1 - reverse]->rla() == it->rla() &&
              it->Hep().get_ID() != 0) {
            hepID[i - startPosition] = it->Hep().get_ID();
            uniqueHepID.insert(it->Hep().get_ID());
            break;
          }
        }
      }
    }
  }

  unsigned num = uniqueHepID.size();
  int* counter = new int[num];
  set< int, less<int> >::iterator u = uniqueHepID.begin();
  for (int i = 0; i < num; ++i) counter[i] = 0;

  for (int i = 0; i < num; ++i) {
    for (int j = 0; j < width; ++j) {
      if (*u == hepID[j]) {
        counter[i] += 1;
      }
    }
    ++u;
  }

  Gen_hepevt_Manager& genMgr = Gen_hepevt_Manager::get_manager();
#if 0
  u = uniqueHepID.begin();
  for (int i = 0; i < num; ++i) {
    std::cout << i << ": TrackID = " << *u - 1
              << ", Count = " << counter[i]
              << ", LundID = " << genMgr[*u - 1].idhep() << std::endl;
    ++u;
  }
#endif

  delete [] hepID;
  delete [] counter;
  if (num == 1) {
    // std::cout << *(uniqueHepID.begin())-1 << std::endl;
    return &(genMgr[*(uniqueHepID.begin()) - 1]);
  } else if (num >= 2) {
    // std::cout << "A svd cluster is not unique." << std::endl;
    return NULL;
  } else {
    return NULL;
  }
}
#endif

#if DEBUG_CURL_SEGMENT
void
TCurlFinder::debugCheckSegments1(void)
{
  // Slow Checker(CPU time increases!!)
  // Neighboring wires should be included in the same segement.
  std::cout << "(TCurlFinder)checking consistency of segement..." << std::endl;
  unsigned nA = m_unusedAxialHitsOriginal.length();
  if (nA >= 2) {
    for (unsigned i = 0; i < nA - 1; ++i) {
      int superLayerId = (int)(m_unusedAxialHitsOriginal[i]->wire()->superLayerId());
      int layerId  = (int)(m_unusedAxialHitsOriginal[i]->wire()->layerId());
      int localId  = (int)(m_unusedAxialHitsOriginal[i]->wire()->localId());
      int localIdP = (int)(m_unusedAxialHitsOriginal[i]->wire()->localIdForPlus());
      int localIdM = (int)(m_unusedAxialHitsOriginal[i]->wire()->localIdForMinus());
      for (unsigned j = i + 1; j < nA; ++j) {
        int superLayerId2 = (int)(m_unusedAxialHitsOriginal[j]->wire()->superLayerId());
        int layerId2 = (int)(m_unusedAxialHitsOriginal[j]->wire()->layerId());
        int localId2 = (int)(m_unusedAxialHitsOriginal[j]->wire()->localId());
        if (superLayerId == superLayerId2) {
          if (layerId2 == layerId) {
            if (localIdP + 1 == localId2 || localIdM - 1 == localId2)
              debugCheckSegments(localId, layerId,
                                 localId2, layerId2);
          } else if (layerId2 == layerId - 1 || layerId2 == layerId + 1) {
            if (offset(layerId) == offset(layerId2)) {
              std::cout << "(TCurlFinder: Waring) Offset is same at the same superlayer!!" << std::endl;
            } else if (offset(layerId) > offset(layerId2)) {
              if (localId == localId2 || localIdP + 1 == localId2)
                debugCheckSegments(localId, layerId,
                                   localId2, layerId2);
            } else {
              if (localId == localId2 || localIdM - 1 == localId2)
                debugCheckSegments(localId, layerId,
                                   localId2, layerId2);
            }
          }
        }
      }
    }
  }
  unsigned nS = m_unusedStereoHitsOriginal.length();
  if (nS >= 2) {
    for (unsigned i = 0; i < nS - 1; ++i) {
      int superLayerId = (int)(m_unusedStereoHitsOriginal[i]->wire()->superLayerId());
      int layerId  = (int)(m_unusedStereoHitsOriginal[i]->wire()->layerId());
      int localId  = (int)(m_unusedStereoHitsOriginal[i]->wire()->localId());
      int localIdP = (int)(m_unusedStereoHitsOriginal[i]->wire()->localIdForPlus());
      int localIdM = (int)(m_unusedStereoHitsOriginal[i]->wire()->localIdForMinus());
      for (unsigned j = i + 1; j < nS; ++j) {
        int superLayerId2 = (int)(m_unusedStereoHitsOriginal[j]->wire()->superLayerId());
        int layerId2 = (int)(m_unusedStereoHitsOriginal[j]->wire()->layerId());
        int localId2 = (int)(m_unusedStereoHitsOriginal[j]->wire()->localId());
        if (superLayerId == superLayerId2) {
          if (layerId2 == layerId) {
            if (localIdP + 1 == localId2 || localIdM - 1 == localId2)
              debugCheckSegments(localId, layerId,
                                 localId2, layerId2);
          } else if (layerId2 == layerId - 1 || layerId2 == layerId + 1) {
            if (offset(layerId) == offset(layerId2)) {
              std::cout << "(TCurlFinder: Waring) Offset is same at the same superlayer!!" << std::endl;
            } else if (offset(layerId) > offset(layerId2)) {
              if (localId == localId2 || localIdP + 1 == localId2)
                debugCheckSegments(localId, layerId,
                                   localId2, layerId2);
            } else {
              if (localId == localId2 || localIdM - 1 == localId2)
                debugCheckSegments(localId, layerId,
                                   localId2, layerId2);
            }
          }
        }
      }
    }
  }
  std::cout << "(TCurlFinder)...done check of segement!" << std::endl;
  std::cout << "(TCurlFinder)...If no warning message exists, check of segement is complete!" << std::endl;
  std::cout << "(TCurlFinder)...Note: a segment size should be 1 or 2 to use this debugger." << std::endl;
  return;
}

void
TCurlFinder::debugCheckSegments(const double localId, const double layerId,
                                const double localId2, const double layerId2)
{
  unsigned nSeg = m_segmentList.length();
  unsigned nFound = 0;
  for (unsigned i = 0; i < nSeg; ++i) {
    unsigned nWire = m_segmentList[i]->list().length();
    unsigned mFound = 0;
    for (unsigned j = 0; j < nWire; ++j) {
      if (((m_segmentList[i]->list())[j])->wire()->layerId() == layerId &&
          ((m_segmentList[i]->list())[j])->wire()->localId() == localId)++mFound;
      if (((m_segmentList[i]->list())[j])->wire()->layerId() == layerId2 &&
          ((m_segmentList[i]->list())[j])->wire()->localId() == localId2)++mFound;
    }
    if (mFound != 0 && mFound != 2) {
      std::cout << "(TCurlFinder: Warning) Segment is inconsistency(0)!! mFound = " << mFound << std::endl;
    }
    if (mFound == 2)++nFound;
  }
  if (nFound != 1)
    std::cout << "(TCurlFinder: Warning) Segment is inconsistency(1)!! nFound = " << nFound << std::endl;
  return;
}

void
TCurlFinder::debugCheckSegments0(void)
{
  unsigned nSeg = m_segmentList.length();
  unsigned nWire = 0;
  for (unsigned i = 0; i < nSeg; ++i)nWire += m_segmentList[i]->list().length();

  unsigned nWireOriginal = m_unusedAxialHitsOriginal.length() +
                           m_unusedStereoHitsOriginal.length();

  std::cout << "(TCurlFinder: SelfChecker) Segment Parts" << std::endl;
  std::cout << "                           MIN_SEGMENT = " << m_param.MIN_SEGMENT << std::endl;
  std::cout << "                           Wire # of Orinal List = " << nWireOriginal << std::endl;
  std::cout << "                           Wire # of Segments    = " << nWire << std::endl;
  std::cout << "                           If MIN_SEGMENT <= 1, above numbers should be same." << std::endl;
  std::cout << "                           If MIN_SEGMENT >  1, former >= latter." << std::endl;
  return;
}

void
TCurlFinder::debugCheckSegments2(void)
{

#define DEBUG_TMP_N_CURL 50

  unsigned nSeg = m_segmentList.length();
  unsigned nWire[DEBUG_TMP_N_CURL] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                                     };
  for (unsigned i = 0; i < nSeg; ++i) {
    if (m_segmentList[i]->list().length() < DEBUG_TMP_N_CURL)
      ++(nWire[m_segmentList[i]->list().length()]);
    else
      ++(nWire[DEBUG_TMP_N_CURL - 1]);
  }
  std::ifstream fin("tmp.wire.data");
  unsigned nTotalWire[DEBUG_TMP_N_CURL] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                                          };
  if (fin) {
    for (int i = 0; i < DEBUG_TMP_N_CURL; ++i)fin >> nTotalWire[i];
  } else {
    std::cout << "(TCurlFinder) tmp.wire.data does not exist!" << std::endl;
  }
  for (int i = 0; i < DEBUG_TMP_N_CURL; ++i)nTotalWire[i] += nWire[i];
  std::ofstream fout("tmp.wire.data");
  if (fout) {
    fout << nTotalWire[0];
    for (int i = 1; i < DEBUG_TMP_N_CURL; ++i)fout << " " << nTotalWire[i];
  } else {
    std::cout << "(TCurlFinder) tmp.wire.data can not be made!" << std::endl;
  }
  return;
}
#endif

#undef DEBUG_CURL_DUMP
#undef DEBUG_CURL_SEGMENT
#undef DEBUG_CURL_GNUPLOT
#undef DEBUG_CURL_MC

#ifdef TRASAN_WINDOW
void
TCurlFinder::displayStatus(const std::string& m) const
{
  _cWindow.clear();
  _cWindow.text(m);
  for (unsigned i = 0; i < 32; i++)
    _cWindow.append(m_unusedAxialHitsOnEachLayer[i]);
  for (unsigned i = 0; i < 18; i++)
    _cWindow.append(m_unusedStereoHitsOnEachLayer[i]);
  _cWindow.append(m_segmentList, leda_orange);
}
#endif

} // namespace Belle

