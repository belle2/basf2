#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import ROOT

import os.path
import validationcomparison
import metaoptions
import sys
import math
import json

from validationfunctions import strip_ext, index_from_revision, get_style


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

    def __init__(self, root_objects, revisions, work_folder):
        """!
        The default constructor for a Plotuple-object.
        @param root_objects: A list of Root-objects which belong
            together (i.e. should be drawn into one histogram or one table)
        @param revisions: The list of revisions (Duh!)
        """

        # the work folder containing the results and plots
        self.work_folder = work_folder

        # The list of Root objects in this Plotuple-object
        self.root_objects = root_objects

        # The list of revisions
        self.revisions = revisions

        # A list of all problems that occured with this Plotuple,
        # e.g. missing reference object, missing meta-information...
        self.warnings = []

        # Find the reference element. If we can't find one, set it to 'None'
        # The reference-object for this Plotuple object
        self.reference = None
        for root_object in self.root_objects:
            if root_object.is_reference:
                self.reference = root_object
                break

        # If we couldn't find a reference element, add that to warnings
        if self.reference is None:
            self.warnings = ['No reference object']

        # All elements of the Plotuple that are not the reference-object
        # Get the elements, i.e. all RootObjects except for the
        # reference object. May be either histograms or n-tuples.
        self.elements = sorted(
            [_ for _ in root_objects if _ is not self.reference],
            key=lambda _: _.date,
            reverse=True
        )

        # The newest element, i.e. the element belonging the revision
        # whose data were created most recently.
        # Should always be self.element[0], except if there is only a
        # reference object
        if self.elements:
            self.newest = self.elements[0]
        else:
            self.newest = self.reference

        # All available meta-information about the plotuple object:

        # The key (more precisely: the name of the key) that all elements of
        # this Plotuple object share
        self.key = self.newest.key

        # The type of the elements in this Plotuple object
        self.type = self.newest.type

        # The description of the histogram/n-tuple which this Plotuple object
        # will yield
        self.description = self.newest.description

        # The 'Check for ...'-guideline for the histogram/n-tuple which this
        # Plotuple object will yield
        self.check = self.newest.check

        # A contact person for the histogram/n-tuple which this Plotuple object
        # will yield
        self.contact = self.newest.contact

        # The meta-options for this Plotuple object
        self.metaoptions = self.newest.metaoptions

        # The package to which the elements in this Plotuple object belong
        self.package = self.newest.package

        # The root file to which the elements in this Plotuple object belong
        self.rootfile = self.newest.rootfile

        # The result of the Chi^2-Test. By default, there is no such result.
        # If the Chi^2-Test has been performed, this variable holds between
        # which objects it has been performed.
        self.chi2test_result = 'n/a'

        self.comparison_result = "not_compared"

        # The p-value that the Chi^2-Test returned.
        self.pvalue = 'n/a'

        #: an comparison error will be shown if the p-value is smaller than
        #: this number
        #: will bet set by the chi2test function
        self.pvalue_error = None

        #: an comparison warning will be shown if the p-value is smaller than
        #: this number
        #: will bet set by the chi2test function
        self.pvalue_warn = None

        # The json file, in which the ntuple information is stored
        self.file = None

        self.html_content = None

        #: width of the plotted image in pixels, will be set by the draw
        #: function
        self.width = None

        #: height of the plotted image in pixels, will be set by the draw
        #: function
        self.height = None

        # Deal with incomplete information
        if self.description == '' or self.description is None:
            self.description = 'n/a'
            self.warnings.append('No description')
        if self.check == '' or self.check is None:
            self.check = 'n/a'
            self.warnings.append('No Check')
        if self.contact == '' or self.contact is None:
            self.contact = 'n/a'
            self.warnings.append('No Contact Person')

        self.plot_folder = os.path.join(
            "plots",
            "_".join(sorted(self.revisions)),
            self.package
        )
        if not os.path.isdir(self.plot_folder):
            os.makedirs(self.plot_folder)

        # Create the actual plot or n-tuple-table
        self.create_plotuple()

    def has_reference(self):
        """!
        @return True if a reference file is found for this plotuple
        """
        return self.reference is not None

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
        else:
            # fixme: shouldn't we rather throw an exception /klieret
            sys.exit('Tried to create histogram/n-tuple, '
                     'but received invalid type')

    def is_expert(self):
        """!
        @return Returns true if this plotuple has the expert option
        """
        mop = metaoptions.MetaOptionParser(self.metaoptions)
        return mop.has_option("expert")

    def chi2test(self):
        """!
        Takes the reference (self.reference.object) and the newest revision
        (self.newest.object) and a canvas. Performs a Chi^2-Test on the
        two histograms and sets the background of the canvas correspondingly.
        Sets self.pvalue to the p-value of the Chi^2-Test.
        @return: None
        """
        mop = metaoptions.MetaOptionParser(self.metaoptions)

        if mop.has_option("nocompare"):
            # is comparison disabled for this plot ?
            self.chi2test_result = 'Chi^2 test is disabled for this plot'
            self.pvalue = None
            return

        fail_message = "Comparison failed: "

        # will be set to true, if for some reason no Chi^2 test could be
        # performed, but the two objects are still different (for example
        # different bin size)
        no_comparison_but_still_different = False

        # execute the chi2 test, extract the relevant values and handle
        # possible exceptions
        pvalue = None
        chi2 = None
        chi2ndf = None
        ndf = None
        try:
            ctest = validationcomparison.Chi2Test(self.reference.object,
                                                  self.newest.object)
            pvalue = ctest.pvalue()
            chi2 = ctest.chi2()
            chi2ndf = ctest.chi2ndf()
            ndf = ctest.ndf()
        except validationcomparison.ObjectsNotSupported as e:
            self.chi2test_result = fail_message + str(e)
        except validationcomparison.DifferingBinCount as e:
            self.chi2test_result = fail_message + str(e)
            no_comparison_but_still_different = True
        except validationcomparison.TooFewBins as e:
            self.chi2test_result = fail_message + str(e)
        except validationcomparison.ComparisonFailed as e:
            self.chi2test_result = fail_message + str(e)
            no_comparison_but_still_different = True

        if no_comparison_but_still_different:
            self.comparison_result = "error"
            self.pvalue = 0.0

        if pvalue is not None:
            # check if there is a custom setting for pvalue sensitivity
            self.pvalue_warn = mop.pvalue_warn()
            self.pvalue_error = mop.pvalue_error()

            if self.pvalue_warn is None:
                self.pvalue_warn = 1.0
            if self.pvalue_error is None:
                self.pvalue_error = 0.01

            # If pvalue < 0.01: Very strong presumption against neutral
            # hypothesis
            if pvalue < self.pvalue_error:
                self.comparison_result = "error"
            # If pvalue < 1: Deviations at least exists
            elif pvalue < self.pvalue_warn:
                self.comparison_result = "warning"
            else:
                self.comparison_result = "equal"

            self.chi2test_result = \
                'Performed Chi^2-Test between reference and {} (Chi^2 = {} ' \
                'NDF = {} Chi^2/NDF = {})'.format(
                    self.newest.revision, chi2, ndf, chi2ndf)
            self.pvalue = pvalue
        else:
            self.pvalue = None

    def set_background(self, canvas):

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

    def draw_ref(self, canvas):
        """!
        Takes the reference RootObject (self.reference.object)
        and a (sub)canvas and plots it with the correct line-style etc.
        @param canvas: Reference to the canvas on which we will draw the
            reference object.
        @return. None
        """
        self.remove_stats_tf1(self.reference.object)

        # Line is thick and black
        self.reference.object.SetLineColor(ROOT.kBlack)
        self.reference.object.SetLineWidth(2)
        self.reference.object.SetLineStyle(1)

        # Area under the curve is solid gray
        self.reference.object.SetFillColor(ROOT.kGray)
        self.reference.object.SetFillStyle(1001)

        # Draw the reference on the canvas
        self.draw_root_object(
            self.type,
            self.reference.object,
            self.reference.object.GetOption()
        )
        canvas.Update()
        canvas.GetFrame().SetFillColor(ROOT.kWhite)

    @staticmethod
    def remove_stats_tf1(obj):
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
        self.width = 700
        if len(self.elements) > 4:
            canvas = ROOT.TCanvas('', '', 700, 1050)
            self.height = 1050
        else:
            canvas = ROOT.TCanvas('', '', 700, 525)
            self.height = 525

        # Split the canvas into enough parts to fit all image_objects
        # Find numbers x and y so that x*y = N (number of histograms to be
        # plotted), and x,y close to sqrt(N)

        if len(self.root_objects) == 1:
            x = y = 1
        elif len(self.root_objects) == 2:
            x = 2
            y = 1
        else:
            x = 2
            y = int(math.floor((len(self.root_objects) + 1) / 2))

        # Actually split the canvas and go to the first pad ('sub-canvas')
        canvas.Divide(x, y)
        pad = canvas.cd(1)
        pad.SetFillColor(ROOT.kWhite)

        # If there is a reference object, plot it first
        if self.reference is not None:
            self.draw_ref(pad)

        # Now draw the normal plots
        items_to_plot_count = len(self.elements)
        for plot in reversed(self.elements):

            # Get the index of the current plot
            index = index_from_revision(plot.revision, self.work_folder)
            style = get_style(index, items_to_plot_count)

            self.remove_stats_tf1(plot.object)

            # Set line properties accordingly
            plot.object.SetLineColor(style.GetLineColor())
            plot.object.SetLineWidth(style.GetLineWidth())
            plot.object.SetLineStyle(style.GetLineStyle())

            # Switch to the correct sub-panel of the canvas. If a ref-plot
            # exists, we have to go one panel further compared to the
            # no-ref-case
            if self.reference is not None:
                i = 2
            else:
                i = 1

            pad = canvas.cd(self.elements.index(plot) + i)
            pad.SetFillColor(ROOT.kWhite)

            # Draw the reference on the canvas
            self.draw_root_object(
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
        canvas.Print(os.path.join(self.plot_folder, self.get_png_filename()))
        canvas.Print(os.path.join(self.plot_folder, self.get_pdf_filename()))

        self.file = os.path.join(
            self.plot_folder,
            "{}_{}".format(
                strip_ext(self.rootfile),
                self.key
            )
        )

    # todo: not super elegant, this is why you should use os.path.join etc. /klieret
    def get_plot_path(self):
        return self.plot_folder + "/"

    def get_png_filename(self):
        return '{}_{}.png'.format(strip_ext(self.rootfile), self.key)

    def get_pdf_filename(self):
        return '{}_{}.pdf'.format(strip_ext(self.rootfile), self.key)

    @staticmethod
    def draw_root_object(typ, obj, options):
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
        self.width = 700
        if mode == '2D' and len(self.elements) > 4:
            self.height = 1050
        else:
            self.height = 525
        canvas = ROOT.TCanvas('', '', self.width, self.height)

        # Allow possibility to turn off the stats box
        if 'nostats' in self.metaoptions:
            ROOT.gStyle.SetOptStat("")
        else:
            ROOT.gStyle.SetOptStat("nemr")

        # If there is a reference object, and the list of plots is not empty,
        # perform a Chi^2-Test on the reference object and the first object in
        # the plot list:
        if self.reference is not None and self.newest:
            self.chi2test()

        # A variable which holds whether we
        # have drawn on the canvas already or not
        # (only used for the 1D case)
        drawn = False

        # Now we distinguish between 1D and 2D histograms
        # If we have a 1D histogram
        if mode == '1D':

            if 'nogrid' not in self.metaoptions:
                canvas.SetGrid()
            if 'logx' in self.metaoptions:
                canvas.SetLogx()
            if 'logy' in self.metaoptions:
                canvas.SetLogy()

            # If there is a reference object, plot it first
            if self.reference is not None:
                self.draw_ref(canvas)
                drawn = True

        # If we have a 2D histogram
        elif mode == '2D':

            # Split the canvas into enough parts to fit all histogram_objects
            # Find numbers x and y so that x*y = N (number of histograms to be
            # plotted), and x,y close to sqrt(N)

            if len(self.root_objects) == 1:
                x = y = 1
            elif len(self.root_objects) == 2:
                x = 2
                y = 1
            else:
                x = 2
                y = int(math.floor((len(self.root_objects) + 1) / 2))

            # Actually split the canvas and go to the first pad ('sub-canvas')
            canvas.Divide(x, y)
            pad = canvas.cd(1)
            pad.SetFillColor(ROOT.kWhite)

            # If there is a reference object, plot it first
            if self.reference is not None:
                self.draw_ref(pad)

        items_to_plot_count = len(self.elements)
        # Now draw the normal plots
        for plot in reversed(self.elements):

            # Get the index of the current plot
            index = index_from_revision(plot.revision, self.work_folder)
            style = get_style(index, items_to_plot_count)

            self.remove_stats_tf1(plot.object)

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
                    additional_options = list(
                        set(additional_options) & set(self.metaoptions)
                    )

                    options_str = plot.object.GetOption() + \
                        ' '.join(additional_options)
                    drawn = True
                else:
                    options_str = "SAME"

                self.draw_root_object(self.type, plot.object, options_str)

                # redraw grid ontop of histogram, if selected
                if 'nogrid' not in self.metaoptions:
                    canvas.RedrawAxis("g")

                canvas.Update()
                canvas.GetFrame().SetFillColor(ROOT.kWhite)

            # If we have a two-dimensional histogram
            elif mode == '2D':
                # Switch to the correct sub-panel of the canvas. If a ref-plot
                # exists, we have to go one panel further compared to the
                # no-ref-case
                if self.reference is not None:
                    i = 2
                else:
                    i = 1

                pad = canvas.cd(self.elements.index(plot) + i)
                pad.SetFillColor(ROOT.kWhite)

                # Get additional options for 2D histograms
                additional_options = ''
                for _ in ['col', 'colz', 'cont', 'contz', 'box']:
                    if _ in self.metaoptions:
                        additional_options += ' ' + _

                # Draw the reference on the canvas
                self.draw_root_object(
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

        if self.newest:
            # if there is at least one revision
            self.set_background(canvas)

        canvas.GetFrame().SetFillColor(ROOT.kWhite)

        # Save the plot as PNG and PDF
        canvas.Print(os.path.join(self.plot_folder, self.get_png_filename()))
        canvas.Print(os.path.join(self.plot_folder, self.get_pdf_filename()))

        self.file = os.path.join(
            self.plot_folder,
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
        self.width = 700
        self.height = 525
        canvas = ROOT.TCanvas('', '', self.width, self.height)

        # Allow possibility to turn off the stats box
        if 'nostats' in self.metaoptions:
            ROOT.gStyle.SetOptStat("")
        else:
            ROOT.gStyle.SetOptStat("nemr")

        # If there is a reference object, and the list of plots is not empty,
        # perform a Chi^2-Test on the reference object and the first object in
        # the plot list:
        if self.reference is not None and self.newest:
            self.chi2test()

        if 'nogrid' not in self.metaoptions:
            canvas.SetGrid()
        if 'logx' in self.metaoptions:
            canvas.SetLogx()
        if 'logy' in self.metaoptions:
            canvas.SetLogy()

        # A variable which holds whether we
        # have drawn on the canvas already or not
        drawn = False

        # If there is a reference object, plot it first
        if self.reference is not None:
            self.draw_ref(canvas)
            drawn = True

        items_to_plot_count = len(self.elements)
        # Now draw the normal plots
        for plot in reversed(self.elements):

            # Get the index of the current plot
            index = index_from_revision(plot.revision, self.work_folder)
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
                    if _ in self.metaoptions:
                        additional_options += ' ' + _

                # Draw the reference on the canvas
                self.draw_root_object(
                    self.type,
                    plot.object,
                    plot.object.GetOption() + additional_options
                )
                drawn = True
            else:
                self.draw_root_object(self.type, plot.object, "SAME")

            # redraw grid ontop of histogram, if selected
            if 'nogrid' not in self.metaoptions:
                canvas.RedrawAxis("g")

            canvas.Update()
            canvas.GetFrame().SetFillColor(ROOT.kWhite)

        if self.newest:
            # if there is at least one revision
            self.set_background(canvas)

        # Save the plot as PNG and PDF
        canvas.Print(os.path.join(self.plot_folder, self.get_png_filename()))
        canvas.Print(os.path.join(self.plot_folder, self.get_pdf_filename()))

        self.file = os.path.join(
            self.plot_folder,
            "{}_{}".format(
                strip_ext(self.rootfile),
                self.key
            )
        )

    def create_html_content(self):

        # self.elements
        self.html_content = ""

        for elem in self.elements:
            self.html_content += "<p>" + \
                                 elem.revision + \
                                 "</p>" + \
                                 elem.object.GetTitle()

        # there is no file storing this, because it is directly in the json
        # file
        self.file = None

    def create_ntuple_table_json(self):
        """!
        If the Plotuple-object contains n-tuples, this will create the
        a JSON file, which is later converted to HTML by the javascript
        function fill_ntuple_table.
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

        mop = metaoptions.MetaOptionParser(self.metaoptions)
        precision = mop.int_value("float-precision", default=4)
        format_str = "{{0:.{}f}}".format(precision)

        def value2str(obj):
            # assuming that I have a float
            return format_str.format(obj)

        colum_names = []
        for key in list(self.newest.object.keys()):
            colum_names.append(key)

        # If there is a reference object, print the reference values as the
        # first row of the table
        if self.reference and 'reference' in self.revisions:
            json_nutple['reference'] = []

            key_list = list(self.reference.object.keys())
            for column in colum_names:
                if column in key_list:
                    value_str = value2str(self.reference.object[column])
                    json_nutple['reference'].append(
                        (column, value_str)
                    )
                else:
                    json_nutple['reference'].append((column, None))

        # Now print the values for all other revisions
        for ntuple in self.elements:
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
            self.plot_folder,
            "{}_{}.json".format(
                strip_ext(self.rootfile),
                self.key
            )
        )

        with open(json_ntuple_file, 'w+') as json_file:
            json.dump(json_nutple, json_file)

        self.file = json_ntuple_file

    def get_plot_title(self):
        if self.file:
            return os.path.basename(self.file).replace(".", "_").strip()
        else:
            # this is for html content which is not stored in any file
            return self.key
