{

#include <map>

  TSQLServer *srv = TSQLServer::Connect("pgsql://localhost/conditions","adm_cond","belle2");


// choose a number for the jun2013leps experiment... 
  int experiment = 900000001;
  int n_dirs = 3;
  const char* dirname[3] = {"/srv/itop_data/jun2013leps/20130609/data/",
			    "/srv/itop_data/jun2013leps/20130611/data/",
			    "/srv/itop_data/jun2013leps/20130613/data/"};
  const char* ext = ".dat";
  
  
  if(srv->IsConnected()){
    

    
    srv->Query(Form("insert into experiment(name, dtm_start, description, dtm_end, dtm_ins, dtm_mod) values('topcrt-e000002','2013-06-09 00:00','itop LEPS June2013 test beam experiment 2 ()','2013-06-10 23:59', now(), now())")); 
    srv->Query(Form("insert into experiment(name, dtm_start, description, dtm_end, dtm_ins, dtm_mod) values('topcrt-e000003','2013-06-11 00:00','itop LEPS June2013 test beam experiment 3 ()','2013-06-12 23:59', now(), now())")); 
    srv->Query(Form("insert into experiment(name, dtm_start, description, dtm_end, dtm_ins, dtm_mod) values('topcrt-e000004','2013-06-13 00:00','itop LEPS June2013 test beam experiment 4 ()','2013-06-14 23:59', now(), now())")); 

    srv->Query(Form("insert into run_type(name, description, dtm_ins, dtm_mod) values('pulser','Electronic pulser run',now(), now())"));
    srv->Query(Form("insert into run_type(name, description, dtm_ins, dtm_mod) values('pedfront','Another pedestal run',now(), now())"));
    srv->Query(Form("insert into run_type(name, description, dtm_ins, dtm_mod) values('beam','Particle beam run',now(), now())"));
    srv->Query(Form("insert into run_type(name, description, dtm_ins, dtm_mod) values('swt','SWT run',now(), now())"));
    
    srv->Query("insert into basf2_package(name,description,dtm_ins,dtm_mod,modified_by) values('top','imaging time of propagation system.',now(),now(),'Root init script')");
    

    srv->Query("insert into basf2_module(basf2_package_id, name, description, dtm_ins, dtm_mod, modified_by) values((select basf2_package_id from basf2_package where name='top'),'Pedestal','Calibration of pedestal values for itop waveform data.', now(), now(), 'Root init script')");


    
    // Add runs... this is annoying.
    for(Int_t c=0;c<n_dirs;c++){
      TSystemDirectory dir(dirname[c],dirname[c]);
      cout<<"directory: "<<dirname[c]<<endl;
      TList *files = dir.GetListOfFiles();
      if(files){
	TSystemFile *file;
	TString fname;
	TIter next(files);
	
	
	map<int , TString > queries;
	
	while ((file = (TSystemFile*)next())){
	  fname = file->GetName();
	  if(!file->IsDirectory() && fname.EndsWith(ext)){
	    
	    TString exp_prefix(fname(0,fname->First('-')));
	    fname = fname(fname->First('-')+1,fname->First('.'));
	    
	    TString run_type(fname(0,fname->First('-')));
	    fname = fname(fname->First('-')+1,fname->First('.'));
	    
	    TString exp_name(fname(0,fname->First('r')));
	    fname = fname(fname->First('r'),fname->First('.'));
	    TString exp_sub(exp_name(1,exp_name.Length()));
	    int exp_number = exp_sub->Atoi();
	    
	    
	    TString run_name(fname(0,fname->First('-')));
	    fname = fname(fname->First('-')+1,fname->First('.'));
	    TString run_sub(run_name(1,run_name.Length()));
	    int run_number = run_sub->Atoi();
	    
	    
	    TString run_query(Form("insert into run(experiment_id, name, run_type_id, dtm_ins, dtm_mod, run_start, run_end) values((select experiment_id from experiment where name='%s-%s'),'%s',(select run_type_id from run_type where name='%s'), now(), now(),",
				   exp_prefix.Data(), exp_name.Data(), run_name.Data(), run_type.Data()));
	    if(exp_number>1&&exp_number<5) queries[run_number] = run_query;
	  }
	}
	
	
	TDatime startdate("2013-06-09 00:00:00");
	startdate.Set(startdate.GetDate()+2*c, startdate.GetTime());
	UInt_t date = startdate.GetDate(), time = startdate.GetTime();
	TTimeStamp ts(date,time,0,true,0);    
	UInt_t run_duration = 15*60; // Need to fake up times for runs... say they were 15 min runs..
	UInt_t run_interval = 1*60; // Need to fake up times for runs... say they took 1 min to start..
	ts.SetSec(ts.GetSec()+run_duration);
	TDatime enddate((Int_t) ts.GetDate(), (Int_t) ts.GetTime());
	
	for(Int_t d=0;d<1000;d++){ /// sigh.. need to sort the runs.  Better if this was done during data taking.
	  TString query = queries[d];
	  if(query.Length()>0) {
	    
	    query += Form("'%s', ", startdate.AsSQLString());
	    query += Form("'%s')", enddate.AsSQLString());
	    //cout<<"queries["<<d<<"]: "<<query<<endl;
	    srv->Query(query.Data());

	    ts.SetSec(ts.GetSec()+run_interval);
	    startdate.Set((Int_t)ts.GetDate(), (Int_t)ts.GetTime()+1);
	    ts.SetSec(ts.GetSec()+run_duration);
	    enddate.Set((Int_t)ts.GetDate(),(Int_t)ts.GetTime());
	    
	    
	  }
	}
      }
    }

    // Now define the payloads.

    // first payload status
    srv->Query(Form("insert into payload_status(name, description, dtm_ins, dtm_mod) values('valid','Payload is valid.',now(), now())"));
    srv->Query(Form("insert into payload_status(name, description, dtm_ins, dtm_mod) values('invalid','Payload is invalid.',now(), now())"));

    // now payloads .. add pedestals
    srv->Query(Form("insert into payload(basf2_module_id, revision, is_default, payload_url, checksum, payload_status_id, deleted, dtm_ins, modified_by) values((select basf2_module_id from basf2_module where name='pedestal' and basf2_package_id=(select basf2_package_id from basf2_package where name='top')), 1, false, 'http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000002_r000508_pedestal_1.0.0.1.root','ca75473bd05c49bc6af0f0357e6aacea', (select payload_status_id from payload_status where name='valid'),false,now(),'Root init script')"));
    srv->Query(Form("insert into payload(basf2_module_id, revision, is_default, payload_url, checksum, payload_status_id, deleted, dtm_ins, modified_by) values((select basf2_module_id from basf2_module where name='pedestal' and basf2_package_id=(select basf2_package_id from basf2_package where name='top')), 2, false, 'http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000002_r000509_pedestal_1.0.0.1.root','5aa6119109cd847b55f77f83c81f1791', (select payload_status_id from payload_status where name='valid'),false,now(),'Root init script')"));
    srv->Query(Form("insert into payload(basf2_module_id, revision, is_default, payload_url, checksum, payload_status_id, deleted, dtm_ins, modified_by) values((select basf2_module_id from basf2_module where name='pedestal' and basf2_package_id=(select basf2_package_id from basf2_package where name='top')), 3, false, 'http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000002_r000540_pedestal_1.0.0.1.root','4b802c2be77a076d0bf43c973c4385bb', (select payload_status_id from payload_status where name='valid'),false,now(),'Root init script')"));
    srv->Query(Form("insert into payload(basf2_module_id, revision, is_default, payload_url, checksum, payload_status_id, deleted, dtm_ins, modified_by) values((select basf2_module_id from basf2_module where name='pedestal' and basf2_package_id=(select basf2_package_id from basf2_package where name='top')), 4, false, 'http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000002_r000571_pedestal_1.0.0.1.root','7730755b2d8e63cc7a53d5d84cdb2a3a', (select payload_status_id from payload_status where name='valid'),false,now(),'Root init script')"));
    srv->Query(Form("insert into payload(basf2_module_id, revision, is_default, payload_url, checksum, payload_status_id, deleted, dtm_ins, modified_by) values((select basf2_module_id from basf2_module where name='pedestal' and basf2_package_id=(select basf2_package_id from basf2_package where name='top')), 5, false, 'http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000002_r000615_pedestal_1.0.0.1.root','f286dc73cbc275140222eef0cbc7dd53', (select payload_status_id from payload_status where name='valid'),false,now(),'Root init script')"));
    srv->Query(Form("insert into payload(basf2_module_id, revision, is_default, payload_url, checksum, payload_status_id, deleted, dtm_ins, modified_by) values((select basf2_module_id from basf2_module where name='pedestal' and basf2_package_id=(select basf2_package_id from basf2_package where name='top')), 6, false, 'http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000003_r000701_pedestal_1.0.0.1.root','e07bee5803aee94898b414f1bda7e042', (select payload_status_id from payload_status where name='valid'),false,now(),'Root init script')"));
    srv->Query(Form("insert into payload(basf2_module_id, revision, is_default, payload_url, checksum, payload_status_id, deleted, dtm_ins, modified_by) values((select basf2_module_id from basf2_module where name='pedestal' and basf2_package_id=(select basf2_package_id from basf2_package where name='top')), 7, false, 'http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000003_r000763_pedestal_1.0.0.1.root','43ba51b2cdce9ebdf2056bfd0bebfd3a', (select payload_status_id from payload_status where name='valid'),false,now(),'Root init script')"));
    srv->Query(Form("insert into payload(basf2_module_id, revision, is_default, payload_url, checksum, payload_status_id, deleted, dtm_ins, modified_by) values((select basf2_module_id from basf2_module where name='pedestal' and basf2_package_id=(select basf2_package_id from basf2_package where name='top')), 8, false, 'http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000003_r000818_pedestal_1.0.0.1.root','6124e0527d6a273eed9260177a758af4', (select payload_status_id from payload_status where name='valid'),false,now(),'Root init script')"));
    srv->Query(Form("insert into payload(basf2_module_id, revision, is_default, payload_url, checksum, payload_status_id, deleted, dtm_ins, modified_by) values((select basf2_module_id from basf2_module where name='pedestal' and basf2_package_id=(select basf2_package_id from basf2_package where name='top')), 9, false, 'http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000004_r000938_pedestal_1.0.0.1.root','11248d7cf14a2294bed9bdce30cb65b8', (select payload_status_id from payload_status where name='valid'),false,now(),'Root init script')"));


    // now IOVs.
    srv->Query(Form("insert into payload_iov(payload_id, initial_run_id, final_run_id, dtm_ins, modified_by) values((select payload_id from payload where payload_url='http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000002_r000540_pedestal_1.0.0.1.root'), (select run_id from run where experiment_id=(select experiment_id from experiment where name='topcrt-e000002') and name='r000500'),(select run_id from run where experiment_id=(select experiment_id from experiment where name='topcrt-e000002') and name='r000570'), now(), 'Root init script')"));
    srv->Query(Form("insert into payload_iov(payload_id, initial_run_id, final_run_id, dtm_ins, modified_by) values((select payload_id from payload where payload_url='http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000002_r000571_pedestal_1.0.0.1.root'), (select run_id from run where experiment_id=(select experiment_id from experiment where name='topcrt-e000002') and name='r000571'),(select run_id from run where experiment_id=(select experiment_id from experiment where name='topcrt-e000002') and name='r000614'), now(), 'Root init script')"));
    srv->Query(Form("insert into payload_iov(payload_id, initial_run_id, final_run_id, dtm_ins, modified_by) values((select payload_id from payload where payload_url='http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000002_r000615_pedestal_1.0.0.1.root'), (select run_id from run where experiment_id=(select experiment_id from experiment where name='topcrt-e000002') and name='r000615'),(select run_id from run where experiment_id=(select experiment_id from experiment where name='topcrt-e000003') and name='r000700'), now(), 'Root init script')"));
    srv->Query(Form("insert into payload_iov(payload_id, initial_run_id, final_run_id, dtm_ins, modified_by) values((select payload_id from payload where payload_url='http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000003_r000701_pedestal_1.0.0.1.root'), (select run_id from run where experiment_id=(select experiment_id from experiment where name='topcrt-e000003') and name='r000701'),(select run_id from run where experiment_id=(select experiment_id from experiment where name='topcrt-e000003') and name='r000762'), now(), 'Root init script')"));
    srv->Query(Form("insert into payload_iov(payload_id, initial_run_id, final_run_id, dtm_ins, modified_by) values((select payload_id from payload where payload_url='http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000003_r000763_pedestal_1.0.0.1.root'), (select run_id from run where experiment_id=(select experiment_id from experiment where name='topcrt-e000003') and name='r000763'),(select run_id from run where experiment_id=(select experiment_id from experiment where name='topcrt-e000003') and name='r000817'), now(), 'Root init script')"));
    srv->Query(Form("insert into payload_iov(payload_id, initial_run_id, final_run_id, dtm_ins, modified_by) values((select payload_id from payload where payload_url='http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000003_r000818_pedestal_1.0.0.1.root'), (select run_id from run where experiment_id=(select experiment_id from experiment where name='topcrt-e000003') and name='r000818'),(select run_id from run where experiment_id=(select experiment_id from experiment where name='topcrt-e000004') and name='r000937'), now(), 'Root init script')"));
    srv->Query(Form("insert into payload_iov(payload_id, initial_run_id, final_run_id, dtm_ins, modified_by) values((select payload_id from payload where payload_url='http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000004_r000938_pedestal_1.0.0.1.root'), (select run_id from run where experiment_id=(select experiment_id from experiment where name='topcrt-e000004') and name='r000938'),(select max(run_id) from run), now(), 'Root init script')"));

    //    // now a global tag.

    // first status
    srv->Query(Form("insert into global_tag_status(name, description, dtm_ins) values('valid','Valid global tag.',now())"));
    srv->Query(Form("insert into global_tag_status(name, description, dtm_ins) values('invalid','Invalid global tag.',now())"));

    // and types
    srv->Query(Form("insert into global_tag_type(name, description, dtm_ins) values('lepsTest','Test global tag type for the LEPs 2013 test.',now())"));

    // Now define a global tag
    srv->Query(Form("insert into global_tag(name, description, dtm_ins, is_default, global_tag_status_id, global_tag_type_id, modified_by) values('leps2013_InitialTest_GlobalTag','Test global tag for the LEPs 2013 test.',now(),false,(select global_tag_status_id from global_tag_status where name='valid'), (select global_tag_type_id from global_tag_type where name='lepsTest'),'Root init script')"));

    // Now add payloads.
    srv->Query(Form("insert into global_tag_payload(global_tag_id, payload_id, dtm_ins) values((select global_tag_id from global_tag where name='leps2013_InitialTest_GlobalTag'),(select payload_id from payload where payload_url='http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000002_r000540_pedestal_1.0.0.1.root'),now())"));
    srv->Query(Form("insert into global_tag_payload(global_tag_id, payload_id, dtm_ins) values((select global_tag_id from global_tag where name='leps2013_InitialTest_GlobalTag'),(select payload_id from payload where payload_url='http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000002_r000571_pedestal_1.0.0.1.root'),now())"));
    srv->Query(Form("insert into global_tag_payload(global_tag_id, payload_id, dtm_ins) values((select global_tag_id from global_tag where name='leps2013_InitialTest_GlobalTag'),(select payload_id from payload where payload_url='http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000002_r000615_pedestal_1.0.0.1.root'),now())"));
    srv->Query(Form("insert into global_tag_payload(global_tag_id, payload_id, dtm_ins) values((select global_tag_id from global_tag where name='leps2013_InitialTest_GlobalTag'),(select payload_id from payload where payload_url='http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000003_r000701_pedestal_1.0.0.1.root'),now())"));
    srv->Query(Form("insert into global_tag_payload(global_tag_id, payload_id, dtm_ins) values((select global_tag_id from global_tag where name='leps2013_InitialTest_GlobalTag'),(select payload_id from payload where payload_url='http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000003_r000763_pedestal_1.0.0.1.root'),now())"));
    srv->Query(Form("insert into global_tag_payload(global_tag_id, payload_id, dtm_ins) values((select global_tag_id from global_tag where name='leps2013_InitialTest_GlobalTag'),(select payload_id from payload where payload_url='http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000003_r000818_pedestal_1.0.0.1.root'),now())"));
    srv->Query(Form("insert into global_tag_payload(global_tag_id, payload_id, dtm_ins) values((select global_tag_id from global_tag where name='leps2013_InitialTest_GlobalTag'),(select payload_id from payload where payload_url='http://belle2db.hep.pnnl.gov/topcaf/pedestal/1.0.0.1/top_pedestal_1.0.0.1_topcrt-e000004_r000938_pedestal_1.0.0.1.root'),now())"));




    srv->Close();

  }
  
}
