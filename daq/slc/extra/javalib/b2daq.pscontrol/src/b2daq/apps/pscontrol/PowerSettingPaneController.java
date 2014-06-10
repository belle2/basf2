package b2daq.apps.pscontrol;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
import b2daq.database.ConfigObject;
import b2daq.hvcontrol.core.HVCommand;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import java.net.URL;
import java.util.HashMap;
import java.util.ResourceBundle;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.ComboBox;
import javafx.scene.control.RadioButton;
import javafx.scene.control.TextField;
import javafx.scene.control.ToggleGroup;
import javafx.scene.layout.VBox;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class PowerSettingPaneController implements Initializable {

    @FXML
    private VBox vbox;
    @FXML
    private ComboBox combo_crate;
    @FXML
    private ComboBox combo_slot;
    @FXML
    private ComboBox combo_channel;
    @FXML
    private ComboBox combo_param;
    @FXML
    private RadioButton radio_on;
    @FXML
    private ToggleGroup group_onoff;
    @FXML
    private RadioButton radio_off;
    @FXML
    private TextField field_param;
    @FXML
    private Button saveButton;
    @FXML
    private Button resetButton;
    @FXML
    private Button applyButton;

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        radio_on.setOnAction(new EventHandler() {
            @Override
            public void handle(Event event) {
                applyButton.setDisable(false);
            }
        });
        radio_off.setOnAction(new EventHandler() {
            @Override
            public void handle(Event event) {
                applyButton.setDisable(false);
            }
        });
        field_param.textProperty().addListener(new ChangeListener() {
            @Override
            public void changed(ObservableValue observable, Object oldValue, Object newValue) {
                applyButton.setDisable(field_param.getText().length()==0);
            }
        });
    }

    @FXML
    public void handleSaveButton() {
    }

    @FXML
    public void handleResetButton() {
        System.out.println(combo_param.getSelectionModel().getSelectedIndex()+" "+combo_param.getSelectionModel().getSelectedItem());
    }

    @FXML
    public void handleApplyButton() {
        String nodename = NSMListenerService.getNSMConfig().getNsmTarget();
        int crate;
        int slot;
        int channel;
        try {
            crate = Integer.parseInt((String)combo_crate.getSelectionModel().getSelectedItem());
        } catch (NumberFormatException e) {
            crate = -1;
        }
        try {
            slot = Integer.parseInt((String)combo_slot.getSelectionModel().getSelectedItem());
        } catch (NumberFormatException e) {
            slot = -1;
        }
        try {
            channel = Integer.parseInt((String)combo_channel.getSelectionModel().getSelectedItem());
        } catch (NumberFormatException e) {
            channel = -1;
        }
        int [] opt = new int [] {crate, slot, channel};
        String text;
        if (combo_param.getSelectionModel().getSelectedIndex() == 0) {
            text = "turnon="+((radio_on.isSelected())?"true":"false");
        } else if (combo_param.getSelectionModel().getSelectedIndex() == 1) {
            text = "rampup_speed="+field_param.getText();
        } else if (combo_param.getSelectionModel().getSelectedIndex() == 2) {
            text = "rampdown_speed="+field_param.getText();
        } else if (combo_param.getSelectionModel().getSelectedIndex() == 3) {
            text = "voltage_limit="+field_param.getText();
        } else if (combo_param.getSelectionModel().getSelectedIndex() == 4) {
            text = "current_limit="+field_param.getText();
        } else if (combo_param.getSelectionModel().getSelectedIndex() == 5) {
            text = "voltage_demand="+field_param.getText();
        } else {
            return;
        }
        NSMListenerService.request(new NSMMessage(nodename, HVCommand.HVAPPLY, opt, text));
    }

    @FXML
    public void handleParamCombo() {
//                                    "On / Off"
//                                    "Ramp up speed" 
//                                    "Ramp downd speed" 
//                                    "Voltage limit" 
//                                    "Current limit" 
//                                    "Demand voltage" 
        System.out.println(combo_param.getSelectionModel().getSelectedItem());
        boolean isOnOff = combo_param.getSelectionModel().getSelectedIndex() == 0;
        radio_on.setVisible(isOnOff);
        if (isOnOff) radio_on.setSelected(true);
        radio_off.setVisible(isOnOff);
        field_param.setVisible(!isOnOff);
        field_param.setText("");
        applyButton.setDisable(true);
    }

    public VBox getVBox() {
        return vbox;
    }

    public ComboBox getComboCrate() {
        return combo_crate;
    }

    public ComboBox getComboSlot() {
        return combo_slot;
    }

    public ComboBox getComboChannel() {
        return combo_channel;
    }

    public ComboBox getComboParam() {
        return combo_param;
    }

    public RadioButton getRadioOn() {
        return radio_on;
    }

    public ToggleGroup getOnOffGroup() {
        return group_onoff;
    }

    public RadioButton getRadioOff() {
        return radio_off;
    }

    public TextField getFieldParam() {
        return field_param;
    }

    void update(ConfigObject cobj) {
        HashMap<Integer, String> crate_m = new HashMap<>();
        HashMap<Integer, String> slot_m = new HashMap<>();
        HashMap<Integer, String> channel_m = new HashMap<>();
        combo_crate.getItems().clear();
        combo_slot.getItems().clear();
        combo_channel.getItems().clear();
        combo_crate.getItems().add("All");
        combo_slot.getItems().add("All");
        combo_channel.getItems().add("All");
        for (ConfigObject obj : cobj.getObjects("channel")) {
            int crate = obj.getInt("crate");
            int slot = obj.getInt("slot");
            int channel = obj.getInt("channel");
            if (!crate_m.containsKey(crate)) {
                crate_m.put(crate, "");
                combo_crate.getItems().add(""+crate);
            }
            if (!slot_m.containsKey(slot)) {
                slot_m.put(slot, "");
                combo_slot.getItems().add(""+slot);
            }
            if (!channel_m.containsKey(channel)) {
                channel_m.put(channel, "");
                combo_channel.getItems().add(""+channel);
            }
        }
    }

}
