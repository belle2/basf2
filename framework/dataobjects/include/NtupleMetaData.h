/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <framework/dataobjects/FileMetaData.h>

#include <iosfwd>
#include <vector>
#include <string>
#include <map>

namespace Belle2 {

  /** Metadata information about a ntuple
   *
   *  See BELLE2-NOTE-TE-2015-028: Event, File, and Dataset Metadata for a
   *  detailed definition. Available at: https://docs.belle2.org/record/287?ln=en
   */
  class NtupleMetaData : public FileMetaData {
  public:
    /** Constructor.
     */
    NtupleMetaData();

    /** Get number of input files.
     */
    int getNInputs() const {return m_inputLfns.size();}

    /** Get LFN of input file.
     *
     *  @note Not range-checked!
     *  @param iInput The number of the input file.
     */
    const std::string& getInput(int iInput) const {return m_inputLfns[iInput];}

    /** Inputs setter.
     *
     *  @param parents The vector of parent IDs.
     */
    void setInputs(const std::vector<std::string>& inputs) {m_inputLfns = inputs;}

    /** Print the content of the meta data object.
     *
     *  @param option Use "all" to print everything, except steering file. Use "steering" for printing (only) steering file. "json" for machine-readable output
     */
    virtual void Print(Option_t* option = "") const override;

    /** Write ntuple meta data in xml format to the output stream.
     *
     *  @param output The output stream.
     *  @param physicalFileName The physical file name.
     *  @return True if metadata could be written
     */
    bool write(std::ostream& output, const std::string& physicalFileName) const;

    /** Get a json representation.
     */
    std::string getJsonStr() const;

    /** Fill the creation info of ntuple meta data: site, user, data */
    void setCreationData();

  private:

    std::vector<std::string> m_inputLfns; /**< LFNs of input files.  */

    friend class VariablesToNtupleModule;

  }; //class

} // namespace Belle2
