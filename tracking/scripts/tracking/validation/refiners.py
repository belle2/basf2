
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


formatter = TolerateMissingKeyFormatter()


class Refiner(object):

    def __init__(self, refiner_function=None):
        self.refiner_function = refiner_function

    def __get__(self, harvesting_module, cls=None):
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
        if crops is None:
            # Decoration mode
            harvesting_module.refiners.append(self)
            return harvesting_module
        else:
            # Refining mode
            return self.refine(harvesting_module, crops, *args, **kwds)

    def refine(self, harvesting_module, *args, **kwds):
        self.refiner_function(harvesting_module, *args, **kwds)


class SaveFiguresOfMeritRefiner(Refiner):
    default_name = "{module.name}_figures_of_merit"
    default_title = "Figures of merit in {module.title}"
    default_contact = "{module.contact}"
    default_description = "Figures of merit are the {aggregation.__name__} of {keys}"
    default_check = "Check for reasonable values"
    default_key = "{aggregation.__name__}_{part_name}"

    @staticmethod
    def mean(xs):
        return np.nanmean(xs)

    # default aggregation if the mean of the parts
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

        super(SaveFiguresOfMeritRefiner, self).__init__()

        self.name = name
        self.title = title

        self.description = description
        self.check = check
        self.contact = contact

        self.key = key
        self.aggregation = aggregation

    def refine(self,
               harvesting_module,
               crops,
               tdirectory=None,
               groupby_part_name=None,
               groupby_value=None,
               **kwds):

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
            groupby=groupby_part_name,
            groupby_value=groupby_value
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


class SaveHistogramsRefiner(Refiner):
    default_name = "{module.name}_{part_name}_histogram{stacked_by_indication}{stackby}"
    default_title = "Histogram of {part_name}{stacked_by_indication}{stackby} in {module.title}"
    default_contact = "{module.contact}"
    default_description = "This is a histogram of {part_name}{stacked_by_indication}{stackby}."
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
                 stackby=""):

        super(SaveHistogramsRefiner, self).__init__()

        self.name = name
        self.title = title

        self.description = description
        self.check = check
        self.contact = contact

        self.lower_bound = lower_bound
        self.upper_bound = upper_bound
        self.bins = bins

        self.outlier_z_score = outlier_z_score
        self.allow_discrete = allow_discrete
        self.stackby = stackby

    def refine(self,
               harvesting_module,
               crops,
               tdirectory=None,
               groupby_part_name=None,
               groupby_value=None,
               **kwds):

        stackby = self.stackby
        if stackby:
            stackby_parts = crops[stackby]
        else:
            stackby_parts = None

        replacement_dict = dict(
            refiner=self,
            module=harvesting_module,
            stackby=stackby,
            stacked_by_indication="_" if stackby else "",
            groupby=groupby_part_name,
            groupby_value=groupby_value
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

            if tdirectory:
                histogram.write(tdirectory)


class SaveProfilesRefiner(Refiner):
    default_name = "{module.name}_{y_part_name}_by_{x_part_name}_profile"
    default_title = "Profile of {y_part_name} by {x_part_name} in {module.title}"
    default_contact = "{module.contact}"
    default_description = "This is a profile of {y_part_name} over {x_part_name}."
    default_check = "Check if the trend line is resonable."

    def __init__(self,
                 y,
                 x=None,
                 name=None,
                 title=None,
                 contact=None,
                 description=None,
                 check=None,
                 y_unit=None,
                 lower_bound=None,
                 upper_bound=None,
                 bins=None,
                 outlier_z_score=None,
                 fit=None,
                 skip_single_valued=False,
                 allow_discrete=False):

        super(SaveProfilesRefiner, self).__init__()

        self.name = name
        self.title = title

        self.description = description
        self.check = check
        self.contact = contact

        self.x = x
        self.y = y
        self.y_unit = y_unit

        self.lower_bound = lower_bound
        self.upper_bound = upper_bound
        self.bins = bins

        self.outlier_z_score = outlier_z_score
        self.allow_discrete = allow_discrete

        self.fit = fit

        self.skip_single_valued = skip_single_valued

    def refine(self,
               harvesting_module,
               crops,
               tdirectory=None,
               groupby_part_name=None,
               groupby_value=None,
               **kwds):

        replacement_dict = dict(
            refiner=self,
            module=harvesting_module,
            groupby=groupby_part_name,
            groupby_value=groupby_value
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

                profile_plot.profile(x_parts,
                                     y_parts,
                                     lower_bound=self.lower_bound,
                                     upper_bound=self.upper_bound,
                                     bins=self.bins,
                                     outlier_z_score=self.outlier_z_score,
                                     allow_discrete=self.allow_discrete)

                profile_plot.title = title
                profile_plot.contact = contact
                profile_plot.description = description
                profile_plot.check = check

                profile_plot.xlabel = compose_axis_label(x_part_name)
                profile_plot.ylabel = compose_axis_label(y_part_name, self.y_unit)

                if self.fit:
                    fit_method_name = 'fit_' + str(self.fit)
                    try:
                        fit_method = getattr(profile_plot, fit_method_name)
                    except:
                        profile_plot.fit(str(fit))
                    else:
                        fit_method()
                if tdirectory:
                    profile_plot.write(tdirectory)

    @staticmethod
    def has_more_than_one_value(xs):
        first_x = xs[0]
        for x in xs:
            if x != first_x:
                return True
        else:
            return False


class SaveClassificationAnalysisRefiner(Refiner):
    default_contact = "{module.contact}"

    default_truth_name = "{part_name}_truth"
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
                 unit=None):

        self.part_name = part_name
        self.contact = contact
        self.estimate_name = estimate_name
        self.truth_name = truth_name

        self.cut = cut
        self.cut_direction = cut_direction

        self.lower_bound = lower_bound
        self.upper_bound = upper_bound
        self.outlier_z_score = outlier_z_score
        self.unit = unit

    def refine(self,
               harvesting_module,
               crops,
               tdirectory=None,
               groupby_part_name=None,
               groupby_value=None,
               **kwds):

        replacement_dict = dict(
            refiner=self,
            module=harvesting_module,
            groupby=groupby_part_name,
            groupby_value=groupby_value
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

        if isinstance(estimate_name, basestring):
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
                                                             unit=self.unit)

            classification_analysis.analyse(estimates, truths)

            if tdirectory:
                classification_analysis.write(tdirectory)


class SavePullAnalysisRefiner(Refiner):
    default_name = "{module.name}_{quantity_name}"
    default_contact = "{module.contact}"
    default_title_postfix = " in {module.name}"

    default_truth_name = "{part_name}_truth"
    default_estimate_name = "{part_name}_estimate"
    default_variance_name = "{part_name}_variance"

    def __init__(self,
                 name=None,
                 contact=None,
                 title_postfix=None,
                 part_name=None,
                 truth_name=None,
                 estimate_name=None,
                 variance_name=None,
                 quantity_name=None,
                 unit=None,
                 outlier_z_score=None,
                 absolute=False):

        self.name = name
        self.contact = contact
        self.title_postfix = title_postfix

        self.part_name = part_name

        self.truth_name = truth_name
        self.estimate_name = estimate_name
        self.variance_name = variance_name

        self.quantity_name = quantity_name or part_name
        self.unit = unit

        self.outlier_z_score = outlier_z_score
        self.absolute = absolute

    def refine(self,
               harvesting_module,
               crops,
               tdirectory=None,
               groupby_part_name=None,
               groupby_value=None,
               **kwds):

        replacement_dict = dict(
            refiner=self,
            module=harvesting_module,
            groupby=groupby_part_name,
            groupby_value=groupby_value
        )

        contact = self.contact or self.default_contact
        contact = formatter.format(contact, **replacement_dict)

        name = self.name or self.default_name
        name = formatter.format(name, part_name=self.part_name, **replacement_dict)
        plot_name = name + "_{subplot_name}"

        title_postfix = self.title_postfix
        if title_postfix is None:
            title_postfix = self.default_title_postfix

        title_postfix = formatter.format(title_postfix, part_name=self.part_name, **replacement_dict)
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

        truth_name = formatter.format(truth_name, part_name=self.part_name)
        estimate_name = formatter.format(estimate_name, part_name=self.part_name)
        variance_name = formatter.format(variance_name, part_name=self.part_name)

        truths = crops[truth_name]
        estimates = crops[estimate_name]
        variances = crops[variance_name]

        pull_analysis = PullAnalysis(self.quantity_name,
                                     unit=self.unit,
                                     absolute=self.absolute,
                                     outlier_z_score=self.outlier_z_score,
                                     plot_name=plot_name,
                                     plot_title=plot_title)

        pull_analysis.analyse(truths, estimates, variances)

        pull_analysis.contact = contact

        if tdirectory:
            pull_analysis.write(tdirectory)


class SaveTreeRefiner(Refiner):
    default_name = "{module.name}_tree"
    default_title = "Tree of {module.name}"

    def __init__(self,
                 name=None,
                 title=None):
        super(SaveTreeRefiner, self).__init__()
        self.name = name
        self.title = title

    def refine(self,
               harvesting_module,
               crops,
               tdirectory=None,
               groupby_part_name=None,
               groupby_value=None,
               **kwds):

        replacement_dict = dict(
            refiner=self,
            module=harvesting_module,
            groupby=groupby_part_name,
            groupby_value=groupby_value
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
        input_value = np.zeros(1, dtype=float)

        branch_type_spec = '%s/D' % part_name
        tbranch = output_ttree.Branch(part_name, input_value, branch_type_spec)

        if output_ttree.GetNbranches() == 1:
            # On filling of the first branch we need to use the fill method of the TTree
            # For all other branches we can use.
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

    def __init__(self, wrapped_refiner, filter=None, on=None):
        self.wrapped_refiner = wrapped_refiner

        if filter is None:
            self.filter = np.nonzero
        else:
            self.filter = filter

        self.on = on

    def refine(self, harvesting_module, crops, *args, **kwds):
        filtered_crops = filter_crops(crops, self.filter, part_name=self.on)
        self.wrapped_refiner(harvesting_module, filtered_crops, *args, **kwds)


class SelectRefiner(Refiner):

    def __init__(self, wrapped_refiner, select=[], exclude=[]):
        self.wrapped_refiner = wrapped_refiner
        self.select = select
        self.exclude = exclude

    def refine(self, harvesting_module, crops, *args, **kwds):
        selected_crops = select_crop_parts(crops, select=self.select, exclude=self.exclude)
        self.wrapped_refiner(harvesting_module, selected_crops, *args, **kwds)


class GroupByRefiner(Refiner):
    default_exclude_by = True

    def __init__(self,
                 wrapped_refiner,
                 by=[],
                 folder_name=None,
                 exclude_by=None):

        self.wrapped_refiner = wrapped_refiner
        self.by = by
        self.exclude_by = exclude_by if exclude_by is not None else self.default_exclude_by

    def refine(self,
               harvesting_module,
               crops,
               groupby_part_name=None,
               groupby_value=None,
               *args,
               **kwds):

        by = self.by

        # A single name to do the group by
        if isinstance(by, basestring) or by is None:
            part_name = by
            # Wrap it into a list an continue with the general case
            by = [part_name, ]

        for part_name in by:
            if part_name is None:
                # Using the empty sting as groupby_value to indicate that all values have been selected
                value = ""
                self.wrapped_refiner(harvesting_module,
                                     crops,
                                     groupby_part_name=part_name,
                                     groupby_value=value,
                                     *args,
                                     **kwds)

            else:
                groupby_parts = crops[part_name]

                # Exclude the groupby variable if desired
                selected_crops = select_crop_parts(crops, exclude=part_name if self.exclude_by else None)

                unique_values = np.unique(groupby_parts)
                for value in unique_values:
                    if np.isnan(value):
                        indices_for_value = np.isnan(groupby_parts)
                    else:
                        indices_for_value = groupby_parts == value

                    filtered_crops = filter_crops(selected_crops, indices_for_value)

                    self.wrapped_refiner(harvesting_module,
                                         filtered_crops,
                                         groupby_part_name=part_name,
                                         groupby_value=value,
                                         *args,
                                         **kwds)


class CdRefiner(Refiner):
    # Folder name to be used if a groupby selection is active.
    default_folder_name = ""
    default_groupby_addition = "groupby_{groupby}_{groupby_value}"

    def __init__(self,
                 wrapped_refiner,
                 folder_name=None,
                 groupby_addition=None):

        self.wrapped_refiner = wrapped_refiner
        self.folder_name = folder_name
        self.groupby_addition = groupby_addition

    def refine(self,
               harvesting_module,
               crops,
               tdirectory=None,
               groupby_part_name=None,
               groupby_value=None,
               *args,
               **kwds):

        folder_name = self.folder_name
        if folder_name is None:
            if groupby_value is not None:
                folder_name = "{groupby_addition}"
            else:
                folder_name = self.default_folder_name

        groupby_addition = self.groupby_addition
        if groupby_addition is None:
            groupby_addition = self.default_groupby_addition

        groupby_addition = formatter.format(groupby_addition,
                                            groupby=groupby_part_name,
                                            groupby_value=groupby_value)

        folder_name = formatter.format(folder_name,
                                       groupby_addition=groupby_addition,
                                       groupby=groupby_part_name,
                                       groupby_value=groupby_value)

        if folder_name:
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

    def __init__(self, wrapped_refiner, above_expert_level=None, below_expert_level=None):
        self.wrapped_refiner = wrapped_refiner
        self.above_expert_level = above_expert_level
        self.below_expert_level = below_expert_level

    def refine(self, harvesting_module, crops, *args, **kwds):
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
    def filter_decorator(wrapped_refiner):
        return CdRefiner(wrapped_refiner, **kwds)
    if refiner is None:
        return filter_decorator
    else:
        return filter_decorator(refiner)


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
        return context_decorator(refiner)


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
def save_classification_analysis(**kwds):
    return SaveClassificationAnalysisRefiner(**kwds)


@refiner_with_context
def save_pull_analysis(**kwds):
    return SavePullAnalysisRefiner(**kwds)


@refiner_with_context
def save_tree(**kwds):
    return SaveTreeRefiner(**kwds)


def select_crop_parts(crops, select=[], exclude=[]):
    if isinstance(select, basestring):
        select = [select, ]

    if isinstance(exclude, basestring):
        exclude = [exclude, ]

    if isinstance(crops, collections.MutableMapping):
        part_names = list(crops.keys())

        if not select and not exclude:
            return crops

        if select:
            not_selected_part_names = [name for name in part_names if name not in select]

            # if the selection item is a callable function do not count it as not selectable yet
            select_not_in_part_names = [name for name in select
                                        if not callable(name) and name not in part_names]
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
            for part_name, new_part_name in select.items():
                if callable(part_name):
                    selected_crops[new_part_name] = part_name(**crops)
                elif part_name in selected_crops:
                    parts = selected_crops[part_name]
                    del selected_crops[part_name]
                    selected_crops[new_part_name] = parts

        return selected_crops

    else:
        raise ValueError("Unrecognised crop %s of type %s" % (crop, type(crop)))


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
        for part_name, parts in crops.items():
            filtered_crops[part_name] = parts[filter_indices]
        return filtered_crops

    else:
        raise ValueError("Unrecognised crop %s of type %s" % (crop, type(crop)))


def iter_items_sorted_for_key(crops):
    # is the type of crops is a dictionary assume, that it should be sorted
    # in all other cases the users class has to take care of the sorting
    if isinstance(crops, dict):
        keys = sorted(crops.keys())
        return ((key, crops[key]) for key in keys)
    else:
        return crops.items()
