/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <set>

class TTree;
class TClass;
class TBranch;

namespace Belle2 {
  class FileMetaData;

  /** Some constants and helpers common to the RootInput and RootOutput modules. */
  namespace RootIOUtilities {
    /** Given a list of input branches and lists of branches to include/exclude, returns a list of branches that are accepted.
     *
     * More precisely, an item b from 'branchesToFilter' will be in the returned set if
     * b not in excludeBranches and (b in branches or empty(branches) or b in relationsBetweenAcceptedBranches) and b not in relationsInvolvingExcludedBranches
     *
     * @param branchesToFilter input
     * @param branches if not empty, the list of branches to be accepted
     * @param excludeBranches branches that should never end up in output (takes precedence over everything else)
     * @param durability Durability being filtered (used for messages only)
     * @param quiet If true don't warn about branches which are missing/extra/duplicate in the lists
     */
    std::set<std::string> filterBranches(const std::set<std::string>& branchesToFilter, const std::vector<std::string>& branches,
                                         const std::vector<std::string>& excludeBranches, int durability, bool quiet = false);

    /** Set Branch to be read or not
     *
     * TTree::SetBranchAddress doesn't do what we want it to do because our branch
     * names seem to not be conform to their expectations. But we don't need to
     * use name matching, we know our structure and it's simple enough so we just
     * recursively go through the branch and all children and set processing on or
     * off.
     *
     * @param branch The branch to change the status for
     * @param process Whether or not to read/process this branch
     * @returns the number of branches enabled/disabled
     */
    size_t setBranchStatus(TBranch* branch, bool process);

    /** Performs wildcard expansion using wordexp(), returns matches. */
    std::vector<std::string> expandWordExpansions(const std::vector<std::string>& filenames);

    /** return entry number with given (event, run, experiment) from tree. Returns -1 if not found. */
    long getEntryNumberWithEvtRunExp(TTree* tree, long event, long run, long experiment);

    /** Build TTreeIndex on tree (assumes EventMetaData branch exists there). */
    void buildIndex(TTree* tree);

    /** Returns true if and only if 'cl' or one of its bases has I/O streamers.
     *
     *  TObject is not considered to have any.
     */
    bool hasStreamer(const TClass* cl);

    /** Returns true if and only if 'cl' has a user-defined streamer
     *
     * In that case, TClonesArrays of this type should be written with BypassStreamer(false) and
     * split-level -1 (no splitting).
     */
    bool hasCustomStreamer(const TClass* cl);

    /** Fill the creation info of a file meta data: site, user, data */
    void setCreationData(FileMetaData& metadata);

    /** Return git SHA1 hashes taking into account local & central release.
     *
     * ID is a combined hash $CENTRAL_SHA1[+$LOCAL_SHA1][-modified],
     * or just SHA1[-modified] if only one release or they are on the
     * same revision.
     * Empty string denotes at least one untracked release directory.
     */
    std::string getCommitID();

    /** Names of trees. */
    extern const std::string c_treeNames[];

    /** Steering parameter names for m_branchNames. */
    extern const std::string c_SteerBranchNames[];

    /** Steering parameter names for m_excludeBranchNames. */
    extern const std::string c_SteerExcludeBranchNames[];

    /** Steering parameter names for m_additionalBranchNames. */
    extern const std::string c_SteerAdditionalBranchNames[];
  }
}
