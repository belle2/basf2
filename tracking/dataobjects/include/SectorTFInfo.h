/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <TObject.h>
#include <TVector3.h>
#include <string>
#include "BaseTFInfo.h"

namespace Belle2 {
  /** Sector TF Info Class
   *
   *  This class is needed for information transport between the VXD Track Finder and the display.
   *
   *  Members:
   *    m_points (TVector3[4]) = 4 Points of the Sector
   *    m_sectorID (int) = real Sector ID
   *  is_only_friend (bool) = true if = sector is only loaded as friend-sector
   *    m_friends (< int >) =  all IDs of the Sector Friends
   *  m_use_counter (int) = Countes the Hits using this cell (alive and connected)
   *  m_max_counter (int) = max. m_use_counter
   *
   *  Important Methodes:
   *  isOverlapped(): returns true if = used more then one time by Hits = Sector is overlapped = m_use_counter > 1
   *
   */



  class SectorTFInfo: public BaseTFInfo {
  public:

    /** Default constructor for the ROOT IO. */
    SectorTFInfo() {
      m_points[0].SetXYZ(0., 0., 0.);
      m_points[1].SetXYZ(0., 0., 0.);
      m_points[2].SetXYZ(0., 0., 0.);
      m_points[3].SetXYZ(0., 0., 0.);
      m_sectorID = -1;
      m_friends.clear();
      m_use_counter = 0;
      m_max_counter = 0;
      is_only_friend = false;
    }


    /** Standard constructor */
    SectorTFInfo(int par_pass_index, int par_sectorid): BaseTFInfo(par_pass_index) {
      m_sectorID = par_sectorid;
      m_points[0].SetXYZ(0., 0., 0.);
      m_points[1].SetXYZ(0., 0., 0.);
      m_points[2].SetXYZ(0., 0., 0.);
      m_points[3].SetXYZ(0., 0., 0.);
      m_sectorID = -1;
      m_use_counter = 0;
      m_max_counter = 0;
      is_only_friend = false;
      m_friends.clear();
    }

    /** getter - getPoint Point int (from POINT 0 to POINT 3 */
    TVector3 getPoint(int value_point)  { return m_points[value_point]; }

    /** setter - Point */
    void setPoint(int value_point, TVector3 value) { m_points[value_point] = value; }

    /** getter - Sector ID */
    int getSectorID()  { return m_sectorID; }

    /** setter - Sector ID */
    void setSectorID(int value) { m_sectorID = value; }

    /** getter - getFriends*/
    std::vector<int>& getFriends()  { return m_friends; }

    /** add new int to Friends */
    void push_back_Friends(int newMember) {
      m_friends.push_back(newMember);
    }

    /** set all Friends */
    void setAllFriends(std::vector<int> par_friends) {
      m_friends = par_friends;
    }

    /** returns size of Friends */
    int sizeFriends() { return m_friends.size(); }

    /** getter - IsOnlyFriend */
    bool getIsOnlyFriend()  { return is_only_friend; }

    /** setter - IsOnlyFriend */
    void setIsOnlyFriend(bool value) { is_only_friend = value; }

    /** getter - UseCounter */
    int getUseCounter()  { return m_use_counter; }

    /** getMaxCounter */
    int getMaxCounter()  { return m_max_counter; }

    /** setter - UseCounter */
    void setUseCounter(int value) {
      m_use_counter = value;
      if (m_use_counter > m_max_counter) {
        m_max_counter = m_use_counter;
      }
    }

    /** UseCounte add / minus */
    void changeUseCounter(int value) {
      m_use_counter = m_use_counter + value;
      if (m_use_counter > m_max_counter) {
        m_max_counter = m_use_counter;
      }
    }


    // Sector is overlapped if it is used more then one time
    /** if the sector is overlaped */
    bool isOverlapped()  {
      if (m_use_counter > 1) {
        return true;
      } else {
        return false;
      }
    }

    /** returns the String for the display - Information */
    TString getDisplayInformation() {

      // NOT FINAL !!!

      return TString::Format("Point 1: (%.3f, %.3f, %.3f)\n Point 2: (%.3f, %.3f, %.3f)\n Point 3: (%.3f, %.3f, %.3f)\n Point 4: (%.3f, %.3f, %.3f)\n Friend Only: %s\n Died_ID: %d ", m_points[0].X(), m_points[0].Y(), m_points[0].Z(), m_points[1].X(), m_points[1].Y(), m_points[1].Z(), m_points[2].X(), m_points[2].Y(), m_points[2].Z(), m_points[3].X(), m_points[3].Y(), m_points[3].Z(), is_only_friend ? "true" : "false", getDiedID());
    }

    /** returns the String for the display - AlternativeBox */
    TString getDisplayAlternativeBox() {

      std::string died_at = getDiedAt();

      return TString::Format("SectorID: %d, PassIndex: %d\n Died_ID: %s, Count Friend Sectors: %d", m_sectorID, getPassIndex(), died_at.c_str(), sizeFriends());
    }


  protected:

    TVector3 m_points[4]; /**< m_points (TVector3[4]) = 4 Points of the Sector **/

    int m_sectorID;   /**< real Sector ID **/
    bool is_only_friend;  /**< true if = sector is only loaded as friend-sector **/

    std::vector<int> m_friends;  /**< all IDs of the Sector Friends **/

    int m_use_counter;  /**< Countes the Hits (alive and connected) */
    int m_max_counter;  /**<  Max. Counts of Hits (max. m_use_counter)  */

    ClassDef(SectorTFInfo, 1)
  };

}
