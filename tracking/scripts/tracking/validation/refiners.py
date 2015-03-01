
import functools
import numpy as np
import collections
import copy

from tracking.validation.plot import ValidationPlot
from tracking.validation.fom import ValidationFiguresOfMerit

from tracking.validation.utilities import root_cd

import ROOT


class Refiner(object):

    def __init__(self, refiner_function=None):
        self.refiner_function = refiner_function

    def __get__(self, harvester_module, cls=None):
        if harvester_module is None:
            # Class access
            return self
        else:
            # Instance access
            refine = self.refine

            def bound_call(*args, **kwds):
                return refine(harvester_module, *args, **kwds)
            return bound_call

    def __call__(self, harvester_module, crops=None, *args, **kwds):
        if crops is None:
            # Decoration mode
            harvester_module.refiners.append(self)
            return harvester_module
        else:
            # Refining mode
            return self.refine(harvester_module, crops, *args, **kwds)

    def refine(self, harvester_module, *args, **kwds):
        self.refiner_function(harvester_module, *args, **kwds)


class SaveFiguresOfMeritRefiner(Refiner):

    def __init__(self,
                 name=None,
                 aggregation=lambda xs: np.mean,
                 title="",
                 prefix="",
                 contact="",
                 description="",
                 check="",
                 fill=None):

        self._aggregation = aggregation
        self.name = name
        self.title = title
        self.prefix = prefix

        self.contact = contact
        self.description = description
        self.check = check

        if isinstance(prefix, basestring):
            self.prefix = prefix
        else:
            self.format_part_name = prefix

        if fill:
            self.fill = fill

    def aggregation(self, xs):
        return self._aggregation(xs)

    def refine(self, harvesting_module, crops, tdirectory=None, **kwds):
        name = self.name or '%s_figures_of_merit' % harvesting_module.name

        figures_of_merit = ValidationFiguresOfMerit(name,
                                                    contact=self.contact,
                                                    description=self.description,
                                                    check=self.check,
                                                    title=self.title)

        self.fill(figures_of_merit, crops)
        figures_of_merit.write(tdirectory)

    def format_part_name(self, part_name):
        return self.prefix + part_name

    def fill(self, figures_of_merit, crops):
        for part_name, parts in iter_crop_part_names_and_parts(crops):
            fom_name = self.format_part_name(part_name)
            figures_of_merit[fom_name] = self.aggregation(parts)


class SaveHistogramsRefiner(Refiner):

    def __init__(self, outlier_z_score=None, allow_discrete=False, stackby=""):
        super(SaveHistogramsRefiner, self).__init__()
        self.outlier_z_score = outlier_z_score
        self.allow_discrete = allow_discrete
        self.stackby = stackby

    def refine(self, harvesting_module, crops, tdirectory=None, **kwds):
        stackby = self.stackby
        if stackby:
            stackby_parts = get_crop_part(crops, stackby)
            stackby_name_postfix = "_" + stackby
            stackby_title_postfix = " stacked by " + stackby

        else:
            stackby_parts = None
            stackby_name_postfix = ""
            stackby_title_postfix = ""

        for part_name, parts in iter_crop_part_names_and_parts(crops, default_name=harvesting_module.name):
            histogram_name = "%s_histogram%s" % (part_name, stackby_name_postfix)
            histogram = ValidationPlot(histogram_name)
            histogram.hist(parts,
                           outlier_z_score=self.outlier_z_score,
                           allow_discrete=self.allow_discrete,
                           stackby=stackby_parts)
            histogram.title = "Histogram of %s%s" % (part_name, stackby_name_postfix)
            histogram.xlabel = part_name

            if tdirectory:
                histogram.write(tdirectory)


class SaveTreeRefiner(Refiner):

    def refine(self, harvesting_module, crops, tdirectory=None, **kwds):
        with root_cd(tdirectory):
            ttree_name = "%s_tree" % harvesting_module.name
            ttree_title = "Tree of %s" % harvesting_module.name

            output_ttree = ROOT.TTree(ttree_name, ttree_title)
            for part_name, parts in iter_crop_part_names_and_parts(crops, default_name=harvesting_module.name):
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

    def __init__(self, wrapped_refiner, filter, on=None):
        self.wrapped_refiner = wrapped_refiner
        self.filter = filter
        self.on = on

    def refine(self, harvester_module, crops, *args, **kwds):
        filtered_crops = filter_crops(crops, self.filter, part_name=self.on)
        self.wrapped_refiner(harvester_module, filtered_crops, *args, **kwds)


class SelectRefiner(Refiner):

    def __init__(self, wrapped_refiner, select=[], exclude=[]):
        self.wrapped_refiner = wrapped_refiner
        self.select = select
        self.exclude = exclude

    def refine(self, harvester_module, crops, *args, **kwds):
        selected_crops = select_crop_parts(crops, select=self.select, exclude=self.exclude)
        self.wrapped_refiner(harvester_module, selected_crops, *args, **kwds)


class GroupByRefiner(Refiner):
    default_folder_template = "groupby_{name}_{value}"

    def __init__(self, wrapped_refiner, by=[], folder_template=default_folder_template):
        self.wrapped_refiner = wrapped_refiner
        self.by = by
        self.folder_template = folder_template

    def refine(self, harvester_module, crops, tdirectory, *args, **kwds):
        by = self.by
        folder_template = self.folder_template

        # A single name to do the group by
        if isinstance(by, basestring) or by is None:
            part_name = by
            # Wrap it into a list an continue with the general case
            by = [part_name, ]

        for part_name in by:
            if part_name is None:
                if folder_template:
                    folder_name = folder_template.format(name=part_name, value="")
                    folder_title = "Group by on {name} for =={value}".format(name="nothing", value="*")
                    groupby_tdirectory = tdirectory.mkdir(folder_name, folder_title)
                else:
                    groupby_tdirectory = tdirectory

                self.wrapped_refiner(harvester_module, crops, tdirectory=groupby_tdirectory, *args, **kwds)

            else:
                groupby_parts = get_crop_part(crops, part_name)

                unique_values = np.unique(groupby_parts)
                for value in unique_values:
                    if np.isnan(value):
                        indices_for_value = np.isnan(groupby_parts)
                    else:
                        indices_for_value = groupby_parts == value

                    filtered_crops = filter_crops(crops, indices_for_value)

                    if folder_template:
                        folder_name = folder_template.format(name=part_name, value=value)
                        folder_title = "Group by on {name} for =={value}".format(name=part_name, value=value)
                        groupby_tdirectory = tdirectory.mkdir(folder_name, folder_title)
                    else:
                        groupby_tdirectory = tdirectory

                    self.wrapped_refiner(harvester_module, filtered_crops, tdirectory=groupby_tdirectory, *args, **kwds)


class CdRefiner(Refiner):

    def __init__(self, wrapped_refiner, folder_name=""):
        self.wrapped_refiner = wrapped_refiner
        self.folder_name = folder_name

    def refine(self, harvester_module, crops, tdirectory, *args, **kwds):
        with root_cd(tdirectory):
            with root_cd(self.folder_name) as tdirectory:
                self.wrapped_refiner(harvester_module, crops, tdirectory, *args, **kwds)


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


def refiner_with_context(refiner_factory):
    @functools.wraps(refiner_factory)
    def module_decorator_with_context(folder_name=None,
                                      filter=None, filter_on=None,
                                      groupby=None, groupby_folder_template=GroupByRefiner.default_folder_template,
                                      select=None, exclude=None,
                                      **kwds_for_refiner_factory):

        refiner = refiner_factory(**kwds_for_refiner_factory)

        # Apply meta refiners in the reverse order that they shall be executed
        if exclude is not None or select is not None:
            refiner = SelectRefiner(refiner, select=select, exclude=exclude)

        if groupby is not None:
            refiner = GroupByRefiner(refiner, by=groupby, folder_template=groupby_folder_template)

        if filter is not None:
            refiner = FilterRefiner(refiner, filter=filter, on=filter_on)

        if folder_name is not None:
            refiner = CdRefiner(refiner, folder_name=folder_name)

        return refiner

    return module_decorator_with_context


@refiner_with_context
def save_histograms(**kwds_for_histograms):
    return SaveHistogramsRefiner(**kwds_for_histograms)


@refiner_with_context
def save_tree(**kwds_for_trees):
    return SaveTreeRefiner(**kwds_for_trees)


@refiner_with_context
def save_fom(**kwds_for_histograms):
    return SaveFiguresOfMeritRefiner(**kwds_for_histograms)


def iter_crop_part_names_and_parts(crops, default_name="value"):
    if isinstance(crops, np.ndarray):
        part_name = default_name
        parts = crops
        yield part_name, parts

    elif isinstance(crops, collections.MutableMapping):
        for part_name, parts in crops.items():
            yield part_name, parts

    elif hasattr(crops, "_asdict"):
        # Special case for named tuples
        for part_name, parts in crops._asdict().items():
            yield part_name, parts

    elif isinstance(crops, collections.Sequence):
        for i_part, parts in enumerate(crops):
            part_name = default_name + str(i_part)
            yield part_name, parts

    else:
        raise ValueError("Unrecognised crop %s of type %s" % (crop, type(crop)))


def get_crop_part(crops, part_name=None):
    if part_name is None:
        return crops

    elif isinstance(crops, np.ndarray):
        raise ValueError("Cannot get a part from a crop with only a single part.")

    else:
        try:
            part = crops[part_name]
            return part
        except KeyError, IndexError:
            return getattr(crops, part_name)


def select_crop_parts(crops, select=[], exclude=[]):
    if isinstance(select, basestring):
        select = [select, ]

    if isinstance(exclude, basestring):
        exclude = [exclude, ]

    if isinstance(crops, np.ndarray):
        raise ValueError("Cannot selection from a crop with only a single part.")

    elif isinstance(crops, collections.MutableMapping):
        part_names = list(crops.keys())

        if select:
            not_selected_part_names = [name for name in part_names if name not in select]
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
        return selected_crops

    elif isinstance(crop, collections.Sequence):
        raise NotImplementedError("Cannot selection from a crop made from a tuple of parts.")
    else:
        raise ValueError("Unrecognised crop %s of type %s" % (crop, type(crop)))


def filter_crops(crops, filter_function, part_name=None):
    if isinstance(filter_function, np.ndarray):
        filter_indices = filter_function
    else:
        parts = get_crop_part(crops, part_name)
        filter_indices = filter_function(parts)

    if isinstance(crops, np.ndarray):
        return crops[filter_indices]

    elif isinstance(crops, collections.MutableMapping):
        # Make a shallow copy
        filtered_crops = copy.copy(crops)
        for part_name, parts in crops.items():
            filtered_crops[part_name] = parts[filter_indices]
        return filtered_crops

    elif isinstance(crop, collections.Sequence):
        raise NotImplementedError("Cannot filter from a crop made from a tuple of parts.")

    else:
        raise ValueError("Unrecognised crop %s of type %s" % (crop, type(crop)))
