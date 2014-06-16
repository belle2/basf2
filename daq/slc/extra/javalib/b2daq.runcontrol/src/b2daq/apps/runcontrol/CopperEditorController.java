/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.database.ConfigObject;
import b2daq.database.DBObject;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import b2daq.runcontrol.core.RCCommand;
import java.net.URL;
import java.util.ArrayList;
import java.util.ResourceBundle;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.TableCell;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;
import javafx.scene.control.cell.CheckBoxTableCell;
import javafx.scene.control.cell.TextFieldTableCell;
import javafx.scene.layout.VBox;
import javafx.util.Callback;
import javafx.util.StringConverter;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class CopperEditorController implements Initializable, NSMObserver {

    @FXML
    private VBox vbox;
    @FXML
    private TableView table_config;
    @FXML
    private TableColumn col_config_copperid;
    @FXML
    private TableColumn col_config_hslba;
    @FXML
    private TableColumn col_config_hslbb;
    @FXML
    private TableColumn col_config_hslbc;
    @FXML
    private TableColumn col_config_hslbd;
    @FXML
    private TableColumn col_config_script;
    @FXML
    private TableColumn col_config_dma;
    @FXML
    private TableView table_setup;
    @FXML
    private TableColumn col_setup_copperid;
    @FXML
    private TableColumn col_setup_hostname;
    @FXML
    private TableColumn col_setup_crate;
    @FXML
    private TableColumn col_setup_slot;
    @FXML
    private TableColumn col_setup_serialid;
    @FXML
    private TableColumn col_setup_prpmc;
    @FXML
    private TableColumn col_setup_ttrx;
    @FXML
    private TableColumn col_setup_ttrxfirm;
    @FXML
    private TableColumn col_setup_hslba;
    @FXML
    private TableColumn col_setup_hslbb;
    @FXML
    private TableColumn col_setup_hslbc;
    @FXML
    private TableColumn col_setup_hslbd;
    @FXML
    private TableColumn col_setup_hslbfirm;
    @FXML
    private TableView table_fee;
    @FXML
    private TableColumn col_fee_copperid;
    @FXML
    private TableColumn col_fee_slot;
    @FXML
    private TableColumn col_fee_board;
    @FXML
    private TableColumn col_fee_firmware;
    @FXML
    private TableColumn col_fee_used;
    @FXML
    private TableView table_param;
    @FXML
    private TableColumn col_param_copperid;
    @FXML
    private TableColumn col_param_slot;
    @FXML
    private TableColumn col_param_board;
    @FXML
    private TableColumn col_param_regisger;
    @FXML
    private TableColumn col_param_address;
    @FXML
    private TableColumn col_param_index;
    @FXML
    private TableColumn col_param_value;

    private final ArrayList<String> nodelist = new ArrayList<>();

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        col_config_hslba.setCellFactory(CheckBoxTableCell.forTableColumn(col_config_hslba));
        col_config_hslbb.setCellFactory(CheckBoxTableCell.forTableColumn(col_config_hslbb));
        col_config_hslbc.setCellFactory(CheckBoxTableCell.forTableColumn(col_config_hslbc));
        col_config_hslbd.setCellFactory(CheckBoxTableCell.forTableColumn(col_config_hslbd));
        col_config_hslba.setEditable(true);
        col_config_hslbb.setEditable(true);
        col_config_hslbc.setEditable(true);
        col_config_hslbd.setEditable(true);
        col_fee_used.setCellFactory(CheckBoxTableCell.forTableColumn(col_fee_used));
        col_fee_used.setEditable(true);
        Callback<TableColumn<CopperFeeParameterProperty, Integer>, TableCell<CopperFeeParameterProperty, Integer>> slotCellFactory = (TableColumn<CopperFeeParameterProperty, Integer> arg0) -> new TextFieldTableCell<>(new StringConverter<Integer>() {

            @Override
            public String toString(Integer value) {
                switch (value) {
                    case 0: return "a";
                    case 1: return "b";
                    case 2: return "c";
                    case 3: return "d";
                }
                return "";
            }

            @Override
            public Integer fromString(String text) {
                if (text.matches("a")) {
                    return 0;
                } else if (text.matches("b")) {
                    return 1;
                } else if (text.matches("c")) {
                    return 2;
                } else if (text.matches("d")) {
                    return 3;
                }
                return -1;
            }
        });
        Callback<TableColumn<CopperFeeParameterProperty, Integer>, TableCell<CopperFeeParameterProperty, Integer>> addressCellFactory = (TableColumn<CopperFeeParameterProperty, Integer> arg0) -> new TextFieldTableCell<>(new StringConverter<Integer>() {

            @Override
            public String toString(Integer value) {
                return String.format("0x%03x",value);
            }

            @Override
            public Integer fromString(String text) {
                try {
                    return Integer.parseInt(text);
                } catch (NumberFormatException e) {
                    return -1;
                }
            }
        });
        col_fee_slot.setCellFactory(slotCellFactory);
        col_param_slot.setCellFactory(slotCellFactory);
        col_param_address.setCellFactory(addressCellFactory);
    }

    @Override
    public void handleOnConnected() {
        vbox.setDisable(false);
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
        if (msg == null) {
            return;
        }
        try {
            RCCommand command = new RCCommand();
            command.copy(msg.getReqName());
            if (command.equals(NSMCommand.DBSET)) {
                ConfigObject obj = NSMListenerService.getDB(msg.getNodeName());
                if (obj != null && obj.getTable().matches("copper")) {
                    CopperProperty pro = new CopperProperty(obj.getNode(), obj.getName(), obj.getId());
                    pro.setCopperID(obj.getText("copperid"));
                    pro.setHslbA(obj.getBool("hslb_a"));
                    pro.setHslbB(obj.getBool("hslb_b"));
                    pro.setHslbC(obj.getBool("hslb_c"));
                    pro.setHslbA(obj.getBool("hslb_a"));
                    pro.setBasf2script(obj.getText("basf2script"));
                    pro.setDmaLimit(obj.getInt("dmalimit"));
                    pro.setEb0Host(obj.getText("eb0_host"));
                    pro.setEb0Port(obj.getInt("eb0_port"));
                    table_config.getItems().add(pro);
                    CopperSetupProperty spro = pro.getSetup();
                    ConfigObject sobj = (ConfigObject) obj.getObject("setup");
                    spro.setCopperID(sobj.getText("copperid"));
                    spro.setCrate(sobj.getInt("crate"));
                    spro.setSlot(sobj.getInt("slot"));
                    spro.setHostname(sobj.getText("hostname"));
                    spro.setSerialID(sobj.getText("serialid"));
                    spro.setPrpmc(sobj.getText("prpmc"));
                    spro.setTtrx(sobj.getText("ttrx"));
                    spro.setTtrxFirmware(sobj.getText("ttrx_firmware"));
                    spro.setHslbA(sobj.getText("hslb_a"));
                    spro.setHslbB(sobj.getText("hslb_b"));
                    spro.setHslbC(sobj.getText("hslb_c"));
                    spro.setHslbD(sobj.getText("hslb_d"));
                    spro.setHslbFirmware(sobj.getText("hslb_firmware"));
                    table_setup.getItems().add(spro);
                    int nobj = obj.getNObjects("fee");
                    for (int io = 0; io < nobj; io++) {
                        ConfigObject fobj = (ConfigObject) obj.getObject("fee", io);
                        CopperFeeProperty fpro = new CopperFeeProperty(fobj.getNode(), fobj.getName(), fobj.getId());
                        fpro.setCopperID(sobj.getText("copperid"));
                        fpro.setSlot(fobj.getInt("slot"));
                        fpro.setBoard(fobj.getText("board"));
                        fpro.setFirmware(fobj.getText("firmware"));
                        switch (fobj.getInt("slot")) {
                            case 0:
                                fpro.setUsed(obj.getBool("hslb_a"));
                                break;
                            case 1:
                                fpro.setUsed(obj.getBool("hslb_b"));
                                break;
                            case 2:
                                fpro.setUsed(obj.getBool("hslb_c"));
                                break;
                            case 3:
                                fpro.setUsed(obj.getBool("hslb_d"));
                                break;
                            default:
                                fpro.setUsed(false);
                                break;
                        }
                        table_fee.getItems().add(fpro);
                        int i = 0;
                        ConfigObject robj = (ConfigObject) fobj.getObject("register");
                        for (ConfigObject pobj : fobj.getObjects("parameter")) {
                            CopperFeeParameterProperty ppro = new CopperFeeParameterProperty(fobj.getNode(), fobj.getName(), fobj.getId());
                            if (pobj.getInt("arrayindex") == 0) {
                                robj = (ConfigObject) fobj.getObject("register", i);
                                i++;
                            }
                            ppro.setCopperID(sobj.getText("copperid"));
                            ppro.setSlot(fobj.getInt("slot"));
                            ppro.setBoard(fobj.getText("board"));
                            ppro.setParamName(robj.getText("paramname"));
                            ppro.setAddress(robj.getInt("address"));
                            ppro.setArrayIndex(pobj.getInt("arrayindex"));
                            ppro.setValue(pobj.getInt("value"));
                            table_param.getItems().add(ppro);
                        }
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();;
        }
    }

    @Override
    public void handleOnDisConnected() {
        vbox.setDisable(true);
    }

    @Override
    public void log(LogMessage log
    ) {
    }

}
