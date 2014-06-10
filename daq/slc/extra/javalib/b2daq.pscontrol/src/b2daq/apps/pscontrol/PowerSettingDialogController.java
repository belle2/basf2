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
import javafx.scene.control.Tab;
import javafx.scene.control.TableView;
import javafx.scene.control.TextField;
import javafx.scene.control.ToggleGroup;
import javafx.scene.layout.VBox;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class PowerSettingDialogController implements Initializable {

    @FXML
    private VBox vbox;
    @FXML
    private ComboBox combo_valueset;
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
    private Button cancelButton;
    @FXML
    private Button applyButton;
    private HVParamEditorPaneController editor;

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
                applyButton.setDisable(field_param.getText().length() == 0);
            }
        });
    }

    @FXML
    public void handleCancelButton() {
        vbox.getScene().getWindow().hide();
    }

    @FXML
    public void handleApplyButton() {
        String valuesetname = (String) combo_valueset.getSelectionModel().getSelectedItem();
        int crate;
        int slot;
        int channel;
        try {
            crate = Integer.parseInt((String) combo_crate.getSelectionModel().getSelectedItem());
        } catch (NumberFormatException e) {
            crate = -1;
        }
        try {
            slot = Integer.parseInt((String) combo_slot.getSelectionModel().getSelectedItem());
        } catch (NumberFormatException e) {
            slot = -1;
        }
        try {
            channel = Integer.parseInt((String) combo_channel.getSelectionModel().getSelectedItem());
        } catch (NumberFormatException e) {
            channel = -1;
        }
        int iparam = combo_param.getSelectionModel().getSelectedIndex();

        for (Tab tab : editor.getTabs()) {
            if (valuesetname.matches("ALL") || valuesetname.matches(tab.getText())) {
                TableView table = (TableView) (tab.getContent());
                for (Object object : table.getItems()) {
                    HVStatusProperty property = (HVStatusProperty) object;
                    if ((crate < 0 || crate == property.getCrate())
                            && (slot < 0 || slot == property.getSlot())
                            && (channel < 0 || channel == property.getChannel())) {
                        if (iparam == 0) {
                            property.setTurnon(radio_on.isSelected());
                        } else {
                            float value;
                            try {
                                value = Float.parseFloat(field_param.getText());
                            } catch (Exception e) {
                                return;
                            }
                            if (iparam == 1) {
                                property.setRampupSpeed(value);
                            } else if (iparam == 2) {
                                property.setRampdownSpeed(value);
                            } else if (iparam == 3) {
                                property.setVoltageLimit(value);
                            } else if (iparam == 4) {
                                property.setCurrentLimit(value);
                            } else if (iparam == 5) {
                                property.setVoltageDemand(value);
                            }
                        }
                    }
                }
            }
        }
        vbox.getScene().getWindow().hide();
    }

    @FXML
    public void handleParamCombo() {
        System.out.println(combo_param.getSelectionModel().getSelectedItem());
        boolean isOnOff = combo_param.getSelectionModel().getSelectedIndex() == 0;
        radio_on.setVisible(isOnOff);
        if (isOnOff) {
            radio_on.setSelected(true);
        }
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

    void init(HVParamEditorPaneController editor) {
        this.editor = editor;
        combo_valueset.getItems().clear();
        combo_valueset.getItems().add("All");
        for (Tab tab : editor.getTabs()) {
            combo_valueset.getItems().add(tab.getText());
        }
        ConfigObject cobj = editor.getConfig();
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
                combo_crate.getItems().add("" + crate);
            }
            if (!slot_m.containsKey(slot)) {
                slot_m.put(slot, "");
                combo_slot.getItems().add("" + slot);
            }
            if (!channel_m.containsKey(channel)) {
                channel_m.put(channel, "");
                combo_channel.getItems().add("" + channel);
            }
        }
    }

}
