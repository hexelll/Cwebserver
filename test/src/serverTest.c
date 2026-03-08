#include "./libs/server.c"

#include <stdio.h>

//#include "libpq-fe.h"

int main(void) {
    /*PGconn *con = PQconnectdb("user=root password=pass4root host=db port=5432 dbname=testdb connect_timeout=10");

    if (PQstatus(con) == CONNECTION_OK) 
        printf("db connection successfull\n");
    else {
        printf("db connection error : %s\n\n",PQerrorMessage(con));
        PQfinish(con);
        exit(1);
    }

    PQfinish(con);*/
    
    struct Server server = ServerDefaultInit(9999,10);
    printf("starting server ... \n\n");
    ServerRun(&server);
}