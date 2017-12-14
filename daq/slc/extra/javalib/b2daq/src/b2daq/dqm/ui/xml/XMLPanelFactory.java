package b2daq.dqm.ui.xml;

import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.dqm.ui.CanvasPanel;
import java.util.ArrayList;
import java.util.HashMap;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.Pane;
import javafx.scene.layout.Region;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class XMLPanelFactory {

    static public String parseUnicode(String str) {
        String str_out = str.replace("&lt;", "<");
        str_out = str_out.replace("&gt;", ">");
        str_out = str_out.replace("&quot;", "\"");
        str_out = str_out.replace("&apos;", "'");
        str_out = str_out.replace("&amp;", "&");
        return str_out;
    }

    private HistoPackage _pack = null;
    private final HashMap<String, HistogramCanvas> _canvas_map = new HashMap<>();
    private final ArrayList<XMLCanvasFactory> _canvas_fact_v = new ArrayList<>();

    public void setPackage(HistoPackage pack) {
        _pack = pack;
    }

    public Region createPanel(Element root) {
        Region panel = null;
        try {
            panel = getPanel((Element) root.getChildNodes().item(1));
            if (root.hasAttribute("title")) {
                //panel.setName(root.getAttribute("title"));
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return panel;
    }

    private Region getPanel(Element el) {
        Region panel = null;
        switch (el.getNodeName()) {
            case "tabbedPanel":
                panel = getTabbedPanel(el);
                break;
            case "canvas":
                panel = getCanvas(el);
                break;
            case "table":
                panel = getTable(el);
                break;
            case "scrollPanel":
                panel = getScrollPanel(el);
                break;
        }
        return panel;
    }

    private Pane getTable(Element el) {
        int col = Integer.parseInt(el.getAttribute("col"));
        int row = Integer.parseInt(el.getAttribute("row"));
        int nel = 0;
        NodeList node_list = el.getChildNodes();
        GridPane table = new GridPane();
        if (el.hasAttribute("width-ratio")) {
        }
        if (el.hasAttribute("height-ratio")) {
        }
        if (el.hasAttribute("cel-width")) {
        }
        if (el.hasAttribute("cel-height")) {
        }
        for (int nc = 0; nc < col; nc++) {
            for (int nr = 0; nr < row; nr++) {
                for (; nel < node_list.getLength(); nel++) {
                    Node node = node_list.item(nel);
                    if (node.getNodeType() == Node.ELEMENT_NODE) {
                        Region widget = getPanel((Element) node);
                        table.getChildren().add(widget);
                        GridPane.setConstraints(widget, col, row);
                        nel++;
                        break;
                    }
                }
            }
        }
        return table;
    }

    private Pane getCanvas(Element el) {
        XMLCanvasFactory c_fact = new XMLCanvasFactory(_pack);
        _canvas_fact_v.add(c_fact);
        CanvasPanel canvas = (CanvasPanel) c_fact.getCanvas(el);
        _canvas_map.put(canvas.getCanvas().getName(), canvas.getCanvas());
        return canvas;
    }

    private TabPane getTabbedPanel(Element el) {
        TabPane tabpanel = new TabPane();
        NodeList node_list = el.getChildNodes();
        for (int n = 0; n < node_list.getLength(); n++) {
            Node node = node_list.item(n);
            if (node.getNodeType() == Node.ELEMENT_NODE) {
                Element ch = (Element) node;
                if (ch.getNodeName().equals("tab")) {
                    if (ch.getChildNodes().getLength() > 2) {
                        Tab tab = new Tab(el.getAttribute("title"));
                        tab.setContent(getPanel((Element) ch.getChildNodes().item(1)));
                        tabpanel.getTabs().add(tab);
                    } else {
                        Tab tab = new Tab(el.getAttribute("title"));
                        tabpanel.getTabs().add(tab);
                    }
                }
            }
        }
        return tabpanel;
    }

    private ScrollPane getScrollPanel(Element el) {
        ScrollPane panel = new ScrollPane();
        panel.setFitToWidth(true);
        for (int n = 0; n < el.getChildNodes().getLength(); n++) {
            try {
                Element ch = (Element) el.getChildNodes().item(n);
                Region cpanel = getPanel(ch);
                panel.setContent(cpanel);
                break;
            } catch (Exception e) {
            }
        }
        return panel;
    }


    public HistoPackage getPackage() {
        return _pack;
    }

}
