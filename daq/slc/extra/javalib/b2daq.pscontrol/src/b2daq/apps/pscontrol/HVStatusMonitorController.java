/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.database.ConfigObject;
import b2daq.database.DBObject;
import b2daq.dqm.core.Histo1D;
import b2daq.dqm.core.Histo2D;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.hvcontrol.core.HVState;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMData;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMNode;
import b2daq.nsm.NSMVSetHandler;
import b2daq.nsm.NSMVar;
import b2daq.nsm.ui.NSMRequestHandlerUI;
import java.io.IOException;
import java.util.HashMap;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class HVStatusMonitorController {

    @FXML
    private TabPane tabpane;
    @FXML
    private HistogramCanvas c_vol_frac;
    @FXML
    private Histo1D h_vol_frac;
    @FXML
    private HistogramCanvas c_cur_frac;
    @FXML
    private Histo1D h_cur_frac;

    @FXML
    private Histo2D h_channel_status;
    @FXML
    private HistogramCanvas c_channel_status;

    private ConfigObject obj;
    private HashMap<Integer, HVStabilityMonitorController> controllers = new HashMap();
    private NSMNode m_node;

    public void handleOnReceived(NSMMessage msg) {
        NSMCommand command = new NSMCommand(msg.getReqName());
        if (command.equals(NSMCommand.DATASET)) {
            NSMData data = (NSMData)msg.getObject();
            if (obj == null || !obj.hasObject("channel")) {
                return;
            }
            h_vol_frac.reset();
            h_cur_frac.reset();
            for (int i = 0; i < data.getNObjects("channel") && i < 9; i++) {
                NSMData cdata = (NSMData) data.getObject("channel", i);
                DBObject cobj = obj.getObject("channel", i);
                double voltage = cdata.getFloat("voltage_mon");
                double current = cdata.getFloat("current_mon");
                h_vol_frac.fill(voltage);
                h_cur_frac.fill(current);
                controllers.get(cobj.getInt("crate")).update(i, voltage, current);
                HVState state = HVState.get(cdata.getInt("state"));
                if (state.equals(HVState.OFF_S)) {
                    h_channel_status.setBinContent(cobj.getInt("channel") - 1, cobj.getInt("crate"), 1.5);
                } else if (state.equals(HVState.STANDBY_S)) {
                    h_channel_status.setBinContent(cobj.getInt("channel") - 1, cobj.getInt("crate"), 2.5);
                } else if (state.equals(HVState.SHOULDER_S)) {
                    h_channel_status.setBinContent(cobj.getInt("channel") - 1, cobj.getInt("crate"), 3.5);
                } else if (state.isTransition()) {
                    h_channel_status.setBinContent(cobj.getInt("channel") - 1, cobj.getInt("crate"), 4.5);
                } else if (state.equals(HVState.PEAK_S)) {
                    h_channel_status.setBinContent(cobj.getInt("channel") - 1, cobj.getInt("crate"), 5.5);
                } else {
                    h_channel_status.setBinContent(cobj.getInt("channel") - 1, cobj.getInt("crate"), 0);
                }
            }
            c_vol_frac.update();
            c_cur_frac.update();
            c_channel_status.update();
            for (HVStabilityMonitorController controller : controllers.values()) {
                controller.update();
            }
        }
    }

    void setNode(NSMNode node) {
        m_node = node;
        NSMRequestHandlerUI.get().add(new NSMVSetHandler(true, m_node.getName(), "ncrates", NSMVar.INT) {
            @Override
            public boolean handleVSet(NSMVar var) {
                int ncrates = var.getInt();
                for (int i = 0; i < ncrates; i++) {
                    try {
                        FXMLLoader loader = new FXMLLoader(HVStabilityMonitorController.class.getResource("HVStabilityMonitor.fxml"));
                        loader.load();
                        HVStabilityMonitorController controller = loader.getController();
                        Tab tab = new Tab();
                        //tab.setText(cobj.getObject("crate", i).getText("label"));
                        tab.setContent(controller.getPane());
                        tab.setClosable(false);
                        tabpane.getTabs().add(tab);
                        //controllers.put(cobj.getObject("crate", i).getInt("crateid"), controller);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
                return true;
            }
        });
    }

}
