import json
import enum
import functools


class JsonBase:
    pass


class Revision(JsonBase):

    def __init__(self, label, creation_date, packages=None):
        self.label = label
        self.creation_date = creation_date
        self.most_recent = False

        self.packages = [] if (packages is None) else packages


class Revisions(JsonBase):

    def __init__(self, revisions):
        self.revisions = revisions


class Script(JsonBase):

    def __init__(self, name, path, status, log_url=None, return_code=None):
        self.name = name
        self.path = path
        self.status = status
        self.log_url = log_url
        self.return_code = return_code


class PlotFile(JsonBase):

    def __init__(self, package, title, rootfile):
        self.package = package
        self.title = title
        self.rootfile = rootfile


class Plot(JsonBase):

    def __init__(self, is_expert=False, description=None, check=None):
        self.is_expert = is_expert
        self.description = description
        self.check = check


class NTuple(JsonBase):

    def __init__(self, is_expert=False, description=None, check=None):
        self.is_expert = is_expert
        self.description = description
        self.check = check


class Package(JsonBase):

    def __init__(self, name, plotfiles=[], scriptfiles=[], fail_count=0):
        self.name = name
        self.plotfiles = plotfiles
        self.scriptfiles = scriptfiles
        self.visible = True
        self.fail_count = fail_count


class ComparisonState(enum.Enum):
    NotCompared = "not_compared"
    NotSupported = "not_supported"
    FailureTechnical = "technical_failure"
    FailureStastical = "statistical_failure"
    Equal = "equal"


class ComparisonResult(JsonBase):

    def __init__(self, state, chi2):
        self.state = state
        self.chi2 = chi2


class ComparisonPlotFile(PlotFile):

    def __init__(self, package, title, rootfile, compared_revisions=None, plots=[], ntuples=[]):
        super().__init__(package, title, rootfile)
        self.compared_revision = compared_revisions
        self.plots = plots
        self.ntuples = ntuples

        self.comparison_error = len([plt for plt in self.plots if plt.comparison_result == "error"])
        self.comparison_warning = len([plt for plt in self.plots if plt.comparison_result == "warning"])


class ComparisonPlot(Plot):

    def __init__(
            self,
            title,
            comparison_result=None,
            png_filename=None,
            pdf_filename=None,
            contact=None,
            description=None,
            check=None,
            is_expert=None,
            plot_path=None,
            comparison_pvalue=None,
            comparison_text=None):
        # todo: move more into the base class
        super().__init__(is_expert=is_expert, description=description, check=check)
        self.title = title
        self.comparison_result = comparison_result
        self.comparison_text = comparison_text
        self.comparison_pvalue = comparison_pvalue
        self.png_filename = png_filename
        self.pdf_filename = pdf_filename
        self.contact = contact
        self.plot_path = plot_path


class ComparisonNTuple(NTuple):

    def __init__(
            self,
            title,
            contact=None,
            description=None,
            check=None,
            is_expert=None,
            json_file_path=None):
        # todo: move more into the base class
        super().__init__(is_expert=is_expert, description=description, check=check)
        self.title = title
        self.contact = contact
        self.json_file_path = json_file_path


class ComparisonPackage(Package):

    def __init__(self, name, plotfiles=[], scriptfiles=[], ntuplefiles=[]):
        super().__init__(name, plotfiles=plotfiles, scriptfiles=scriptfiles)

        # compute from the plotfiles ... and flatten list
        self.comparison_error = sum([pf.comparison_error for pf in plotfiles])
        self.comparison_warning = sum([pf.comparison_warning for pf in plotfiles])


class ComparisonRevision(Revision):

    def __init__(self, label, creation_date=None, color=None):
        # todo: creation_date
        super().__init__(label, creation_date=None)
        self.color = color


class Comparison(JsonBase):
    """ Contains information and plots generated for comparisons
    between revisions"""

    def __init__(self, revisions=[], packages=[]):
        self.revisions = revisions
        self.packages = packages
        sorted_revs = sorted(revisions, key=lambda x: x.label)
        self.label = functools.reduce(lambda x, y: x + "_" + y.label, sorted_revs, "")[1:]


def dump(file_name, obj):
    with open(file_name, 'w+') as f:
        json.dump(dump_rec(obj), f, sort_keys=True, indent=4)


def dumps(obj):
    kk = dump_rec(obj)
    return json.dumps(kk, sort_keys=True, indent=4)


def dump_rec(top_object):
    """
    Recursive generating of dictionary from a tree
    of JsonBase objects
    """

    this_dict = {}
    # iterate through the dictionary of the top object
    for (k, v) in top_object.__dict__.items():

        # list which needs special handing ?
        if isinstance(v, list):
            print("list " + str(type(v)) + " - " + str(v))
            obj_list = []
            for it in v:
                # one of our object's in the list, which needs
                # special treatment?
                print("item  " + str(type(it)))
                if isinstance(it, JsonBase):
                    # yen, recurse in to the object
                    obj_list.append(dump_rec(it))
                else:
                    # no, just add value to list
                    obj_list.append(it)

            # store compiled list with corresponding key
            this_dict[k] = obj_list
        # one of our objets, which might be nested and
        # needs special treatment ?
        elif isinstance(v, JsonBase):
            this_dict[k] = dump_rec(v)
        # treat enum classes
        elif isinstance(v, enum.Enum):
            this_dict[k] = v.value
        # regular value, just store with correct key
        else:
            this_dict[k] = v
    return this_dict
