package b2daq.nsm;

import b2daq.core.Enum;

public class NSMCommand extends Enum {

    final static public int DISABLED = 0;
    final static public int ENABLED = 1;
    final static public int SUGGESTED = 2;

    final static public NSMCommand UNKNOWN = new NSMCommand(0, "UNKNOWN");
    final static public NSMCommand OK = new NSMCommand(1, "OK");
    final static public NSMCommand ERROR = new NSMCommand(2, "ERROR");
    final static public NSMCommand LOG = new NSMCommand(5, "LOG");
    final static public NSMCommand VGET = new NSMCommand(11, "VGET");
    final static public NSMCommand VSET = new NSMCommand(12, "VSET");
    final static public NSMCommand VLISTGET = new NSMCommand(13, "VLISTGET");
    final static public NSMCommand VLISTSET = new NSMCommand(14, "VLISTSET");
    final static public NSMCommand VREPLY = new NSMCommand(15, "VREPLY");
    final static public NSMCommand DATASET = new NSMCommand(401, "DATASET");
    final static public NSMCommand DATAGET = new NSMCommand(402, "DATAGET");
    final static public NSMCommand DBSET = new NSMCommand(403, "DBSET");
    final static public NSMCommand DBGET = new NSMCommand(404, "DBGET");
    final static public NSMCommand DBLISTSET = new NSMCommand(405, "DBLISTSET");
    final static public NSMCommand DBLISTGET = new NSMCommand(406, "DBLISTGET");
    final static public NSMCommand LOGLIST = new NSMCommand(407, "LOGLIST");

    public NSMCommand() {
        copy(NSMCommand.UNKNOWN);
    }

    protected NSMCommand(int id, String label) {
        super(id, label);
    }

    public NSMCommand(NSMCommand command) {
        copy(command);
    }

    public NSMCommand(String command) {
        copy(command);
    }

    public boolean copy(String msg) {
        if (msg.equals(UNKNOWN.getLabel())) {
            copy(UNKNOWN);
            return true;
        } else if (msg.equals(OK.getLabel())) {
            copy(OK);
            return true;
        } else if (msg.equals(ERROR.getLabel())) {
            copy(ERROR);
            return true;
        } else if (msg.equals(LOG.getLabel())) {
            copy(LOG);
            return true;
        } else if (msg.equals(VGET.getLabel())) {
            copy(VGET);
            return true;
        } else if (msg.equals(VSET.getLabel())) {
            copy(VSET);
            return true;
        } else if (msg.equals(VLISTGET.getLabel())) {
            copy(VLISTGET);
            return true;
        } else if (msg.equals(VLISTSET.getLabel())) {
            copy(VLISTSET);
            return true;
        } else if (msg.equals(VREPLY.getLabel())) {
            copy(VREPLY);
            return true;
        } else if (msg.equals(DATAGET.getLabel())) {
            copy(DATAGET);
            return true;
        } else if (msg.equals(DATASET.getLabel())) {
            copy(DATASET);
            return true;
        } else if (msg.equals(DBGET.getLabel())) {
            copy(DBGET);
            return true;
        } else if (msg.equals(DBSET.getLabel())) {
            copy(DBSET);
            return true;
        } else if (msg.equals(DBLISTGET.getLabel())) {
            copy(DBLISTGET);
            return true;
        } else if (msg.equals(DBLISTSET.getLabel())) {
            copy(DBLISTSET);
            return true;
        } else if (msg.equals(LOGLIST.getLabel())) {
            copy(LOGLIST);
            return true;
        }
        return false;
    }

    public boolean copy(int id) {
        if (id == UNKNOWN.getId()) {
            copy(UNKNOWN);
            return true;
        } else if (id == OK.getId()) {
            copy(OK);
            return true;
        } else if (id == ERROR.getId()) {
            copy(ERROR);
            return true;
        } else if (id == LOG.getId()) {
            copy(LOG);
            return true;
        } else if (id == VGET.getId()) {
            copy(VGET);
            return true;
        } else if (id == VSET.getId()) {
            copy(VSET);
            return true;
        } else if (id == VLISTGET.getId()) {
            copy(VLISTGET);
            return true;
        } else if (id == VLISTSET.getId()) {
            copy(VLISTSET);
            return true;
        } else if (id == VREPLY.getId()) {
            copy(VREPLY);
            return true;
        } else if (id == DATAGET.getId()) {
            copy(DATAGET);
            return true;
        } else if (id == DATASET.getId()) {
            copy(DATASET);
            return true;
        } else if (id == DBGET.getId()) {
            copy(DBGET);
            return true;
        } else if (id == DBSET.getId()) {
            copy(DBSET);
            return true;
        } else if (id == DBLISTGET.getId()) {
            copy(DBLISTGET);
            return true;
        } else if (id == DBLISTSET.getId()) {
            copy(DBLISTSET);
            return true;
        } else if (id == LOGLIST.getId()) {
            copy(LOGLIST);
            return true;
        }
        return false;
    }

}
