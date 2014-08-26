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
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMData;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import java.io.IOException;
import java.net.URL;
import java.util.HashMap;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class HVStatusMonitorController implements Initializable, NSMObserver {

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

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
    }

    @Override
    public void handleOnConnected() {
        //h1.setLineColor(Color.LIMEGREEN);
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
        if (msg == null) {
            return;
        }
        NSMCommand command = new NSMCommand(msg.getReqName());
        if (command.equals(NSMCommand.NSMSET)) {
            NSMData data = NSMListenerService.getData(msg.getNodeName());//;"CDC_HV_STATUS");
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
                    h_channel_status.setBinContent(cobj.getInt("channel") -1, cobj.getInt("crate"), 1.5);
                } else if (state.equals(HVState.STANDBY_S)) {
                    h_channel_status.setBinContent(cobj.getInt("channel") -1, cobj.getInt("crate"), 2.5);
                } else if (state.equals(HVState.SHOULDER_S)) {
                    h_channel_status.setBinContent(cobj.getInt("channel") -1, cobj.getInt("crate"), 3.5);
                } else if (state.isTransition()) {
                    h_channel_status.setBinContent(cobj.getInt("channel") -1, cobj.getInt("crate"), 4.5);
                } else if (state.equals(HVState.PEAK_S)) {
                    h_channel_status.setBinContent(cobj.getInt("channel") -1, cobj.getInt("crate"), 5.5);
                } else {
                    h_channel_status.setBinContent(cobj.getInt("channel") -1, cobj.getInt("crate"), 0);
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

    @Override
    public void handleOnDisConnected() {
        //h1.setLineColor(Color.DEEPPINK);
    }

    @Override
    public void log(LogMessage log) {
    }

    void setDB(ConfigObject cobj) {
        if (cobj == null || !cobj.hasObject("crate")) return;
        this.obj = cobj;
        for (int i = 0; i < cobj.getNObjects("crate"); i++) {
            try {
                FXMLLoader loader = new FXMLLoader(HVStabilityMonitorController.class.getResource("HVStabilityMonitor.fxml"));
                loader.load();
                HVStabilityMonitorController controller = loader.getController();
                Tab tab = new Tab();
                tab.setText(cobj.getObject("crate", i).getText("label"));
                tab.setContent(controller.getPane());
                tab.setClosable(false);
                tabpane.getTabs().add(tab);
                controllers.put(cobj.getObject("crate", i).getInt("crateid"), controller);
            } catch (IOException e) {
                e.printStackTrace();
                Logger.getLogger(PowerSupplyStatusPaneController.class.getName()).log(Level.SEVERE, null, e);
            }
        }
    }

}
