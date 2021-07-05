##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Nice display features imports
from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule
from tracking.harvest.harvesting import HarvestingModule
import os
import multiprocessing


class QueueDrawer(CDCSVGDisplayModule):

    """
    A wrapper around the svg drawer in the tracking package that
    writes its output files as a list to the queue
    """

    def __init__(self, queue=None, label=None, *args, **kwargs):
        """ The same as the base class, except:

        Arguments
        ---------

        queue: The queue to write to
        label: The key name in the queue
        """
        #: The queue to handle
        self.queue = queue

        #: The label for writing to the queue
        self.label = label

        CDCSVGDisplayModule.__init__(self, interactive=False, *args, **kwargs)

        #: We want to use cpp for sure
        self.use_cpp = True

        #: We store the files in a list and this list must be accessible also in multiprocessing, so we us a manager here.
        self.manager = multiprocessing.Manager()

        #: The list of created paths
        self.file_list = self.manager.list([])

    def terminate(self):
        """ Overwrite the terminate to put the list to the queue"""
        CDCSVGDisplayModule.terminate(self)
        file_list = self.file_list
        files = [file_list[i] for i in range(len(file_list))]
        if self.queue:
            self.queue.put(self.label, files)

    def new_output_filename(self):
        """ Overwrite the function to listen for every new filename """
        output_file_name = CDCSVGDisplayModule.new_output_filename(self)
        self.file_list.append(output_file_name)
        return output_file_name


def show_image(filename, show=True):
    """ Display an image file in ipython """
    from IPython.core.display import Image, display

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
