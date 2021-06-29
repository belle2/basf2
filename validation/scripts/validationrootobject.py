#!/usr/bin/env python3

# std
import pprint
from typing import Optional, List


pp = pprint.PrettyPrinter(depth=6, indent=1, width=80)


# todo: this could be implemented so so so much easier and also it has triple
#  docstrings for everything
class RootObject:

    """!
    Wraps a ROOT object (either a histogram or an n-tuple) together with the
    available meta-information about it.
    Storing the information in a dictionary is necessary to make the objects
    searchable, i.e. implement a function that can return for example all
    objects from a certain revision.

    @var data: A dict with all information about the Root-object
    @var revision: The revision to which the object belongs to
    @var package: The package to which the object belongs to
    @var rootfile: The root file to which the object belongs to
    @var key: The key (more precisely: the name of the key) which the object
        has within the root file
    @var object: The root object itself
    @var type: The type, i.e. whether its a histogram or an n-tuple
    @var description: The description, what the histogram/n-tuple contains
    @var check: A brief description how the histogram or the values should
        look like
    @var contact: A contact person for this histogram/n-tuple
    @var date: The date of the object (identical with the date of its rootfile)
    @var is_reference: Boolean value if it is an object from a reference file
        or not
    """

    def __init__(
        self,
        revision: str,
        package: str,
        rootfile: str,
        key: str,
        root_object,
        root_object_type: str,
        date: Optional[int],
        description: str,
        check: str,
        contact: str,
        metaoptions: List[str],
        is_reference: bool,
    ):
        """!
        The constructor. Sets the element up and store the information in a
        dict, but also sets up object variables for simplified access.

        @param revision: The revision of the object, e.g. 'release-00-04-01'
        @param package: The package of the object, e.g. 'analysis'
        @param rootfile: The absolute path to the ROOT file that contains
                this object
        @param key: The key of the object, which is basically its name.
                Example: 'P_Eff_k_e'. For each revision, there should be one
                object with the same key.
        @param root_object: The ROOT object itself. Storing works only for
                histograms.
        @param root_object_type: The type of the object. Possible values are
                'TH1' (1D histogram), 'TH2' (2D histogram), and 'TNtuple'
        @param date: The date when the containing revision folder was last
                modified. Important to find the most recent object.
        @param description: A short description of what is displayed in the
                plot. May also contain LaTeX-Code (enclosed in $...$),
                which will later be parsed by MathJax
        @param check: A short description of how the data in the plot should
                look like, i.e. for example the target location of a peak etc.
        @param contact: A name or preferably an e-mail address of the person
                who is responsible for this plot and may be contacted in case
                of problems
        @param metaoptions: Meta-options for the plot, e.g. 'colz' for histo-
                grams, or log-scale for the axes, etc.
        @param is_reference: A boolean value telling if an object is a
                reference object or a normal plot/n-tuple object from a
                revision. Possible Values: True for reference objects,
                False for revision objects.
        """

        # A dict with all information about the Root-object
        # Have all information as a dictionary so that we can search and
        # filter the objects by properties
        self.data = {
            "revision": revision,
            "package": package,
            "rootfile": rootfile,
            "key": key,
            "object": root_object,
            "type": root_object_type,
            "check": check,
            "description": description,
            "contact": contact,
            "date": date,
            "metaoptions": metaoptions,
            "is_reference": is_reference,
        }

    # For convenient access, define the following properties, which are
    # only references to the values from the dict

    @property
    def revision(self):
        """ The revision to which the object belongs to """
        return self.data["revision"]

    @property
    def package(self):
        """ The package to which the object belongs to"""
        return self.data["package"]

    @property
    def rootfile(self):
        """ The root file to which the object belongs to"""
        return self.data["rootfile"]

    @property
    def key(self):
        """ The key (more precisely: the name of they) which the object has
        within the root file
        """
        return self.data["key"]

    @property
    def object(self):
        """ The root object itself """
        return self.data["object"]

    @property
    def type(self):
        """ The type, i.e. whether its a histogram or an n-tuple """
        return self.data["type"]

    @property
    def description(self):
        """ The description, what the histogram/n-tuple contains """
        return self.data["description"]

    @property
    def check(self):
        """ A brief description how the histogram or the values should look
        like (e.g. characteristic peaks etc.) """
        return self.data["check"]

    @property
    def contact(self):
        """ A contact person for this histogram/n-tuple """
        return self.data["contact"]

    @property
    def date(self):
        """ The date of the object (identical with the date of its rootfile) """
        return self.data["date"]

    @property
    def metaoptions(self):
        """ Meta-options for the object, e.g. colz or log-scale for the axes """
        return self.data["metaoptions"]

    @property
    def is_reference(self):
        """ Boolean value if it is an object from a reference file or not """
        return self.data["is_reference"]

    def __str__(self):
        return str(self.data)

    def dump(self):
        """!
        Allows to print out all information about a RootObject to the command
        line (for debugging purposes).
        @return: None
        """
        pp.pprint(self.data)
