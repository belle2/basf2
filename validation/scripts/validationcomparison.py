#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import ROOT
from enum import Enum
import numpy


class ComparisonFailed(Exception):
    """
    The comparison failed for some reason. For example
    because ROOT was not able to compute the Chi^2 properly
    """
    pass


class ObjectsNotSupported(Exception):
    """
    The type and/or combination of provided ROOT objects
    is not supported for comparison
    """
    pass


class DifferingBinCount(Exception):
    """
    The two ROOT objects provided have a different bin count
    and therefor, cannot be compared using the Chi2 test
    """
    pass


class TooFewBins(Exception):
    """
    Not sufficient bins to perform the Chi^2 test
    """
    pass


class ComparisonBase:
    """
    Base class for all comparison implementations
    """
    pass


class Chi2Test(ComparisonBase):

    """
    Perform a Chi2Test for ROOT objects
    """

    def __init__(self, objectA, objectB):
        """
        Store the two histograms/profiles operated on
        """

        #: store the first object to compare
        self.objectA = objectA

        #: store the second object to compare
        self.objectB = objectB

        #: used to store, whether the quantities have already been compared
        self.computed = False

    def can_compare(self):
        """
        @return: True if the two objects can be compared, False otherwise
        """
        if not self.correct_types():
            return False

        return self.has_compatible_bins()

    def correct_types(self):
        """
        @return: True if the two objects have a) a type supported for comparison and b)
                 can be compared with each other
        """

        if self.objectA is None or self.objectA is None:
            return False

        supported_types = ["TProfile", "TH1D", "TH1F", "TEfficiency"]
        # check if the supplied object inherit from one of the supported types
        # and if they are of the same type
        if len([type for type in supported_types if self.objectA.ClassName() == type and self.objectB.ClassName() == type]) == 0:
            return False

        if self.objectA.ClassName() == "TEfficiency":
            # can only handle TEfficiencies with dimension one atm
            if self.objectA.GetDimension() > 1:
                return False

        return True

    def pvalue(self):
        """
        @return the probaility value of the comparison
        """
        self.ensure_compute()
        return self.__pvalue

    def chi2(self):
        """
        @return the chi2 value of the comparison
        """
        self.ensure_compute()
        return self.__chi2

    def chi2ndf(self):
        """
        @return the chi2 divided by the number of degrees of freedom
        """
        self.ensure_compute()
        return self.__chi2ndf

    def ndf(self):
        """
        @return the number of degrees of freedom
        """
        self.ensure_compute()
        return self.__ndf

    def ensure_compute(self):
        """
        Ensure all required quantities get computed and are cached inside the class
        """
        if self.computed:
            return

        #: compute and store quantities
        self.__pvalue, self.__chi2, self.__chi2ndf, self.__ndf = self.__internal_compare()
        self.computed = True

    def ensure_zero_error_has_no_content(self, a, b):
        """
        Ensure there are no bins which have a content set, but 0 error
        This bin content will be set to 0 to disable this bin completely during
        the comparison
        """
        nbins = a.GetNbinsX()
        for ibin in range(1, nbins + 1):
            if (not a.GetBinError(ibin) > 0.0) and (not b.GetBinError(ibin) > 0.0):
                # set the bin content of the profile plots to zero so ROOT will ignore this
                # bin in its comparison
                a.SetBinContent(ibin, 0.0)
                b.SetBinContent(ibin, 0.0)

    def has_compatible_bins(self):
        """
        Check if both ROOT obeject have the same amount of bins
        @return: True if the bins are equal, otherwise False
        """
        if (self.objectA.ClassName() == "TEfficiency") and (self.objectB.ClassName() == "TEfficiency"):
            return self.objectA.GetTotalHistogram().GetNbinsX() == self.objectB.GetTotalHistogram().GetNbinsX()

        return self.objectA.GetNbinsX() == self.objectB.GetNbinsX()

    def __convert_teff_to_hist(self, teffA):
        """
        Convert the content of a TEfficiency plot to a histogram and set
        the bin content and errors
        """
        conv_hist = teffA.GetTotalHistogram()
        xbinCount = conv_hist.GetNbinsX()
        xbin_low = conv_hist.GetXaxis().GetXmin()
        xbin_max = conv_hist.GetXaxis().GetXmax()

        th1 = ROOT.TH1D(teffA.GetName() + "root_conversion", teffA.GetName(), xbinCount, xbin_low, xbin_max)
        # starting from the first to the last bin, ignoring the under/overflow bins
        for i in range(1, xbinCount):
            th1.SetBinContent(i, teffA.GetEfficiency(i))
            th1.SetBinError(i, teffA.GetEfficiencyErrorLow(i))

        return th1

    def __internal_compare(self):
        """
        Performs the actual Chi^2 test
        @return: The request result quantity
        """
        if not self.correct_types():
            raise ObjectsNotSupported(
                """Comparison of {} (Type {}) with {} (Type {}) not supported.
Please contact Thomas.Hauth@kit.edu if you need this supported.""".format(
                    self.objectA.GetName(),
                    self.objectA.ClassName(),
                    self.objectB.GetName(),
                    self.objectB.ClassName()))
        if not self.has_compatible_bins():
            raise DifferingBinCount("The objects have differing x bin count: {} has {} vs. {} has {}".format(
                self.objectA.GetName(), self.objectA.GetNbinsX(), self.objectB.GetName(), self.objectB.GetNbinsX()))

        local_objectA = self.objectA
        local_objectB = self.objectB

        # very special handling for TEfficiencies
        if self.objectA.ClassName() == "TEfficiency":
            local_objectA = self.__convert_teff_to_hist(self.objectA)
            local_objectB = self.__convert_teff_to_hist(self.objectB)

        nbins = local_objectA.GetNbinsX()

        if nbins < 2:
            raise TooFewBins("{} bin is to few to perform the Chi2 test.".format(nbins))

        compareWeightOne = (
            local_objectA.ClassName() == "TProfile") or (
            local_objectA.ClassName() == "TH1D") or (
            local_objectA.ClassName() == "TH1F")
        compareWeightTwo = (
            local_objectB.ClassName() == "TProfile") or (
            local_objectB.ClassName() == "TH1D") or (
            local_objectB.ClassName() == "TH1F")

        # clone, because possibly some content of profiles will
        # be set to zero
        firstObj = local_objectA.Clone()
        secondObj = local_objectB.Clone()

        if compareWeightOne is True and compareWeightTwo is False:
            # switch histograms, because ROOT can only have the first one unweighted
            firstObj, secondObj = secondObj, firstObj

        # for debugging output
        compareOptions = "P "

        # construct the option string for the Chi2Test call
        if compareWeightOne and compareWeightTwo:
            compareOptions += "WW"
        elif compareWeightOne or compareWeightTwo:
            compareOptions += "UW"
        else:
            compareOptions += "UU"

        if compareWeightOne and compareWeightTwo:
            self.ensure_zero_error_has_no_content(firstObj, secondObj)

        # use numpy arrays to support ROOT's pass-by-reference interface here
        res_chi2 = numpy.array(1, numpy.float64)
        res_igood = numpy.array(1, numpy.int32)
        res_ndf = numpy.array(1, numpy.int32)

        res_pvalue = firstObj.Chi2TestX(secondObj, res_chi2, res_ndf, res_igood, compareOptions)

        if res_ndf < 1:
            raise ComparisonFailed("Comparison failed, no Chi^2 could be computed. Please send the ROOT files " +
                                   "involved to Thomas.Hauth@kit.edu.")

        res_chi2ndf = res_chi2 / res_ndf

        return (res_pvalue, res_chi2, res_chi2ndf, res_ndf)
