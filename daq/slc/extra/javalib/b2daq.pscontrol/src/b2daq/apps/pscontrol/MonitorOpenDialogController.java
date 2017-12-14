/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.dqm.core.TimedGraph1D;
import b2daq.dqm.ui.CanvasPanel;
import b2daq.hvcontrol.core.HVState;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMVSetHandler;
import b2daq.nsm.NSMVar;
import b2daq.nsm.ui.NSMRequestHandlerUI;
import java.net.URL;
import java.util.HashMap;
import java.util.ResourceBundle;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.ComboBox;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.paint.Color;
import javafx.stage.Modality;
import javafx.stage.Stage;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class MonitorOpenDialogController implements Initializable {

    @FXML
    private Button openButton;
    @FXML
    private Button cancelButton;
    @FXML
    private ComboBox combo;

    private boolean isOpen = false;
    private PSSettingMainPanelController m_pmain;
    private CanvasPanel canvas;
    private final HashMap<String, TimedGraph1D> gr = new HashMap<>();
    private final Color[] color = {
        Color.BLACK, Color.RED, Color.BLUE, Color.GREEN, Color.MAGENTA, Color.LIGHTGRAY,
        Color.ORANGE, Color.LIGHTSTEELBLUE, Color.DARKSLATEBLUE, Color.ORANGERED,
        Color.BLUEVIOLET, Color.CHOCOLATE
    };

    public boolean isOpened() {
        return isOpen;
    }

    public void setSettingMainPanel(PSSettingMainPanelController pmain) {
        m_pmain = pmain;
        for (PSCrateSettingPanelController c : m_pmain.getCrates()) {
            combo.getItems().add(c.getName());
        }
    }

    @FXML
    protected void handleOpenButton() {
        openButton.getScene().getWindow().hide();
        String name = (String) combo.getSelectionModel().getSelectedItem();
        PSCrateSettingPanelController pcrate = m_pmain.getCrate(name);
        if (pcrate != null) {
            canvas = new CanvasPanel();
            for (PSChannelSettingPanelController ch : pcrate.getChannels()) {
                String vname = ch.getVName("vmon");
                TimedGraph1D g = new TimedGraph1D(vname, ";Time;Voltage [V]", 1000, 0, 3600);
                g.setLineColor(color[gr.keySet().size()]);
                gr.put(vname, g);
                canvas.getCanvas().addHisto(g);
                canvas.getCanvas().setFillColor(Color.LIGHTGRAY);
                canvas.getCanvas().getPad().setY(0.07);
                canvas.getCanvas().getAxisX().setNdivisions(10);
                NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, m_pmain.getNode().getName(), vname, NSMVar.FLOAT) {
                    @Override
                    public boolean handleVSet(NSMVar var) {
                        gr.get(var.getName()).addPoint(var.getFloat());
                        canvas.update();
                        return true;
                    }
                });
            }
        }
        canvas.setPrefSize(520, 440);
        Scene scene = new Scene(canvas);
        scene.getStylesheets().add(LogMessage.getCSSPath());
        scene.getStylesheets().add(HVState.getCSSPath());
        Stage dialog = new Stage();//StageStyle.UTILITY);
        dialog.setScene(scene);
        dialog.initModality(Modality.WINDOW_MODAL);
        dialog.setTitle(name);
        dialog.centerOnScreen();
        dialog.show();
    }

    @FXML
    protected void handleCancelButton() {
        handleCloseAction(false);
    }

    private void handleCloseAction(boolean isOpen) {
        this.isOpen = isOpen;
        openButton.getScene().getWindow().hide();
    }

    @Override
    public void initialize(URL url, ResourceBundle rb) {
        openButton.setOnKeyPressed((KeyEvent ke) -> {
            if (ke.getCode().equals(KeyCode.ENTER)) {
                handleCloseAction(true);
            }
        });
        cancelButton.setOnKeyPressed((KeyEvent ke) -> {
            if (ke.getCode().equals(KeyCode.ENTER)) {
                handleCloseAction(false);
            }
        });
    }

}
