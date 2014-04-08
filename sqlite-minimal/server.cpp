#include <iostream>
#include <cstdlib>
#include <sqlite3.h>

using namespace std;

int main(void)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;

    int rc, nrows, ncols;

    rc = sqlite3_open("test.db", &db);

    if (rc)
    {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        exit(1);
    }

    string sql("select * from t;");
    string sqlk("select * from t where rowid = 1;");
    const char* tail;

    rc = sqlite3_prepare(db, sql.c_str(), -1, &stmt, &tail);

    if (SQLITE_OK == rc)
    {
        rc = sqlite3_step(stmt);
        ncols = sqlite3_column_count(stmt);

        while (SQLITE_ROW == rc)
        {
            for (int i=0; i < ncols; i++)
            {
                string s(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
                cout <<  s << " ";
            }
            cout << endl;
            rc = sqlite3_step(stmt);
        }

        sqlite3_finalize(stmt);

        char **results ;
        char *error;
        rc = sqlite3_get_table(db, sqlk.c_str(), &results, &nrows, &ncols, &error);

        if (SQLITE_OK == rc)
        {
            cout << "rows: " << nrows << ", cols: " << ncols << endl;

            for (int i = 0; i < ncols; i++)
                cout << results[i] << endl;

            int p = ncols;
            for (int i = 0; i < nrows; i++)
                for (int j = 0; j < ncols; j++)
                {
                    cout << results[p++] << endl;
                }

            sqlite3_free_table(results);
        }
        else
        {
            cerr << "SQL error: " << error << endl;

            sqlite3_free(error);
        }
    }
    else
    {
        cerr << "SQL error: " << sqlite3_errmsg(db) << endl;
    }

    sqlite3_close(db);
    return 0;
}

