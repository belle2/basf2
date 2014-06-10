package b2daq.nsm;

import b2daq.core.Enum;

public class NSMCommand extends Enum {

    final static public int DISABLED = 0;
    final static public int ENABLED = 1;
    final static public int SUGGESTED = 2;

    final static public NSMCommand OK = new NSMCommand(1, "OK");
    final static public NSMCommand ERROR = new NSMCommand(2, "ERROR");//param[0] : command if >0
    final static public NSMCommand LOG = new NSMCommand(3, "LOG");
    final static public NSMCommand STATECHECK = new NSMCommand(201, "STATECHECK");
    final static public NSMCommand STATE = new NSMCommand(202, "STATE");
    final static public NSMCommand NSMINIT = new NSMCommand(400, "NSMINIT");
    final static public NSMCommand NSMSET = new NSMCommand(401, "NSMSET");
    final static public NSMCommand NSMGET = new NSMCommand(402, "NSMGET");
    final static public NSMCommand DBSET = new NSMCommand(403, "DBSET");
    final static public NSMCommand DBGET = new NSMCommand(404, "DBGET");
    final static public NSMCommand LISTSET = new NSMCommand(405, "LISTSET");
    final static public NSMCommand LISTGET = new NSMCommand(406, "LISTGET");

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
        } else if (msg.equals(STATECHECK.getLabel())) {
            copy(STATECHECK);
            return true;
        } else if (msg.equals(STATE.getLabel())) {
            copy(STATE);
            return true;
        } else if (msg.equals(NSMGET.getLabel())) {
            copy(NSMGET);
            return true;
        } else if (msg.equals(NSMSET.getLabel())) {
            copy(NSMSET);
            return true;
        } else if (msg.equals(DBGET.getLabel())) {
            copy(DBGET);
            return true;
        } else if (msg.equals(DBSET.getLabel())) {
            copy(DBSET);
            return true;
        } else if (msg.equals(LISTGET.getLabel())) {
            copy(LISTGET);
            return true;
        } else if (msg.equals(LISTSET.getLabel())) {
            copy(LISTSET);
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
        } else if (id == STATECHECK.getId()) {
            copy(STATECHECK);
            return true;
        } else if (id == STATE.getId()) {
            copy(STATE);
            return true;
        } else if (id == NSMGET.getId()) {
            copy(NSMGET);
            return true;
        } else if (id == NSMSET.getId()) {
            copy(NSMSET);
            return true;
        } else if (id == DBGET.getId()) {
            copy(DBGET);
            return true;
        } else if (id == DBSET.getId()) {
            copy(DBSET);
            return true;
        } else if (id == LISTGET.getId()) {
            copy(LISTGET);
            return true;
        } else if (id == LISTSET.getId()) {
            copy(LISTSET);
            return true;
        }
        return false;
    }

}
