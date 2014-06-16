/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.database.ConfigObject;
import b2daq.hvcontrol.core.HVState;
import b2daq.hvcontrol.ui.StateLabel;
import b2daq.logger.ui.LogViewPaneController;
import b2daq.nsm.NSMConfig;
import b2daq.nsm.NSMData;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.control.Label;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.layout.GridPane;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class PowerSupplyStatusPaneController implements Initializable {

    @FXML
    private TabPane tabpane;
    @FXML
    private GridPane grid_crates;
    
    @FXML
    private Label label_ps;
    @FXML
    private Label label0;
    @FXML
    private Label label1;
    @FXML
    private Label label2;
    @FXML
    private Label label3;
    @FXML
    private Label label4;

    @FXML
    private Label label_update;
    //@FXML
    //private Label label_modules;
    //@FXML
    //private Label label_channels;
    @FXML
    private Label label_config;
    @FXML
    private StateLabel state_ps;

    private final ObservableList<PowerSupplyCrateStatusTableController> 
            tables = FXCollections.observableArrayList();
    private final ObservableList<StateLabel> states
            = FXCollections.observableArrayList();

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        Label[] labels = new Label[]{label0, label1, label2, label3, label4};
        for (Label label : labels) {
            StateLabel slabel = addLabel(label, grid_crates);
            if (slabel == null) {
                continue;
            }
            try {
                states.add(slabel);
                FXMLLoader loader = new FXMLLoader(PowerSupplyCrateStatusTableController.class.getResource("PowerSupplyCrateStatusTable.fxml"));
                loader.load();
                PowerSupplyCrateStatusTableController controller = loader.getController();
                Tab tab = new Tab();
                tab.setText(slabel.getName());
                tab.setContent(controller.getTable());
                tab.setClosable(false);
                tabpane.getTabs().add(tab);
                tables.add(controller);
            } catch (Exception e) {
                Logger.getLogger(PowerSupplyStatusPaneController.class.getName()).log(Level.SEVERE, null, e);
            }
        }
    }

    public void setConnected(NSMConfig config) {
        label_ps.setText(config.getNsmTarget());
    }

    public Label getLabelConfig() {
        return label_config;
    }

    public StateLabel getTextHVState() {
        return state_ps;
    }

    private StateLabel addLabel(Label label, GridPane g) {
        if (label == null || label.getText().length() == 0) {
            return null;
        }
        int icol = 0;
        int irow = 0;
        try {
            icol = GridPane.getColumnIndex(label);
        } catch (Exception e) {
        }
        try {
            irow = GridPane.getRowIndex(label);
        } catch (Exception e) {
        }
        StateLabel slabel = new StateLabel(label.getText());
        g.add(slabel, icol + 1, irow);
        return slabel;
    }

    public void setDB(ConfigObject cobj, LogViewPaneController logview) {
        if (cobj.hasObject("channel")) {
            for (ConfigObject obj : cobj.getObjects("channel")) {
                int crate = obj.getInt("crate");
                if (tables.size() > crate) {
                    tables.get(crate).setDB(obj);
                }
            }
        }
        label_config.setText(cobj.getName());
    }

    void update(HVState state, ConfigObject cobj, NSMData data) {
        state_ps.update(state);
        if (cobj.hasObject("channel")) {
            final SimpleDateFormat dateformat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
            label_update.setText(dateformat.format(new Date()));
            int n = 0, crate_tmp = 0;
            boolean isoff = true;
            boolean istransition = false;
            boolean iserror = false;
            int id = HVState.PEAK_S.getId();
            int ncrate = 0, ncrate_active = 0;
            int nchannel = 0, nchannel_active = 0;
            for (ConfigObject obj : cobj.getObjects("channel")) {
                int crate = obj.getInt("crate");
                if (crate != crate_tmp) {
                    n = 0;
                    if (isoff) {
                        id = HVState.OFF_S.getId();
                    } else if (istransition) {
                        id = HVState.TRANSITION_TS.getId();
                        ncrate_active++;
                    } else {
                        ncrate_active++;
                    }
                    if (iserror) {
                        id = HVState.ERROR_ES.getId();
                    }
                    state.copy(id);
                    if (tables.size() <= crate) {
                        continue;
                    }
                    states.get(crate_tmp).update(state);
                    isoff = true;
                    istransition = false;
                    iserror = false;
                    crate_tmp = crate;
                    ncrate++;
                }
                NSMData ch_data = (NSMData) data.getObject("channel", obj.getIndex());
                if (tables.size() <= crate) {
                    continue;
                }
                HVStatusProperty status = (HVStatusProperty) tables.get(crate).getStatus(n);
                state.copy(ch_data.getInt("state"));
                status.setState(state);
                if (!state.equals(HVState.OFF_S)) {
                    isoff = false;
                    if (state.isStable()) {
                        nchannel_active++;
                        if (state.getId() < id) {
                            id = state.getId();
                        }
                    }
                }
                if (state.isError()) {
                    iserror = true;
                }
                if (state.isTransition()) {
                    istransition = true;
                    nchannel_active++;
                }
                status.setVoltage(ch_data.getFloat("voltage_mon"));
                status.setCurrent(ch_data.getFloat("current_mon"));
                nchannel++;
                n++;
            }
            if (isoff) {
                id = HVState.OFF_S.getId();
            } else if (istransition) {
                id = HVState.TRANSITION_TS.getId();
                ncrate_active++;
            } else {
                ncrate_active++;
            }
            if (iserror) {
                id = HVState.ERROR_ES.getId();
            }
            ncrate++;
            //label_modules.setText(ncrate_active + " / " + ncrate);
            //label_channels.setText(nchannel_active + " / " + nchannel);
            state.copy(id);
            if (crate_tmp < states.size()) {
                states.get(crate_tmp).update(state);
            }
        }
    }

    boolean update(int id) {
        return state_ps.update(HVState.get(id));
    }

}
