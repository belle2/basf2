/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm.ui;

import java.net.URL;
import java.util.ArrayList;
import java.util.ResourceBundle;
import javafx.collections.ObservableList;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.TableCell;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableColumn.CellEditEvent;
import javafx.scene.control.TableView;
import javafx.scene.control.TextField;
import javafx.scene.control.cell.TextFieldTableCell;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.util.Callback;
import javafx.util.StringConverter;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class NSMConfigDialogController implements Initializable {

    @FXML
    private TableView table_nsm;
    @FXML
    private TableColumn col_nsm_dataname;
    @FXML
    private TableColumn col_nsm_format;
    @FXML
    private TableColumn col_nsm_revision;
    @FXML
    private TextField field_nsm_target;
    @FXML
    private Button startButton;
    @FXML
    private Button cancelButton;

    private boolean isOK = false;

    @FXML
    private void handleOKButton() {
        handleCloseAction(true);
    }

    @FXML
    private void handleCancelButton() {
        handleCloseAction(false);
    }

    @FXML
    private void handleAddButton() {
        table_nsm.getItems().add(new NSMDataProperty());
    }

    private void handleCloseAction(boolean isOK) {
        this.isOK = isOK;
        startButton.getScene().getWindow().hide();
    }

    public void init(String nodename, ArrayList<NSMDataProperty> pros) {
        field_nsm_target.setText(nodename);
        for (NSMDataProperty pro: pros) {
            table_nsm.getItems().add(pro);
        }
    }
    
    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        table_nsm.getItems();
        startButton.setOnKeyPressed(new EventHandler<KeyEvent>() {
            @Override
            public void handle(KeyEvent ke) {
                if (ke.getCode().equals(KeyCode.ENTER)) {
                    handleCloseAction(true);
                }
            }
        });
        cancelButton.setOnKeyPressed(new EventHandler<KeyEvent>() {
            @Override
            public void handle(KeyEvent ke) {
                if (ke.getCode().equals(KeyCode.ENTER)) {
                    handleCloseAction(false);
                }
            }
        });
        table_nsm.setEditable(true);
        col_nsm_dataname.setCellFactory(TextFieldTableCell.forTableColumn());
        col_nsm_format.setCellFactory(TextFieldTableCell.forTableColumn());
        col_nsm_revision.setCellFactory(new Callback<TableColumn<NSMDataProperty, Integer>, TableCell<NSMDataProperty, Integer>>() {
            @Override
            public TableCell<NSMDataProperty, Integer> call(TableColumn<NSMDataProperty, Integer> arg0) {
                return new TextFieldTableCell<>(new StringConverter<Integer>() {

                    @Override
                    public String toString(Integer object) {
                        return "" + object;
                    }

                    @Override
                    public Integer fromString(String string) {
                        try {
                            return Integer.parseInt(string);
                        } catch (Exception e) {
                            return 0;
                        }
                    }
                });
            }
        });
        col_nsm_dataname.setOnEditCommit(new EventHandler<CellEditEvent<NSMDataProperty, String>>() {
            @Override
            public void handle(CellEditEvent<NSMDataProperty, String> t) {
                ((NSMDataProperty) t.getTableView().getItems().get(t.getTablePosition().getRow())).setDataname(t.getNewValue());
            }
        });
        col_nsm_format.setOnEditCommit(new EventHandler<CellEditEvent<NSMDataProperty, String>>() {
            @Override
            public void handle(CellEditEvent<NSMDataProperty, String> t) {
                ((NSMDataProperty) t.getTableView().getItems().get(t.getTablePosition().getRow())).setFormat(t.getNewValue());
            }
        });
        col_nsm_revision.setOnEditCommit(new EventHandler<CellEditEvent<NSMDataProperty, Integer>>() {
            @Override
            public void handle(CellEditEvent<NSMDataProperty, Integer> t) {
                ((NSMDataProperty) t.getTableView().getItems().get(t.getTablePosition().getRow())).setRevision(t.getNewValue());
            }
        });
        startButton.setOnKeyPressed(new EventHandler<KeyEvent>() {
            @Override
            public void handle(KeyEvent ke) {
                if (ke.getCode().equals(KeyCode.ENTER)) {
                    handleCloseAction(true);
                }
            }
        });
        cancelButton.setOnKeyPressed(new EventHandler<KeyEvent>() {
            @Override
            public void handle(KeyEvent ke) {
                if (ke.getCode().equals(KeyCode.ENTER)) {
                    handleCloseAction(false);
                }
            }
        });
    }

    public void add(NSMDataProperty nsm) {
        table_nsm.getItems().add(nsm);
    }

    public boolean isStarted() {
        return isOK;
    }

    public String getNSMTarget() {
        return field_nsm_target.getText();
    }

    public ObservableList<NSMDataProperty> getNSMDataProperties() {
        return table_nsm.getItems();
    }
}
