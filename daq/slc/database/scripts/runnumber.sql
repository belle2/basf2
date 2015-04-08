create table runnumber (
  record_time  timestamp with time zone  not null default current_timestamp,
  id serial not null primary key,
  label vchar(256) not null,
);

create index runnumber_id_index on runnumber(id);