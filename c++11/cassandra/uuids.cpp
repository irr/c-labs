#include <cassandra.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>

void print_error(CassFuture* future) {
  const char* message;
  size_t message_length;
  cass_future_error_message(future, &message, &message_length);
  fprintf(stderr, "Error: %.*s\n", (int)message_length, message);
}

CassError execute_query(CassSession* session, const char* query) {
  CassError rc = CASS_OK;
  CassFuture* future = NULL;
  CassStatement* statement = cass_statement_new(query, 0);

  future = cass_session_execute(session, statement);
  cass_future_wait(future);

  rc = cass_future_error_code(future);
  if (rc != CASS_OK) {
    print_error(future);
  }

  cass_future_free(future);
  cass_statement_free(statement);

  return rc;
}

int main() {
  CassFuture* connect_future = NULL;
  CassCluster* cluster = cass_cluster_new();
  CassSession* session = cass_session_new();

  cass_cluster_set_contact_points(cluster, "127.0.0.1");

  connect_future = cass_session_connect(session, cluster);

  if (cass_future_error_code(connect_future) == CASS_OK) {
    CassFuture* close_future = NULL;

    CassStatement* statement = cass_statement_new("CREATE KEYSPACE IF NOT EXISTS irr "
            "WITH REPLICATION = { 'class' : 'NetworkTopologyStrategy', 'datacenter1' : 1 };", 0);
    CassFuture* result_future = cass_session_execute(session, statement);
    cass_future_wait(result_future);
    cass_future_free(result_future);
    cass_statement_free(statement);

    statement = cass_statement_new("CREATE TABLE irr.series (id text, ts timeuuid, val int, PRIMARY KEY (id,ts),)"
           " WITH CLUSTERING ORDER BY (ts DESC);", 0);
    result_future = cass_session_execute(session, statement);
    cass_future_wait(result_future);
    cass_future_free(result_future);
    cass_statement_free(statement);

    for (int i = 0; i < 5; i++) {
        statement = cass_statement_new("INSERT INTO irr.series (id, ts, val) VALUES (?, now(), ?) USING TTL 20;", 2);
        cass_statement_bind_string(statement, 0, "irr-d");
        cass_statement_bind_int32(statement, 1, i);
        result_future = cass_session_execute(session, statement);
        cass_future_get_result(result_future);
        cass_future_free(result_future);
        cass_statement_free(statement);
    }

    statement = cass_statement_new("SELECT id, ts, val "
                           "FROM irr.series", 0);

    result_future = cass_session_execute(session, statement);

    if(cass_future_error_code(result_future) == CASS_OK) {
      const CassResult* result = cass_future_get_result(result_future);
      CassIterator* rows = cass_iterator_from_result(result);

      while(cass_iterator_next(rows)) {
        const CassRow* row = cass_iterator_get_row(rows);
        const CassValue* value = cass_row_get_column_by_name(row, "ts");

        CassUuid time;
        char time_str[CASS_UUID_STRING_LENGTH];
        cass_value_get_uuid(value, &time);
        cass_uuid_string(time, time_str);

        printf("ts: '%s'\n", time_str);
      }

      cass_result_free(result);
      cass_iterator_free(rows);
    } else {
      const char* message;
      size_t message_length;
      cass_future_error_message(result_future, &message, &message_length);
      fprintf(stderr, "Unable to run query: '%.*s'\n",
              (int)message_length, message);
    }

    cass_statement_free(statement);
    cass_future_free(result_future);

    close_future = cass_session_close(session);
    cass_future_wait(close_future);
    cass_future_free(close_future);
  } else {
    const char* message;
    size_t message_length;
    cass_future_error_message(connect_future, &message, &message_length);
    fprintf(stderr, "Unable to connect: '%.*s'\n",
            (int)message_length, message);
  }

  cass_future_free(connect_future);
  cass_cluster_free(cluster);
  cass_session_free(session);

  return 0;
}

