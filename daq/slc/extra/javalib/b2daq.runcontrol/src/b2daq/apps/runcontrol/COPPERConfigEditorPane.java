/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.database.ConfigObject;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import java.io.IOException;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.control.CheckBox;
import javafx.scene.control.TextField;
import javafx.scene.layout.VBox;

/**
 *
 * @author tkonno
 */
class COPPERConfigEditorPane extends VBox implements ConfigManager {

    @FXML
    private TextField fieldHostName;
    @FXML
    private TextField fieldCopperid;
    @FXML
    private TextField fieldEB0Host;
    @FXML
    private TextField fieldEB0Port;
    @FXML
    private CheckBox checkHSLB_a;
    @FXML
    private CheckBox checkHSLB_b;
    @FXML
    private CheckBox checkHSLB_c;
    @FXML
    private CheckBox checkHSLB_d;
    @FXML
    private TextField fieldScript;
    @FXML
    private TextField fieldHSLBFirm;
    @FXML
    private TextField fieldTTRXFirm;
    @FXML
    private TextField fieldCrate;
    @FXML
    private TextField fieldSlot;
    @FXML
    private TextField fieldCopperI_hard;
    @FXML
    private TextField fieldSerialid;
    @FXML
    private TextField fieldPrPMC;
    @FXML
    private TextField fieldTTRX;
    @FXML
    private TextField fieldFINNESE_a;
    @FXML
    private TextField fieldFINNESE_b;
    @FXML
    private TextField fieldFINNESE_c;
    @FXML
    private TextField fieldFINNESE_d;

    public TextField fieldHostNamePropery() {
        return fieldHostName;
    }

    public TextField fieldCopperidPropery() {
        return fieldCopperid;
    }

    public TextField fieldEB0HostPropery() {
        return fieldEB0Host;
    }

    public TextField fieldEB0PortPropery() {
        return fieldEB0Port;
    }

    public CheckBox checkHSLB_aPropery() {
        return checkHSLB_a;
    }

    public CheckBox checkHSLB_bPropery() {
        return checkHSLB_b;
    }

    public CheckBox checkHSLB_cPropery() {
        return checkHSLB_c;
    }

    public CheckBox checkHSLB_dPropery() {
        return checkHSLB_d;
    }

    public TextField fieldScriptPropery() {
        return fieldScript;
    }

    public TextField fieldHSLBFirmPropery() {
        return fieldHSLBFirm;
    }

    public TextField fieldTTRXFirmPropery() {
        return fieldTTRXFirm;
    }

    public TextField fieldCratePropery() {
        return fieldCrate;
    }

    public TextField fieldSlotPropery() {
        return fieldSlot;
    }

    public TextField fieldCopperI_hardPropery() {
        return fieldCopperI_hard;
    }

    public TextField fieldSerialidPropery() {
        return fieldSerialid;
    }

    public TextField fieldPrPMCPropery() {
        return fieldPrPMC;
    }

    public TextField fieldTTRXPropery() {
        return fieldTTRX;
    }

    public TextField fieldFINNESE_aPropery() {
        return fieldFINNESE_a;
    }

    public TextField fieldFINNESE_bPropery() {
        return fieldFINNESE_b;
    }

    public TextField fieldFINNESE_cPropery() {
        return fieldFINNESE_c;
    }

    public TextField fieldFINNESE_dPropery() {
        return fieldFINNESE_d;
    }

    public COPPERConfigEditorPane() {
        FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("COPPERConfigEditorPane.fxml"));
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
        checkHSLB_a.disableProperty().bind(fieldFINNESE_a.textProperty().isEmpty());
        checkHSLB_b.disableProperty().bind(fieldFINNESE_b.textProperty().isEmpty());
        checkHSLB_c.disableProperty().bind(fieldFINNESE_c.textProperty().isEmpty());
        checkHSLB_d.disableProperty().bind(fieldFINNESE_d.textProperty().isEmpty());
    }

    @Override
    public ConfigObject update(String nodename, String newconf) {
        ConfigObject obj = NSMListenerService.getDB(nodename);
        obj.setName(newconf);
        obj.setText("copperid", fieldCopperidPropery().getText());
        obj.setBool("hslb_a", checkHSLB_aPropery().isSelected());
        obj.setBool("hslb_b", checkHSLB_bPropery().isSelected());
        obj.setBool("hslb_c", checkHSLB_cPropery().isSelected());
        obj.setBool("hslb_d", checkHSLB_dPropery().isSelected());
        obj.setText("basf2script", fieldScriptPropery().getText());
        obj.setText("eb0_host", fieldEB0HostPropery().getText());
        obj.setInt("eb0_port", Integer.parseInt(fieldEB0PortPropery().getText()));
        ConfigObject sobj = (ConfigObject) obj.getObject("setup");
        sobj.setName(newconf);
        sobj.setText("copperid", fieldCopperI_hardPropery().getText());
        sobj.setInt("crate", Integer.parseInt(fieldCratePropery().getText()));
        sobj.setInt("slot", Integer.parseInt(fieldSlotPropery().getText()));
        sobj.setText("hostname", fieldHostNamePropery().getText());
        sobj.setText("serialid", fieldSerialidPropery().getText());
        sobj.setText("prpmc", fieldPrPMCPropery().getText());
        sobj.setText("ttrx", fieldTTRXPropery().getText());
        sobj.setText("ttrx_firmware", fieldTTRXFirmPropery().getText());
        sobj.setText("hslb_a", fieldFINNESE_aPropery().getText());
        sobj.setText("hslb_b", fieldFINNESE_bPropery().getText());
        sobj.setText("hslb_c", fieldFINNESE_cPropery().getText());
        sobj.setText("hslb_d", fieldFINNESE_dPropery().getText());
        sobj.setText("hslb_firmware", fieldHSLBFirmPropery().getText());
        return obj;
    }

    @Override
    public void reload(NSMMessage msg) {
        ConfigObject obj = NSMListenerService.getDB(msg.getNodeName());
        fieldCopperidPropery().setText(obj.getText("copperid"));
        checkHSLB_aPropery().setSelected(obj.getBool("hslb_a"));
        checkHSLB_bPropery().setSelected(obj.getBool("hslb_b"));
        checkHSLB_cPropery().setSelected(obj.getBool("hslb_c"));
        checkHSLB_dPropery().setSelected(obj.getBool("hslb_d"));
        fieldScriptPropery().setText(obj.getText("basf2script"));
        fieldEB0HostPropery().setText(obj.getText("eb0_host"));
        fieldEB0PortPropery().setText("" + obj.getInt("eb0_port"));
        ConfigObject sobj = (ConfigObject) obj.getObject("setup");
        fieldCopperI_hardPropery().setText(sobj.getText("copperid"));
        fieldCratePropery().setText("" + sobj.getInt("crate"));
        fieldSlotPropery().setText("" + sobj.getInt("slot"));
        fieldHostNamePropery().setText(sobj.getText("hostname"));
        fieldSerialidPropery().setText(sobj.getText("serialid"));
        fieldPrPMCPropery().setText(sobj.getText("prpmc"));
        fieldTTRXPropery().setText(sobj.getText("ttrx"));
        fieldTTRXFirmPropery().setText(sobj.getText("ttrx_firmware"));
        fieldFINNESE_aPropery().setText(sobj.getText("hslb_a"));
        fieldFINNESE_bPropery().setText(sobj.getText("hslb_b"));
        fieldFINNESE_cPropery().setText(sobj.getText("hslb_c"));
        fieldFINNESE_dPropery().setText(sobj.getText("hslb_d"));
        fieldHSLBFirmPropery().setText(sobj.getText("hslb_firmware"));
    }

}
