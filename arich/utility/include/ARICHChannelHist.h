/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <string>
#include <TH2Poly.h>
#include <TGraph.h>
#include <TLine.h>
#include <TText.h>

namespace Belle2 {

  /**
   * ARICH histogram with HAPD plane
   * 3 options for bin segmentation are available
   * type 0 - one bin per HAPD, type 1 - one bin per channel, type 2 - one bin per APD chip
   * HAPD numbering is 1-420 (software numbering), channel is ASIC channel, APD is ASIC channel / 36
   */
  class ARICHChannelHist: public TH2Poly {

  public:

    /**
     * Default constructor
     */
    ARICHChannelHist() {};

    /**
     * Constructor with name, title, and type (0 for channel bins (144/HAPD), 1 for HAPD bins (1/HAPD), 2 for APD bins (4/HAPD))
     * @param name name
     * @param title title
     * @param type type
     */
    ARICHChannelHist(const char* name, const char* title, int type = 0,
                     const std::vector<unsigned>& moduleIDs = std::vector<unsigned>());

    /**
     * Add entry to bin corresponding to hapd hapdID and channel chID
     * @param hapdID ID number of HAPD module
     * @param chID channel ID (asic channel) or APD ID
     */
    void fillBin(unsigned hapdID, unsigned chID, double weight = 1.);

    /**
     * Set content of bin corresponding to hapd hapdID and channel chID
     * @param hapdID ID number of HAPD module
     * @param chID channel ID (asic channel) or APD ID
     * @param value bin content
     */
    void setBinContent(unsigned hapdID, unsigned chID, double value);

    /**
     * Add entry to bin corresponding to hapd hapdID
     * @param hapdID ID number of HAPD module
     */
    void fillBin(unsigned hapdID, double weight = 1.);

    /**
     * Set content of bin corresponding to hapd hapdID
     * @param hapdID ID number of HAPD module
     * @param value bin content
     */
    void setBinContent(unsigned hapdID, double value);

    /**
     * Draw the histogram
     * @param option root draw option
     */
    void Draw(Option_t* option = "") override;

    /**
     * Fill the channelHist from the histogram
     * Type 0 channelHist has to be filled with 420*144bin TH1 (each channel)
     * Type 1 channelHist can be filled with 420*144 (channels) or 420*4 (chips), or 420 (HAPDs) bin TH1
     * Type 2 channelHist can be filled with 420*144 or 420*4 bin TH1
     * In case histogram has more bins than channelHist bin content is summed up
     * Exampl: histogram with hits/channel can be used to make ChannelHist with hits/hapd
     * @param hist historam
     */
    void fillFromTH1(TH1* hist);


  protected:

    TLine lines[6];
    TText labels[6];
    int m_type = 0; /**< histogram type */
    std::vector<unsigned> m_hapd2binMap; /**< map of bins*/
    ClassDefOverride(ARICHChannelHist, 2); /**< ClassDef */

  };

} // end namespace Belle2
