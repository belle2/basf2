from time import sleep

import basf2
from ROOT import Belle2

import ctypes


class PrinterModule(basf2.Module):
    def __init__(self, print_message, raise_exception):
        self.print_message = print_message
        self.raise_exception = raise_exception

        basf2.Module.__init__(self)

    def event(self):
        meta_data = Belle2.PyStoreObj("EventMetaData")
        basf2.B2INFO("{msg}: {num}".format(msg=self.print_message, num=meta_data.getEvent()))
        sleep(0.1)
#        if meta_data.getEvent() == 3 and self.raise_exception:
#             i = ctypes.c_char(b'a')
#             j = ctypes.pointer(i)
#             c = 0
#             while True:
#                j[c] = b'a'
#                c += 1
#             j
#             raise ValueError


if __name__ == '__main__':
    basf2.set_debug_level(100)
    basf2.set_log_level(basf2.LogLevel.DEBUG)
    for n_process in [1]:

        path = basf2.create_path()
        basf2.set_nprocesses(n_process)

        path.add_module("EventInfoSetter", evtNumList=[5])
        path.add_module(PrinterModule("Input", False))

        path.add_module(PrinterModule("Reco", True)).set_property_flags(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)

        path.add_module(PrinterModule("Output", False))

        basf2.process(path)
        print(basf2.statistics)


# import basf2
# from time import sleep


# class PrinterModule(basf2.Module):
#    def __init__(self, print_message):
#        self.print_message = print_message

#        basf2.Module.__init__(self)

#    def event(self):
#        basf2.B2INFO(self.print_message)
#        sleep(1)


# if __name__ == '__main__':
#    path = basf2.create_path()
#    basf2.set_nprocesses(2)

#    path.add_module("EventInfoSetter", evtNumList=[10])
#    path.add_module(PrinterModule("Input"))

#    path.add_module(PrinterModule("Reco")).set_property_flags(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
#    path.add_module(PrinterModule("Output"))

#    basf2.process(path)
#    print(basf2.statistics)
