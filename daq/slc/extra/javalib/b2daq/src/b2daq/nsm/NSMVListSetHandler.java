/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm;

import java.io.IOException;
import java.util.ArrayList;

/**
 *
 * @author tkonno
 */
public abstract class NSMVListSetHandler extends NSMRequestHandler {

    protected String m_node = "";

    public NSMVListSetHandler(boolean once, String node) {
        super(once);
        m_node = node.toUpperCase();
    }

    @Override
    public boolean connected() {
        try {
            NSMCommunicator.get().requestVListGet(m_node);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return true;

    }

    public class VarEntry {

        public String name;
        public int id;
        public String type;
        public boolean useGet;
        public boolean useSet;

        public VarEntry() {

        }

        public VarEntry(String name, int id, String type, boolean useGet, boolean useSet) {
            this.name = name;
            this.id = id;
            this.type = type;
            this.useGet = useGet;
            this.useSet = useSet;
        }
        
        public String toString() {
            return name;
        }
        
    }

    @Override
    public boolean handle(NSMCommand cmd, NSMMessage msg) {
        if (m_node.equals(msg.getNodeName()) && cmd.equals(NSMCommand.VLISTSET)) {
            ArrayList<VarEntry> vars = new ArrayList<>();
            String[] s = msg.getData().split("\n");
            for (String cs : s) {
                String[] ss = cs.split(":");
                if (ss.length > 4) {
                    vars.add(new VarEntry(ss[0], Integer.parseInt(ss[4]),
                            ss[1], ss[2].equals("1"), ss[3].equals("1")));
                }
            }
            handleVListSet(vars);
            return true;
        }
        return false;
    }

    abstract public boolean handleVListSet(ArrayList<VarEntry> vars);
}
