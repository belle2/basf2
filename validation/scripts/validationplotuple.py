#!/usr/bin/env python3


# std
import os.path
import math
import json
from typing import List, Optional

# 3rd
import ROOT

# ours
import metaoptions
import validationcomparison
import validationpath
from validationfunctions import strip_ext, index_from_revision, get_style
import json_objects
from validationrootobject import RootObject


# todo: [Ref, low prio, medium work] Refactor into class with uniform interface
#   and subclasses implementing actual functionality for Plot/Tuple etc.
#   /klieret
class Plotuple:

    """!
    A Plotuple is either a Plot or an N-Tuple

    @var work_folder: the work folder containing the results and plots
    @var root_objects: A list of Root-objects which belong
        together (i.e. should be drawn into one histogram or one table)
    @var revisions: The list of revisions
    @var warnings: A list of warnings that occured while creating the
        plots/tables for this Plotuple object
    @var reference: The reference RootObject for this Plotuple
    @var elements: The elements (RootObject of different revisions) for this
        Plotuple
    @var newest: The newest element in elements
    @var key: The key of the object within the corresponding ROOT file
    @var type: The type of the elements (TH1, TH2, TNtuple)
    @var description: The description of this Plotuple object
    @var check: Hint how the Plotuple object should look like
    @var contact: The contact person for this Plotuple object
    @var package: The package to which this Plotuple object belongs to
    @var rootfile: The rootfile to which the Plotuple object belongs to
    @var chi2test_result: The result of the Chi^2-Test. By default, there is no
        such result. If the Chi^2-Test has been performed, this variable holds
        the information between which objects it has been performed.
    @var pvalue: The p-value that the Chi^2-Test returned
    @var file: The file, in which the histogram or the HMTL-table (for
        n-tuples) are stored (without the file extension!)
    """

    def __init__(
            self,
            root_objects: List[RootObject],
            revisions: List[str],
            work_folder: str
    ):
        """!
        The default constructor for a Plotuple-object.
        @param root_objects: A list of Root-objects which belong
            together (i.e. should be drawn into one histogram or one table)
        @param revisions: The list of revisions (Duh!)
        """

        # the work folder containing the results and plots
        self._work_folder = work_folder

        # The list of Root objects in this Plotuple-object
        self._root_objects = root_objects

        # The list of revisions
        self._revisions = revisions

        # A list of all problems that occured with this Plotuple,
        # e.g. missing reference object, missing meta-information...
        self.warnings = []  # type: List[str]

        # Find the reference element. If we can't find one, set it to 'None'
        # The reference-object for this Plotuple object
        self._reference = None  # type: Optional[RootObject]
        for root_object in self._root_objects:
            if root_object.is_reference:
                self._reference = root_object
                break

        # If we couldn't find a reference element, add that to warnings
        if self._reference is None:
            self.warnings = ['No reference object']

        # All elements of the Plotuple that are not the reference-object
        # Get the elements, i.e. all RootObjects except for the
        # reference object. May be either histograms or n-tuples.
        # Note that the reference doesn't have a date set (and if we only plot
        # the reference, then is_reference is probably not set), so we have
        # to be careful of how to sort
        self._elements = [ro for ro in root_objects if not ro.is_reference]
        self._elements.sort(
            key=lambda ro: ro.date if ro.date else 0,
            reverse=True
        )

        # The newest element, i.e. the element belonging the revision
        # whose data were created most recently.
        # Should always be self.element[0], except if there is only a
        # reference object
        if self._elements:
            self._newest = self._elements[0]
        else:
            self._newest = self._reference

        # All available meta-information about the plotuple object:

        # The key (more precisely: the name of the key) that all elements of
        # this Plotuple object share
        self.key = self._newest.key

        # The type of the elements in this Plotuple object
        self.type = self._newest.type

        if self.type == "TNamed":
            # Sometimes, we use TNamed to encode extra information about the
            # ROOT file. In order to avoid that this will be plotted, we
            # catch it here and assign it the type 'Meta'
            meta_fields = ["description"]
            if self._newest.object.GetName().lower().strip() in meta_fields:
                self.type = "meta"

        # The description of the histogram/n-tuple which this Plotuple object
        # will yield
        self._description = self._newest.description

        # The 'Check for ...'-guideline for the histogram/n-tuple which this
        # Plotuple object will yield
        self._check = self._newest.check

        # A contact person for the histogram/n-tuple which this Plotuple object
        # will yield
        self._contact = self._newest.contact

        # MetaOptionParser for the meta-options for this Plotuple object
        self._mop = metaoptions.MetaOptionParser(self._newest.metaoptions)

        # The package to which the elements in this Plotuple object belong
        self.package = self._newest.package

        # The root file to which the elements in this Plotuple object belong
        self.rootfile = self._newest.rootfile

        # The result of the Chi^2-Test. By default, there is no such result.
        # If the Chi^2-Test has been performed, this variable holds between
        # which objects it has been performed.
        self._comparison_result_long = 'n/a'

        self.comparison_result = "not_compared"

        # The json file, in which the ntuple information is stored
        self._file = None  # type: Optional[str]

        self._html_content = None  # type: Optional[str]

        #: width of the plotted image in pixels, will be set by the draw
        #: function
        self._width = None  # type: Optional[int]

        #: height of the plotted image in pixels, will be set by the draw
        #: function
        self._height = None  # type: Optional[int]

        # Deal with incomplete information
        if self._description == '' or self._description is None:
            self._description = 'n/a'
            self.warnings.append('No description')
        if self._check == '' or self._check is None:
            self._check = 'n/a'
            self.warnings.append('No Check')
        if self._contact == '' or self._contact is None:
            self._contact = 'n/a'
            self.warnings.append('No Contact Person')

        #: The folder in which the plots will be found. Note that this should
        #: be relative to the ``html`` directory, because this is the string
        #: that will be used for the JavaScript queries to get the files
        self._plot_folder = os.path.join(
            validationpath.get_html_plots_tag_comparison_folder(
                work_folder, tags=revisions
            ),
            self.package
        )
        os.makedirs(self._plot_folder, exist_ok=True)

    def has_reference(self):
        """!
        @return True if a reference file is found for this plotuple
        """
        return self._reference is not None

    def create_plotuple(self):
        """!
        Creates the histogram/table/image that belongs to this Plotuble-object.
        """

        if self.type == 'TH1' or self.type == 'TEfficiency':
            self.create_histogram_plot('1D')
        elif self.type == 'TGraph':
            self.create_graph_plot()
        elif self.type == 'TH2':
            self.create_histogram_plot('2D')
        # used to store HTML user content
        elif self.type == 'TNamed':
            self.create_html_content()
        elif self.type == 'TASImage':
            self.create_image_plot()
        elif self.type == 'TNtuple':
            self.create_ntuple_table_json()
        elif self.type == "meta":
            pass
        else:
            raise ValueError('Tried to create histogram/n-tuple, but received'
                             'invalid type')

    def is_expert(self):
        """!
        @return Returns true if this plotuple has the expert option
        """
        return not self._mop.has_option("shifter")

    def perform_comparison(self):
        """!
        Takes the reference (self.reference.object) and the newest revision
        (self.newest.object) and a canvas. Performs a comparison of the
        two objects.
        @return: None
        """

        tester = validationcomparison.get_comparison(
            self._reference.object,
            self._newest.object,
            self._mop
        )

        self._comparison_result_long = tester.comparison_result_long.format(
            revision1=self._reference.revision,
            revision2=self._newest.revision
        )
        self.comparison_result = tester.comparison_result

    def _set_background(self, canvas):

        # kRed         #FF0000 Red
        # kRed - 9     #FF9999 Sweet pink

        # kOrange + 1  #FF9832 Sun
        # kOrange - 9  #FFCC9A Manhattan

        # kGreen - 3   #33CC33 Lime green
        # kGreen - 10  #CCFFCC Chinook

        # kAzure       #0032FE Medium blue
        # kAzure - 2   #3265FE Medium slate blue
        # kAzure - 9   #98CBFF Jordy blue

        colors = {
            "error": ROOT.kRed,
            "warning": ROOT.kOrange + 1,
            "equal": ROOT.kGreen - 3,
            "not_compared": ROOT.kAzure - 2
        }
        colors_expert = {
            "error": ROOT.kRed - 9,
            "warning": ROOT.kOrange - 9,
            "equal": ROOT.kGreen - 10,
            "not_compared": ROOT.kAzure - 9
        }

        if self.is_expert():
            color = colors_expert[self.comparison_result]
        else:
            color = colors[self.comparison_result]

        canvas.SetFillColor(color)
        canvas.GetFrame().SetFillColor(ROOT.kWhite)

    def _draw_ref(self, canvas):
        """!
        Takes the reference RootObject (self.reference.object)
        and a (sub)canvas and plots it with the correct line-style etc.
        @param canvas: Reference to the canvas on which we will draw the
            reference object.
        @return. None
        """
        self._remove_stats_tf1(self._reference.object)

        # Line is thick and black
        self._reference.object.SetLineColor(ROOT.kBlack)
        self._reference.object.SetLineWidth(2)
        self._reference.object.SetLineStyle(1)

        # Area under the curve is solid gray
        self._reference.object.SetFillColor(ROOT.kGray)
        self._reference.object.SetFillStyle(1001)

        # Draw the reference on the canvas
        self._draw_root_object(
            self.type,
            self._reference.object,
            self._reference.object.GetOption()
        )
        canvas.Update()
        canvas.GetFrame().SetFillColor(ROOT.kWhite)

    @staticmethod
    def _remove_stats_tf1(obj):
        # removed TF1s which might have been added by validation scripts
        # in tracking/scripts/tracking/validation/plot.py:1597
        tf1 = obj.FindObject("FitAndStats")
        if tf1:
            function_list = obj.GetListOfFunctions()
            function_list.Remove(tf1)

    # TODO: is this actually used or can it be removed ?
    def create_image_plot(self):
        """!
        Creates image plot for TASImage-objects.
        @return: None
        """

        # Create a ROOT canvas on which we will draw our histograms
        self._width = 700
        if len(self._elements) > 4:
            canvas = ROOT.TCanvas('', '', 700, 1050)
            self._height = 1050
        else:
            canvas = ROOT.TCanvas('', '', 700, 525)
            self._height = 525

        # Split the canvas into enough parts to fit all image_objects
        # Find numbers x and y so that x*y = N (number of histograms to be
        # plotted), and x,y close to sqrt(N)

        if len(self._root_objects) == 1:
            x = y = 1
        elif len(self._root_objects) == 2:
            x = 2
            y = 1
        else:
            x = 2
            y = int(math.floor((len(self._root_objects) + 1) / 2))

        # Actually split the canvas and go to the first pad ('sub-canvas')
        canvas.Divide(x, y)
        pad = canvas.cd(1)
        pad.SetFillColor(ROOT.kWhite)

        # If there is a reference object, plot it first
        if self._reference is not None:
            self._draw_ref(pad)

        # Now draw the normal plots
        items_to_plot_count = len(self._elements)
        for plot in reversed(self._elements):

            # Get the index of the current plot
            index = index_from_revision(plot.revision, self._work_folder)
            style = get_style(index, items_to_plot_count)

            self._remove_stats_tf1(plot.object)

            # Set line properties accordingly
            plot.object.SetLineColor(style.GetLineColor())
            plot.object.SetLineWidth(style.GetLineWidth())
            plot.object.SetLineStyle(style.GetLineStyle())

            # Switch to the correct sub-panel of the canvas. If a ref-plot
            # exists, we have to go one panel further compared to the
            # no-ref-case
            if self._reference is not None:
                i = 2
            else:
                i = 1

            pad = canvas.cd(self._elements.index(plot) + i)
            pad.SetFillColor(ROOT.kWhite)

            # Draw the reference on the canvas
            self._draw_root_object(
                self.type, plot.object,
                plot.object.GetOption()
            )
            pad.Update()
            pad.GetFrame().SetFillColor(ROOT.kWhite)

            # Write the title in the correct color
            title = pad.GetListOfPrimitives().FindObject('title')
            if title:
                title.SetTextColor(style.GetLineColor())

        # Save the plot as PNG and PDF
        canvas.Print(os.path.join(self._plot_folder, self.get_png_filename()))
        canvas.Print(os.path.join(self._plot_folder, self.get_pdf_filename()))

        self._file = os.path.join(
            self._plot_folder,
            "{}_{}".format(
                strip_ext(self.rootfile),
                self.key
            )
        )

    def get_png_filename(self):
        return '{}_{}.png'.format(strip_ext(self.rootfile), self.key)

    def get_pdf_filename(self):
        return '{}_{}.pdf'.format(strip_ext(self.rootfile), self.key)

    @staticmethod
    def _draw_root_object(typ, obj, options):
        """
        Special handling of the ROOT Draw calls, as some
        ROOT objects have a slightly differen flavour.
        """

        if typ == 'TEfficiency' or typ == "TGraph":
            # TEff does not provide DrawCopy
            obj.Draw(options)
        else:
            obj.DrawCopy(options)

    def create_histogram_plot(self, mode):
        """!
        Plots all histogram-objects in this Plotuple together in one histogram,
        which is then given the name of the key.
        @param mode: Determines whether it is a one- or
        two-dimensional histogram.
            Accepted values are '1D' and '2D'
        @return: None
        """

        # If we don't get a valid 'mode', we can stop right here
        if mode not in ['1D', '2D']:
            return

        # Create a ROOT canvas on which we will draw our histograms
        self._width = 700
        if mode == '2D' and len(self._elements) > 4:
            self._height = 1050
        else:
            self._height = 525
        canvas = ROOT.TCanvas('', '', self._width, self._height)

        # Allow possibility to turn off the stats box
        if self._mop.has_option('nostats'):
            ROOT.gStyle.SetOptStat("")
        else:
            ROOT.gStyle.SetOptStat("nemr")

        # If there is a reference object, and the list of plots is not empty,
        # perform a Chi^2-Test on the reference object and the first object in
        # the plot list:
        if self._reference is not None and self._newest \
                and not self._reference == self._newest:
            self.perform_comparison()

        # A variable which holds whether we
        # have drawn on the canvas already or not
        # (only used for the 1D case)
        drawn = False

        # Now we distinguish between 1D and 2D histograms
        # If we have a 1D histogram
        if mode == '1D':

            if not self._mop.has_option('nogrid'):
                canvas.SetGrid()
            if self._mop.has_option('logx'):
                canvas.SetLogx()
            if self._mop.has_option('logy'):
                canvas.SetLogy()

            # If there is a reference object, plot it first
            if self._reference is not None:
                self._draw_ref(canvas)
                drawn = True

        # If we have a 2D histogram
        elif mode == '2D':

            # Split the canvas into enough parts to fit all histogram_objects
            # Find numbers x and y so that x*y = N (number of histograms to be
            # plotted), and x,y close to sqrt(N)

            if len(self._root_objects) == 1:
                x = y = 1
            elif len(self._root_objects) == 2:
                x = 2
                y = 1
            else:
                x = 2
                y = int(math.floor((len(self._root_objects) + 1) / 2))

            # Actually split the canvas and go to the first pad ('sub-canvas')
            canvas.Divide(x, y)
            pad = canvas.cd(1)
            pad.SetFillColor(ROOT.kWhite)

            # If there is a reference object, plot it first
            if self._reference is not None:
                self._draw_ref(pad)

        items_to_plot_count = len(self._elements)
        # Now draw the normal plots
        for plot in reversed(self._elements):

            # Get the index of the current plot
            index = index_from_revision(plot.revision, self._work_folder)
            style = get_style(index, items_to_plot_count)

            self._remove_stats_tf1(plot.object)

            # Set line properties accordingly
            plot.object.SetLineColor(style.GetLineColor())
            plot.object.SetLineWidth(style.GetLineWidth())
            plot.object.SetLineStyle(style.GetLineStyle())

            # If we have a one-dimensional histogram
            if mode == '1D':
                if not drawn:
                    # Get additional options for 1D histograms
                    # (Intersection with self.metaoptions)
                    additional_options = ['C']
                    additional_options = [
                        option for option in additional_options
                        if self._mop.has_option(option)
                    ]
                    options_str = plot.object.GetOption() + \
                        ' '.join(additional_options)
                    drawn = True
                else:
                    options_str = "SAME"

                self._draw_root_object(self.type, plot.object, options_str)

                # redraw grid ontop of histogram, if selected
                if not self._mop.has_option('nogrid'):
                    canvas.RedrawAxis("g")

                canvas.Update()
                canvas.GetFrame().SetFillColor(ROOT.kWhite)

            # If we have a two-dimensional histogram
            elif mode == '2D':
                # Switch to the correct sub-panel of the canvas. If a ref-plot
                # exists, we have to go one panel further compared to the
                # no-ref-case
                if self._reference is not None:
                    i = 2
                else:
                    i = 1

                pad = canvas.cd(self._elements.index(plot) + i)
                pad.SetFillColor(ROOT.kWhite)

                # Get additional options for 2D histograms
                additional_options = ''
                for _ in ['col', 'colz', 'cont', 'contz', 'box']:
                    if self._mop.has_option(_):
                        additional_options += ' ' + _

                # Draw the reference on the canvas
                self._draw_root_object(
                    self.type,
                    plot.object,
                    plot.object.GetOption() + additional_options
                )
                pad.Update()
                pad.GetFrame().SetFillColor(ROOT.kWhite)

                # Write the title in the correct color
                title = pad.GetListOfPrimitives().FindObject('title')
                if title:
                    title.SetTextColor(style.GetLineColor())

        if self._newest:
            # if there is at least one revision
            self._set_background(canvas)

        canvas.GetFrame().SetFillColor(ROOT.kWhite)

        # Save the plot as PNG and PDF
        canvas.Print(os.path.join(self._plot_folder, self.get_png_filename()))
        canvas.Print(os.path.join(self._plot_folder, self.get_pdf_filename()))

        self._file = os.path.join(
            self._plot_folder,
            "{}_{}".format(
                strip_ext(self.rootfile),
                self.key
            )
        )

    def create_graph_plot(self):
        """!
        Plots as TGraph/TGraphErrors
        @return: None
        """

        # Create a ROOT canvas on which we will draw our plots
        self._width = 700
        self._height = 525
        canvas = ROOT.TCanvas('', '', self._width, self._height)

        # Allow possibility to turn off the stats box
        if self._mop.has_option('nostats'):
            ROOT.gStyle.SetOptStat("")
        else:
            ROOT.gStyle.SetOptStat("nemr")

        # If there is a reference object, and the list of plots is not empty,
        # perform a Chi^2-Test on the reference object and the first object in
        # the plot list:
        if self._reference is not None and self._newest \
                and not self._reference == self._newest:
            self.perform_comparison()

        if not self._mop.has_option('nogrid'):
            canvas.SetGrid()
        if self._mop.has_option('logx'):
            canvas.SetLogx()
        if self._mop.has_option('logy'):
            canvas.SetLogy()

        # A variable which holds whether we
        # have drawn on the canvas already or not
        drawn = False

        # If there is a reference object, plot it first
        if self._reference is not None:
            self._draw_ref(canvas)
            drawn = True

        items_to_plot_count = len(self._elements)
        # Now draw the normal plots
        for plot in reversed(self._elements):

            # Get the index of the current plot
            index = index_from_revision(plot.revision, self._work_folder)
            style = get_style(index, items_to_plot_count)

            # self.remove_stats_tf1(plot.object)

            # Set line properties accordingly
            plot.object.SetLineColor(style.GetLineColor())
            plot.object.SetLineWidth(style.GetLineWidth())
            plot.object.SetLineStyle(style.GetLineStyle())

            # If we have a one-dimensional histogram
            if not drawn:

                # todo: refactor like in plot hist
                # Get additional options for 1D histograms
                additional_options = ''
                for _ in ['C']:
                    if self._mop.has_option(_):
                        additional_options += ' ' + _

                # Draw the reference on the canvas
                self._draw_root_object(
                    self.type,
                    plot.object,
                    plot.object.GetOption() + additional_options
                )
                drawn = True
            else:
                self._draw_root_object(self.type, plot.object, "SAME")

            # redraw grid ontop of histogram, if selected
            if not self._mop.has_option('nogrid'):
                canvas.RedrawAxis("g")

            canvas.Update()
            canvas.GetFrame().SetFillColor(ROOT.kWhite)

        if self._newest:
            # if there is at least one revision
            self._set_background(canvas)

        # Save the plot as PNG and PDF
        canvas.Print(os.path.join(self._plot_folder, self.get_png_filename()))
        canvas.Print(os.path.join(self._plot_folder, self.get_pdf_filename()))

        self._file = os.path.join(
            self._plot_folder,
            "{}_{}".format(
                strip_ext(self.rootfile),
                self.key
            )
        )

    def create_html_content(self):

        # self.elements
        self._html_content = ""

        for elem in self._elements:
            self._html_content += "<p>" + \
                                  elem.revision + \
                                 "</p>" + \
                                  elem.object.GetTitle()

        # there is no file storing this, because it is directly in the json
        # file
        self._file = None

    def get_meta_information(self):
        assert self.type == "meta"
        key = self._newest.object.GetName().strip().lower()
        value = self._newest.object.GetTitle()
        return key, value

    def create_ntuple_table_json(self):
        """!
        If the Plotuple-object contains n-tuples, this will create the
        a JSON file, which is later converted to HTML by the javascript
        function fillNtupleTable.
        """

        json_nutple = {}

        # The dictionary will have the following form
        # {
        #     "reference (if exist)": [
        #         ('variable 1', 'reference value for variable 1'),
        #         ('variable 2', 'reference value for variable 2'),
        #         ...
        #     ],
        #     "revision": [
        #         ...
        #     ]
        # }

        precision = self._mop.int_value("float-precision", default=4)
        format_str = f"{{0:.{precision}f}}"

        def value2str(obj):
            # assuming that I have a float
            return format_str.format(obj)

        colum_names = []
        for key in list(self._newest.object.keys()):
            colum_names.append(key)

        # If there is a reference object, print the reference values as the
        # first row of the table
        if self._reference and 'reference' in self._revisions:
            json_nutple['reference'] = []

            key_list = list(self._reference.object.keys())
            for column in colum_names:
                if column in key_list:
                    value_str = value2str(self._reference.object[column])
                    json_nutple['reference'].append(
                        (column, value_str)
                    )
                else:
                    json_nutple['reference'].append((column, None))

        # Now print the values for all other revisions
        for ntuple in self._elements:
            if ntuple.revision not in json_nutple:
                json_nutple[ntuple.revision] = []

            for column in colum_names:
                if column in ntuple.object:
                    value_str = value2str(ntuple.object[column])
                    json_nutple[ntuple.revision].append(
                        (column, value_str)
                    )
                else:
                    json_nutple[ntuple.revision].append((column, None))

        json_ntuple_file = os.path.join(
            self._plot_folder,
            "{}_{}.json".format(
                strip_ext(self.rootfile),
                self.key
            )
        )

        with open(json_ntuple_file, 'w+') as json_file:
            json.dump(json_nutple, json_file)

        self._file = json_ntuple_file

    def get_plot_title(self):
        if self._file:
            return os.path.basename(self._file).replace(".", "_").strip()
        else:
            # this is for html content which is not stored in any file
            return self.key

    def create_json_object(self):
        if self.type == 'TNtuple':
            return json_objects.ComparisonNTuple(
                title=self.get_plot_title(),
                description=self._description,
                contact=self._contact,
                check=self._check,
                is_expert=self.is_expert(),
                json_file_path=os.path.relpath(
                    self._file,
                    validationpath.get_html_folder(self._work_folder)
                ),
            )
        elif self.type == 'TNamed':
            return json_objects.ComparisonHtmlContent(
                title=self.get_plot_title(),
                description=self._description,
                contact=self._contact,
                check=self._check,
                is_expert=self.is_expert(),
                html_content=self._html_content
            )
        elif self.type == "meta":
            return None
        else:
            return json_objects.ComparisonPlot(
                title=self.get_plot_title(),
                comparison_result=self.comparison_result,
                comparison_text=self._comparison_result_long,
                description=self._description,
                contact=self._contact,
                check=self._check,
                height=self._height,
                width=self._width,
                is_expert=self.is_expert(),
                plot_path=os.path.relpath(
                    self._plot_folder,
                    validationpath.get_html_folder(self._work_folder)
                ) + "/",
                png_filename=self.get_png_filename(),
                pdf_filename=self.get_pdf_filename(),
                warnings=self.warnings
            )
