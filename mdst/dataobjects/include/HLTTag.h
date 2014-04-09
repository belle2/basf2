#ifndef HLTTAG_H
#define HLTTAG_H
//+
// File : HLTTag.h
// Description : Format of HLT tag object
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 31 - Dec - 2013
//-

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  /**
   * The HLTTag class
   * Contains the information of Highlevel trigger
   */
  class HLTTag : public TObject {

  public:
    /// Trigger algorism
    enum HLTTrigAlgo { Global, Level3,
                       Hadronic, Tautau, LowMulti,
                       Bhabha, Mumu,
                       Cosmic, Random, Injection,
                       Calib1, Calib2, Calib3, Calib4, Calib5,
                       BeamTest, MAXALGO
                     };

  public:
    //! Default constructor
    HLTTag();
    //! Destructor
    virtual ~HLTTag();

    // Access Functions
    //! HLT's Event ID
    int HLTEventID();
    //! Set HLT's Event ID
    void HLTEventID(int id);

    //! HLT's unit ID
    int HLTUnitID();
    //! Set HLT's unit ID
    void HLTUnitID(int id);

    //! Processed time at HLT
    time_t HLTTime();
    //! Set time at HLT
    void HLTTime(time_t tm);
    //! Set current time
    void HLTSetCurrentTime();

    //! Access functions to Summary word
    bool Accepted(HLTTrigAlgo algo = Global);
    //! Access functions to Summary word
    bool Discarded(HLTTrigAlgo algo = Global);

    //! Access functions to Summary word
    void Accept(HLTTrigAlgo algo = Global);
    //! Access functions to Summary word
    void Discard(HLTTrigAlgo algo = Global);

    //! Get Summary word
    int GetSummaryWord();
    //! Set Summary word
    void SetSummaryWord(int summary);

    //! Access functions to each algorithm word
    int GetAlgoInfo(HLTTrigAlgo algo);
    //! Access functions to each algorithm word
    void SetAlgoInfo(HLTTrigAlgo algo, int infobits);

  private:
    /// Event number put by HLT
    int m_HLTEventID;

    /// HLT unit ID
    int m_HLTUnitID;

    /// Time
    time_t m_HLTtime;

    // HLT Summary
    //  - Bit assignment
    // Bit 0:  Summary bit : taken = 1, discarded = 0
    // Bit 1:  algorithm 1 : taken = 1 / discarded = 0
    // Bit 2:  algorithm 2 : taken = 1 / discarded = 0
    // Bit 3:  algorithm 3 : taken = 1 / discarded = 0
    // Bit 4:  algorithm 4 : taken = 1 / discarded = 0
    //        .....
    // Bit MAXALGO: argorithm MAXALGO : taken = 1, discard = 0
    /// .......
    int m_HLTSummary;

    // Detailed info on selection
    //  * 32bit info for each of selection algorithm
    ///  * Usage dependent on each algorithm
    int m_HLTAlgoInfo[MAXALGO];

    /// To derive from TObject
    ClassDef(HLTTag, 1);
  };
}

#endif
