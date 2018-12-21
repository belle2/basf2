#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Compare ROOT objects and perform e.g. chi2 tests.
A small command line interface for testing/debugging purposes is included.
Run `python3 validationcomparison.py --help` for more information. """

# std
import argparse
import os.path
import numpy

# external
import ROOT


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


# todo: perhaps give it
class Chi2Test(ComparisonBase):

    """
    Perform a Chi2Test for ROOT objects. The chi2 test method is e.g. described
    in the documentation of TH1::Chi2Test. Basically this class wraps around
    this Chi2Test function, and takes care that we can call perform these
    tests for a wider selection of ROOT objects.
    """

    def __init__(self, object_a, object_b, debug=False):
        """
        Constructor. Store the two histograms/profiles operated on.
        :param object_a: First object
        :param object_b: Second object
        :param debug: Print debug information?
        """

        #: store the first object to compare
        self.object_a = object_a

        #: store the second object to compare
        self.object_b = object_b

        #: used to store, whether the quantities have already been compared
        self.computed = False

        #: enable debug?
        self.debug = debug

        # Those will only be accessed via methods.
        self._pvalue = None
        self._chi2 = None
        self._chi2ndf = None
        self._ndf = None

    def can_compare(self):
        """
        @return: True if the two objects can be compared, False otherwise
        """
        return self.correct_types() and self.has_compatible_bins()

    def correct_types(self):
        """
        @return: True if the two objects have a) a type supported for comparison
            and b) can be compared with each other
        """
        if self.object_a is None or self.object_b is None:
            return False

        # check if the supplied object inherit from one of the supported types
        # and if they are of the same type
        supported_types = ["TProfile", "TH1D", "TH1F", "TEfficiency"]
        if self.object_a.ClassName() != self.object_b.ClassName():
            return False
        if self.object_a.ClassName() not in supported_types:
            return False

        if self.object_a.ClassName() == "TEfficiency":
            # can only handle TEfficiencies with dimension one atm
            if self.object_a.GetDimension() > 1:
                return False

        return True

    def pvalue(self):
        """
        @return the probaility value of the comparison
        """
        self.ensure_compute()
        return self._pvalue

    def chi2(self):
        """
        @return the chi2 value of the comparison
        """
        self.ensure_compute()
        return self._chi2

    def chi2ndf(self):
        """
        @return the chi2 divided by the number of degrees of freedom
        """
        self.ensure_compute()
        return self._chi2ndf

    def ndf(self):
        """
        @return the number of degrees of freedom
        """
        self.ensure_compute()
        return self._ndf

    def ensure_compute(self):
        """
        Ensure all required quantities get computed and are cached inside the
        class
        """
        if self.computed:
            return

        #: compute and store quantities
        self._pvalue, self._chi2, self._chi2ndf, self._ndf = \
            self.__internal_compare()
        self.computed = True

    def ensure_zero_error_has_no_content(self, a, b):
        """
        Ensure there are no bins which have a content set, but 0 error
        This bin content will be set to 0 to disable this bin completely during
        the comparison
        """
        nbins = a.GetNbinsX()
        for ibin in range(1, nbins + 1):
            if a.GetBinError(ibin) <= 0.0 and b.GetBinError(ibin) <= 0.0:
                # set the bin content of the profile plots to zero so ROOT
                # will ignore this bin in its comparison
                a.SetBinContent(ibin, 0.0)
                b.SetBinContent(ibin, 0.0)
                if self.debug:
                    print("DEBUG: Warning: Setting bin content of bin {} to "
                          "zero for both histograms, because both histograms "
                          "have vanishing errors there.".format(ibin))

    def has_compatible_bins(self):
        """
        Check if both ROOT obeject have the same amount of bins
        @return: True if the bins are equal, otherwise False
        """
        if self.object_a.ClassName() == "TEfficiency" == \
                self.object_b.ClassName():
            nbins_a = self.object_a.GetTotalHistogram().GetNbinsX()
            nbins_b = self.object_b.GetTotalHistogram().GetNbinsX()
        else:
            nbins_a = self.object_a.GetNbinsX()
            nbins_b = self.object_b.GetNbinsX()

        return nbins_a == nbins_b

    @staticmethod
    def __convert_teff_to_hist(teff_a):
        """
        Convert the content of a TEfficiency plot to a histogram and set
        the bin content and errors
        """
        conv_hist = teff_a.GetTotalHistogram()
        xbin_count = conv_hist.GetNbinsX()
        xbin_low = conv_hist.GetXaxis().GetXmin()
        xbin_max = conv_hist.GetXaxis().GetXmax()

        th1 = ROOT.TH1D(
            teff_a.GetName() + "root_conversion",
            teff_a.GetName(),
            xbin_count,
            xbin_low,
            xbin_max
        )
        # starting from the first to the last bin, ignoring the under/overflow
        # bins
        for i in range(1, xbin_count):
            th1.SetBinContent(i, teff_a.GetEfficiency(i))
            th1.SetBinError(i, teff_a.GetEfficiencyErrorLow(i))

        return th1

    def __internal_compare(self):
        """
        Performs the actual Chi^2 test
        @return: The request result quantity
        """
        if not self.correct_types():
            msg = "Comparison of {} (Type {}) with {} (Type {}) not " \
                  "supported.\nPlease contact Thomas.Hauth@kit.edu if you " \
                  "need this supported. "
            raise ObjectsNotSupported(
                msg.format(
                    self.object_a.GetName(),
                    self.object_a.ClassName(),
                    self.object_b.GetName(),
                    self.object_b.ClassName()
                )
            )
        if not self.has_compatible_bins():
            msg = "The objects have differing x bin count: {} has {} vs. {} " \
                  "has {}."
            raise DifferingBinCount(
                msg.format(
                    self.object_a.GetName(),
                    self.object_a.GetNbinsX(),
                    self.object_b.GetName(),
                    self.object_b.GetNbinsX()
                )
            )

        local_object_a = self.object_a
        local_object_b = self.object_b

        # very special handling for TEfficiencies
        if self.object_a.ClassName() == "TEfficiency":
            local_object_a = self.__convert_teff_to_hist(self.object_a)
            local_object_b = self.__convert_teff_to_hist(self.object_b)
            if self.debug:
                print("Converting TEfficiency objects to histograms.")

        nbins = local_object_a.GetNbinsX()

        if nbins < 2:
            raise TooFewBins("{} bin(s) is to few to perform the Chi2 "
                             "test.".format(nbins))

        weighted_types = ["TProfile", "TH1D", "TH1F"]
        comp_weight_a = local_object_a.ClassName() in weighted_types
        comp_weight_b = local_object_b.ClassName() in weighted_types

        # clone, because possibly some content of profiles will
        # be set to zero
        first_obj = local_object_a.Clone()
        second_obj = local_object_b.Clone()

        if comp_weight_a and not comp_weight_b:
            # switch histograms, because ROOT can only have the first one
            # to be unweighted
            first_obj, second_obj = second_obj, first_obj
            if self.debug:
                print("Debug: Warning: Switching the two objects, because "
                      "ROOT can only have the first one to be unweighted")

        # Construct the option string for the Chi2Test call
        comp_options = "P "  # for debugging output
        if comp_weight_a and comp_weight_b:
            comp_options += "WW"
        elif comp_weight_a or comp_weight_b:
            comp_options += "UW"
        else:
            comp_options += "UU"

        if comp_weight_a and comp_weight_b:
            self.ensure_zero_error_has_no_content(first_obj, second_obj)

        # use numpy arrays to support ROOT's pass-by-reference interface here
        res_chi2 = numpy.array([1], numpy.float64)
        res_igood = numpy.array([1], numpy.int32)
        res_ndf = numpy.array([1], numpy.int32)

        res_pvalue = first_obj.Chi2TestX(
            second_obj,
            res_chi2,
            res_ndf,
            res_igood,
            comp_options
        )

        if self.debug:
            print("Performing our own chi2 test, with bin-by-bin results: ")
            print()
            print_contents_and_errors(first_obj, second_obj)
            print()
            print("Here's what ROOT's Chi2Test gave us (comp_options: '{}'):"
                  " ".format(comp_options))

            tp = TablePrinter(3, width=(10, 10, 40))
            print()
            tp.print_divider()
            tp.print(["Key", "Value", "Comment"])
            tp.print_divider()
            tp.print(["chi2", numpy.asscalar(res_chi2),
                      "Should roughly match above 'Total chi2'"])
            tp.print(["ndf", numpy.asscalar(res_ndf), "#Non-empty bins - 1"])
            tp.print(["chi2/ndf", numpy.asscalar(res_chi2/res_ndf), ""])
            tp.print(["igood", numpy.asscalar(res_igood),
                      "a debug indicator, 0 if all good"])
            tp.print(["pvalue", res_pvalue, ""])
            tp.print_divider()
            print()
            print("See https://root.cern.ch/doc/master/classTH1.html for more "
                  "information.")
            print()

        if res_ndf < 1:
            msg = "Comparison failed, no Chi^2 could be computed. For " \
                  "debugging, you can use the CLI of " \
                  "'validation/scripts/validationcomparison.py' on your root " \
                  "file and the reference. Run 'validationcomparison.py " \
                  "--help' for info. "
            raise ComparisonFailed(msg)

        res_chi2ndf = res_chi2 / res_ndf

        return res_pvalue, res_chi2, res_chi2ndf, res_ndf


class TablePrinter(object):
    """ A tiny class to print columns of fixed width numbers. """
    def __init__(self, ncols, width=None):
        """
        Constructor.
        @param ncols: Number of columns
        @param width: Width of each column. Either int or list.
        """
        self.ncols = ncols
        if not width:
            width = 10
        if isinstance(width, int):
            self.widths = [width] * ncols
        elif isinstance(width, list) or isinstance(width, tuple):
            # let's hope this is a list then.
            self.widths = width

    @property
    def tot_width(self):
        """ Total width of the table """
        width = 0
        # the widths of each column
        width += sum(self.widths)
        # three characters between each two columns
        width += (self.ncols - 1) * 3
        # 2 characters at the very left and right
        width += 2 * 2
        return width

    def print_divider(self, char="="):
        """ Print a divider made up from repeated chars """
        print(char * self.tot_width)

    def print(self, cols):
        """ Print one row """
        assert(len(cols) == self.ncols)
        out = []
        for icol, col in enumerate(cols):
            width = self.widths[icol]
            if isinstance(col, int):
                form = "{{:{}d}}".format(width)
                out.append(form.format(col))
            elif isinstance(col, float):
                form = "{{:{}.{}f}}".format(width, width//2)
                out.append(form.format(col))
            else:
                # convert everything else to a string if it isn't already
                col = str(col)
                col = col[:width].rjust(width)
                out.append(col)
        print("| " + " | ".join(out) + " |")


def print_contents_and_errors(obj_a, obj_b):
    """
    Print contents, errors and chi2 deviation for each bin as well as
    some other information about two TH1-like objects.
    @param obj_a: First TH1-like object
    @param obj_b: Second TH1-like object
    @return: None
    """
    nbins = obj_a.GetNbinsX()

    total_a = sum([obj_a.GetBinContent(ibin) for ibin in range(0, nbins + 2)])
    total_b = sum([obj_b.GetBinContent(ibin) for ibin in range(0, nbins + 2)])

    print("Total events/summed weights in object 1: {:10.5f}".format(total_a))
    print("Total events/summed weights in object 2: {:10.5f}".format(total_b))

    chi2_tot = 0

    cp = TablePrinter(6)
    print()
    cp.print_divider()
    cp.print(["ibin", "a", "err a", "b", "err b", "chi2"])
    cp.print_divider()
    for ibin in range(1, nbins + 1):
        content_a = obj_a.GetBinContent(ibin)
        content_b = obj_b.GetBinContent(ibin)
        error_a = obj_a.GetBinError(ibin)
        error_b = obj_b.GetBinError(ibin)
        # This is implemented according to
        # https://root.cern.ch/doc/master/classTH1.html
        try:
            chi2 = (total_b * content_a - total_a * content_b)**2 / \
                   (total_b**2 * error_a**2 + total_a**2 * error_b**2)
            chi2_tot += chi2
        except ZeroDivisionError:
            chi2 = "nan"
        cp.print([
            ibin,
            content_a,
            error_a,
            content_b,
            error_b,
            chi2
        ])
    cp.print_divider()
    print()

    print("Total chi2: {:10.5f}".format(chi2_tot))


def debug_cli():
    """ A small command line interface for debugging purposes. """

    # 1. Get command line arguments
    # =============================

    desc = "For testing purposes: Run the chi2 comparison with objects from " \
           "two root files."
    parser = argparse.ArgumentParser(desc)

    _ = "Rootfile to read the first object from"
    parser.add_argument("rootfile_a", help=_)

    _ = "Name of object inside first rootfile."
    parser.add_argument("name_a", help=_)

    _ = "Rootfile to read the second object from"
    parser.add_argument("rootfile_b", help=_)

    _ = "Name of object inside second rootfile."
    parser.add_argument("name_b", help=_)

    args = parser.parse_args()

    # 2. Open rootfiles and get objects
    # =================================

    if not os.path.exists(args.rootfile_a):
        raise ValueError("Could not find '{}'.".format(args.rootfile_a))

    if not os.path.exists(args.rootfile_b):
        raise ValueError("Could not find '{}'.".format(args.rootfile_b))

    rootfile_a = ROOT.TFile(args.rootfile_a)
    obj_a = rootfile_a.Get(args.name_a)
    if not obj_a:
        raise ValueError("Could not find object '{}' in file '{}'.".format(
            args.name_a, args.rootfile_a
        ))

    rootfile_b = ROOT.TFile(args.rootfile_b)
    obj_b = rootfile_b.Get(args.name_b)
    if not obj_b:
        raise ValueError("Could not find object '{}' in file '{}'.".format(
            args.name_b, args.rootfile_b
        ))

    # 3. Performe testing with debug option
    # =====================================

    test = Chi2Test(obj_a, obj_b, debug=True)
    test.ensure_compute()

    print("If you see this message, then no exception was thrown.")

    # 4. Close files
    # ==============

    rootfile_a.Close()
    rootfile_b.Close()

if __name__ == "__main__":
    # Run command line interface for testing purposes.
    debug_cli()
