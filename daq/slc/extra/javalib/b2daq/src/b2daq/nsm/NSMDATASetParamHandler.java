/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm;

import b2daq.database.FieldInfo;
import static b2daq.database.FieldInfo.CHAR;
import static b2daq.database.FieldInfo.DOUBLE;
import static b2daq.database.FieldInfo.FLOAT;
import static b2daq.database.FieldInfo.INT;
import static b2daq.database.FieldInfo.SHORT;

/**
 *
 * @author tkonno
 */
public abstract class NSMDATASetParamHandler extends NSMRequestHandler {

    private String m_name = "";
    private  String m_pname = "";
    private  int m_type = FieldInfo.NONE;

    String getDataName() { return m_name; }
    String getParName() { return m_pname; }
    int getType() { return m_type; }

    public NSMDATASetParamHandler(boolean once, String name, String pname, int type) {
        super(once);
        m_name = name;
        m_pname = pname;
        m_type = type;
    }

    public NSMDATASetParamHandler(String name, String pname, int type) {
        super(false);
        m_name = name;
        m_pname = pname;
        m_type = type;
    }

    @Override
    public boolean handle(NSMCommand cmd, NSMMessage msg) {
        if (cmd.equals(NSMCommand.DATASET)) {
            NSMData data = (NSMData) msg.getObject();
            if (!m_name.equals(data.getName())) return false;
            try {
                Object obj = data.find(m_pname);
                if (obj == null) {
                    System.out.println("No data for "+m_pname);
                    return false;
                }
                switch (m_type) {
                    case CHAR:
                        return handleDataSetParam((Character)obj);
                    case SHORT:
                        return handleDataSetParam((Short)obj);
                    case INT:
                        return handleDataSetParam((Integer)obj);
                    case FLOAT:
                        return handleDataSetParam((Float)obj);
                    case DOUBLE:
                        return handleDataSetParam((Double)obj);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        System.out.println("No data handled "+m_pname);
        return false;
    }

    @Override
    public boolean connected() {
        return true;
    }

    public boolean handleDataSetParam(char val) {
        return true;
    }

    public boolean handleDataSetParam(short val) {
        return true;
    }

    public boolean handleDataSetParam(int val) {
        return true;
    }

    public boolean handleDataSetParam(float val) {
        return true;
    }

    public boolean handleDataSetParam(double val) {
        return true;
    }

}
