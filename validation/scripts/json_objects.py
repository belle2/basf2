import json
import enum
import functools


class JsonBase:
    """!
    Base object for all json-serializable objects of the validation suite
    """
    pass


class Revision(JsonBase):
    """!
    Contains information about a specific revsion

    @var label: label (or tag) used to display this revision
    @var creation_date: date when the validation output of this revision
                        was created, as datetime object
    @var most_recent: is this the most recent revision in the list this
                      revision is contained ?
    @var packages: list of packages contained in this revision
    """

    def __init__(self, label, creation_date, packages=None):
        self.label = label
        self.creation_date = creation_date
        self.most_recent = False

        self.packages = [] if (packages is None) else packages


class Revisions(JsonBase):
    """!
    Container for a list of revisions

    @var revisions: the actual list
    """

    def __init__(self, revisions):
        self.revisions = revisions


class Script(JsonBase):
    """!
    Contains information about a script and its execution output

    @var name: the name of the script file
    @var path: path the script file is located
    @var status: Output status of the script execution, can be one
                 of the strings  "failed", "finished", "running",
                 "skipped", "waiting"
    @var log_url: location where the log output of the script execution
                  can be found
    @var return_code: integer which is the return code of the script
                      execution
    """

    def __init__(self, name, path, status, log_url=None, return_code=None):
        self.name = name
        self.path = path
        self.status = status
        self.log_url = log_url
        self.return_code = return_code


class PlotFile(JsonBase):
    """!
    Wrapper for a file containing a set of plots, only
    root files up to now

    @var package: name of the package which created this file
    @var title: Display name of this file
    @var rootfile: filename of the root file
    @var plots: list of plots which are contained inside this plot file
    """

    def __init__(self, package, title, rootfile, plots):
        self.package = package
        self.title = title
        self.rootfile = rootfile
        self.plots = plots


class Plot(JsonBase):
    """!
    Wrapper for one specfic plot.

    @var is_expert: true if this is marked as an expert-only plot
    @var description: telling description for this plot
    @var check: What should be checked for in this plot ?
    @var contact: Who is the contact person for this plot ?
    """

    def __init__(self, is_expert=False, description=None, check=None, contact=None):
        self.is_expert = is_expert
        self.description = description
        self.check = check
        self.contact = contact


class NTuple(JsonBase):
    """!
    Wrapper for NTuple lists. This is not a graphical plot, but a list of values

    @var is_expert: true if this is marked as an expert-only ntuple list
    @var description: telling description for this ntuple
    @var check: What should be checked for in this ntuple ?
    """

    def __init__(self, is_expert=False, description=None, check=None):
        self.is_expert = is_expert
        self.description = description
        self.check = check


class Package(JsonBase):
    """!
    One high-level package of the validation suites which contains a set of scripts
    and output plot files

    @var name: name of the package
    @var plotfiles: list of plotfiles which were produced by the scripts in this package
    @var scriptfiles: scripts which were run or skipped as this package was executed
    @var visible: true if this package is displayed on the default validation website
    @var fail_count: contains the number how many scripts failed to execute without error
    """

    def __init__(self, name, plotfiles=[], scriptfiles=[], fail_count=0):
        self.name = name
        self.plotfiles = plotfiles
        self.scriptfiles = scriptfiles
        self.visible = True
        self.fail_count = fail_count


class ComparisonState(enum.Enum):
    """!
    Enum to classify the comparison result of two validation plots

    @cvar NotCompared: no comparison was performed, for example because
                       comparison was disabled
    @cvar NotSupportd: comparison is not supported for the supplied type of
                       root objects
    @cvar FailureTechnical: comparison failed because of a technical problem, for
                            example because of a differing bin count
    @cvar FailureStastical: The two distributions did not comply with the allowed
                            differences
    @cvar Equal: The two distrubtions were equal in the envelope given by the
                 comparison configuration of this plot
    """

    NotCompared = "not_compared"
    NotSupported = "not_supported"
    FailureTechnical = "technical_failure"
    FailureStastical = "statistical_failure"
    Equal = "equal"


class ComparisonResult(JsonBase):
    """!
    Contains the comparison result of two plots

    @var state: a string containing a describtion of the comparison's outcome
    @var chi2: the chi2 value computed during the comparison
    """

    def __init__(self, state, chi2):
        self.state = state
        self.chi2 = chi2


class ComparisonPlotFile(PlotFile):
    """!
    Contains information about a file containing pltos and the comparison which have
    been performed for the content of this file

    @var compared_revision: label of the revision which were used in this comparison
    @var ntuples: the ntuples which were compared
    @var comparison_error: the number of failed comparisons in this file
    @var comparison_warning: the number of comparisons which resulted a warning
    """

    def __init__(self, package, title, rootfile, compared_revisions=None, plots=[], ntuples=[]):
        super().__init__(package, title, rootfile, plots)
        self.compared_revision = compared_revisions
        self.ntuples = ntuples

        self.comparison_error = len([plt for plt in self.plots if plt.comparison_result == "error"])
        self.comparison_warning = len([plt for plt in self.plots if plt.comparison_result == "warning"])


class ComparisonPlot(Plot):
    """!
    One indidividual plot including its comparison outcome.

    @var title: tile used to display this plot
    @var comparison_result: text string for the comparison outcome
    @var comparison_text: verbose text describing the outcome of the comparison
    @var comparison_pvalue: the p-value computed during the comparison
    @var png_filename: the filename of the png file plotted with the comparison graphs
    @var pdf_filename: the filename of the pdf file plotted with the comparison graphs
    @var plot_path: path were the png and pdf files are located
    """

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
        super().__init__(is_expert=is_expert, description=description, check=check, contact=contact)
        self.title = title
        self.comparison_result = comparison_result
        self.comparison_text = comparison_text
        self.comparison_pvalue = comparison_pvalue
        self.png_filename = png_filename
        self.pdf_filename = pdf_filename
        self.plot_path = plot_path


class ComparisonNTuple(NTuple):
    """!
    Comparison outcome for NTuples

    @var title: Text used as title for the ntuple item
    @var contact: name of contact person
    @var json_file_path: path to the json file which contains the individual numbers of the ntuple

    """

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
    """!
    Informtion about a Package which was used in a comparison operation

    @var comparison_error: the number of failed comparisons in this package
    @var comparison_warning: the number of comparisons which resulted a warning
    """

    def __init__(self, name, plotfiles=[], scriptfiles=[], ntuplefiles=[]):
        super().__init__(name, plotfiles=plotfiles, scriptfiles=scriptfiles)

        # compute from the plotfiles ... and flatten list
        self.comparison_error = sum([pf.comparison_error for pf in plotfiles])
        self.comparison_warning = sum([pf.comparison_warning for pf in plotfiles])


class ComparisonRevision(Revision):
    """!
    Revision information enriched by the information gained during
    a comparison.

    @var color: the color which was used for this revision in the
                comparison plots

    """

    def __init__(self, label, creation_date=None, color=None):
        # todo: creation_date
        super().__init__(label, creation_date=None)
        self.color = color


class Comparison(JsonBase):
    """! Contains information and plots generated for comparisons
    between revisions

    @var revisions: the list of revisions used in this comparison
    @var packages: the list of packages looked at in this comparison
    @var label: the unique label of this comparison
    """

    def __init__(self, revisions=[], packages=[]):
        self.revisions = revisions
        self.packages = packages
        sorted_revs = sorted(revisions, key=lambda x: x.label)
        self.label = functools.reduce(lambda x, y: x + "_" + y.label, sorted_revs, "")[1:]


def dump(file_name, obj):
    """!
    Output a tree of objects into a json file
    """

    with open(file_name, 'w+') as f:
        json.dump(dump_rec(obj), f, sort_keys=True, indent=4)


def dumps(obj):
    """!
    Convert a tree of python objects into a json file
    """

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
