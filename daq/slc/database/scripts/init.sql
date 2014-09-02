drop function if exists groupname(int);
drop function if exists groupname_by_node(int);
drop function if exists nodename(int);
drop function if exists tablename(int);
drop function if exists tablename_by_field(int);
drop function if exists tablename_by_configinfo(text, text);
drop function if exists fieldname(int);
drop function if exists fieldtype(int);
drop function if exists fieldalias(int);
drop function if exists fieldnames();
drop function if exists fieldnames2();
drop function if exists fieldnames(text, int);
drop function if exists configid(text, text, text, int);
drop function if exists configid(text, text);
drop function if exists confignames();
drop function if exists confignames(text);
drop function if exists confignames(text, text);
drop function if exists loggernames2(text);
drop function if exists loggernames(text);
drop function if exists loggernames();
drop function if exists nodenames(text);
drop function if exists nodenames();
drop function if exists addnode(text, text);
drop function if exists addgroup(text);
drop function if exists addtable(text, int, boolean, boolean);
drop function if exists addtable(text, boolean, boolean);
drop function if exists addconfig(text, text, text, int);
drop function if exists addlogger(text, text);
drop function if exists addfield(text, int, text, int, int);
drop function if exists loggerid(text, text, int);
drop function if exists addrunnumber(int, int, int, boolean);

drop table if exists fieldinfo_names2;
drop table if exists fieldinfo_names;
drop table if exists loggerinfo_names;
drop table if exists loggerinfo;
drop table if exists configinfo_names;
drop table if exists configinfo;
drop table if exists "fieldinfo.type";
drop table if exists "fieldinfo";
drop table if exists tableinfo;
drop table if exists nodeinfo_names;
drop table if exists nodeinfo;
drop table if exists groupinfo;
drop table if exists runnumberinfo;
drop table if exists fileinfo;

create or replace function create_if_not_exists (table_name text, create_stmt text)
returns void as $$
begin

  if not exists (
    select * from   pg_catalog.pg_tables 
    where  tablename  = table_name
  ) then
  execute create_stmt;
end if;

end;
$$ language plpgsql;


create table groupinfo (
  record_time timestamp with time zone not null default current_timestamp,
  id serial primary key, name text not null unique
);

create table nodeinfo (
  record_time timestamp with time zone not null default current_timestamp,
  id serial primary key, name text not null, 
  groupid int not null references groupinfo(id), 
  unique (name, groupid)
);

create table runnumberinfo (
  record_time timestamp with time zone not null default current_timestamp,
  id serial primary key not null,
  expno int not null, 
  runno int not null, 
  subno int not null,
  isstart boolean not null
  constraint examination check (expno > 0 and 
  runno > 0 and subno < 256 and subno >= 0),
  unique (expno, runno, subno, isstart)
);

create table tableinfo (
  record_time timestamp with time zone not null default current_timestamp,
  id serial primary key, 
  name text not null, 
  isconfig boolean not null,
  revision int not null, 
  isroot boolean,
  unique (name, revision)
);

create table fieldinfo (
  record_time timestamp with time zone not null default current_timestamp,
  id serial primary key, 
  name text not null, 
  tableid int references tableinfo(id), 
  type int not null,
  length smallint not null,
  unique (name, tableid)
);

create table "fieldinfo.type" (
  id serial primary key, 
  name text not null unique,
  alias text not null
);

insert into "fieldinfo.type" (name, alias) values ('bool', 'boolean');
insert into "fieldinfo.type" (name, alias) values ('char', 'char');
insert into "fieldinfo.type" (name, alias) values ('short', 'smallint');
insert into "fieldinfo.type" (name, alias) values ('int', 'int');
insert into "fieldinfo.type" (name, alias) values ('long', 'bigint');
insert into "fieldinfo.type" (name, alias) values ('float', 'float');
insert into "fieldinfo.type" (name, alias) values ('double', 'double precision');
insert into "fieldinfo.type" (name, alias) values ('text', 'text');
insert into "fieldinfo.type" (name, alias) values ('object', 'int');
/* insert into "fieldinfo.type" (name, alias) values ('date', 'timestamp');*/
insert into "fieldinfo.type" (name, alias) values ('nsm::char', 'smallint');
insert into "fieldinfo.type" (name, alias) values ('nsm::int16', 'smallint');
insert into "fieldinfo.type" (name, alias) values ('nsm::int32', 'int');
insert into "fieldinfo.type" (name, alias) values ('nsm::int64', 'bigint');
insert into "fieldinfo.type" (name, alias) values ('nsm::byte8', 'smallint');
insert into "fieldinfo.type" (name, alias) values ('nsm::uint16', 'int');
insert into "fieldinfo.type" (name, alias) values ('nsm::uint32', 'int');
insert into "fieldinfo.type" (name, alias) values ('nsm::uint64', 'bigint');
insert into "fieldinfo.type" (name, alias) values ('nsm::float', 'float');
insert into "fieldinfo.type" (name, alias) values ('nsm::double', 'double precision');
insert into "fieldinfo.type" (name, alias) values ('nsm::object', 'bigint');

create table configinfo (
  record_time timestamp with time zone not null default current_timestamp, 
  id serial primary key, 
  name text not null, 
  nodeid int references nodeinfo(id) not null, 
  tableid int references tableinfo(id) not null,
  unique (name, nodeid, tableid)
);

create table loggerinfo (
  record_time timestamp with time zone not null default current_timestamp, 
  id bigserial primary key, 
  runnumberid int references runnumberinfo(id), 
  nodeid int references nodeinfo(id), 
  tableid int references tableinfo(id)
);

create or replace function groupname(int) returns text as $$
  select name from groupinfo where id = $1;
$$ language sql;
      
create or replace function nodename(int) returns text as $$
  select name from nodeinfo where id = $1;
$$ language sql;

create table nodeinfo_names (
  record_time double precision,
  id int references nodeinfo(id),
  name text,
  "group" text
);

create or replace function nodenames(text) returns setof nodeinfo_names as $$
  select extract(epoch from nodeinfo.record_time) as record_time,
  nodeinfo.id as id, nodeinfo.name as name,
  groupinfo.name as "group" from nodeinfo, groupinfo 
  where groupinfo.id = nodeinfo.groupid 
  and groupinfo.name = $1;
$$ language sql;

create or replace function nodenames() returns setof nodeinfo_names as $$
  select extract(epoch from nodeinfo.record_time) as record_time,
  nodeinfo.id as id, nodeinfo.name as name,
  groupinfo.name as "group" from nodeinfo, groupinfo 
  where groupinfo.id = nodeinfo.groupid;
$$ language sql;

create or replace function addnode(text, text) returns int as $$
  insert into nodeinfo (name, groupid) select 
  $1, (select id from groupinfo where name = $2)
  where not exists (select * from nodeinfo where name = $1 
  and groupid = (select id from groupinfo where name = $2))
  returning id;
$$ language sql;

create or replace function addgroup(text) returns int as $$
  insert into groupinfo (name) select 
  $1 where not exists (select * from groupinfo where name = $1)
  returning id; 
$$ language sql;

create or replace function addtable(text, int, boolean, boolean) returns int as $$
  insert into tableinfo (name, revision, isconfig, isroot) select 
  $1, $2, $3, $4 where not exists 
  (select * from tableinfo where name = $1 and revision = $2)
  returning id; 
$$ language sql;

create or replace function addtable(text, boolean, boolean) returns int as $$
  insert into tableinfo (name, revision, isconfig, isroot) select 
  $1, (select max(revision)+1 from tableinfo where name = $1),
  $2, $3 returning id; 
$$ language sql;

create or replace function tablename_by_configinfo(text, text) returns text as $$
  select configinfo.name||','||nodeinfo.name
  ||','||tableinfo.name||','||tableinfo.revision 
  as table from configinfo, nodeinfo, tableinfo
  where configinfo.nodeid = nodeinfo.id and
  nodeinfo.name = $2 and configinfo.tableid = tableinfo.id
  and configinfo.name = $1;		    
$$ language sql;

create or replace function tablename_by_configinfo(int) returns text as $$
  select configinfo.name||','||nodeinfo.name
  ||','||tableinfo.name||','||tableinfo.revision 
  as table from configinfo, nodeinfo, tableinfo
  where configinfo.nodeid = nodeinfo.id and
  configinfo.tableid = tableinfo.id
  and configinfo.id = $1
$$ language sql;

create or replace function groupname_by_node(int) returns text as $$
  select groupinfo.name from groupinfo, nodeinfo 
  where nodeinfo.id = $1 and nodeinfo.groupid = groupinfo.id;
$$ language sql;
      
create or replace function tablename(int) returns text as $$
  select name from tableinfo where id = $1;
$$ language sql;

create or replace function tablename_by_field(int) returns text as $$
  select tableinfo.name from tableinfo, fieldinfo 
  where tableinfo.id = fieldinfo.tableid and fieldinfo.id = $1;
$$ language sql;

create or replace function fieldtype(int) returns text as $$
  select "fieldinfo.type".name from "fieldinfo.type", fieldinfo 
  where "fieldinfo.type".id = fieldinfo.id and fieldinfo.id = $1;
$$ language sql;

create or replace function fieldname(int) returns text as $$
  select name from "fieldinfo.type" where id = $1;
$$ language sql;

create or replace function fieldalias(int) returns text as $$
  select alias from "fieldinfo.type" where id = $1;
$$ language sql;

create table fieldinfo_names (
  record_time timestamp with time zone,
  id int references fieldinfo(id),
  "table" text,
  revision int,
  name text,
  type int, 
  length smallint
);

create table fieldinfo_names2 (
  record_time timestamp with time zone,
  id int references fieldinfo(id),
  "table" text,
  revision int,
  name text,
  type text, 
  length smallint
);

create or replace function fieldnames() returns setof fieldinfo_names as $$
  select fieldinfo.record_time as record_time,
  fieldinfo.id as id, 
  tableinfo.name as "table",
  tableinfo.revision as revision,
  fieldinfo.name as name,
  fieldinfo.type as type, 
  fieldinfo.length as length 
  from fieldinfo, tableinfo
  where fieldinfo.tableid = tableinfo.id;
$$ language sql;

create or replace function fieldnames2() returns setof fieldinfo_names2 as $$
  select fieldinfo.record_time as record_time,
  fieldinfo.id as id, 
  tableinfo.name as "table",
  tableinfo.revision as revision,
  fieldinfo.name as name,
  "fieldinfo.type".name as type, 
  fieldinfo.length as length 
  from fieldinfo, tableinfo, "fieldinfo.type"
  where fieldinfo.tableid = tableinfo.id and 
  fieldinfo.type = "fieldinfo.type".id;
$$ language sql;

create or replace function fieldnames(text, int) returns setof fieldinfo_names as $$
  select fieldinfo.record_time as record_time,
  fieldinfo.id as id, 
  tableinfo.name as "table",
  tableinfo.revision as revision,
  fieldinfo.name as name,
  fieldinfo.type as type, 
  fieldinfo.length as length 
  from fieldinfo, tableinfo
  where fieldinfo.tableid = tableinfo.id
  and tableinfo.name = $1 and tableinfo.revision = $2;
$$ language sql;

create or replace function configid(text, text, text, int) returns int as $$
  select configinfo.id from configinfo, nodeinfo, tableinfo
   where configinfo.nodeid = nodeinfo.id 
   and configinfo.tableid = tableinfo.id
   and configinfo.name = $1 and nodeinfo.name = $2
   and tableinfo.name = $3 and tableinfo.revision = $4;
$$ language sql;

create or replace function configid(text, text) returns int as $$
  select configinfo.id from configinfo, nodeinfo, tableinfo
  where configinfo.nodeid = nodeinfo.id 
  and configinfo.tableid = tableinfo.id
  and configinfo.name = $1 and nodeinfo.name = $2
  and tableinfo.isroot = true;
$$ language sql;

create table configinfo_names (
  record_time timestamp with time zone,
  id int references configinfo(id),
  name text,
  node text,
  "table" text,
  revision int
);

create or replace function confignames() returns setof configinfo_names as $$
  select configinfo.record_time as record_time,
  configinfo.id as id, configinfo.name as name,
  nodeinfo.name as node, tableinfo.name as "table",
  tableinfo.revision as revision
  from configinfo, nodeinfo, tableinfo
  where configinfo.nodeid = nodeinfo.id 
  and configinfo.tableid = tableinfo.id;
$$ language sql;

create or replace function confignames(text) returns setof configinfo_names as $$
  select configinfo.record_time as record_time,
  configinfo.id as id, configinfo.name as name,
  nodeinfo.name as node, tableinfo.name as "table",
  tableinfo.revision as revision
  from configinfo, nodeinfo, tableinfo
  where configinfo.nodeid = nodeinfo.id 
  and configinfo.tableid = tableinfo.id
  and nodeinfo.name = $1 and tableinfo.isroot = true;
$$ language sql;

create or replace function confignames(text, text) returns setof configinfo_names as $$
  select configinfo.record_time as record_time,
  configinfo.id as id, configinfo.name as name,
  nodeinfo.name as node, tableinfo.name as "table",
  tableinfo.revision as revision
  from configinfo, nodeinfo, tableinfo
  where configinfo.name = $1 
  and configinfo.nodeid = nodeinfo.id 
  and configinfo.tableid = tableinfo.id
  and nodeinfo.name = $2 and tableinfo.isroot = true;
$$ language sql;

create or replace function addconfig(text, text, text, int) returns int as $$
  insert into configinfo (name, nodeid, tableid)
  select $1, (select id from nodeinfo where name = $2),
  (select id from tableinfo where name = $3)
  where not exists (select * from configinfo where name = $1
  and tableid = (select id from tableinfo where name = $3 
  and revision = $4) and nodeid = (select id from nodeinfo 
  where name = $2)) returning id;
$$ language sql;

create or replace function addfield(text, int, text, int, int) returns int as $$
  insert into fieldinfo (name, type, tableid, length) select
  $1, $2, (select id from tableinfo where name = $3 and revision = $4), $5
  where not exists (select * from fieldinfo where name = $1 and
  tableid = (select id from tableinfo where name = $3)) returning id;
$$ language sql;

create or replace function addlogger(text, text) returns bigint as $$
  insert into loggerinfo (runnumberid, nodeid, tableid)
  select (select max(id) from runnumberinfo limit 1),
  (select id from nodeinfo where name = $1),
  (select id from tableinfo where name = $2)
  returning id;
$$ language sql;

create table loggerinfo_names (
  record_time timestamp with time zone, 
  id bigint references loggerinfo(id),
  node text,
  "table" text
);

create or replace function loggernames(text) returns setof loggerinfo_names as $$
  select loggerinfo.record_time as record_time,
  loggerinfo.id as id, nodeinfo.name as node,
  tableinfo.name as "table"
  from loggerinfo, nodeinfo, tableinfo
  where loggerinfo.nodeid = nodeinfo.id 
  and loggerinfo.tableid = tableinfo.id
  and nodeinfo.name = $1 and tableinfo.isroot = true;
$$ language sql;

create or replace function loggernames2(text) returns setof loggerinfo_names as $$
  select loggerinfo.record_time as record_time,
  loggerinfo.id as id, nodeinfo.name as node,
  tableinfo.name as "table"
  from loggerinfo, nodeinfo, tableinfo
  where loggerinfo.nodeid = nodeinfo.id 
  and loggerinfo.tableid = tableinfo.id
  and tableinfo.name = $1 and tableinfo.isroot = true;
$$ language sql;

create or replace function loggernames() returns setof loggerinfo_names as $$
  select loggerinfo.record_time as record_time,
  loggerinfo.id as id, nodeinfo.name as node,
  tableinfo.name as "table"
  from loggerinfo, nodeinfo, tableinfo
  where loggerinfo.nodeid = nodeinfo.id 
  and loggerinfo.tableid = tableinfo.id;
$$ language sql;

create or replace function loggerid(text, text, int) returns bigint as $$
  select loggerinfo.id from loggerinfo, nodeinfo, tableinfo
  where loggerinfo.nodeid = nodeinfo.id 
  and loggerinfo.tableid = tableinfo.id
  and tableinfo.name = $2 and tableinfo.revision = $3
  and nodeinfo.name = $1
  order by loggerinfo.id desc limit 1;
$$ language sql;

create or replace function addrunnumber(int, int, int, boolean) 
returns runnumberinfo as $$
  insert into runnumberinfo (expno, runno, subno, isstart)
  select $1, $2, $3, $4
  returning *;
$$ language sql;

create table fileinfo (
  path text not null, 
  expno int not null,
  runno int not null,
  fileid int not null,
  diskid int not null,
  time_create timestamp with time zone,
  time_close timestamp with time zone,
  time_copy timestamp with time zone,
  time_delete timestamp with time zone
);

