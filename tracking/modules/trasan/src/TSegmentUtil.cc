

#include <math.h>
#include <vector>
#include <set>
#include <algorithm>

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


#include "tracking/modules/trasan/AList.h"


#if defined(BELLE_NAMESPACE)
namespace Belle {
#endif


  unsigned
  CDCSegId(unsigned localId,
           unsigned layerId,
           unsigned& segId0,
           unsigned& segId1)
  {
    segId1 = 9999;
    if (layerId == 0 ||
        layerId == 2 ||
        layerId == 4
       ) {
      segId0 = unsigned(localId / 2.0);
      return 1;
    } else if (layerId ==  9 ||
               layerId == 11 ||
               layerId == 13
              ) {
      segId0 = unsigned(localId / 3.0);
      return 1;
    } else if (layerId == 18 ||
               layerId == 20 ||
               layerId == 22
              ) {
      segId0 = unsigned(localId / 4.5);
      return 1;
    } else if (layerId == 27 ||
               layerId == 29 ||
               layerId == 31
              ) {
      segId0 = unsigned(localId / 6.0);
      return 1;
    } else if (layerId == 36 ||
               layerId == 38 ||
               layerId == 40
              ) {
      segId0 = unsigned(localId / 7.5);
      return 1;
    } else if (layerId == 45 ||
               layerId == 47 ||
               layerId == 49
              ) {
      segId0 = unsigned(localId / 9.0);
      return 1;
    } else if (layerId == 1 ||
               layerId == 3 ||
               layerId == 5
              ) {
      if (localId == 0) {
        segId0 = unsigned(localId / 2.0);
        segId1 = unsigned(63 / 2.0);
        return 2;
      } else {
        segId0 = unsigned(localId / 2.0);
        segId1 = unsigned((localId - 1) / 2.0);
        if (segId0 == segId1) {
          segId1 = 9999;
          return 1;
        } else return 2;
      }
    } else if (layerId == 10 ||
               layerId == 12 ||
               layerId == 14
              ) {
      if (localId == 0) {
        segId0 = unsigned(localId / 3.0);
        segId1 = unsigned(95 / 3.0);
        return 2;
      } else {
        segId0 = unsigned(localId / 3.0);
        segId1 = unsigned((localId - 1) / 3.0);
        if (segId0 == segId1) {
          segId1 = 9999;
          return 1;
        } else return 2;
      }
    } else if (layerId == 19 ||
               layerId == 21
              ) {
      if (localId == 0) {
        segId0 = unsigned(localId / 4.5);
        segId1 = unsigned(143 / 4.5);
        return 2;
      } else {
        segId0 = unsigned(localId / 4.5);
        segId1 = unsigned((localId - 1) / 4.5);
        if (segId0 == segId1) {
          segId1 = 9999;
          return 1;
        } else return 2;
      }
    } else if (layerId == 28 ||
               layerId == 30
              ) {
      if (localId == 0) {
        segId0 = unsigned(localId / 6.0);
        segId1 = unsigned(191 / 6.0);
        return 2;
      } else {
        segId0 = unsigned(localId / 6.0);
        segId1 = unsigned((localId - 1) / 6.0);
        if (segId0 == segId1) {
          segId1 = 9999;
          return 1;
        } else return 2;
      }
    } else if (layerId == 37 ||
               layerId == 39
              ) {
      if (localId == 0) {
        segId0 = unsigned(localId / 7.5);
        segId1 = unsigned(239 / 7.5);
        return 2;
      } else {
        segId0 = unsigned(localId / 7.5);
        segId1 = unsigned((localId - 1) / 7.5);
        if (segId0 == segId1) {
          segId1 = 9999;
          return 1;
        } else return 2;
      }
    } else if (layerId == 46 ||
               layerId == 48
              ) {
      if (localId == 0) {
        segId0 = unsigned(localId / 9.0);
        segId1 = unsigned(287 / 9.0);
        return 2;
      } else {
        segId0 = unsigned(localId / 9.0);
        segId1 = unsigned((localId - 1) / 9.0);
        if (segId0 == segId1) {
          segId1 = 9999;
          return 1;
        } else return 2;
      }
    }
    return 0;
  }

  unsigned
  CDCSegLayerId(unsigned id) // CDC Global Wire ID
  {
    if (id < 192)  return 0;
    else if (id < 384)  return 1;
    else if (id < 624)  return 9999;
    else if (id < 912)  return 2;
    else if (id < 1200) return 3;
    else if (id < 1584) return 9999;
    else if (id < 2304) return 4;
    else if (id < 2944) return 9999;
    else if (id < 3904) return 5;
    else if (id < 4736) return 9999;
    else if (id < 5936) return 6;
    else if (id < 6960) return 9999;
    else if (id < 8400) return 7;
    else return 9999;
  }

  unsigned
  CDCSuperLayerId(unsigned id) // CDC Global Wire ID
  {
    if (id < 192)  return 0;
    else if (id < 384)  return 0;
    else if (id < 624)  return 9999;
    else if (id < 912)  return 1;
    else if (id < 1200) return 1;
    else if (id < 1584) return 9999;
    else if (id < 2304) return 2;
    else if (id < 2944) return 9999;
    else if (id < 3904) return 3;
    else if (id < 4736) return 9999;
    else if (id < 5936) return 4;
    else if (id < 6960) return 9999;
    else if (id < 8400) return 5;
    else return 9999;
  }

  int
  CDCMovedSegId(unsigned refSegId,
                unsigned segId,
                unsigned segLayerId,
                unsigned type)
  {
    int movedSegId = 9999;
    if (type == 0) { // CDC SuperLayer = 0
      movedSegId = (int)segId - (int)refSegId;
      if (segLayerId >= 6)
        movedSegId = 9999;
      else if (segLayerId == 0 && abs(movedSegId) >= 5)
        movedSegId = 9999;
      else if (segLayerId == 1 && abs(movedSegId) >= 5)
        movedSegId = 9999;
      else if (segLayerId == 2 && abs(movedSegId) >= 4)
        movedSegId = 9999;
      else if (segLayerId == 3 && abs(movedSegId) >= 6)
        movedSegId = 9999;
      else if (segLayerId == 4 && abs(movedSegId) >= 6)
        movedSegId = 9999;
      else if (segLayerId == 4 && abs(movedSegId) == 0)
        movedSegId = 9999;
      else if (segLayerId == 5 && abs(movedSegId) >= 6)
        movedSegId = 9999;
      else if (segLayerId == 5 && abs(movedSegId) <= 1)
        movedSegId = 9999;
    } else if (type == 1) { // CDC SuperLayer = 1
      movedSegId = (int)segId - (int)refSegId;
      if (segLayerId >= 6)
        movedSegId = 9999;
      else if (segLayerId == 0 && abs(movedSegId) >= 5)
        movedSegId = 9999;
      else if (segLayerId == 1 && abs(movedSegId) >= 5)
        movedSegId = 9999;
      else if (segLayerId == 2 && abs(movedSegId) >= 4)
        movedSegId = 9999;
      else if (segLayerId == 3 && abs(movedSegId) >= 6)
        movedSegId = 9999;
      else if (segLayerId == 4 && abs(movedSegId) >= 6)
        movedSegId = 9999;
      else if (segLayerId == 4 && abs(movedSegId) == 0)
        movedSegId = 9999;
      else if (segLayerId == 5 && abs(movedSegId) >= 6)
        movedSegId = 9999;
      else if (segLayerId == 5 && abs(movedSegId) <= 1)
        movedSegId = 9999;
    } else if (type == 2) { // SVD
      movedSegId = (int)segId - (int)(0.5 * (double)refSegId);
      if (segLayerId >= 6)
        movedSegId = 9999;
      else if (segLayerId == 0 && abs(movedSegId) >= 2)
        movedSegId = 9999;
      else if (segLayerId == 1 && abs(movedSegId) >= 4)
        movedSegId = 9999;
      else if (segLayerId == 2 && abs(movedSegId) >= 6)
        movedSegId = 9999;
      else if (segLayerId == 2 && movedSegId == 0)
        movedSegId = 9999;
      else if (segLayerId == 3 && abs(movedSegId) >= 7)
        movedSegId = 9999;
      else if (segLayerId == 3 && movedSegId == 0)
        movedSegId = 9999;
      else if (segLayerId == 4 && abs(movedSegId) >= 7)
        movedSegId = 9999;
      else if (segLayerId == 4 && abs(movedSegId) <= 1)
        movedSegId = 9999;
      else if (segLayerId == 5 && abs(movedSegId) >= 7)
        movedSegId = 9999;
      else if (segLayerId == 5 && abs(movedSegId) <= 2)
        movedSegId = 9999;
    }
    return movedSegId;
  }

  int
  CDCMovedSegId2(unsigned refSegId,
                 unsigned segId,
                 unsigned,
                 unsigned type)
  {
    // wide region version
    // This is used in mapping test.
    int movedSegId = 9999;
    if (type == 0) { // CDC SuperLayer = 0
      movedSegId = (int)segId - (int)refSegId;
      if (abs(movedSegId) >= 15)movedSegId = 9999;
    } else if (type == 1) { // CDC SuperLayer = 1
      movedSegId = (int)segId - (int)refSegId;
      if (abs(movedSegId) >= 15)movedSegId = 9999;
    } else if (type == 2) { // SVD
      movedSegId = (int)segId - (int)(0.5 * (double)refSegId);
      if (abs(movedSegId) >= 15)movedSegId = 9999;
    }
    return movedSegId;
  }

  unsigned
  SVDSegLayerId(unsigned id)
  {
    if (id < 16)return 0;
    else if (id < 46)return 1;
    else if (id < 102)return 2;
    else return 9999;
  }

  unsigned
  SVDSegId(double x,
           double y,
           unsigned& segId0,
           unsigned& segId1)
  {
#if 1
    // OFFSET = OFF
    double phi = atan2(y, x);
    if (phi < 0.)phi += 2.*M_PI;
    const double PHI = M_PI / 32.0;  // 2pi/64
    segId0 = unsigned(phi / PHI);

    const double dPHI = M_PI / 256.0; // 2pi/64/8
    if (phi - dPHI >= 0.)segId1 = unsigned((phi - dPHI) / PHI);
    else segId1 = unsigned((2.*M_PI + phi - dPHI) / PHI);
    if (segId1 != segId0)return 2;
    if (phi - dPHI < 2.*M_PI)segId1 = unsigned((phi + dPHI) / PHI);
    else segId1 = unsigned((phi + dPHI - 2.*M_PI) / PHI);
    if (segId1 != segId0)return 2;
    segId1 = 9999;
    return 1;
#else
    // OFFSET = ON
    double phi = atan2(y, x);
    if (phi < 0.)phi += 2.*M_PI;
    const double PHI = M_PI / 32.0;  // 2pi/64
    const double offPHI = M_PI / 64.0; // 2pi/128
    phi -= offPHI;
    if (phi >= 0.)segId0 = unsigned(phi / PHI);
    else {
      phi += 2.*M_PI;
      segId0 = unsigned(phi / PHI);
    }

    const double dPHI = M_PI / 256.0; // 2pi/64/8
    if (phi - dPHI >= 0.)segId1 = unsigned((phi - dPHI) / PHI);
    else segId1 = unsigned((2.*M_PI + phi - dPHI) / PHI);
    if (segId1 != segId0)return 2;
    if (phi - dPHI < 2.*M_PI)segId1 = unsigned((phi + dPHI) / PHI);
    else segId1 = unsigned((phi + dPHI - 2.*M_PI) / PHI);
    if (segId1 != segId0)return 2;
    segId1 = 9999;
    return 1;
#endif
  }

  int
  SVDMovedSegId(unsigned refSegId,
                unsigned segId,
                unsigned,
                unsigned type)
  {
    int movedSegId = 9999;
    if (type == 0) { // CDC SuperLayer = 0
      movedSegId = (int)segId - (int)(refSegId * 2);
      if (abs(movedSegId) >= 9)movedSegId = 9999;
    } else if (type == 1) { // CDC SuperLayer = 1
      movedSegId = (int)segId - (int)(refSegId * 2);
      if (abs(movedSegId) >= 9)movedSegId = 9999;
    } else if (type == 2) { // SVD
      if (refSegId % 2 == 0) {
        // std::cout << "seed1 = " << refSegId << std::endl;
        movedSegId = (int)segId - (int)(refSegId);
      } else {
        // std::cout << "seed2 = " << refSegId << std::endl;
        movedSegId = (int)segId - (int)(refSegId - 1);
      }
      if (abs(movedSegId) >= 2)movedSegId = 9999;
    }
    return movedSegId;
  }

  int
  SVDMovedSegId2(unsigned refSegId,
                 unsigned segId,
                 unsigned,
                 unsigned type)
  {
    // wide region version
    // This is used in mapping test.
    int movedSegId = 9999;
    if (type == 0) { // CDC SuperLayer = 0
      movedSegId = (int)segId - (int)(refSegId * 2);
      if (abs(movedSegId) >= 20)movedSegId = 9999;
    } else if (type == 1) { // CDC SuperLayer = 1
      movedSegId = (int)segId - (int)(refSegId * 2);
      if (abs(movedSegId) >= 20)movedSegId = 9999;
    } else if (type == 2) { // SVD
      if (refSegId % 2 == 0) {
        movedSegId = (int)segId - (int)(refSegId);
      } else {
        movedSegId = (int)segId - (int)(refSegId - 1);
      }
      if (abs(movedSegId) >= 20)movedSegId = 9999;
    }
    return movedSegId;
  }


// //
// // cluster2hep
// //
// #if defined(__GNUG__)
// int
// sortBySvdRLA( const Datsvd_hit **a, const Datsvd_hit **b ) {
//   if( (*a)->rla() < (*b)->rla() ){
//     return -1;
//   }else if( (*a)->rla() == (*b)->rla() ){
//     return 0;
//   }else{
//     return 1;
//   }
// }
// #else
// extern "C" int
// sortBySvdRLA( const void *av, const void *bv ) {
//   const Datsvd_hit **a((const Datsvd_hit **)av);
//   const Datsvd_hit **b((const Datsvd_hit **)bv);
//   if( (*a)->rla() < (*b)->rla() ){
//     return -1;
//   }else if( (*a)->rla() == (*b)->rla() ){
//     return 0;
//   }else{
//     return 1;
//   }
// }
// #endif

// Gen_hepevt *
// cluster2hep(const Recsvd_cluster *clus) {
//   HepAList<Datsvd_hit> m_datsvd_hit;
//   Datsvd_hit_Manager& svdHitMgr = Datsvd_hit_Manager::get_manager();
//   for(Datsvd_hit_Manager::iterator it  = svdHitMgr.begin();
//       it != svdHitMgr.end(); ++it){
//     m_datsvd_hit.append(*it);
//   }
//   m_datsvd_hit.sort(sortBySvdRLA); // Ookiijun

//   unsigned size = m_datsvd_hit.length();
//   int startPosition = -1;
//   int direction = 1;

//   for(unsigned i=0;i<size;++i){
//     if(clus->hit().get_ID() == m_datsvd_hit[i]->get_ID()){
//       startPosition = i;
//       if(static_cast<double>(m_datsvd_hit[i]->amp()-1) > clus->lsa())direction = -1;
//       break;
//     }
//   }
//   if(startPosition == -1)return NULL;
//   int width = clus->width();

// #if 0
//   //SVD_Map map;
//   if(direction == -1){
//     std::cout << "DATSVD_HIT size = " << size << std::endl;
//     for(unsigned i=0;i<size;++i){
//       std::cout << i << ": " << clus->hit().get_ID() << " <--> " << m_datsvd_hit[i]->get_ID()
//            << ", RLA = " << m_datsvd_hit[i]->rla() << ", LSA = " << m_datsvd_hit[i]->amp()
//            << ", Width = " << clus->width()
//            << ", Cluster LSA = " << clus->lsa() << std::endl;
//       // std::cout << "MAP R: " << map.RLA2PhiLSA(m_datsvd_hit[i]->rla()) << std::endl;
//       // std::cout << "MAP Z: " << map.RLA2ZLSA(m_datsvd_hit[i]->rla()) << std::endl;
//     }
//     std::cout << "Start # = " << startPosition
//          << ", Width = " << width
//          << ", Direction = " << direction <<std::endl;
//   }
// #endif

//   int* hepID = new int[width];
//   std::set< int, std::less<int> > uniqueHepID;

//   for(int i=startPosition;i<startPosition+width;++i)hepID[i-startPosition] = 0;
//   Datsvd_mcdata_Manager& svdMcHitMgr = Datsvd_mcdata_Manager::get_manager();
//   if(direction == 1){
//     for(int i=startPosition;i<startPosition+width;++i){
//       for(Datsvd_mcdata_Manager::iterator
//             it  = svdMcHitMgr.begin(),
//             end = svdMcHitMgr.end();
//           it != end; ++it){
//         if(it->Hep()){
//           if(m_datsvd_hit[i]->rla() == it->rla() &&
//              it->Hep().get_ID() != 0){
//             hepID[i-startPosition] = (int)(it->Hep().get_ID());
//             uniqueHepID.insert((int)(it->Hep().get_ID()));
//             break;
//           }
//         }
//       }
//     }
//   }else{
//     int reverse = 0;
//     for(int i=startPosition;i<startPosition+width;++i){
//       ++reverse;
//       for(Datsvd_mcdata_Manager::iterator
//             it  = svdMcHitMgr.begin(),
//             end = svdMcHitMgr.end();
//           it != end; ++it){
//         if(it->Hep()){
//           if(m_datsvd_hit[startPosition+1-reverse]->rla() == it->rla() &&
//              it->Hep().get_ID() != 0){
//             hepID[i-startPosition] = (int)(it->Hep().get_ID());
//             uniqueHepID.insert((int)(it->Hep().get_ID()));
//             break;
//           }
//         }
//       }
//     }
//   }

//   unsigned num = uniqueHepID.size();
//   int* counter = new int[num];
//   std::set< int, std::less<int> >::iterator u = uniqueHepID.begin();
//   for(int i=0;i<(int) num;++i) counter[i] = 0;

//   for(int i=0;i<(int) num;++i){
//     for(int j=0;j<width;++j){
//       if(*u == hepID[j]){
//         counter[i] += 1;
//       }
//     }
//     ++u;
//   }

//   Gen_hepevt_Manager& genMgr = Gen_hepevt_Manager::get_manager();
// #if 0
//   u = uniqueHepID.begin();
//   for(int i=0;i<num;++i){
//     std::cout << i << ": TrackID = "<< *u - 1
//          << ", Count = " << counter[i]
//          << ", LundID = " << genMgr[*u-1].idhep() << std::endl;
//     ++u;
//   }
// #endif

//   delete [] hepID;
//   delete [] counter;
//   if(num == 1){
//     // std::cout << "HepID = " << *(uniqueHepID.begin())-1 << std::endl;
//     return &(genMgr[*(uniqueHepID.begin())-1]);
//   }else if(num >= 2){
//     // std::cout << "A svd cluster is not unique." << std::endl;
//     return NULL;
//   }else{
//     return NULL;
//   }
// }


//
// wirhit2hep
//
// Gen_hepevt *
// wirhit2hep(const struct reccdc_wirhit *wirhit) {
//   if(!((*wirhit).m_geo))return NULL;

//   Reccdc_wirhit_Manager   & hitMgr = Reccdc_wirhit_Manager::get_manager();
//   Datcdc_mcwirhit_Manager & mcMgr  = Datcdc_mcwirhit_Manager::get_manager();
//   for(Datcdc_mcwirhit_Manager::iterator j=mcMgr.begin();
//       j!=mcMgr.end();++j){
//     if((*j).geo() && (*j).hep() &&
//        (*j).geo().get_ID() == hitMgr[(wirhit->m_ID)-1].geo().get_ID()){
//       return (Gen_hepevt*)&((*j).hep());
//     }
//   }
//   return NULL;
// }
#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif
