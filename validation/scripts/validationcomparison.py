#!/usr/bin/env python3

""" Compare ROOT objects and perform e.g. chi2 tests.
A small command line interface for testing/debugging purposes is included.
Run `python3 validationcomparison.py --help` for more information. """

# std
from abc import ABC, abstractmethod
import argparse
import numpy
import os.path
from typing import Optional

# 3rd
import ROOT

# ours
from metaoptions import MetaOptionParser

# Unfortunately doxygen has some trouble with inheritance of attributes, so
# we disable it.
# @cond SUPPRESS_DOXYGEN


# ==============================================================================
# Custom Exceptions
# ==============================================================================


class ComparisonFailed(Exception):
    """
    The comparison failed for some reason. For example
    because ROOT was not able to compute the Chi^2 properly
    """


class ObjectsNotSupported(Exception):
    """
    The type and/or combination of provided ROOT objects
    is not supported for comparison
    """


class DifferingBinCount(Exception):
    """
    The two ROOT objects provided have a different bin count
    and therefor, cannot be compared using the Chi2 test
    """


class TooFewBins(Exception):
    """
    Not sufficient bins to perform the Chi^2 test
    """


# ==============================================================================
# Comparison class selector
# ==============================================================================


def get_comparison(
        object_1,
        object_2,
        mop: Optional[MetaOptionParser]
) -> "ComparisonBase":
    """ Uses the metaoptions to determine which comparison algorithm is used
    and initializes the corresponding subclass of :class:`ComparisonBase` that
    implements the actual comparison and holds the results.
    @param object_1 ROOT TObject
    @param object_2 ROOT TObject
    @param mop Metaoption parser
    """
    if mop.has_option("kolmogorov"):
        tester = KolmogorovTest
    elif mop.has_option("andersondarling"):
        tester = AndersonDarlingTest
    else:
        tester = Chi2Test

    test = tester(
        object_1,
        object_2,
        mop=mop
    )

    return test


# ==============================================================================
# Comparison Base Class
# ==============================================================================

class ComparisonBase(ABC):
    """
    Base class for all comparison implementations.

    Follows 3 steps:

    1. Initialize the class together with two ROOT objects of different
    revisions (that are to be compared) and the metaoptions (given in the
    corresponding validation (steering) file), that determine how to compare
    them.

    2. The Comparison class saves the ROOT objects and the metaoptions
    internally, but does not compute anything yet

    3. If :meth:`ensure_compute` is called, or any property is accessed that
    depends on computation, the internal implementation :meth:`_compute`
    (to be implemented in the subclass) is called.

    4. :meth:`_compute` ensures that all values, like chi2, p-value etc. are
    computed

    5. Two properties :meth:`comparison_result` (pass/warning/error) and
    :meth:`comparison_result_long` (longer description of the comparison result)
    allow to access the results.
    """

    def __init__(self, object_a, object_b,
                 mop: Optional[MetaOptionParser] = None, debug=False):
        """
        Initialize ComparisonBase class

        :param object_a:
        :param object_b:
        :param mop: MetaOptionParser
        :param debug (bool): Debug mode enabled?
        """
        #: store the first object to compare
        self.object_a = object_a

        #: store the second object to compare
        self.object_b = object_b

        #: MetaOptionParser
        if mop is None:
            mop = MetaOptionParser()
        self.mop = mop

        #: enable debug?
        self.debug = debug

        #: used to store, whether the quantities have already been compared
        self.computed = False

        #: Comparison result, i.e. equal/warning/error
        self._comparison_result = "not_compared"
        #: Longer description of the comparison result (e.g. 'performed Chi2
        #: Test ... with chi2 = ...').
        self._comparison_result_long = ""

    def ensure_compute(self):
        """
        Ensure all required quantities get computed and are cached inside the
        class
        """
        if self.computed:
            return

        if self.mop.has_option("nocompare"):
            # is comparison disabled for this plot ?
            self._comparison_result_long = 'Testing is disabled for this plot'
            return

        fail_message = "Comparison failed: "

        # Note: default for comparison_result is "not_compared"
        try:
            self._compute()
        except ObjectsNotSupported as e:
            self._comparison_result_long = fail_message + str(e)
        except DifferingBinCount as e:
            self._comparison_result = "error"
            self._comparison_result_long = fail_message + str(e)
        except TooFewBins as e:
            self._comparison_result_long = fail_message + str(e)
        except ComparisonFailed as e:
            self._comparison_result = "error"
            self._comparison_result_long = fail_message + str(e)
        except Exception as e:
            self._comparison_result = "error"
            self._comparison_result_long = "Unknown error occurred. Please " \
                                           "submit a bug report. " + str(e)
        else:
            # Will be already set in case of errors above and we don't want
            # to overwrite this.
            self._comparison_result_long = self._get_comparison_result_long()
            self._comparison_result = self._get_comparison_result()

        self.computed = True

    @abstractmethod
    def _get_comparison_result(self) -> str:
        """ Used to format the value of :attr:`_comparison_result`. """

    @abstractmethod
    def _get_comparison_result_long(self) -> str:
        """ Used to format the value of :attr:`_comparison_result_long`. """

    @property
    def comparison_result(self):
        """ Comparison result, i.e. pass/warning/error """
        self.ensure_compute()
        return self._comparison_result

    @property
    def comparison_result_long(self):
        """ Longer description of the comparison result """
        self.ensure_compute()
        return self._comparison_result_long

    @abstractmethod
    def _compute(self):
        """ This method performs the actual computations. """

    def can_compare(self):
        """
        @return: True if the two objects can be compared, False otherwise
        """
        return self._has_correct_types() and self._has_compatible_bins()

    def _has_correct_types(self) -> bool:
        """
        @return: True if the two objects have a) a type supported for
            comparison and b) can be compared with each other
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

    def _raise_has_correct_types(self) -> None:
        """
        Raise Exception if not the two objects have a) a type supported for
        comparison and b) can be compared with each other
        @return: None
        """
        if not self._has_correct_types():
            msg = "Comparison of {} (Type {}) with {} (Type {}) not " \
                  "supported.\nPlease open a JIRA issue (validation " \
                  "component) if you need this supported. "
            raise ObjectsNotSupported(
                msg.format(
                    self.object_a.GetName(),
                    self.object_a.ClassName(),
                    self.object_b.GetName(),
                    self.object_b.ClassName()
                )
            )

    def _has_compatible_bins(self) -> bool:
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

    def _raise_has_compatible_bins(self) -> None:
        """
        Raise Exception if not both ROOT obeject have the same amount of bins
        @return: None
        """
        if not self._has_compatible_bins():
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

    @staticmethod
    def _convert_teff_to_hist(teff_a):
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


class PvalueTest(ComparisonBase):
    """ Test with a pvalue """

    #: Default pvalue below which a warning is issued (unless supplied in
    #: metaoptions)
    _default_pvalue_warn = 1.0

    #: Default pvalue below which an error is issued (unless supplied in
    #: metaoptions)
    _default_pvalue_error = 0.01

    def __init__(self, *args, **kwargs):
        """ Initialize Pvalue test

        Args:
            *args: Positional arguments to ComparisonBase
            **kwargs: Keyword arguments to ComparisonBase
        """
        super().__init__(*args, **kwargs)
        #: pvalue
        self._pvalue = None
        #: pvalue below which a warning is issued
        self._pvalue_warn = self.mop.pvalue_warn()
        #: pvalue below which an error is issued
        self._pvalue_error = self.mop.pvalue_error()

        if self._pvalue_warn is None:
            self._pvalue_warn = self._default_pvalue_warn
        if self._pvalue_error is None:
            self._pvalue_error = self._default_pvalue_error

    def _get_comparison_result(self) -> str:
        if self._pvalue is None:
            return "error"

        if self._pvalue < self._pvalue_error:
            return "error"
        elif self._pvalue < self._pvalue_warn:
            return "warning"
        else:
            return "equal"

    @abstractmethod
    def _compute(self):
        pass

    @abstractmethod
    def _get_comparison_result_long(self):
        pass


# ==============================================================================
# Implementation of specific comparison algorithms
# ==============================================================================

# ------------------------------------------------------------------------------
# Chi2 Test
# ------------------------------------------------------------------------------

class Chi2Test(PvalueTest):

    """
    Perform a Chi2Test for ROOT objects. The chi2 test method is e.g. described
    in the documentation of TH1::Chi2Test. Basically this class wraps around
    this Chi2Test function, and takes care that we can call perform these
    tests for a wider selection of ROOT objects.
    """

    def __init__(self, *args, **kwargs):
        """
        Initialize Chi2Test.
        :param args: See arguments of :class:`ComparisonBase`
        :param kwargs:  See arguments of :class:`ComparisonBase`
        """
        super().__init__(*args, **kwargs)

        # The following attributes will be set in :meth:`_compute`

        #: chi2
        self._chi2 = None
        #: chi2 / number of degrees of freedom
        self._chi2ndf = None
        #: number of degrees of freedom
        self._ndf = None

    def _ensure_zero_error_has_no_content(self, a, b):
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

    def _compute(self) -> None:
        """
        Performs the actual Chi^2 test
        @return: None
        """
        self._raise_has_correct_types()
        self._raise_has_compatible_bins()

        local_object_a = self.object_a
        local_object_b = self.object_b

        # very special handling for TEfficiencies
        if self.object_a.ClassName() == "TEfficiency":
            local_object_a = self._convert_teff_to_hist(self.object_a)
            local_object_b = self._convert_teff_to_hist(self.object_b)
            if self.debug:
                print("Converting TEfficiency objects to histograms.")

        nbins = local_object_a.GetNbinsX()

        if nbins < 2:
            raise TooFewBins("{} bin(s) is too few to perform the Chi2 "
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
            self._ensure_zero_error_has_no_content(first_obj, second_obj)

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
            tp.print(["chi2/ndf", numpy.asscalar(res_chi2 / res_ndf), ""])
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
                  "--help' for info. If problem persists, please open " \
                  "JIRA issue (validation component)."
            raise ComparisonFailed(msg)

        res_chi2ndf = res_chi2 / res_ndf

        self._pvalue, self._chi2, self._chi2ndf, self._ndf = \
            res_pvalue, res_chi2[0], res_chi2ndf[0], res_ndf[0]

    def _get_comparison_result_long(self) -> str:
        if self._pvalue is None or self._chi2ndf is None or self._chi2 is None:
            return r"Could not perform $\chi^2$-Test  between {{revision1}} " \
                   r"and {{revision2}} due to an unknown error. Please " \
                   r"submit a bug report."

        return r'Performed $\chi^2$-Test between {{revision1}} ' \
               r'and {{revision2}} ' \
               r'($\chi^2$ = {chi2:.4f}; NDF = {ndf}; ' \
               r'$\chi^2/\text{{{{NDF}}}}$ = {chi2ndf:.4f}).' \
               r' <b>p-value: {pvalue:.6f}</b> (p-value warn: {pvalue_warn}, ' \
               r'p-value error: {pvalue_error})'.format(
                   chi2=self._chi2, ndf=self._ndf, chi2ndf=self._chi2ndf,
                   pvalue=self._pvalue, pvalue_warn=self._pvalue_warn,
                   pvalue_error=self._pvalue_error
               )

# ------------------------------------------------------------------------------
# Kolmogorov Test
# ------------------------------------------------------------------------------


class KolmogorovTest(PvalueTest):
    """ Kolmogorov-Smirnov Test """

    def __init__(self, *args, **kwargs):
        """
        Initialize Kolmogorov test.
        @param args: See arguments of :class:`ComparisonBase`
        @param kwargs:  See arguments of :class:`ComparisonBase`
        """
        super().__init__(*args, **kwargs)

    def _compute(self):
        """
        Perform the actual test
        @return: None
        """
        self._raise_has_correct_types()
        self._raise_has_compatible_bins()

        local_object_a = self.object_a
        local_object_b = self.object_b

        # very special handling for TEfficiencies
        if self.object_a.ClassName() == "TEfficiency":
            local_object_a = self._convert_teff_to_hist(self.object_a)
            local_object_b = self._convert_teff_to_hist(self.object_b)
            if self.debug:
                print("Converting TEfficiency objects to histograms.")

        option_str = "UON"
        if self.debug:
            option_str += "D"

        self._pvalue = local_object_a.KolmogorovTest(local_object_b, option_str)

    def _get_comparison_result_long(self) -> str:
        if self._pvalue is None:
            return r"Could not perform Kolmogorov test between {{revision1}} " \
                   r"and {{revision2}} due to an unknown error. Please submit " \
                   r"a bug report."

        return r'Performed Komlogorov test between {{revision1}} ' \
               r'and {{revision2}} ' \
               r' <b>p-value: {pvalue:.6f}</b> (p-value warn: {pvalue_warn}, ' \
               r'p-value error: {pvalue_error})'.format(
                   pvalue=self._pvalue, pvalue_warn=self._pvalue_warn,
                   pvalue_error=self._pvalue_error
               )

# ------------------------------------------------------------------------------
# Anderson Darling Test
# ------------------------------------------------------------------------------


class AndersonDarlingTest(PvalueTest):
    """ Anderson-Darling test"""

    def __init__(self, *args, **kwargs):
        """
        Initialize Kolmogorov test.
        @param args: See arguments of :class:`ComparisonBase`
        @param kwargs:  See arguments of :class:`ComparisonBase`
        """
        super().__init__(*args, **kwargs)

    def _compute(self):
        """
        Perform the actual test
        @return: None
        """
        self._raise_has_correct_types()
        # description on
        # https://root.cern.ch/doc/master/classTH1.html#aa6b386786876dc304d73ab6b2606d4f6
        # sounds like we don't have to have the same bins

        local_object_a = self.object_a
        local_object_b = self.object_b

        # very special handling for TEfficiencies
        if self.object_a.ClassName() == "TEfficiency":
            local_object_a = self._convert_teff_to_hist(self.object_a)
            local_object_b = self._convert_teff_to_hist(self.object_b)
            if self.debug:
                print("Converting TEfficiency objects to histograms.")

        option_str = ""
        if self.debug:
            option_str += "D"

        self._pvalue = local_object_a.KolmogorovTest(local_object_b, option_str)

    def _get_comparison_result_long(self) -> str:
        if self._pvalue is None:
            return r"Could not perform Anderson Darling test between " \
                   r"{{revision1}} and {{revision2}} due to an unknown error." \
                   r" Please support a bug report."

        return r'Performed Anderson Darling test between {{revision1}} ' \
               r'and {{revision2}} ' \
               r' <b>p-value: {pvalue:.6f}</b> (p-value warn: {pvalue_warn}, ' \
               r'p-value error: {pvalue_error})'.format(
                   pvalue=self._pvalue, pvalue_warn=self._pvalue_warn,
                   pvalue_error=self._pvalue_error
               )

# ==============================================================================
# Helpers
# ==============================================================================


class TablePrinter:
    """ A tiny class to print columns of fixed width numbers. """

    def __init__(self, ncols, width=None):
        """
        Constructor.
        @param ncols: Number of columns
        @param width: Width of each column. Either int or list.
        """
        #: the number of columns
        self.ncols = ncols
        if not width:
            width = 10
        if isinstance(width, int):
            #: width of each column
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
                form = f"{{:{width}d}}"
                out.append(form.format(col))
            elif isinstance(col, float):
                form = "{{:{}.{}f}}".format(width, width // 2)
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

    print(f"Total events/summed weights in object 1: {total_a:10.5f}")
    print(f"Total events/summed weights in object 2: {total_b:10.5f}")

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

    print(f"Total chi2: {chi2_tot:10.5f}")


# ==============================================================================
# Command Line Interface
# ==============================================================================

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
        raise ValueError(f"Could not find '{args.rootfile_a}'.")

    if not os.path.exists(args.rootfile_b):
        raise ValueError(f"Could not find '{args.rootfile_b}'.")

    rootfile_a = ROOT.TFile(args.rootfile_a)
    obj_a = rootfile_a.Get(args.name_a)
    if not obj_a:
        raise ValueError(
            f"Could not find object '{args.name_a}' "
            f"in file '{args.rootfile_a}'.")

    rootfile_b = ROOT.TFile(args.rootfile_b)
    obj_b = rootfile_b.Get(args.name_b)
    if not obj_b:
        raise ValueError(
            f"Could not find object '{args.name_b}' "
            f"in file '{args.rootfile_b}'.")

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

# End suppression of doxygen checks
# @endcond
