import functools
import numpy as np
import collections
import copy

from tracking.validation.plot import ValidationPlot, compose_axis_label
from tracking.validation.fom import ValidationFiguresOfMerit
from tracking.validation.classification import ClassificationAnalysis
from tracking.validation.pull import PullAnalysis
from tracking.validation.tolerate_missing_key_formatter import TolerateMissingKeyFormatter
from tracking.root_utils import root_cd, root_save_name

import ROOT

import logging


def get_logger():
    return logging.getLogger(__name__)


#: string formatter that handles missing keys gracefully
formatter = TolerateMissingKeyFormatter()


class Refiner(object):
    """Python module to refine a peeled dictionary"""

    def __init__(self, refiner_function=None):
        """Constructor of the Refiner instance"""
        #: cached copy of the instance's refiner function
        self.refiner_function = refiner_function

    def __get__(self, harvesting_module, cls=None):
        """Getter of the Refiner instance"""
        if harvesting_module is None:
            # Class access
            return self
        else:
            # Instance access
            refine = self.refine

            def bound_call(*args, **kwds):
                return refine(harvesting_module, *args, **kwds)
            return bound_call

    def __call__(self, harvesting_module, crops=None, *args, **kwds):
        """implementation of the function-call of the Refiner instance
             r = Refiner()
             r(harvester) # decoration
             r(harvester, crops, args, keywords) # refinement
        """
        if crops is None:
            # Decoration mode
            harvesting_module.refiners.append(self)
            return harvesting_module
        else:
            # Refining mode
            return self.refine(harvesting_module, crops, *args, **kwds)

    def refine(self, harvesting_module, *args, **kwds):
        """Apply the instance's refiner function"""
        self.refiner_function(harvesting_module, *args, **kwds)


class SaveFiguresOfMeritRefiner(Refiner):
    """Refiner for figures of merit"""
    #: default name for this refiner
    default_name = "{module.id}_figures_of_merit{groupby_key}"
    #: default title for this refiner
    default_title = "Figures of merit in {module.title}"
    #: default contact person for this refiner
    default_contact = "{module.contact}"
    #: default description for this refiner
    default_description = "Figures of merit are the {aggregation.__name__} of {keys}"
    #: default user-check action for this refiner
    default_check = "Check for reasonable values"
    #: default key name for this refiner
    default_key = "{aggregation.__name__}_{part_name}"

    #: return the mean of the parts, ignoring NaNs
    @staticmethod
    def mean(xs):
        return np.nanmean(xs)

    #: default aggregation is the mean of the parts
    default_aggregation = mean

    def __init__(self,
                 name=None,
                 title=None,
                 contact=None,
                 description=None,
                 check=None,
                 key=None,
                 aggregation=None,
                 ):
        """Constructor for this refiner"""

        super(SaveFiguresOfMeritRefiner, self).__init__()

        #: cached name of the figure of merit
        self.name = name
        #: cached title of the figure of merit
        self.title = title

        #: cached description of the figure of merit
        self.description = description
        #: cached user-check action of the figure of merit
        self.check = check
        #: cached contact person of the figure of merit
        self.contact = contact

        #: cached copy of the figures-of-merit key
        self.key = key
        #: cached copy of the crops-aggregation method
        self.aggregation = aggregation

    def refine(self,
               harvesting_module,
               crops,
               tdirectory=None,
               groupby_part_name=None,
               groupby_value=None,
               **kwds):
        """Process the figures of merit"""

        name = self.name or self.default_name
        title = self.title or self.default_title
        contact = self.contact or self.default_contact
        description = self.description or self.default_description
        check = self.check or self.default_check

        aggregation = self.aggregation or self.default_aggregation

        replacement_dict = dict(
            refiner=self,
            module=harvesting_module,
            aggregation=aggregation,
            groupby_key='_' + groupby_part_name + groupby_value if groupby_part_name else "",
            groupby=groupby_part_name,  # deprecated
            groupby_value=groupby_value,  # deprecated
        )

        name = formatter.format(name, **replacement_dict)
        title = formatter.format(title, **replacement_dict)
        contact = formatter.format(contact, **replacement_dict)

        figures_of_merit = ValidationFiguresOfMerit(name,
                                                    contact=contact,
                                                    title=title)

        for part_name, parts in iter_items_sorted_for_key(crops):
            key = self.key or self.default_key
            key = formatter.format(key, part_name=part_name, **replacement_dict)
            figures_of_merit[key] = aggregation(parts)

        keys = list(figures_of_merit.keys())

        description = formatter.format(description, keys=keys, **replacement_dict)
        check = formatter.format(check, keys=keys, **replacement_dict)

        figures_of_merit.description = description
        figures_of_merit.check = check

        if tdirectory:
            figures_of_merit.write(tdirectory)

        print(figures_of_merit)


class SaveHistogramsRefiner(Refiner):
    """Refiner for histograms"""
    #: default name for this histogram
    default_name = "{module.id}_{part_name}_histogram{groupby_key}{stackby_key}"
    #: default title for this histogram
    default_title = "Histogram of {part_name}{groupby_key}{stackby_key} from {module.title}"
    #: default contact person for this histogram
    default_contact = "{module.contact}"
    #: default description for this histogram
    default_description = "This is a histogram of {part_name}{groupby_key}{stackby_key}."
    #: default user-check action for this histogram
    default_check = "Check if the distribution is reasonable"

    def __init__(self,
                 name=None,
                 title=None,
                 contact=None,
                 description=None,
                 check=None,
                 lower_bound=None,
                 upper_bound=None,
                 bins=None,
                 outlier_z_score=None,
                 allow_discrete=False,
                 stackby="",
                 fit=None,
                 fit_z_score=None):
        """Constructor for this refiner"""

        super(SaveHistogramsRefiner, self).__init__()

        #: cached user-defined name for this histogram
        self.name = name
        #: cached user-defined title for this histogram
        self.title = title

        #: cached user-defined description for this histogram
        self.description = description
        #: cached user-defined user-check action for this histogram
        self.check = check
        #: cached user-defined contact person for this histogram
        self.contact = contact

        #: cached lower bound for this histogram
        self.lower_bound = lower_bound
        #: cached upper bound for this histogram
        self.upper_bound = upper_bound
        #: cached number of bins for this histogram
        self.bins = bins

        #: cached Z-score (for outlier detection) for this histogram
        self.outlier_z_score = outlier_z_score
        #: cached flag to allow discrete values for this histogram
        self.allow_discrete = allow_discrete
        #: cached stacking selection for this histogram
        self.stackby = stackby

        #: cached fit for this histogram
        self.fit = fit
        #: cached fit Z-score (for outlier detection) for this histogram
        self.fit_z_score = fit_z_score

    def refine(self,
               harvesting_module,
               crops,
               tdirectory=None,
               groupby_part_name=None,
               groupby_value=None,
               **kwds):
        """Process the histogram"""

        stackby = self.stackby
        if stackby:
            stackby_parts = crops[stackby]
        else:
            stackby_parts = None

        replacement_dict = dict(
            refiner=self,
            module=harvesting_module,
            stackby_key=' stacked by ' + stackby if stackby else "",
            groupby_key=' in group ' + groupby_part_name + groupby_value if groupby_part_name else "",
        )

        contact = self.contact or self.default_contact
        contact = formatter.format(contact, **replacement_dict)

        for part_name, parts in iter_items_sorted_for_key(crops):
            name = self.name or self.default_name
            title = self.title or self.default_title
            description = self.description or self.default_description
            check = self.check or self.default_check

            name = formatter.format(name, part_name=part_name, **replacement_dict)
            title = formatter.format(title, part_name=part_name, **replacement_dict)
            description = formatter.format(description, part_name=part_name, **replacement_dict)
            check = formatter.format(check, part_name=part_name, **replacement_dict)

            histogram = ValidationPlot(name)
            histogram.hist(parts,
                           lower_bound=self.lower_bound,
                           upper_bound=self.upper_bound,
                           bins=self.bins,
                           outlier_z_score=self.outlier_z_score,
                           allow_discrete=self.allow_discrete,
                           stackby=stackby_parts)

            histogram.title = title
            histogram.contact = contact
            histogram.description = description
            histogram.check = check

            histogram.xlabel = compose_axis_label(part_name)

            if self.fit:
                if self.fit_z_score is None:
                    kwds = dict()
                else:
                    kwds = dict(z_score=self.fit_z_score)

                fit_method_name = 'fit_' + str(self.fit)
                try:
                    fit_method = getattr(histogram, fit_method_name)
                except AttributeError:
                    histogram.fit(str(self.fit), **kwds)
                else:
                    fit_method(**kwds)

            if tdirectory:
                histogram.write(tdirectory)


class Plot2DRefiner(Refiner):
    """Refiner for profile histograms and 2D scatterplots"""
    #: by default, this refiner is for profile histograms
    plot_kind = "profile"

    def __init__(self,
                 y,
                 x=None,
                 name=None,
                 title=None,
                 contact=None,
                 description=None,
                 check=None,
                 stackby=None,
                 y_unit=None,
                 y_binary=None,
                 y_log=None,
                 lower_bound=None,
                 upper_bound=None,
                 bins=None,
                 outlier_z_score=None,
                 fit=None,
                 fit_z_score=None,
                 skip_single_valued=False,
                 allow_discrete=False):
        """Constructor for this refiner"""

        super().__init__()

        #: cached user-defined name for this profile histogram / scatterplot
        self.name = name
        #: cached user-defined title for this profile histogram / scatterplot
        self.title = title

        #: cached user-defined description for this profile histogram / scatterplot
        self.description = description
        #: cached user-defined user-check action for this profile histogram / scatterplot
        self.check = check
        #: cached user-defined contact person for this profile histogram / scatterplot
        self.contact = contact

        #: cached value of abscissa
        self.x = x
        #: cached value of ordinate
        self.y = y
        #: cached stacking selection for this profile histogram / scatterplot
        self.stackby = stackby
        #: cached measurement unit for ordinate
        self.y_unit = y_unit

        #: cached lower bound for this profile histogram / scatterplot
        self.lower_bound = lower_bound
        #: cached upper bound for this profile histogram / scatterplot
        self.upper_bound = upper_bound
        #: cached number of bins for this profile histogram / scatterplot
        self.bins = bins
        #: cached flag for probability y axis (range 0.0 .. 1.05) for this profile histogram / scatterplot
        self.y_binary = y_binary
        #: cached flag for logarithmic y axis for this profile histogram / scatterplot
        self.y_log = y_log

        #: cached Z-score (for outlier detection) for this profile histogram / scatterplot
        self.outlier_z_score = outlier_z_score
        #: cached flag to allow discrete values for this profile histogram / scatterplot
        self.allow_discrete = allow_discrete

        #: cached fit for this profile histogram / scatterplot
        self.fit = fit
        #: cached fit Z-score (for outlier detection) for this profile histogram / scatterplot
        self.fit_z_score = fit_z_score

        #: cached flag to skip single-valued bins for this profile histogram / scatterplot
        self.skip_single_valued = skip_single_valued

    def refine(self,
               harvesting_module,
               crops,
               tdirectory=None,
               groupby_part_name=None,
               groupby_value=None,
               **kwds):
        """Process the profile histogram / scatterplot"""

        stackby = self.stackby
        if stackby:
            stackby_parts = crops[stackby]
        else:
            stackby_parts = None

        replacement_dict = dict(
            refiner=self,
            module=harvesting_module,
            stackby_key=' stacked by ' + stackby if stackby else "",
            groupby_key=' in group ' + groupby_part_name + groupby_value if groupby_part_name else "",
        )

        contact = self.contact or self.default_contact
        contact = formatter.format(contact, **replacement_dict)

        y_crops = select_crop_parts(crops, select=self.y)
        x_crops = select_crop_parts(crops, select=self.x, exclude=self.y)

        for y_part_name, y_parts in iter_items_sorted_for_key(y_crops):
            for x_part_name, x_parts in iter_items_sorted_for_key(x_crops):

                if self.skip_single_valued and not self.has_more_than_one_value(x_parts):
                    get_logger().info('Skipping "%s" by "%s" profile because x has only a single value "%s"',
                                      y_part_name,
                                      x_part_name,
                                      x_parts[0])
                    continue

                if self.skip_single_valued and not self.has_more_than_one_value(y_parts):
                    get_logger().info('Skipping "%s" by "%s" profile because y has only a single value "%s"',
                                      y_part_name,
                                      x_part_name,
                                      y_parts[0])
                    continue

                name = self.name or self.default_name
                title = self.title or self.default_title
                description = self.description or self.default_description
                check = self.check or self.default_check

                name = formatter.format(name,
                                        x_part_name=x_part_name,
                                        y_part_name=y_part_name,
                                        **replacement_dict)

                title = formatter.format(title,
                                         x_part_name=x_part_name,
                                         y_part_name=y_part_name,
                                         **replacement_dict)

                description = formatter.format(description,
                                               x_part_name=x_part_name,
                                               y_part_name=y_part_name,
                                               **replacement_dict)

                check = formatter.format(check,
                                         x_part_name=x_part_name,
                                         y_part_name=y_part_name,
                                         **replacement_dict)

                profile_plot = ValidationPlot(name)

                plot_kind = self.plot_kind
                if plot_kind == "profile":
                    profile_plot.profile(x_parts,
                                         y_parts,
                                         lower_bound=self.lower_bound,
                                         upper_bound=self.upper_bound,
                                         bins=self.bins,
                                         y_binary=self.y_binary,
                                         y_log=self.y_log,
                                         outlier_z_score=self.outlier_z_score,
                                         allow_discrete=self.allow_discrete,
                                         stackby=stackby_parts)

                    if self.fit:
                        if self.fit_z_score is None:
                            kwds = dict()
                        else:
                            kwds = dict(z_score=self.fit_z_score)

                        fit_method_name = 'fit_' + str(self.fit)
                        try:
                            fit_method = getattr(profile_plot, fit_method_name)
                        except BaseException:
                            profile_plot.fit(str(self.fit), **kwds)
                        else:
                            fit_method(**kwds)

                elif plot_kind == "scatter":
                    profile_plot.scatter(x_parts,
                                         y_parts,
                                         lower_bound=self.lower_bound,
                                         upper_bound=self.upper_bound,
                                         outlier_z_score=self.outlier_z_score,
                                         stackby=stackby_parts)

                profile_plot.title = title
                profile_plot.contact = contact
                profile_plot.description = description
                profile_plot.check = check

                profile_plot.xlabel = compose_axis_label(x_part_name)
                profile_plot.ylabel = compose_axis_label(y_part_name, self.y_unit)

                if tdirectory:
                    profile_plot.write(tdirectory)

    @staticmethod
    def has_more_than_one_value(xs):
        """check if a list has at least two unique values"""
        first_x = xs[0]
        for x in xs:
            if x != first_x:
                return True
        else:
            return False


class SaveProfilesRefiner(Plot2DRefiner):
    """Refiner for profile histograms"""
    #: default name for this profile histogram
    default_name = "{module.id}_{y_part_name}_by_{x_part_name}_profile{groupby_key}{stackby_key}"
    #: default title for this profile histogram
    default_title = "Profile of {y_part_name} by {x_part_name} from {module.title}"
    #: default contact person for this profile histogram
    default_contact = "{module.contact}"
    #: default description for this profile histogram
    default_description = "This is a profile of {y_part_name} over {x_part_name}."
    #: default user-check action for this profile histogram
    default_check = "Check if the trend line is reasonable."

    #: specify this as a profile histogram rather than a scatterplot
    plot_kind = "profile"


class SaveScatterRefiner(Plot2DRefiner):
    """Refiner for 2D scatterplots"""
    #: default name for this scatterplot
    default_name = "{module.id}_{y_part_name}_by_{x_part_name}_scatter{groupby_key}{stackby_key}"
    #: default title for this scatterplot
    default_title = "Scatter of {y_part_name} by {x_part_name} from {module.title}"
    #: default contact person for this scatterplot
    default_contact = "{module.contact}"
    #: default description for this scatterplot
    default_description = "This is a scatter of {y_part_name} over {x_part_name}."
    #: default user-check action for this scatterplot
    default_check = "Check if the distributions is reasonable."

    #: specify this as a scatterplot rather than a profile histogram
    plot_kind = "scatter"


class SaveClassificationAnalysisRefiner(Refiner):
    """Refiner for truth-classification analyses"""

    #: default contact person for this truth-classification analysis
    default_contact = "{module.contact}"

    #: default name for the truth-classification analysis truth-values collection
    default_truth_name = "{part_name}_truth"
    #: default name for the truth-classification analysis estimates collection
    default_estimate_name = "{part_name}_estimate"

    def __init__(self,
                 part_name=None,
                 contact=None,
                 estimate_name=None,
                 truth_name=None,
                 cut_direction=None,
                 cut=None,
                 lower_bound=None,
                 upper_bound=None,
                 outlier_z_score=None,
                 allow_discrete=False,
                 unit=None):
        """Constructor for this refiner"""

        #: cached part name for this truth-classification analysis
        self.part_name = part_name
        #: cached contact person for this truth-classification analysis
        self.contact = contact
        #: cached estimates-collection name for this truth-classification analysis
        self.estimate_name = estimate_name
        #: cached truth-values-collection name for this truth-classification analysis
        self.truth_name = truth_name

        #: cached threshold of estimates for this truth-classification analysis
        self.cut = cut
        #: cached cut direction (> or <) of estimates for this truth-classification analysis
        self.cut_direction = cut_direction

        #: cached lower bound of estimates for this truth-classification analysis
        self.lower_bound = lower_bound
        #: cached upper bound of estimates for this truth-classification analysis
        self.upper_bound = upper_bound
        #: cached Z-score (for outlier detection) of estimates for this truth-classification analysis
        self.outlier_z_score = outlier_z_score
        #: cached discrete-value flag of estimates for this truth-classification analysis
        self.allow_discrete = allow_discrete
        #: cached measurement unit of estimates for this truth-classification analysis
        self.unit = unit

    def refine(self,
               harvesting_module,
               crops,
               tdirectory=None,
               groupby_part_name=None,
               groupby_value=None,
               **kwds):
        """Process the truth-classification analysis"""

        replacement_dict = dict(
            refiner=self,
            module=harvesting_module,
            groupby_key='_' + groupby_part_name + groupby_value if groupby_part_name else "",
            groupby=groupby_part_name,  # deprecated
            groupby_value=groupby_value,  # deprecated
        )

        contact = self.contact or self.default_contact
        contact = formatter.format(contact, **replacement_dict)

        if self.truth_name is not None:
            truth_name = self.truth_name
        else:
            truth_name = self.default_truth_name

        truth_name = formatter.format(truth_name, part_name=self.part_name)
        truths = crops[truth_name]

        if self.estimate_name is not None:
            estimate_name = self.estimate_name
        else:
            estimate_name = self.default_estimate_name

        if isinstance(estimate_name, str):
            estimate_names = [estimate_name, ]
        else:
            estimate_names = estimate_name

        for estimate_name in estimate_names:
            estimate_name = formatter.format(estimate_name, part_name=self.part_name)
            estimates = crops[estimate_name]

            classification_analysis = ClassificationAnalysis(quantity_name=estimate_name,
                                                             contact=contact,
                                                             cut_direction=self.cut_direction,
                                                             cut=self.cut,
                                                             lower_bound=self.lower_bound,
                                                             upper_bound=self.upper_bound,
                                                             outlier_z_score=self.outlier_z_score,
                                                             allow_discrete=self.allow_discrete,
                                                             unit=self.unit)

            classification_analysis.analyse(estimates, truths)

            if tdirectory:
                classification_analysis.write(tdirectory)


class SavePullAnalysisRefiner(Refiner):
    """Refiner for pull analyses"""

    #: default name for this pull analysis
    default_name = "{module.id}_{quantity_name}"
    #: default contact person for this pull analysis
    default_contact = "{module.contact}"
    #: default suffix for the title of this pull analysis
    default_title_postfix = " from {module.title}"

    #: default name for the pull analysis truth-values collection
    default_truth_name = "{part_name}_truth"
    #: default name for the pull analysis estimates collection
    default_estimate_name = "{part_name}_estimate"
    #: default name for the pull analysis variances collection
    default_variance_name = "{part_name}_variance"

    def __init__(self,
                 name=None,
                 contact=None,
                 title_postfix=None,
                 part_name=None,
                 part_names=None,
                 truth_name=None,
                 estimate_name=None,
                 variance_name=None,
                 quantity_name=None,
                 aux_names=None,
                 unit=None,
                 outlier_z_score=None,
                 absolute=False,
                 which_plots=None):
        """Constructor for this refiner"""
        if aux_names is None:
            aux_names = []
        #: cached name for this pull analysis
        self.name = name
        #: cached contact person for this pull analysis
        self.contact = contact
        #: cached suffix for the title of this pull analysis
        self.title_postfix = title_postfix

        #: cached array of part names for this pull analysis
        self.part_names = []
        if part_names is not None:
            self.part_names = part_names

        if part_name is not None:
            self.part_names.append(part_name)

        #: cached name for the pull analysis truth-values collection
        self.truth_name = truth_name
        #: cached name for the pull analysis estimates collection
        self.estimate_name = estimate_name
        #: cached name for the pull analysis variances collection
        self.variance_name = variance_name

        #: cached name of the quantity for the pull analysis
        self.quantity_name = quantity_name
        #: cached measurement unit for the pull analysis
        self.unit = unit

        #: cached auxiliary names for the pull analysis
        self.aux_names = aux_names

        #: cached Z-score (for outlier detection) for the pull analysis
        self.outlier_z_score = outlier_z_score
        #: cached absolute-value-comparison flag for the pull analysis
        self.absolute = absolute
        #: cached list of plots produced by the pull analysis
        self.which_plots = which_plots

    def refine(self,
               harvesting_module,
               crops,
               tdirectory=None,
               groupby_part_name=None,
               groupby_value=None,
               **kwds):
        """Process the pull analysis"""

        replacement_dict = dict(
            refiner=self,
            module=harvesting_module,
            # stackby_key='_' + stackby if stackby else "",
            groupby_key='_' + groupby_part_name + groupby_value if groupby_part_name else "",
            groupby=groupby_part_name,  # deprecated
            groupby_value=groupby_value,  # deprecated
        )

        contact = self.contact or self.default_contact
        contact = formatter.format(contact, **replacement_dict)

        name = self.name or self.default_name

        if self.aux_names:
            auxiliaries = select_crop_parts(crops, self.aux_names)
        else:
            auxiliaries = {}

        for part_name in self.part_names:
            name = formatter.format(name, part_name=part_name, **replacement_dict)
            plot_name = name + "_{subplot_name}"

            title_postfix = self.title_postfix
            if title_postfix is None:
                title_postfix = self.default_title_postfix

            title_postfix = formatter.format(title_postfix, part_name=part_name, **replacement_dict)
            plot_title = "{subplot_title} of {quantity_name}" + title_postfix

            if self.truth_name is not None:
                truth_name = self.truth_name
            else:
                truth_name = self.default_truth_name

            if self.estimate_name is not None:
                estimate_name = self.estimate_name
            else:
                estimate_name = self.default_estimate_name

            if self.variance_name is not None:
                variance_name = self.variance_name
            else:
                variance_name = self.default_variance_name

            truth_name = formatter.format(truth_name, part_name=part_name)
            estimate_name = formatter.format(estimate_name, part_name=part_name)
            variance_name = formatter.format(variance_name, part_name=part_name)

            truths = crops[truth_name]
            estimates = crops[estimate_name]
            try:
                variances = crops[variance_name]
            except KeyError:
                variances = None

            quantity_name = self.quantity_name or part_name

            which_plots = self.which_plots

            pull_analysis = PullAnalysis(quantity_name,
                                         unit=self.unit,
                                         absolute=self.absolute,
                                         outlier_z_score=self.outlier_z_score,
                                         plot_name=plot_name,
                                         plot_title=plot_title)

            pull_analysis.analyse(truths,
                                  estimates,
                                  variances,
                                  auxiliaries=auxiliaries,
                                  which_plots=which_plots)

            pull_analysis.contact = contact

            if tdirectory:
                pull_analysis.write(tdirectory)


class SaveTreeRefiner(Refiner):
    """Refiner for ROOT TTrees"""

    #: default name for this TTree
    default_name = "{module.id}_tree"
    #: default title for this TTree
    default_title = "Tree of {module.id}"

    def __init__(self,
                 name=None,
                 title=None):
        """Constructor for this refiner"""
        super(SaveTreeRefiner, self).__init__()

        #: cached name for this TTree
        self.name = name
        #: cached title for this TTree
        self.title = title

    def refine(self,
               harvesting_module,
               crops,
               tdirectory=None,
               groupby_part_name=None,
               groupby_value=None,
               **kwds):
        """Process the TTree"""

        replacement_dict = dict(
            refiner=self,
            module=harvesting_module,
            groupby_key='_' + groupby_part_name + groupby_value if groupby_part_name else "",
            groupby=groupby_part_name,  # deprecated
            groupby_value=groupby_value,  # deprecated
        )

        with root_cd(tdirectory):
            name = self.name or self.default_name
            title = self.title or self.default_title

            name = formatter.format(name, **replacement_dict)
            title = formatter.format(title, **replacement_dict)

            output_ttree = ROOT.TTree(root_save_name(name), title)
            for part_name, parts in iter_items_sorted_for_key(crops):
                self.add_branch(output_ttree, part_name, parts)

            output_ttree.FlushBaskets()
            output_ttree.Write()

    def add_branch(self, output_ttree, part_name, parts):
        """Add a TBranch to the TTree"""
        input_value = np.zeros(1, dtype=float)

        branch_type_spec = '%s/D' % part_name
        tbranch = output_ttree.Branch(part_name, input_value, branch_type_spec)

        if output_ttree.GetNbranches() == 1:
            # On filling of the first branch we need to use the fill method of the TTree
            # For all other branches we can use the one of the branch
            # #justrootthings
            for value in parts:
                input_value[0] = value
                output_ttree.Fill()

        else:
            for value in parts:
                input_value[0] = value
                tbranch.Fill()

        output_ttree.GetEntry(0)
        output_ttree.ResetBranchAddress(tbranch)
        also_subbranches = True  # No subbranches here but we drop the buffers just in case.
        output_ttree.DropBranchFromCache(tbranch, also_subbranches)


class FilterRefiner(Refiner):
    """Refiner for filters"""

    def __init__(self, wrapped_refiner, filter=None, on=None):
        """Constructor for this refiner"""

        #: cached value of the wrapped refiner
        self.wrapped_refiner = wrapped_refiner

        if filter is None:
            #: cached value of the filter
            self.filter = np.nonzero
        else:
            self.filter = filter

        #: cached value of the part name to filter on
        self.on = on

    def refine(self, harvesting_module, crops, *args, **kwds):
        """Process this filter"""
        filtered_crops = filter_crops(crops, self.filter, part_name=self.on)
        self.wrapped_refiner(harvesting_module, filtered_crops, *args, **kwds)


class SelectRefiner(Refiner):
    """Refiner for selection"""

    def __init__(self, wrapped_refiner, select=None, exclude=None):
        """Constructor for this refiner"""
        if select is None:
            select = []
        if exclude is None:
            exclude = []
        #: cached value of the wrapped refiner
        self.wrapped_refiner = wrapped_refiner
        #: cached value of the selector
        self.select = select
        #: cached value of the exclusion flag
        self.exclude = exclude

    def refine(self, harvesting_module, crops, *args, **kwds):
        """Process this selection"""
        selected_crops = select_crop_parts(crops, select=self.select, exclude=self.exclude)
        self.wrapped_refiner(harvesting_module, selected_crops, *args, **kwds)


class GroupByRefiner(Refiner):
    """Refiner for grouping"""

    #: default value of the exclude-by classifier
    default_exclude_by = True

    def __init__(self,
                 wrapped_refiner,
                 by=None,
                 exclude_by=None):
        """Constructor for this refiner"""
        if by is None:
            by = []
        #: cached value of the wrapped refiner
        self.wrapped_refiner = wrapped_refiner
        #: cached value of the group-by classifier
        self.by = by
        #: cached value of the exclude-by classifier
        self.exclude_by = exclude_by if exclude_by is not None else self.default_exclude_by

    def refine(self,
               harvesting_module,
               crops,
               groupby_part_name=None,
               groupby_value=None,
               *args,
               **kwds):
        """Process this grouping"""

        by = self.by

        # A single name to do the group by
        if isinstance(by, str) or by is None:
            part_name = by
            # Wrap it into a list an continue with the general case
            by = [part_name, ]

        for groupby_spec in by:
            if groupby_spec is None:
                # Using empty string as groupby_value to indicate that all values have been selected
                value = None
                self.wrapped_refiner(harvesting_module,
                                     crops,
                                     groupby_part_name=None,
                                     groupby_value=value,
                                     *args,
                                     **kwds)
                continue

            elif isinstance(groupby_spec, str):
                part_name = groupby_spec
                groupby_parts = crops[part_name]
                unique_values, index_of_values = np.unique(groupby_parts, return_inverse=True)
                groupby_values = [" = {value}]".format(value=value) for value in unique_values]

            elif isinstance(groupby_spec, tuple):
                part_name = groupby_spec[0]
                cuts = groupby_spec[1]

                groupby_parts = crops[part_name]

                # Take care of nans
                digitization_cuts = list(np.sort(cuts))
                if digitization_cuts[-1] != np.inf:
                    digitization_cuts.append(np.inf)
                index_of_values = np.digitize(groupby_parts, digitization_cuts, right=True)

                groupby_values = ["below {upper_bound}".format(upper_bound=digitization_cuts[0])]
                bin_bounds = list(zip(digitization_cuts[0:], digitization_cuts[1:]))
                for lower_bound, upper_bound in bin_bounds:
                    if lower_bound == upper_bound:
                        # degenerated bin case
                        groupby_values.append("= {lower_bound}".format(lower_bound=lower_bound))
                    elif upper_bound == np.inf:
                        groupby_values.append("above {lower_bound}".format(lower_bound=lower_bound))
                    else:
                        groupby_values.append("between {lower_bound} and {upper_bound}".format(lower_bound=lower_bound,
                                                                                               upper_bound=upper_bound))
                groupby_values.append("is nan")
                assert len(groupby_values) == len(digitization_cuts) + 1

            else:
                raise ValueError("Unknown groupby specification %s" % groupby_spec)

            # Exclude the groupby variable if desired
            selected_crops = select_crop_parts(crops, exclude=part_name if self.exclude_by else None)
            for index_of_value, groupby_value in enumerate(groupby_values):
                indices_for_value = index_of_values == index_of_value
                if not np.any(indices_for_value):
                    continue

                filtered_crops = filter_crops(selected_crops, indices_for_value)

                self.wrapped_refiner(harvesting_module,
                                     filtered_crops,
                                     groupby_part_name=part_name,
                                     groupby_value=groupby_value,
                                     *args,
                                     **kwds)


class CdRefiner(Refiner):
    """Refiner for change-directory"""

    #: Folder name to be used if a groupby selection is active.
    default_folder_name = ""
    #: Default suffix for a groupby selection
    default_groupby_addition = "_groupby_{groupby}_{groupby_value}"

    def __init__(self,
                 wrapped_refiner,
                 folder_name=None,
                 groupby_addition=None):
        """Constructor for this refiner"""

        #: cached value of the wrapped refiner
        self.wrapped_refiner = wrapped_refiner
        #: cached value of the folder name
        self.folder_name = folder_name
        #: cached value of the suffix for a groupby selection
        self.groupby_addition = groupby_addition

    def refine(self,
               harvesting_module,
               crops,
               tdirectory=None,
               groupby_part_name=None,
               groupby_value=None,
               *args,
               **kwds):
        """Process the change-directory"""

        folder_name = self.folder_name
        if folder_name is None:
            if groupby_value is not None:
                folder_name = "{groupby_addition}"
            else:
                folder_name = self.default_folder_name

        groupby_addition = self.groupby_addition

        if groupby_addition is None:
            groupby_addition = self.default_groupby_addition

        if groupby_part_name is None and groupby_value is None:
            groupby_addition = ""
        else:
            groupby_addition = formatter.format(groupby_addition,
                                                groupby=groupby_part_name,
                                                groupby_value=groupby_value)

        folder_name = formatter.format(folder_name,
                                       groupby_addition=groupby_addition,
                                       groupby=groupby_part_name,
                                       groupby_value=groupby_value)

        folder_name = '/'.join(root_save_name(name) for name in folder_name.split('/'))

        with root_cd(tdirectory):
            with root_cd(folder_name) as tdirectory:
                self.wrapped_refiner(harvesting_module,
                                     crops,
                                     tdirectory=tdirectory,
                                     groupby_part_name=groupby_part_name,
                                     groupby_value=groupby_value,
                                     *args,
                                     **kwds)


class ExpertLevelRefiner(Refiner):
    """Refiner for expert-level categorization"""

    def __init__(self, wrapped_refiner, above_expert_level=None, below_expert_level=None):
        """Constructor for this refiner"""

        #: cached value of the wrapped refiner
        self.wrapped_refiner = wrapped_refiner
        #: cached value of the upper range of the expert level
        self.above_expert_level = above_expert_level
        #: cached value of the lower range of the expert level
        self.below_expert_level = below_expert_level

    def refine(self, harvesting_module, crops, *args, **kwds):
        """Process the expert-level categorization"""

        above_expert_level = self.above_expert_level
        below_expert_level = self.below_expert_level

        proceed = True
        if above_expert_level is not None:
            proceed = proceed and harvesting_module.expert_level > above_expert_level

        if below_expert_level is not None:
            proceed = proceed and harvesting_module.expert_level < below_expert_level

        if proceed:
            self.wrapped_refiner(harvesting_module, crops, *args, **kwds)


# Meta refiner decorators
def groupby(refiner=None, **kwds):
    def group_decorator(wrapped_refiner):
        return GroupByRefiner(wrapped_refiner, **kwds)
    if refiner is None:
        return group_decorator
    else:
        return group_decorator(refiner)


def select(refiner=None, **kwds):
    def select_decorator(wrapped_refiner):
        return SelectRefiner(wrapped_refiner, **kwds)
    if refiner is None:
        return select_decorator
    else:
        return select_decorator(refiner)


def filter(refiner=None, **kwds):
    def filter_decorator(wrapped_refiner):
        return FilterRefiner(wrapped_refiner, **kwds)
    if refiner is None:
        return filter_decorator
    else:
        return filter_decorator(refiner)


def cd(refiner=None, **kwds):
    def cd_decorator(wrapped_refiner):
        return CdRefiner(wrapped_refiner, **kwds)
    if refiner is None:
        return cd_decorator
    else:
        return cd_decorator(refiner)


def context(refiner=None,
            above_expert_level=None, below_expert_level=None,
            folder_name=None, folder_groupby_addition=None,
            filter=None, filter_on=None,
            groupby=None, exclude_groupby=None,
            select=None, exclude=None):

    def context_decorator(wrapped_refiner):
        # Apply meta refiners in the reverse order that they shall be executed
        if exclude is not None or select is not None:
            wrapped_refiner = SelectRefiner(wrapped_refiner,
                                            select=select, exclude=exclude)

        if folder_name is not None or groupby is not None or folder_groupby_addition is not None:
            wrapped_refiner = CdRefiner(wrapped_refiner,
                                        folder_name=folder_name,
                                        groupby_addition=folder_groupby_addition)

        if groupby is not None:
            wrapped_refiner = GroupByRefiner(wrapped_refiner,
                                             by=groupby,
                                             exclude_by=exclude_groupby)

        if filter is not None or filter_on is not None:
            wrapped_refiner = FilterRefiner(wrapped_refiner,
                                            filter=filter,
                                            on=filter_on)

        if above_expert_level is not None or below_expert_level is not None:
            wrapped_refiner = ExpertLevelRefiner(wrapped_refiner,
                                                 above_expert_level=above_expert_level,
                                                 below_expert_level=below_expert_level)

        if not isinstance(wrapped_refiner, Refiner):
            wrapped_refiner = Refiner(wrapped_refiner)

        return wrapped_refiner

    if refiner is None:
        return context_decorator
    else:
        return functools.wraps(refiner)(context_decorator(refiner))


def refiner_with_context(refiner_factory):
    @functools.wraps(refiner_factory)
    def module_decorator_with_context(above_expert_level=None, below_expert_level=None,
                                      folder_name=None, folder_groupby_addition=None,
                                      filter=None, filter_on=None,
                                      groupby=None, exclude_groupby=None,
                                      select=None, exclude=None,
                                      **kwds_for_refiner_factory):

        refiner = refiner_factory(**kwds_for_refiner_factory)

        return context(refiner,
                       above_expert_level=above_expert_level, below_expert_level=below_expert_level,
                       folder_name=folder_name, folder_groupby_addition=folder_groupby_addition,
                       filter=filter, filter_on=filter_on,
                       groupby=groupby, exclude_groupby=exclude_groupby,
                       select=select, exclude=exclude)

    return module_decorator_with_context


@refiner_with_context
def save_fom(**kwds):
    return SaveFiguresOfMeritRefiner(**kwds)


@refiner_with_context
def save_histograms(**kwds):
    return SaveHistogramsRefiner(**kwds)


@refiner_with_context
def save_profiles(**kwds):
    return SaveProfilesRefiner(**kwds)


@refiner_with_context
def save_scatters(**kwds):
    return SaveScatterRefiner(**kwds)


@refiner_with_context
def save_classification_analysis(**kwds):
    return SaveClassificationAnalysisRefiner(**kwds)


@refiner_with_context
def save_pull_analysis(**kwds):
    return SavePullAnalysisRefiner(**kwds)


@refiner_with_context
def save_tree(**kwds):
    return SaveTreeRefiner(**kwds)


def select_crop_parts(crops, select=None, exclude=None):
    if select is None:
        select = []
    if exclude is None:
        exclude = []

    if isinstance(select, str):
        select = [select, ]

    if isinstance(exclude, str):
        exclude = [exclude, ]

    if isinstance(crops, collections.MutableMapping):
        part_names = list(crops.keys())

        if not select and not exclude:
            return crops

        if select:
            not_selected_part_names = [name for name in part_names if name not in select]

            # if the selection item is a callable function do not count it as not selectable yet
            select_not_in_part_names = [name for name in select
                                        if not isinstance(name, collections.Callable) and name not in part_names]
            if select_not_in_part_names:
                get_logger().warning("Cannot select %s, because they are not in crop part names %s",
                                     select_not_in_part_names, sorted(part_names))
        else:
            not_selected_part_names = []

        if exclude:
            excluded_part_names = [name for name in part_names if name in exclude]
        else:
            excluded_part_names = []

        excluded_part_names.extend(not_selected_part_names)

        # Make a shallow copy
        selected_crops = copy.copy(crops)
        for part_name in set(excluded_part_names):
            del selected_crops[part_name]

        if isinstance(select, collections.Mapping):
            # select is a rename mapping
            for part_name, new_part_name in list(select.items()):
                if isinstance(part_name, collections.Callable):
                    selected_crops[new_part_name] = part_name(**crops)
                elif part_name in selected_crops:
                    parts = selected_crops[part_name]
                    del selected_crops[part_name]
                    selected_crops[new_part_name] = parts

        return selected_crops

    else:
        raise ValueError("Unrecognised crop %s of type %s" % (crops, type(crops)))


def filter_crops(crops, filter_function, part_name=None):
    if isinstance(filter_function, np.ndarray):
        filter_indices = filter_function
    else:
        parts = crops[part_name]
        filter_indices = filter_function(parts)

    if isinstance(crops, np.ndarray):
        return crops[filter_indices]

    elif isinstance(crops, collections.MutableMapping):
        # Make a shallow copy
        filtered_crops = copy.copy(crops)
        for part_name, parts in list(crops.items()):
            filtered_crops[part_name] = parts[filter_indices]
        return filtered_crops

    else:
        raise ValueError("Unrecognised crop %s of type %s" % (crops, type(crops)))


def iter_items_sorted_for_key(crops):
    # is the type of crops is a dictionary assume, that it should be sorted
    # in all other cases the users class has to take care of the sorting
    if isinstance(crops, dict):
        keys = sorted(crops.keys())
        return ((key, crops[key]) for key in keys)
    else:
        return list(crops.items())
