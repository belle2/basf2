/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.nsm.NSMNode;
import b2daq.nsm.NSMVSetHandler;
import b2daq.nsm.NSMVar;
import b2daq.nsm.ui.NSMRequestHandlerUI;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.JavaFXBuilderFactory;
import javafx.scene.Parent;
import javafx.scene.layout.VBox;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class PSCrateSettingPanelController {

    @FXML
    VBox vbox;
    private int m_crateid;

    private final ArrayList<PSChannelSettingPanelController> channel = new ArrayList();
    private NSMNode m_hvnode;
    private String m_name = "";

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public ArrayList<PSChannelSettingPanelController> getChannels() {
        return channel;
    }

    public void setCrateId(int crateid) {
        m_crateid = crateid;
    }

    public void setNode(NSMNode node) {
        m_hvnode = node;
        NSMRequestHandlerUI.get().add(new CrateNChannelHandler(m_hvnode.getName(), "crate[" + m_crateid + "].nchannels"));
    }

    public void allOn(boolean allon) {
        for (PSChannelSettingPanelController ch : channel) {
            ch.turnon(allon);
        }
    }

    private class CrateNChannelHandler extends NSMVSetHandler {

        public CrateNChannelHandler(String node, String name) {
            super(true, node, name, NSMVar.INT);
        }

        @Override
        public boolean handleVSet(NSMVar var) {
            try {
                int nchannels = var.getInt();
                for (int i = 0; i < nchannels; i++) {
                    URL location = getClass().getResource("PSChannelSettingPanel.fxml");
                    FXMLLoader loader = new FXMLLoader();
                    loader.setLocation(location);
                    loader.setBuilderFactory(new JavaFXBuilderFactory());
                    Parent root = (Parent) loader.load(location.openStream());
                    PSChannelSettingPanelController controller
                            = ((PSChannelSettingPanelController) loader.getController());
                    vbox.getChildren().add(root);
                    controller.setCrateId(m_crateid);
                    controller.setIndex(i);
                    controller.setNode(m_hvnode);
                    channel.add(controller);
                }
            } catch (IOException ex) {
                Logger.getLogger(PSCrateSettingPanelController.class.getName()).log(Level.SEVERE, null, ex);
            }
            return true;
        }

    }

}
