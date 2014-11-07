/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.database.ConfigObject;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.runcontrol.core.RCCommand;
import java.io.IOException;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.control.Button;
import javafx.scene.control.ComboBox;
import javafx.scene.control.TextField;
import javafx.scene.layout.VBox;

/**
 *
 * @author tkonno
 */
final class TTDConfigEditorPane extends VBox implements ConfigManager {

    @FXML
    private TextField fieldFTSWID;
    @FXML
    private ComboBox comboTriggerType;
    @FXML
    private TextField fieldDummyRate;
    @FXML
    private TextField fieldTriggerLimit;
    @FXML
    private VBox vboxConnections;
    @FXML
    private Button buttonTrigft;
    private String nodename;

    public TextField fieldFTSWIDPropery() {
        return fieldFTSWID;
    }

    public ComboBox comboTriggerTypePropery() {
        return comboTriggerType;
    }

    public TextField fieldDummyRatePropery() {
        return fieldDummyRate;
    }

    public TextField fieldTriggerLimitPropery() {
        return fieldTriggerLimit;
    }

    public VBox vboxConnectionsPropery() {
        return vboxConnections;
    }

    public Button buttonTriftPropery() {
        return buttonTrigft;
    }

    public TTDConfigEditorPane() {
        FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("TTDConfigEditorPane.fxml"));
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
        comboTriggerType.getItems().add(0, "none");
        comboTriggerType.getItems().add(1, "aux");
        comboTriggerType.getItems().add(2, "i");
        comboTriggerType.getItems().add(3, "tlu");
        comboTriggerType.getItems().add(4, "pulse");
        comboTriggerType.getItems().add(5, "revo");
        comboTriggerType.getItems().add(6, "random");
        comboTriggerType.getItems().add(7, "possion");
        comboTriggerType.getItems().add(8, "once");
        comboTriggerType.getItems().add(9, "stop");
    }

    public TTDConfigEditorPane(String node) {
        this();
        nodename = node;
        buttonTrigft.setOnAction((event) -> {
            int[] pars = new int[3];
            pars[0] = comboTriggerTypePropery().getSelectionModel().getSelectedIndex();
            pars[1] = Integer.parseInt(fieldDummyRate.getText());
            pars[2] = Integer.parseInt(fieldTriggerLimit.getText());
            NSMListenerService.request(new NSMMessage(nodename, RCCommand.TRGIFT, pars));
        });

    }
    /*
     public void addLink(ROPCCopperConfigPanel copper) {
     vboxCopperHosts.getChildren().add(copper);
     copper.fieldPortPropery().disableProperty().bind(checkRecvStream0.selectedProperty().not());
     }
     */

    @Override
    public ConfigObject update(String nodename, String newconf) {
        ConfigObject obj = NSMListenerService.getDB(nodename);
        obj.setName(newconf);
        obj.setShort("trigger_type", (short) comboTriggerTypePropery().getSelectionModel().getSelectedIndex());
        obj.setShort("dummy_rate", (short) Integer.parseInt(fieldDummyRate.getText()));
        obj.setShort("trigger_limit", (short) Integer.parseInt(fieldTriggerLimit.getText()));
        return obj;
    }

    @Override
    public void reload(NSMMessage msg) {
        ConfigObject obj = NSMListenerService.getDB(msg.getNodeName());
        comboTriggerTypePropery().getSelectionModel().select(obj.getShort("trigger_type"));
        fieldDummyRate.setText("" + obj.getShort("dummy_rate"));
        fieldTriggerLimit.setText("" + obj.getShort("trigger_limit"));
    }

}
