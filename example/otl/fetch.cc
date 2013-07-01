
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#define OTL_ORA11G_R2
#define OTL_STL
#include <otlv4.h>

extern "C" {
#include <ora_util.h>
}
#include <ret_check.h>

using namespace std;

otl_connect db;

// little bench (select of almost 1849990 to /dev/null):
// name               s
// pc-noprefetch      58
// pc-prefetch-1024   5.3
// otl-string-non-opt 1.9
// pc-array-1024      1.4
// otl-string-opt     1.5
// otl-char-arr-opt   1.36
// occi-1024          1.9

void select()
{
  otl_stream s(1024, // buffer
    //"SELECT station_id :#1<string>, lufttemperatur :#2<string> FROM tageswerte_tbl "
     "SELECT station_id :#1<char[10]>, lufttemperatur :#2<char[10]>  FROM tageswerte_tbl "
    //"SELECT station_id :#1<long>, lufttemperatur FROM tageswerte_tbl "
    //"SELECT station_id , lufttemperatur FROM tageswerte_tbl "
    "  WHERE "
    "    lufttemperatur != -999 ",
    db);
  while (!s.eof()) {
    // also works
    //string station_id, lufttemperatur;
    // but this is (of course) a little bit faster
    char station_id[11] = {0};
    char lufttemperatur[11] = {0};

    //long station_id = 0;
    //double lufttemperatur = 0.0;
    s >> station_id >> lufttemperatur;
    //cout <<  station_id << ';'
    //    << lufttemperatur << '\n';
    cout << setw(7) << station_id << ';'
         << setw(7) << lufttemperatur << '\n';
  }
}

int main(int argc, char **argv)
{
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(0);

  otl_connect::otl_initialize();

  Ora_Connect c = {0};
  int ret = ora_init_vars(&c);
  IFTRUERET(ret, 0, 1);
  ostringstream l;
  l << c.username << '/' << c.password << '@' << c.dbspec;

  try {
    db.rlogon(l.str().c_str()); // connect to Oracle

    select();

  }

  catch(const otl_exception& e){
    cerr << e.msg      << '\n'
         << e.stm_text << '\n'
         << e.var_info << '\n';
  }

  db.logoff(); // disconnect from Oracle
  return 0;
}
