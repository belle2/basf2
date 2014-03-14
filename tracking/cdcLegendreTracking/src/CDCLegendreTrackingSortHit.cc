#include <tracking/cdcLegendreTracking/CDCLegendreTrackingSortHit.h>

#include <boost/tuple/tuple.hpp>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>

using namespace Belle2;
using namespace std;

bool CDCLegendreTrackingSortHit::operator()(CDCLegendreTrackHit* hit1,
                                            CDCLegendreTrackHit* hit2)
{
  bool result = true;
  boost::tuple<int, double, int, double> tuple1(hit1->getStoreIndex(),
                                                hit1->getWirePosition().Mag(), hit1->getWireId());
  boost::tuple<int, double, int, double> tuple2(hit2->getStoreIndex(),
                                                hit2->getWirePosition().Mag(), hit2->getWireId());

  //the comparison function for the tuples created by the sort function

  if ((int) tuple1.get<1>() == (int) tuple2.get<1>()) {
    //special case: several hits in the same layer
    //now we have to proceed differently for positive and negative tracks
    //in a common case we just have to check the wireIds and decide the order according to the charge
    //if however the track is crossing the wireId 0, we have again to treat it specially
    //the number 100 is just a more or less arbitrary number, assuming that no track will be 'crossing' 100 different wireIds

    //in general this solution does not look very elegant, so if you have some suggestions how to improve it, do not hesitate to tell me

    if (m_charge < 0) {
      //negative charge

      //check for special case with wireId 0
      if (tuple1.get<2>() == 0 && tuple2.get<2>() > 100) {
        result = false;
      }
      if (tuple1.get<2>() > 100 && tuple2.get<2>() == 0) {
        result = true;
      }
      //'common' case
      if (tuple1.get<2>() < tuple2.get<2>()) {
        result = true;
      }
      if (tuple1.get<2>() > tuple2.get<2>()) {
        result = false;
      }
    } //end negative charge

    else {
      //positive charge

      //check for special case with wireId 0
      if (tuple1.get<2>() == 0 && tuple2.get<2>() > 100) {
        result = true;
      }
      if (tuple1.get<2>() > 100 && tuple2.get<2>() == 0) {
        result = false;
      }
      //'common' case
      if (tuple1.get<2>() < tuple2.get<2>()) {
        result = false;
      }
      if (tuple1.get<2>() > tuple2.get<2>()) {
        result = true;
      }
    } //end positive charge

  }

  //usual case: hits sorted by the rho value
  else
    result = (tuple1.get<1>() < tuple2.get<1>());

  return result;

}
