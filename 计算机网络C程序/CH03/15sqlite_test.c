#include <stdio.h>
#include <unistd.h>
#include <sqlite3.h>
#include <time.h>

#define TEST_CNT       200000
#define TEST_BURST_CNT    100
#define TEST_DUMP_STEP    100

//#define TEST_CSV_OUTPUT

void testOneRun(int idx, sqlite3 *conn);

int main()
{
    sqlite3 *conn;
    sqlite3_stmt *stmt;
    int newfile = 0;
    int i;

    static char *db_filename = "testdb.db";
    static char *sql_createtable = 
        "create table maintbl ( "
        "    id int, "
        "    info int, "
        "    PRIMARY KEY(id) "
        ");";

    if ( access(db_filename, F_OK) == 0 )
        newfile = 0;
    else
        newfile = 1;

    sqlite3_open(db_filename, &conn);

    if ( newfile ) {
        sqlite3_prepare(conn, sql_createtable, -1, &stmt, NULL);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    sqlite3_prepare(conn, "delete from maintbl;", -1, &stmt, NULL);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    for (i = 0; i < TEST_CNT; i += TEST_BURST_CNT ) {
        testOneRun(i, conn);
    }

    sqlite3_close(conn);
}

void testOneRun(int idx, sqlite3 *conn)
{
    sqlite3_stmt *stmt;
    char sql_stmt_buf[1024];
    int i;
    struct timespec t1, t2, diff;
    static unsigned long long nsins, nssel;

    static char *sql_fmt_insert = "insert into maintbl values (%d, 'test string at %d');";
    static char *sql_fmt_select = "select * from maintbl where id=%d;";

    if ( idx % TEST_DUMP_STEP == 0 ) {
        if ( idx > 0 )
#ifdef TEST_CSV_OUTPUT
            printf("%d, %lf, %lf\n", idx,
                   (double)nsins / TEST_DUMP_STEP / 1000000000.0,
                   (double)nssel / TEST_DUMP_STEP / 1000000000.0);
#else
            printf("    Perf: Insert = %lf s/row; Select = %lf s/row\n",
                   (double)nsins / TEST_DUMP_STEP / 1000000000.0,
                   (double)nssel / TEST_DUMP_STEP / 1000000000.0);
#endif
        nsins = 0;
        nssel = 0;
    }

    if ( idx >= TEST_CNT )
        return;

    for ( i = 0; i < TEST_BURST_CNT; i++ ) {
        sprintf(sql_stmt_buf, sql_fmt_insert, idx + i, idx + i);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        sqlite3_prepare(conn, sql_stmt_buf, -1, &stmt, NULL);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        nsins += (t2.tv_sec * 1000000000 + t2.tv_nsec - t1.tv_sec * 1000000000 - t1.tv_nsec);
    }
    
    for ( i = 0; i < TEST_BURST_CNT; i++ ) {
        sprintf(sql_stmt_buf, sql_fmt_select, idx + i);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        sqlite3_prepare(conn, sql_stmt_buf, -1, &stmt, NULL);
        sqlite3_step(stmt);

#ifndef TEST_CSV_OUTPUT
        if ( (idx + i) % TEST_DUMP_STEP == 0 ) {
            printf("Dump %6d: ", idx + i);
            printf("%d, %s\n", sqlite3_column_int(stmt, 0),
                               sqlite3_column_text(stmt, 1));
        }
#endif
        sqlite3_finalize(stmt);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        nssel += (t2.tv_sec * 1000000000 + t2.tv_nsec - t1.tv_sec * 1000000000 - t1.tv_nsec);
    }
}
