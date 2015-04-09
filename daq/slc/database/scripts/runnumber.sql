create table runnumber (
  record_time  timestamp with time zone  not null default current_timestamp,
  id serial not null primary key,
  config text not null,
  expno int not null,
  runno int not null,
  subno int not null,
  isstart boolean not null,
  unique (expno, runno, subno, isstart)
);

create index runnumber_id_index on runnumber(id);