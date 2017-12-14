/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.hvcontrol.core.HVState;
import b2daq.hvcontrol.ui.HVStateLabel;
import b2daq.nsm.NSMCommunicator;
import b2daq.nsm.NSMNode;
import b2daq.nsm.NSMVSetHandler;
import b2daq.nsm.NSMVar;
import b2daq.nsm.ui.NSMRequestHandlerUI;
import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class PSChannelSettingPanelController {

    @FXML
    Label label;
    @FXML
    Button button_turnon;
    @FXML
    TextField field_rampup;
    @FXML
    TextField field_rampdown;
    @FXML
    TextField field_vdemand;
    @FXML
    TextField field_vlimit;
    @FXML
    TextField field_climit;
    @FXML
    HVStateLabel slabel_state;
    @FXML
    Label label_vmon;
    @FXML
    Label label_cmon;

    private int m_crateid;
    private int m_index;
    private int m_slot;
    private int m_channel;
    private NSMNode m_hvnode;

    public void setCrateId(int crateid) {
        m_crateid = crateid;
    }

    public void setIndex(int index) {
        m_index = index;
    }

    public void setNode(NSMNode node) {
        m_hvnode = node;
        String vname = "crate[" + m_crateid + "].channel[" + m_index + "].";
        NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, m_hvnode.getName(), vname + "channel", NSMVar.INT) {
            @Override
            public boolean handleVSet(NSMVar var) {
                m_channel = var.getInt();
                label.setText(m_slot + " - " + m_channel);
                addValues();
                return true;
            }
        });
        NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, m_hvnode.getName(), vname + "slot", NSMVar.INT) {
            @Override
            public boolean handleVSet(NSMVar var) {
                m_slot = var.getInt();
                label.setText(m_slot + " - " + m_channel);
                addValues();
                return true;
            }
        });
    }

    private void addValues() {
        if (m_slot > 0 && m_channel >= 0) {
            NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, m_hvnode.getName(), getVName("rampup"), NSMVar.FLOAT) {
                @Override
                public boolean handleVSet(NSMVar var) {
                    field_rampup.setText("" + var.getFloat());
                    return true;
                }
            });
            NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, m_hvnode.getName(), getVName("rampdown"), NSMVar.FLOAT) {
                @Override
                public boolean handleVSet(NSMVar var) {
                    field_rampdown.setText("" + var.getFloat());
                    return true;
                }
            });
            NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, m_hvnode.getName(), getVName("vdemand"), NSMVar.FLOAT) {
                @Override
                public boolean handleVSet(NSMVar var) {
                    field_vdemand.setText("" + var.getFloat());
                    return true;
                }
            });
            NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, m_hvnode.getName(), getVName("vlimit"), NSMVar.FLOAT) {
                @Override
                public boolean handleVSet(NSMVar var) {
                    field_vlimit.setText("" + var.getFloat());
                    return true;
                }
            });
            NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, m_hvnode.getName(), getVName("climit"), NSMVar.FLOAT) {
                @Override
                public boolean handleVSet(NSMVar var) {
                    field_climit.setText("" + var.getFloat());
                    return true;
                }
            });
            NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, m_hvnode.getName(), getVName("vmon"), NSMVar.FLOAT) {
                @Override
                public boolean handleVSet(NSMVar var) {
                    label_vmon.setText("" + var.getFloat() + " [V]");
                    return true;
                }
            });
            NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, m_hvnode.getName(), getVName("cmon"), NSMVar.FLOAT) {
                @Override
                public boolean handleVSet(NSMVar var) {
                    label_cmon.setText("" + var.getFloat() + " [uA]");
                    return true;
                }
            });
            NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, m_hvnode.getName(), getVName("switch"), NSMVar.TEXT) {
                @Override
                public boolean handleVSet(NSMVar var) {
                    if (var.getText().equals("OFF")) {
                        button_turnon.setText("ON");
                    } else if (var.getText().equals("ON")) {
                        button_turnon.setText("OFF");
                    }
                    return true;
                }
            });
            NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, m_hvnode.getName(), getVName("state"), NSMVar.TEXT) {
                @Override
                public boolean handleVSet(NSMVar var) {
                    slabel_state.update(new HVState(var.getText()));
                    return true;
                }
            });
        }
    }

    public String getVName() {
        return "crate[" + m_crateid + "].slot[" + m_slot + "].channel[" + m_channel + "]";
        //return "crate[" + m_crateid + "].channel[" + m_index + "]";
    }

    public String getVName(String name) {
        return getVName() + "." + name;
    }

    public int getCratId() {
        return m_crateid;
    }

    public int getIndex() {
        return m_index;
    }

    public int getSlot() {
        return m_slot;
    }

    public int getChannel() {
        return m_channel;
    }

    protected void turnon(boolean turnon) {
        try {
            NSMCommunicator.get().requestVSet(m_hvnode.getName(), new NSMVar(getVName("switch"), (turnon ? "ON" : "OFF")));
            button_turnon.setText(((!turnon) ? "ON" : "OFF"));
            NSMCommunicator.get().requestVGet(m_hvnode.getName(), getVName("switch"));
        } catch (Exception e) {

        }
    }

    @FXML
    protected void handleTurnon() {
        try {
            turnon(button_turnon.getText().equals("ON"));
            NSMCommunicator.get().requestVGet(m_hvnode.getName(), getVName("switch"));
        } catch (Exception e) {

        }
    }

    @FXML
    protected void handleRampdown() {
        try {
            float v = Float.parseFloat(field_rampdown.getText());
            NSMCommunicator.get().requestVSet(m_hvnode.getName(), new NSMVar(getVName("rampdown"), v));
            NSMCommunicator.get().requestVGet(m_hvnode.getName(), getVName("rampup"));
        } catch (Exception e) {
        }
    }

    @FXML
    protected void handleRampup() {
        try {
            float v = Float.parseFloat(field_rampup.getText());
            NSMCommunicator.get().requestVSet(m_hvnode.getName(), new NSMVar(getVName("rampup"), v));
            NSMCommunicator.get().requestVGet(m_hvnode.getName(), getVName("rampdown"));
        } catch (Exception e) {
        }
    }

    @FXML
    protected void handleVdemand() {
        try {
            float v = Float.parseFloat(field_vdemand.getText());
            NSMCommunicator.get().requestVSet(m_hvnode.getName(), new NSMVar(getVName("vdemand"), v));
            NSMCommunicator.get().requestVGet(m_hvnode.getName(), getVName("vdemand"));
        } catch (Exception e) {
        }
    }

    @FXML
    protected void handleVlimit() {
        try {
            float v = Float.parseFloat(field_vlimit.getText());
            NSMCommunicator.get().requestVSet(m_hvnode.getName(), new NSMVar(getVName("vlimit"), v));
            NSMCommunicator.get().requestVGet(m_hvnode.getName(), getVName("vlimit"));
        } catch (Exception e) {
        }
    }

    @FXML
    protected void handleClimit() {
        try {
            NSMCommunicator.get().requestVGet(null, null);
            float v = Float.parseFloat(field_climit.getText());
            NSMCommunicator.get().requestVSet(m_hvnode.getName(), new NSMVar(getVName("climit"), v));
            NSMCommunicator.get().requestVGet(m_hvnode.getName(), getVName("climit"));
        } catch (Exception e) {
        }
    }

}
