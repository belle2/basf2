import json
import enum
import functools

# todo: shouldn't I call super().__init__() or similar to make sure that I
# execute code from mother classes?? This seems to only have been done for
# some of the subclasses here... /klieret

"""
Define datatypes for later serialization by json
"""

# todo: write a short overview over the many classes and their relationships here /klieret

# ==============================================================================
# Data classes
# ==============================================================================


class JsonBase:

    """
    Base object for all json-serializable objects of the validation suite
    """


class Revision(JsonBase):

    """
    Contains information about a specific revision
    """

    def __init__(
        self,
        label,
        git_hash=None,
        creation_date=None,
        packages=None,
        creation_timezone=None,
    ):
        """
        Create a new Revision object and fill all members
        """

        #: label (or tag) used to display this revision
        self.label = label

        #: date when the validation output of this revision
        #  was created, as datetime object
        self.creation_date = creation_date

        #: timezone used by the creation date
        self.creation_timezone = creation_timezone

        #: The git commit hash which has the HEAD while the validation
        # scripts were executed
        self.git_hash = git_hash

        #: is this the most recent revision in the list this
        #  revision is contained ?
        self.most_recent = False

        #: list of packages contained in this revision
        self.packages = [] if (packages is None) else packages


class Revisions(JsonBase):

    """
    Container for a list of revisions
    """

    def __init__(self, revisions):
        """
        Create a new Revisions object and fill all members
        """

        #: the actual list
        self.revisions = revisions


class Script(JsonBase):

    """
    Contains information about a script and its execution output
    """

    def __init__(self, name, path, status, log_url=None, return_code=None):
        """
        Create a new Script object and fill all members
        """

        #: the name of the script file
        self.name = name
        #: path the script file is located
        self.path = path
        #: Output status of the script execution, can be one
        #  of the strings  "failed", "finished", "running",
        #  "skipped", "waiting"
        self.status = status
        #: location where the log output of the script execution
        #  can be found
        self.log_url = log_url
        #: integer which is the return code of the script
        #  execution
        self.return_code = return_code


class PlotFile(JsonBase):

    """
    Wrapper for a file containing a set of plots, only
    root files up to now
    """

    def __init__(self, package, title, rootfile, plots, description=""):
        """
        Create a new PlotFile object and fill all members
        """

        #: name of the package which created this file
        self.package = package
        #: Display name of this file
        self.title = title
        #: filename of the root file
        self.rootfile = rootfile
        #: list of plots which are contained inside this plot file
        self.plots = plots
        #: Description of plot file
        self.description = description
        #: Number of shifter plots
        self.n_shifter_plots = sum([not plot.is_expert for plot in self.plots])


class Plot(JsonBase):

    """
    Wrapper for one specfic plot.
    """

    def __init__(
        self,
        is_expert=False,
        description=None,
        check=None,
        contact=None,
        width=None,
        height=None,
    ):
        """
        Create a new Plot object and fill all members
        """

        #: true if this is marked as an expert-only plot
        self.is_expert = is_expert
        #: telling description for this plot
        self.description = description
        #: What should be checked for in this plot ?
        self.check = check
        #: Who is the contact person for this plot ?
        self.contact = contact
        #: width of the plot in pixels
        self.width = width
        #: height of the plot in pixels
        self.height = height


class NTuple(JsonBase):

    """
    Wrapper for NTuple lists. This is not a graphical plot, but a list of
    values
    """

    def __init__(self, is_expert=False, description=None, check=None):
        """
        Create a new NTuple object and fill all members
        """

        #: true if this is marked as an expert-only ntuple list
        self.is_expert = is_expert
        #: telling description for this ntuple
        self.description = description
        #: what should be checked for in this ntuple ?
        self.check = check


class HtmlContent(JsonBase):

    """
    Wrapper for user HTML Content. This is not a graphical plot but HTML
    code which will be directly output on the validation website.

    """

    def __init__(self, is_expert=False, description=None, check=None):
        """
        Create a new NTuple object and fill all members
        """

        #: true if this is marked as an expert-only HTML code
        self.is_expert = is_expert
        #: telling description for this HTML code
        self.description = description
        #: what should be checked for in this HTML code
        self.check = check


class Package(JsonBase):

    """
    One high-level package of the validation suites which contains a set of
    scripts and output plot files
    """

    def __init__(self, name, plotfiles=None, scriptfiles=None, fail_count=0):
        """
        Create a new NTuple object and fill all members
        """

        if not plotfiles:
            plotfiles = []
        if not scriptfiles:
            scriptfiles = []

        #: name of the package
        self.name = name
        #: list of plotfiles which were produced by the scripts in this package
        self.plotfiles = plotfiles
        #: scripts which were run or skipped as this package was executed
        self.scriptfiles = scriptfiles
        #: true if this package is displayed on the default validation website
        self.visible = True
        #: contains the number how many scripts failed to execute with error
        self.fail_count = fail_count


class ComparisonState(enum.Enum):

    """
    Enum to classify the comparison result of two validation plots
    """

    #: no comparison was performed, for example because
    #  comparison was disabled
    NotCompared = "not_compared"
    #: comparison is not supported for the supplied type of
    #  root objects
    NotSupported = "not_supported"
    #: comparison failed because of a technical problem, for
    #  example because of a differing bin count
    FailureTechnical = "technical_failure"
    #: The two distributions did not comply with the allowed
    #  differences
    FailureStastical = "statistical_failure"
    #: The two distrubtions were equal in the envelope given by the
    #  comparison configuration of this plot
    Equal = "equal"


class ComparisonResult(JsonBase):

    """
    Contains the comparison result of two plots
    """

    def __init__(self, state, chi2):
        """
        Create a new ComparisonResult object and fill all members
        """

        #: a string containing a description of the comparison's outcome
        self.state = state
        #: the chi2 value computed during the comparison
        self.chi2 = chi2


class ComparisonPlotFile(PlotFile):

    """
    Contains information about a file containing plots and the comparison which
    have been performed for the content of this file
    """

    def __init__(
        self,
        package,
        title,
        rootfile,
        compared_revisions=None,
        plots=None,
        has_reference=False,
        ntuples=None,
        html_content=None,
        description=None,
    ):
        """
        Create a new ComparisonPlotFile object and fill all members
        """

        if not plots:
            plots = []
        if not ntuples:
            ntuples = []
        if not html_content:
            html_content = []

        super().__init__(
            package, title, rootfile, plots, description=description
        )
        #: label of the revision which were used in this comparison
        self.compared_revision = compared_revisions
        #: the ntuples which were compared
        self.ntuples = ntuples
        #: user's html content
        self.html_content = html_content

        #: true if a reference file is available for this plot file
        self.has_reference = has_reference

        #: the number of failed comparisons in this file
        self.comparison_error = len(
            [plt for plt in self.plots if plt.comparison_result == "error"]
        )
        #: the number of failed comparisons of shifter plots in this file
        self.comparison_error_shifter = len(
            [
                plt
                for plt in self.plots
                if (not plt.is_expert) and plt.comparison_result == "error"
            ]
        )
        #: the number of comparisons which resulted in a warning
        self.comparison_warning = len(
            [plt for plt in self.plots if plt.comparison_result == "warning"]
        )
        #: the number of comparisons of shifter plots in this file which
        #: resulted in a warning
        self.comparison_warning_shifter = len(
            [
                plt
                for plt in self.plots
                if (not plt.is_expert) and plt.comparison_result == "warning"
            ]
        )

        #: Number of shifter ntuples
        self.n_shifter_ntuples = sum(
            [not tuple.is_expert for tuple in self.ntuples]
        )

        #: Show to shifter, i.e. is there at least one non-expert plot?
        self.show_shifter = bool(
            self.n_shifter_plots or self.n_shifter_ntuples or self.html_content
        )


class ComparisonPlot(Plot):

    """
    One individual plot including its comparison outcome.
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
        comparison_text=None,
        height=None,
        width=None,
        warnings=None,
    ):
        """
        Create a new ComparisonPlot object and fill all members
        """

        # todo: move more into the base class
        super().__init__(
            is_expert=is_expert,
            description=description,
            check=check,
            contact=contact,
            height=height,
            width=width,
        )
        #: tile used to display this plot
        self.title = title

        #: text string for the comparison outcome
        self.comparison_result = comparison_result

        #: verbose text describing the outcome of the comparison
        self.comparison_text = comparison_text

        #: the filename of the png file plotted with the comparison graphs
        self.png_filename = png_filename

        #: the filename of the pdf file plotted with the comparison graphs
        self.pdf_filename = pdf_filename

        #: path were the png and pdf files are located (relative to the html
        #: directory; has to end with trailing '/'!)
        self.plot_path = plot_path

        #: Warnings ("no contact" person etc.)
        if warnings is None:
            warnings = []
        self.warnings = warnings


class ComparisonNTuple(NTuple):

    """
    Comparison outcome for NTuples
    """

    def __init__(
        self,
        title,
        contact=None,
        description=None,
        check=None,
        is_expert=None,
        json_file_path=None,
    ):
        """
        Create a new ComparisonNTuple object and fill all members
        """

        # todo: move more into the base class
        super().__init__(
            is_expert=is_expert, description=description, check=check
        )
        #: Text used as title for the ntuple item
        self.title = title
        #: name of contact person
        self.contact = contact
        #: path to the json file which contains the individual numbers of
        #: the ntuple (must be relative to html dir!)
        self.json_file_path = json_file_path


class ComparisonHtmlContent(HtmlContent):

    """
    Compiled HTLM Content
    """

    def __init__(
        self,
        title,
        contact=None,
        description=None,
        check=None,
        is_expert=None,
        html_content=None,
    ):
        """
        Create a new ComparisonNTuple object and fill all members
        """

        # todo: move more into the base class
        super().__init__(
            is_expert=is_expert, description=description, check=check
        )
        #: Text used as title for the ntuple item
        self.title = title
        #: name of contact person
        self.contact = contact
        #: path to the json file which contains the individual numbers of
        #: the ntuple
        self.html_content = html_content


class ComparisonPackage(Package):

    """
    Information about a Package which was used in a comparison operation
    """

    def __init__(
        self, name, plotfiles=None, scriptfiles=None, ntuplefiles=None
    ):
        """
        Create a new ComparisonPackage object and fill all members
        """

        if not plotfiles:
            plotfiles = []
        if not scriptfiles:
            scriptfiles = []
        if not ntuplefiles:
            ntuplefiles = []

        super().__init__(name, plotfiles=plotfiles, scriptfiles=scriptfiles)

        #: the number of failed comparisons in this package
        self.comparison_error = sum([pf.comparison_error for pf in plotfiles])
        #: the number of failed comparisons of shifter plots in this package
        self.comparison_error_shifter = sum(
            [pf.comparison_error_shifter for pf in plotfiles]
        )
        #: the number of comparisons which resulted in a warning
        self.comparison_warning = sum(
            [pf.comparison_warning for pf in plotfiles]
        )
        #: the number of comparisons of shifter plots which resulted in a
        #: warning
        self.comparison_warning_shifter = sum(
            [pf.comparison_warning_shifter for pf in plotfiles]
        )


class ComparisonRevision(Revision):

    """
    Revision information enriched by the information gained during
    a comparison.
    """

    def __init__(self, label, git_hash=None, creation_date=None, color=None):
        """
        Create a new ComparisonRevision object and fill all members
        """

        # todo: creation_date
        super().__init__(label, git_hash=git_hash, creation_date=None)
        #: the color which was used for this revision in the
        #  comparison plots
        self.color = color


class Comparison(JsonBase):

    """
    Contains information and plots generated for comparisons
    between revisions
    """

    def __init__(self, revisions=None, packages=None):
        """
        Create a new ComparisonRevision object and fill all members
        """

        if not revisions:
            revisions = []
        if not packages:
            packages = []

        #: the list of revisions used in this comparison
        self.revisions = revisions
        #: the list of packages looked at in this comparison
        self.packages = packages
        sorted_revs = sorted(revisions, key=lambda x: x.label)
        #: the unique label of this comparison
        self.label = functools.reduce(
            lambda x, y: x + "_" + y.label, sorted_revs, ""
        )[1:]


# ==============================================================================
# Functions
# ==============================================================================


def dump(file_name, obj):
    """
    Output a tree of objects into a json file
    """

    with open(file_name, "w+") as f:
        json.dump(dump_rec(obj), f, sort_keys=True, indent=4)


def dumps(obj):
    """
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
            obj_list = []
            for it in v:
                # one of our object's in the list, which needs
                # special treatment?
                if isinstance(it, JsonBase):
                    # yen, recurse in to the object
                    obj_list.append(dump_rec(it))
                else:
                    # no, just add value to list
                    obj_list.append(it)

            # store compiled list with corresponding key
            this_dict[k] = obj_list
        # one of our objects, which might be nested and
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
