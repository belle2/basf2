/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGECLTCMAPPING_FLAG_
#define TRGECLTCMAPPING_FLAG_

#include <vector>
#include <Math/Vector3D.h>
//
//
//
namespace Belle2 {
//
//
  /*! A class of TC Mapping.*/
//
  class TrgEclMapping {

  public:

    /** Constructor */
    TrgEclMapping();

    /** Destructor */
    virtual ~TrgEclMapping() {};

  public:

    /** get [TC ID] from [Xtal ID] */
    int getTCIdFromXtalId(int);
    /** get [Xtal ID] from [TC ID] (Output as a vector)*/
    std::vector<int> getXtalIdFromTCId(int);
    /** get [TC sub ID] from [Xtal ID]*/
    int getTCSubIdFromXtalId(int);
    /** get [TC Theta ID] from [TC ID] */
    int getTCThetaIdFromTCId(int);
    /** get [TC Phi ID] from [TC ID] */
    int getTCPhiIdFromTCId(int);
    /**TC position (cm)*/
    ROOT::Math::XYZVector getTCPosition(int);
    /** Get TC from FAM # and Channel #*/
    int getTCIdFromFAMChannel(int, int);
    /** Get FAM # and Channel # from TC Id*/
    std::vector<int> getFAMChannelFromTCId(int);
    //! get TCId from phi and theta position(LSB = 1.4)
    int getTCIdFromPosition(int, int);
  private:


    /** TC Id */
    int _tcid;       // ID = 1-576
    /** TC Sub Id*/
    int _tcsubid;    // ID = 0-1
    /** TC Theta Id*/
    int _tcthetaid;  // ID = 1-17
    /** TC Phi Id*/
    int _tcphiid;    // ID = 1-36(1-32 for both endcap)

    /** Matrix of the relation of TC and Xtal*/
    std::vector<std::vector<int>> TC2Xtal;
  };
//
//
//
} // namespace Belle2

#endif /* TRGECLTCMAPPING_FLAG_ */
