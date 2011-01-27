"""Dashboard reporter IAction implementation.
 Uses a gangaRobot Extraction to  construct a dashboard for the VO
"""

from GangaRobot.Framework import Utility
from GangaRobot.Lib.Base.BaseReporter import BaseReporter
from GangaRobot.Lib.Base.Report import Link, Heading, Table


class DashboardReporter(BaseReporter):

    """Dashboard reporter IAction implementation.
    
        Uses a gangaRobot Extraction to  construct a dashboard for the VO
    """

    def handlereport(self, report, runnode):
        """Add statistics on generic data to the report.
        
        Keyword arguments:
        report -- A Report.Report for the run, to be emailed.
        runnode -- A the extracted data for the run.
        
        If the report title is undefined a title 'CoreReporter ${runid}' is set.

        If the configurable option CoreReporter_ExtractUrl is defined then a
        link is added from 'Run id', replacing ${runid} with the current run id.
        e.g. http://localhost/robot/extract/${runid}.xml
        
        Example of report generated (plain text version):
        CoreReporter 2007-06-27_10.49.40
        ********************************
        
        Core Analysis
        *************
        
        Run id       : 2007-06-27_10.49.40 (http://localhost/robot/extract/2007-06-27_10.49.40.xml)
        Start time   : 2007/06/27 10:49:40
        Extract time : 2007/06/27 10:49:55
        
        Status               | Subtotal
        -------------------------------
        completed            |        3
        submitted            |        1
        failed               |        1
        Total                |        5
        
        ActualCE                                                    | Status
        -------------------------------------------------------------------------------
        lx09.hep.ph.ic.ac.uk                                        |  3 / 5
        
        Non-completed Jobs
        ==================
        Id    | Status     | Backend  | Backend.id             | ActualCE              
        -------------------------------------------------------------------------------
        51    | failed     | Local    | 13418                  | lx09.hep.ph.ic.ac.uk  
        53    | submitted  | Local    | None                   | lx09.hep.ph.ic.ac.uk  
        
        """
        runid = runnode.getvalue('core.id')

        # get configuration options
        extracturl = Utility.expand(self.getoption('CoreReporter_ExtractUrl'), runid = runid)
        
        #CoreReporter id
        if not report.title:
            report.title = 'Belle VO Dashboard (' + runid + ')'

        #Core Analysis
        report.addline(Heading('Last Run', 2))
        report.addline()
        
        #Run id       : ...
        report.addline('Run id       :')
        if extracturl:
            report.addelement(Link(runid, extracturl))
        else:
            report.addelement(runid)
        #Start time   : ...
        #Extract time : ...
        report.addline('Start time   : ' + runnode.getvalue('core.start-time'))
        report.addline('Extract time : ' + runnode.getvalue('core.extract-time'))
        report.addline()
        
        #Status | Subtotal
        #...
        #Total          10
        report.addline(self._getstatustable(runnode))
        report.addline()
        
        #ActualCE | Completed | Total
        #...
        report.addline(self._getcetable(runnode))
        report.addline()
        
        #Non-completed Jobs
        report.addline(Heading('Non-completed Jobs'))
        #Id | Status | Backend | Backend.id | ActualCE
        #...
        report.addline(self._getnoncompletedtable(runnode))
        report.addline()
    
    def _getstatustable(self, runnode):
        t = Table()
        t.hformat = '%-20s | %8s'
        t.header = ('Status', 'Subtotal')
        t.rformat = '%-20s | %8d'
        
        statussubtotals = {} # dict {'my-status':subtotal, ...}
        total = 0
        for status in runnode.getvalues('job.core.status'):
            if not status in statussubtotals:
                statussubtotals[status] = 0
            statussubtotals[status] += 1
            total += 1
        
        # add rows ('my-status', subtotal)    
        for (status, subtotal) in statussubtotals.items():
            t.addrow(status, subtotal)
        
        # add row ('Total', total)
        t.addrow('Total', total)

        return t

    def _getcetable(self, runnode):
        t = Table()
        t.hformat = '%-59s | %9s | %9s'
        t.header = ('ActualCE', 'Result', 'Reason')
        t.rformat = '%-59s | %9s | %9s'
        
        cestats = {} # dict {'my-ce': (completed, total)}
        for corenode in runnode.getnodes('job.core'):
            ce = corenode.getvalue('backend-actualCE')
            status = corenode.getvalue('status')
            if not ce in cestats:
                stats = [0, 0, 'nfi']
                cestats[ce] = stats
            else:
                stats = cestats[ce]
            if status == 'completed':
                stats[0] += 1 #increment completed
            stats[1] += 1 #increment total
            stats[2] = runnode.getvalue('job.backend-reason')

        # add rows ('my-ce', completed, total)
        for (ce, stats) in cestats.items():
            t.addrow(ce, str(stats[0]) + '/' + str(stats[1]), stats[2])
        
        return t

    def _getnoncompletedtable(self, runnode):
        t = Table()
        t.hformat = '%-5s | %-10s | %-8s | %-22s'
        t.header = ('Id', 'Status', 'Backend.id', 'ActualCE')
        t.rformat = '%-5s | %-10s | %-8s | %-22s'
        
        for corenode in runnode.getnodes('job.core'):
            if corenode.getvalue('status') != 'completed':
                t.addrow(corenode.getvalue('id'),
                         corenode.getvalue('status'),
                         corenode.getvalue('backend-id'),
                         corenode.getvalue('backend-actualCE'))
        
        return t
