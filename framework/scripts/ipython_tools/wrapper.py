# Nice display features imports
from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule
from IPython.core.display import Image, display
from tracking.validation.harvesting import HarvestingModule

import os


class QueueDrawer(CDCSVGDisplayModule):

    """
    A wrapper around the svg drawer in the tracking package that
    writes its output files as a list to the queue
    """

    def __init__(self, queue, label, *args, **kwargs):
        """ The same as the base class, except:

        Arguments
        ---------

        queue: The queue to write to
        label: The key name in the queue
        """
        self.queue = queue
        self.label = label
        CDCSVGDisplayModule.__init__(self, interactive=False, *args, **kwargs)
        self.use_cpp = True

        self.file_list = []

    def terminate(self):
        """ Overwrite the terminate to put the list to the queue"""
        CDCSVGDisplayModule.terminate(self)
        self.queue.put(self.label, self.file_list)

    def new_output_filename(self):
        """ Overwrite the function to listen for every new filename """
        output_file_name = CDCSVGDisplayModule.new_output_filename(self)
        self.file_list.append(output_file_name)
        return output_file_name


def show_image(filename, show=True):
    """ Display an image file in ipython """
    os.system("convert " + filename + " " + filename[:-3] + str("png"))
    image = Image(filename[:-3] + str("png"))
    if show:
        display(image)
    return image


class QueueHarvester(HarvestingModule):

    """ Wrapper for the HarvestingModule to write its output file name to the queue """

    def __init__(self, queue, foreach, output_file_name, name=None, title=None, contact=None, expert_level=None):
        """ The same as the base class except for the queue argument """
        queue.put(self.__class__.__name__ + "_output_file_name", output_file_name)
        HarvestingModule.__init__(self, foreach=foreach,
                                  output_file_name=output_file_name,
                                  name=name, title=title, contact=contact,
                                  expert_level=expert_level)
