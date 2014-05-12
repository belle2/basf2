select "loggerinfo:runsetting:1".loggerid, loggerinfo.record_time,runnumberinfo.expno, 
       runnumberinfo.runno, runnumberinfo.subno, configinfo.name as runtype, 
       "loggerinfo:runsetting:1".operators, "loggerinfo:runsetting:1".comment from 
       "loggerinfo:runsetting:1", loggerinfo, runnumberinfo, configinfo where 
       loggerinfo.id = "loggerinfo:runsetting:1".loggerid and 
       runnumberinfo.id = "loggerinfo:runsetting:1".runnumberid and 
       "loggerinfo:runsetting:1".runcontrol = configinfo.id;

select "loggerinfo:runsetting:1".loggerid, loggerinfo.record_time,runnumberinfo.expno, 
       runnumberinfo.runno, runnumberinfo.subno, configinfo.name as runtype, 
       "fieldinfo.type.enum".name as state from loggerinfo, runnumberinfo, configinfo, 
       "loggerinfo:runsummary:1", "loggerinfo:runsetting:1", fieldnames() as fieldinfo2, 
       "fieldinfo.type.enum" where loggerinfo.id = "loggerinfo:runsummary:1".loggerid and 
       "loggerinfo:runsummary:1".runsetting = "loggerinfo:runsetting:1".loggerid and 
       runnumberinfo.id = "loggerinfo:runsetting:1".runnumberid and 
       "loggerinfo:runsetting:1".runcontrol = configinfo.id and 
       fieldinfo2.table = 'runsummary' and fieldinfo2.name = 'state' and 
       fieldinfo2.id = "fieldinfo.type.enum".fieldid and 
       "loggerinfo:runsummary:1".state = "fieldinfo.type.enum".index;

select r1.record_time as runstart, r1.expno, r1.runno, r1.subno, 
       extract(epoch from r2.record_time) - extract(epoch from r1.record_time) as runlength 
       from (select * from runnumberinfo where isstart = true) as r1 , 
       (select * from runnumberinfo where isstart = false) as r2 where r1.expno = r2.expno 
       and r1.runno = r2.runno and r1.subno = r2.subno;
