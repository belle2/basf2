/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package b2daq.dqm.io;

import b2daq.dqm.core.HistoPackage;
import java.util.ArrayList;

/**
 *
 * @author tkonno
 */
public interface DQMObserver {

    public void reset();
    public void init(ArrayList<HistoPackage> _pack_v, ArrayList<PackageInfo> _info_v);
    public void update(int expno, int runno, int stateno);
    public void update();
    
}
