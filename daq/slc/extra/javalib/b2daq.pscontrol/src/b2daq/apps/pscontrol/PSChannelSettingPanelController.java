/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.hvcontrol.core.HVState;
import b2daq.hvcontrol.ui.StateLabel;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMNode;
import b2daq.nsm.NSMObserver;
import b2daq.nsm.NSMVar;
import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class PSChannelSettingPanelController implements NSMObserver {

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
    StateLabel slabel_state;
    @FXML
    Label label_vmon;
    @FXML
    Label label_cmon;

    private int m_crateid;
    private int m_index;
    private int m_slot;
    private int m_channel;
    private NSMNode m_node;

    public void setCrateId(int crateid) {
        m_crateid = crateid;
    }

    public void setIndex(int index) {
        m_index = index;
    }

    public void setNode(NSMNode node) {
        m_node = node;
    }

    public String getVName() {
        return "crate[" + m_crateid + "].slot[" + m_slot + "].channel[" + m_channel + "]";
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
            NSMListenerService.requestVSet(m_node.getName(), new NSMVar(getVName("switch"), (turnon?"ON":"OFF")));
            button_turnon.setText(((!turnon)?"ON":"OFF"));
            NSMListenerService.requestVGet(m_node.getName(), getVName("switch"));
        } catch (Exception e) {

        }
    }

    @FXML
    protected void handleTurnon() {
        try {
            turnon(button_turnon.getText().equals("ON"));
            NSMListenerService.requestVGet(m_node.getName(), getVName("switch"));
        } catch (Exception e) {

        }
    }

    @FXML
    protected void handleRampdown() {
        try {
            float v = Float.parseFloat(field_rampdown.getText());
            NSMListenerService.requestVSet(m_node.getName(), new NSMVar(getVName("rampdown"), v));
            NSMListenerService.requestVGet(m_node.getName(), getVName("rampup"));
        } catch (Exception e) {
        }
    }

    @FXML
    protected void handleRampup() {
        try {
            float v = Float.parseFloat(field_rampup.getText());
            NSMListenerService.requestVSet(m_node.getName(), new NSMVar(getVName("rampup"), v));
            NSMListenerService.requestVGet(m_node.getName(), getVName("rampdown"));
        } catch (Exception e) {
        }
    }

    @FXML
    protected void handleVdemand() {
        try {
            float v = Float.parseFloat(field_vdemand.getText());
            NSMListenerService.requestVSet(m_node.getName(), new NSMVar(getVName("vdemand"), v));
            NSMListenerService.requestVGet(m_node.getName(), getVName("vdemand"));
        } catch (Exception e) {
        }
    }

    @FXML
    protected void handleVlimit() {
        try {
            float v = Float.parseFloat(field_vlimit.getText());
            NSMListenerService.requestVSet(m_node.getName(), new NSMVar(getVName("vlimit"), v));
            NSMListenerService.requestVGet(m_node.getName(), getVName("vlimit"));
        } catch (Exception e) {
        }
    }

    @FXML
    protected void handleClimit() {
        try {
            float v = Float.parseFloat(field_climit.getText());
            NSMListenerService.requestVSet(m_node.getName(), new NSMVar(getVName("climit"), v));
            NSMListenerService.requestVGet(m_node.getName(), getVName("climit"));
        } catch (Exception e) {
        }
    }

    @Override
    public void handleOnConnected() {
        NSMListenerService.requestVGet(m_node.getName(), "crate[" + m_crateid + "].channel[" + m_index + "].slot");
        NSMListenerService.requestVGet(m_node.getName(), "crate[" + m_crateid + "].channel[" + m_index + "].channel");
    }

    public void requestUpdate() {
        NSMListenerService.requestVGet(m_node.getName(), getVName("state"));
        NSMListenerService.requestVGet(m_node.getName(), getVName("vmon"));
        NSMListenerService.requestVGet(m_node.getName(), getVName("cmon"));
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
        NSMCommand cmd = new NSMCommand(msg.getReqName());
        if (cmd.equals(NSMCommand.VSET)) {
            NSMVar var = (NSMVar) msg.getObject();
            switch (var.getType()) {
                case NSMVar.INT:
                    if (var.getName().contains(".channel[" + m_index + "].channel")) {
                        m_channel = var.getInt();
                        label.setText(m_slot + " - " + m_channel);
                    } else if (var.getName().contains(".channel[" + m_index + "].slot")) {
                        m_slot = var.getInt();
                        label.setText(m_slot + " - " + m_channel);
                    } else {
                        break;
                    }
                    if (m_channel > 0 && m_slot > 0) {
                        NSMListenerService.requestVGet(m_node.getName(), getVName("rampup"));
                        NSMListenerService.requestVGet(m_node.getName(), getVName("rampdown"));
                        NSMListenerService.requestVGet(m_node.getName(), getVName("vdemand"));
                        NSMListenerService.requestVGet(m_node.getName(), getVName("vlimit"));
                        NSMListenerService.requestVGet(m_node.getName(), getVName("climit"));
                        NSMListenerService.requestVGet(m_node.getName(), getVName("switch"));
                        NSMListenerService.requestVGet(m_node.getName(), getVName("state"));
                        NSMListenerService.requestVGet(m_node.getName(), getVName("vmon"));
                        NSMListenerService.requestVGet(m_node.getName(), getVName("cmon"));
                    }
                    break;
                case NSMVar.FLOAT:
                    if (var.getName().equals(getVName("rampup"))) {
                        field_rampup.setText("" + var.getFloat());
                    } else if (var.getName().equals(getVName("rampdown"))) {
                        field_rampdown.setText("" + var.getFloat());
                    } else if (var.getName().equals(getVName("vdemand"))) {
                        field_vdemand.setText("" + var.getFloat());
                    } else if (var.getName().equals(getVName("vlimit"))) {
                        field_vlimit.setText("" + var.getFloat());
                    } else if (var.getName().equals(getVName("climit"))) {
                        field_climit.setText("" + var.getFloat());
                    } else if (var.getName().equals(getVName("vmon"))) {
                        label_vmon.setText("" + var.getFloat() + " [V]");
                    } else if (var.getName().equals(getVName("cmon"))) {
                        label_cmon.setText("" + var.getFloat() + " [uA]");
                    }
                    break;
                case NSMVar.TEXT:
                    if (var.getName().equals(getVName("switch"))) {
                        if (var.getText().equals("OFF")) {
                            button_turnon.setText("ON");
                        } else if (var.getText().equals("ON")) {
                            button_turnon.setText("OFF");
                        }
                    } else if (var.getName().equals(getVName("state"))) {
                        slabel_state.update(new HVState(var.getText()));
                    }
                    break;
            }
        }
    }

    @Override
    public void handleOnDisConnected() {

    }

    @Override
    public void log(LogMessage log) {

    }
}
