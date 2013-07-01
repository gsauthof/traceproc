

#include <occi.h>

#include <string>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace oracle::occi;

#include "ret_check.h"
extern "C" {
#include "ora_util.h"
}

int main()
{
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(0);

  ora_print_signals("Before createEnvironment()");
  Environment *env = Environment::createEnvironment();
  ora_print_signals("After createEnvironment()");
  ora_reset_signals();
  // XXX signals

  Ora_Connect c = {0};
  int ret = ora_init_vars(&c);
  IFTRUERET(ret, 0, -1);

  Connection *con = env->createConnection(c.username, c.password, c.dbspec);

  Statement *stmt = con->createStatement(
      "SELECT station_id, lufttemperatur FROM tageswerte_tbl "
      "  WHERE "
      "    lufttemperatur != -999 "
      );
  stmt->setPrefetchRowCount(1024);
  ResultSet *set = stmt->executeQuery();

  while (set->next()) {
    string station_id, lufttemperatur;
    station_id = set->getString(1);
    lufttemperatur = set->getString(2);
    cout << setw(7) << station_id << ';'
         << setw(7) << lufttemperatur << '\n';
  }

  stmt->closeResultSet(set);
  con->terminateStatement(stmt);


  env->terminateConnection(con);
  Environment::terminateEnvironment(env);

  return 0;
}
