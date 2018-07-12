/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <set>

class TTree;
class TClass;

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
     */
    std::set<std::string> filterBranches(const std::set<std::string>& branchesToFilter, const std::vector<std::string>& branches,
                                         const std::vector<std::string>& excludeBranches, int durability);

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
