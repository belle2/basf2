#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import ROOT
from enum import Enum


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

    class ResultQuantity(Enum):
        """
        Used to specific the quantity to extract during the Chi2Test
        """

        # Chi^2
        chi2 = "CHI2"

        # Chi^2 / number of degrees of freedom
        chi2ndf = "CHI2/NDF"

        # the probability value
        pvalue = ""

    def __init__(self, objectA, objectB):
        """
        Store the two histograms/profiles operated on
        """

        # store the first object to compare
        self.objectA = objectA

        # store the second object to compare
        self.objectB = objectB

        # used to store, whether the quantities have already been compared
        self.computed = False

    def can_compare(self):
        """
        @return: True if the two objects can be compared, False otherwise
        """
        if not self.has_compatible_bins():
            return False

        return self.correct_types()

    def correct_types(self):
        """
        @return: True if the two objects have a) a type supported for comparison and b)
                 can be compared with each other
        """

        if self.objectA is None or self.objectA is None:
            return False

        supported_types = ["TProfile", "TH1D", "TH1F"]
        # check if the supplied object inherit from one of the supported types
        # and if they are of the same type
        if len([type for type in supported_types if self.objectA.ClassName() == type and self.objectB.ClassName() == type]) == 0:
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
        @return the chi2 divided by the number of dregrees of freedom
        """
        self.ensure_compute()
        return self.__chi2ndf

    def ensure_compute(self):
        """
        Ensure all required quantities get computed and are cached inside the class
        """

        # todo: improve the diagnostics of this error message
        # can only be properly set, once the igood value of Chi2Test
        # can be accessed via PyROOT and python3
        # fail_message = "No Chi^2 could be computed by ROOT."
        if self.computed:
            #    if not self.__chi2 > 0.0:
            #        raise ComparisonFailed(fail_message)
            return

        # compute and store pvalue
        self.__pvalue = self.__internal_compare(self.ResultQuantity.pvalue)
        # compute and store chi^2
        self.__chi2 = self.__internal_compare(self.ResultQuantity.chi2)
        # compute and store chi^2 / ndf
        self.__chi2ndf = self.__internal_compare(self.ResultQuantity.chi2ndf)

        self.computed = True

        # see comment above
        # if not self.__chi2 > 0.0:
        #    raise ComparisonFailed(fail_message)

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
        return self.objectA.GetNbinsX() == self.objectB.GetNbinsX()

    def __internal_compare(self, quantity):
        """
        Performs the actual Chi^2 test
        @param quantity: the quantity which should be returned, of type ResultQuantity
        @return: The request result quantity
        """
        if not self.correct_types():
            raise ObjectsNotSupported(
                "Comparison of {} (Type {}) with {} (Type {}) not supported." +
                "Please contact Thomas.Hauth@kit.edu if you need this supported.".format(
                    self.objectA.GetName(),
                    self.objectA.ClassName(),
                    self.objectB.GetName(),
                    self.objectB.ClassName()))
        if not self.has_compatible_bins():
            raise DifferingBinCount("The objects have differing x bin count: {} has {} vs. {} has {}".format(
                self.objectA.GetName(), self.objectA.GetNbinsX(), self.objectB.GetName(), self.objectB.GetNbinsX()))

        nbins = self.objectA.GetNbinsX()

        if nbins < 2:
            raise TooFewBins("{} bin is to few to perform the Chi2 test.".format(nbins))

        compareWeightOne = (
            self.objectA.ClassName() == "TProfile") or (
            self.objectA.ClassName() == "TH1D") or (
            self.objectA.ClassName() == "TH1F")
        compareWeightTwo = (
            self.objectA.ClassName() == "TProfile") or (
            self.objectA.ClassName() == "TH1D") or (
            self.objectA.ClassName() == "TH1F")

        # clone, because possibly some content of profiles will
        # be set to zero
        firstObj = self.objectA.Clone()
        secondObj = self.objectB.Clone()

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

        if quantity == self.ResultQuantity.chi2:
            compareOptions += " " + self.ResultQuantity.chi2.value
        elif quantity == self.ResultQuantity.chi2ndf:
            compareOptions += " " + self.ResultQuantity.chi2ndf.value

        if compareWeightOne and compareWeightTwo:
            self.ensure_zero_error_has_no_content(firstObj, secondObj)

        # for python3, Chi2TestX is not working atm:
        # https://root.cern.ch/phpBB3/viewtopic.php?t=19751
        result = firstObj.Chi2Test(secondObj, compareOptions)

        return result
