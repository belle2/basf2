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
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {
  /** Base TF Info Class
   *
   *  This class is needed for information transport between the VXD Track Finder and the display.
   *
   *  Members:
   *    pass_index (int) = Index of the Pass (to access object, unique for each pass)
   *    died_at (string) = Information (Filter, ...) where the object died at
   *  accepted (< int >) =  all Filter IDs accepted the object
   *    rejected (< int >) =  all Filter IDs rejected the object
   *  m_died_id (int) = ID of Died at Part
   *
   *  Important Methodes:
   *  getActive(): returns true if = object is still active = no Information at died_at
   */

  class BaseTFInfo: public RelationsObject {
  public:

    /** Default constructor for the ROOT IO. */
    BaseTFInfo() {
      m_died_at = "";
      m_accepted.clear();
      m_rejected.clear();
      m_pass_index = -1;
      m_died_id = -1;
    };

    // Standard constructor
    BaseTFInfo(int par_pass_index) {
      m_died_at = "";
      m_accepted.clear();
      m_rejected.clear();
      m_pass_index = par_pass_index;
      m_died_id = -1;
    };

    /** getter - getActive Active*/
    bool getActive() {
      if (m_died_at.size() == 0) {
        return true;
      } else {
        return false;
      }
    }

    /** getter - getDiedAt Died At */
    std::string getDiedAt()  { return m_died_at; }

    /** setter - Died At */
    // add all Strings for diedAt
    void setDiedAt(std::string value) {

      if (value.size() > 0) {

        // If same Died at is not already writen
        if (m_died_at.find(value) == std::string::npos) {

          if (m_died_at.size() > 0) {
            m_died_at.append("-");
          }

          m_died_at.append(value);

        }
      }
    }

    /** getter - getAccepted */
    std::vector<int>& getAccepted()  { return m_accepted; }

    /** getter - getRejected */
    std::vector<int>& getRejected()  { return m_rejected; }

    /** add new int to Accepted */
    void insert_Accepted(std::vector<int> newMember) {
      if (int(newMember.size()) != 0) {
        m_accepted.insert(m_accepted.end(), newMember.begin(), newMember.end());
      }
    }

    /** add new int to Rejected */
    void insert_Rejected(std::vector<int> newMember) {
      if (int(newMember.size()) != 0) {
        m_rejected.insert(m_rejected.end(), newMember.begin(), newMember.end());
      }

    }

    /** returns size of Accepted */
    int sizeAccepted() { return m_accepted.size(); }

    /** returns size of Rejected */
    int sizeRejected() { return m_rejected.size(); }


    /** clear all maps */
    void clear() {
      m_accepted.clear();
      m_rejected.clear();
    }

    /** getter - getState Pass Index */
    int getPassIndex()  { return m_pass_index; }

    /** setter - Pass Index */
    void setPassIndex(int value) { m_pass_index = value; }

    /** getter - getState Pass Index */
    int getDiedID()  { return m_died_id; }

    /** setter - Pass Index */
    void setDiedID(int value) { m_died_id = value; }


  protected:

    int m_pass_index; /**< Pass Index */
    std::string m_died_at; /**< Died at */
    int m_died_id; /**< ID of Died at Part */

    std::vector<int> m_accepted;  /** filters accepted it */
    std::vector<int> m_rejected;  /**< filters rejected it */

    ClassDef(BaseTFInfo, 1)
  };
}
