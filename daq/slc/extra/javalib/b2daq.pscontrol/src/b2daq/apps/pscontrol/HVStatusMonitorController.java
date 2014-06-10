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
import b2daq.dqm.core.TimedGraph1D;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.hvcontrol.core.HVState;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMData;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import java.net.URL;
import java.util.ResourceBundle;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class HVStatusMonitorController implements Initializable, NSMObserver {

    @FXML
    private HistogramCanvas c_node0_vol_gr;
    @FXML
    private TimedGraph1D gr_vol_node0_ch0;
    @FXML
    private TimedGraph1D gr_vol_node0_ch1;
    @FXML
    private TimedGraph1D gr_vol_node0_ch2;
    @FXML
    private TimedGraph1D gr_vol_node0_ch3;
    @FXML
    private TimedGraph1D gr_vol_node0_ch4;
    @FXML
    private TimedGraph1D gr_vol_node0_ch5;
    @FXML
    private TimedGraph1D gr_vol_node0_ch6;
    @FXML
    private TimedGraph1D gr_vol_node0_ch7;
    @FXML
    private TimedGraph1D gr_vol_node0_ch8;
    @FXML
    private HistogramCanvas c_vol_frac;
    @FXML
    private Histo1D h_vol_frac;

    private TimedGraph1D gr_vol_node0[];
    
    @FXML
    private HistogramCanvas c_node0_cur_gr;
    @FXML
    private TimedGraph1D gr_cur_node0_ch0;
    @FXML
    private TimedGraph1D gr_cur_node0_ch1;
    @FXML
    private TimedGraph1D gr_cur_node0_ch2;
    @FXML
    private TimedGraph1D gr_cur_node0_ch3;
    @FXML
    private TimedGraph1D gr_cur_node0_ch4;
    @FXML
    private TimedGraph1D gr_cur_node0_ch5;
    @FXML
    private TimedGraph1D gr_cur_node0_ch6;
    @FXML
    private TimedGraph1D gr_cur_node0_ch7;
    @FXML
    private TimedGraph1D gr_cur_node0_ch8;
    @FXML
    private HistogramCanvas c_cur_frac;
    @FXML
    private Histo1D h_cur_frac;

    private TimedGraph1D gr_cur_node0[];

    @FXML
    private Histo2D h_channel_status;
    @FXML
    private HistogramCanvas c_channel_status;
    private ConfigObject obj;
    
    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        gr_vol_node0 = new TimedGraph1D[] {
            gr_vol_node0_ch0, gr_vol_node0_ch1, gr_vol_node0_ch2, 
            gr_vol_node0_ch3, gr_vol_node0_ch4, gr_vol_node0_ch5, 
            gr_vol_node0_ch6, gr_vol_node0_ch7, gr_vol_node0_ch8, 
        };
        gr_cur_node0 = new TimedGraph1D[] {
            gr_cur_node0_ch0, gr_cur_node0_ch1, gr_cur_node0_ch2, 
            gr_cur_node0_ch3, gr_cur_node0_ch4, gr_cur_node0_ch5, 
            gr_cur_node0_ch6, gr_cur_node0_ch7, gr_cur_node0_ch8, 
        };
    }

    @Override
    public void handleOnConnected() {
        //h1.setLineColor(Color.LIMEGREEN);
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
        if (msg == null) return;
        NSMCommand command = new NSMCommand(msg.getReqName());
        if (command.equals(NSMCommand.NSMSET)) {
            NSMData data = NSMListenerService.getData("ARICH_HV_STATUS");
            if (obj == null || !obj.hasObject("channel")) return;
            h_vol_frac.reset();
            h_cur_frac.reset();
            for (int i = 0; i < data.getNObjects("channel") && i < 9; i++) {
                NSMData cdata = (NSMData)data.getObject("channel", i);
                DBObject cobj = obj.getObject("channel", i);
                double voltage = cdata.getFloat("voltage_mon");
                if (i < gr_vol_node0.length ) {
                    gr_vol_node0[i].addPoint(voltage);
                    h_vol_frac.fill(voltage);
                }
                double current = cdata.getFloat("current_mon");
                if (i < gr_cur_node0.length ) {
                    gr_cur_node0[i].addPoint(current);
                    h_cur_frac.fill(current);
                }
                HVState state = HVState.get(cdata.getInt("state"));
                if (state.equals(HVState.OFF_S)) {
                    h_channel_status.setBinContent(cobj.getInt("slot") - 1, cobj.getInt("crate"), 1.5);
                } else if (state.equals(HVState.PEAK_S)) {
                    h_channel_status.setBinContent(cobj.getInt("slot") - 1, cobj.getInt("crate"), 4.5);
                } else if (state.isStable()) {
                    h_channel_status.setBinContent(cobj.getInt("slot") - 1, cobj.getInt("crate"), 2.5);
                } else if (state.isTransition()) {
                    h_channel_status.setBinContent(cobj.getInt("slot") - 1, cobj.getInt("crate"), 3.5);
                } else {
                    h_channel_status.setBinContent(cobj.getInt("slot") - 1, cobj.getInt("crate"), 0);
                }
            }
            c_node0_vol_gr.update();
            c_vol_frac.update();
            c_node0_cur_gr.update();
            c_cur_frac.update();
            c_channel_status.update();
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
        this.obj = cobj;
    }

}
