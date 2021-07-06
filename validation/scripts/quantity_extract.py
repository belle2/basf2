#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


"""
Functionality to extract quantities from various ROOT objects (TH1).
"""


def default_extractor():
    """
    Returns a list of default extractors for the most common
    ROOT Objects
    """

    def computeMean(profile_obj):
        """
        compute the mean values of the y dimension, also with
        zero supression
        """
        nbinsx = profile_obj.GetNbinsX()
        sum = 0.0
        sumZeroSuppressed = 0.0
        countZeroSuppressed = 0
        for i in range(nbinsx):
            v = profile_obj.GetBinContent(i + 1)
            sum = (
                sum + v
            )  # from first bin, ignored underflow (i=0) and overflow (i=nbinsx+1) bins
            if v > 0.0:
                sumZeroSuppressed = sumZeroSuppressed + v
                countZeroSuppressed = countZeroSuppressed + 1
        meanY = sum / nbinsx
        meanYzeroSuppressed = sum / countZeroSuppressed

        return (meanY, meanYzeroSuppressed)

    def extractNtupleValues(ntuple_obj):
        # only get the first entry for now
        results = []
        if ntuple_obj.GetEntries() > 0:
            # ent0 = ntuple_obj.GetEntry(0)
            for branch in ntuple_obj.GetListOfBranches():
                branch_name = branch.GetName()
                # create tuple with the branch name and value
                results.append(
                    (
                        ntuple_obj.GetName() + "_" + branch_name,
                        float(getattr(ntuple_obj, branch_name)),
                    )
                )
        return results

    th1_like = [
        lambda x: [("mean_x", x.GetMean(1))],
        lambda x: [("entries", x.GetEntries())],
        lambda x: [("mean_y", computeMean(x)[0])],
        lambda x: [("mean_y_zero_suppressed", computeMean(x)[1])],
    ]

    tprofile = [
        lambda x: [("mean_y", computeMean(x)[0])],
        lambda x: [("mean_y_zero_suppressed", computeMean(x)[1])],
    ]

    tntuple = [extractNtupleValues]

    return {
        "TH1D": th1_like,
        "TH1F": th1_like,
        "TH1": th1_like,
        "TProfile": tprofile,
        "TNtuple": tntuple,
    }


class RootQuantityExtract:
    """
    Class to automatically quntaties from ROOT Objects
    """

    def __init__(self, extractor=None):
        """
        Initialize the class with a set of quantity extractors
        """

        #: the dictionary used for data extraction, key is the ROOT type
        #  and value a list of extractors
        self.extractor = extractor

        if self.extractor is None:
            # use default
            self.extractor = default_extractor()

    def addExtractor(self, class_type, extractor):
        """
        Adds an extractor to this class

        @param class_type: the string you get when
        running <root_object>.IsA().GetName()
        This is used to map the correct extractor to
        this root object
        """

        # check if extractors for this type are already
        # registered
        if class_type in self.extractor:
            self.extractor[class_type].append(extractor)
        else:
            self.extractor[class_type] = [extractor]

    def extract(self, obj):
        """
        Extract quantities from a root object

        @return: a dictionary of extracted quantities
        """
        this_class_type = obj.IsA().GetName()

        if this_class_type in self.extractor:
            ext_list = self.extractor[this_class_type]

            # run list of extractors
            # convert the returned tuple list to a dictionary
            result = []
            for x in ext_list:
                # run the extractor
                result_list = x(obj)
                result += result_list
            # convert the list of tuples into a dictionary
            return dict(result)
        else:
            # no extractors available for this type
            return {}
