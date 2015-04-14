/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.nsm.NSMCommunicator;
import b2daq.nsm.NSMVListSetHandler;
import b2daq.nsm.NSMVSetHandler;
import b2daq.nsm.NSMVar;
import b2daq.nsm.ui.NSMRequestHandlerUI;
import java.io.IOException;
import java.util.ArrayList;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.VBox;
import javafx.stage.Modality;
import javafx.stage.Stage;
import javafx.stage.StageStyle;

/**
 *
 * @author tkonno
 */
public class VSetDialog extends VBox {

    @FXML
    private Label label;
    @FXML
    private TextField input;
    @FXML
    private ComboBox combo;
    @FXML
    private Button okButton;
    @FXML
    private Button cancelButton;

    private boolean m_ok = false;
    private String m_nodename;

    private void handleCloseAction(boolean isOK) {
        m_ok = isOK;
        if (m_ok) {
            String value = input.getText();
            NSMVListSetHandler.VarEntry var = (NSMVListSetHandler.VarEntry) combo.getSelectionModel().getSelectedItem();
            switch (var.type) {
                case "text":
                    try {
                        System.out.println(m_nodename + "<<" + var.name + "=" + value);
                        NSMCommunicator.get().requestVSet(m_nodename, new NSMVar(var.name, value));
                    } catch (IOException ex) {
                        ex.printStackTrace();
                    }   break;
                case "int":
                    try {
                        int ival = Integer.parseInt(value);
                        System.out.println(m_nodename + "<<" + var.name + "=int(" + ival + ")");
                        NSMCommunicator.get().requestVSet(m_nodename, new NSMVar(var.name, ival));
                    } catch (Exception ex) {
                        ex.printStackTrace();
                }   break;
                case "float":
                    try {
                        float fval = Float.parseFloat(value);
                        System.out.println(m_nodename + "<<" + var.name + "=float(" + fval + ")");
                        NSMCommunicator.get().requestVSet(m_nodename, new NSMVar(var.name, fval));
                    } catch (Exception ex) {
                        ex.printStackTrace();
                }   break;
            }
        }
        label.getScene().getWindow().hide();
    }

    public void set(String message, ArrayList<NSMVListSetHandler.VarEntry> vars) {
        label.setText(message);
        ObservableList<NSMVListSetHandler.VarEntry> list = FXCollections.observableArrayList();
        for (NSMVListSetHandler.VarEntry en : vars) {
            if (en.useSet) {
                list.add(en);
            }
        }
        combo.getItems().addAll(list);
    }

    public String getText() {
        return (m_ok) ? input.getText() : null;
    }

    private VSetDialog(String nodename) {
        m_nodename = nodename;
        FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("VSetDialog.fxml"));
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
        setOnKeyPressed((KeyEvent ke) -> {
            if (ke.getCode().equals(KeyCode.ENTER)) {
                handleCloseAction(true);
            }
        });
        okButton.setOnAction((ActionEvent e) -> {
            handleCloseAction(true);
        });
        cancelButton.setOnAction((ActionEvent e) -> {
            handleCloseAction(false);
        });
        combo.setOnAction((event) -> {
            NSMVListSetHandler.VarEntry en = (NSMVListSetHandler.VarEntry) combo.getSelectionModel().getSelectedItem();
            NSMRequestHandlerUI.get().add(new NSMVSetHandler(true, m_nodename, en.name, NSMVar.NONE) {
                @Override
                public boolean handleVSet(NSMVar var) {
                    if (var.getType() == NSMVar.INT) {
                        input.setText(""+var.getInt());
                    } else if (var.getType() == NSMVar.FLOAT) {
                        input.setText(""+var.getFloat());
                    } else if (var.getType() == NSMVar.TEXT) {
                        input.setText(var.getText());
                    }
                    return true;
                }
            });
        });
    }

    public static void showDialog(Scene pscene, String title,
            String message, String nodename) {
        NSMRequestHandlerUI.get().add(new NSMVListSetHandler(true, nodename) {
            @Override
            public boolean handleVListSet(ArrayList<NSMVListSetHandler.VarEntry> vars) {
                VSetDialog dialog = new VSetDialog(nodename);
                dialog.set(message, vars);
                Scene scene = new Scene(dialog);
                Stage stage = new Stage(StageStyle.UTILITY);
                stage.setScene(scene);
                if (pscene != null) {
                    stage.initOwner(pscene.getWindow());
                }
                stage.initModality(Modality.WINDOW_MODAL);
                stage.setResizable(false);
                stage.setTitle(title);
                stage.showAndWait();
                dialog.getText();
                return true;
            }
        });
    }
}
