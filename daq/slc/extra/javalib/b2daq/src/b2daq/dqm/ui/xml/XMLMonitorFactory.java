package b2daq.dqm.ui.xml;

import java.util.ArrayList;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.layout.Region;

import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

public class XMLMonitorFactory {

    private final ArrayList<XMLPanelFactory> _fact_v = new ArrayList<>();
    private final TabPane _tab_panel = new TabPane();

    public XMLMonitorFactory() {
    }

    public TabPane parse(Document doc) throws ParserConfigurationException {
        Element root = null;
        for (int index = 0; index < doc.getChildNodes().getLength(); index++) {
            if (doc.getChildNodes().item(index).getNodeName().matches("online-monitor")) {
                root = (Element) doc.getChildNodes().item(index);
                break;
            }
        }
        Element config_root = (Element) root.getElementsByTagName("config").item(0);
        NodeList config_list = config_root.getElementsByTagName("package");
        for (int index = 0; index < config_list.getLength(); index++) {
            Element el = (Element) config_list.item(index);
            XMLPanelFactory fact = new XMLPanelFactory();
            fact.setPackage(XMLPackageFactory.readConfig(el));
            _fact_v.add(fact);
        }
        Element layout_root = (Element) root.getElementsByTagName("layout").item(0);
        NodeList layout_list = layout_root.getElementsByTagName("package");
        for (int index = 0; index < layout_list.getLength(); index++) {
            Element el = (Element) layout_list.item(index);
            Region widget = _fact_v.get(index).createPanel(el);
            Tab tab = new Tab(el.getAttribute("title"));
            tab.setContent(widget);
            _tab_panel.getTabs().add(tab);
        }
        return _tab_panel;
    }

    public void parseData(Document doc) {
        Element root = null;
        for (int index = 0; index < doc.getChildNodes().getLength(); index++) {
            if (doc.getChildNodes().item(index).getNodeName().matches("online-monitor")) {
                root = (Element) doc.getChildNodes().item(index);
                break;
            }
        }
        NodeList node_list = root.getElementsByTagName("package");
        for (int n = 0; n < _fact_v.size() && n < node_list.getLength(); n++) {
            XMLPackageFactory.readUpdate(_fact_v.get(n).getPackage(), (Element) node_list.item(n));

        }
    }

    public TabPane getPanel() {
        return _tab_panel;
    }

    public ArrayList<XMLPanelFactory> getFactories() {
        return _fact_v;
    }

}
