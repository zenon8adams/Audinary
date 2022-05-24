#ifndef HTTP_SERVER
#define HTTP_SERVER

void failureExit( const char *msg, const char *detail);

void failureExit( const char *msg);

void handleClient( int sock);

void serve( const int serv_port);

#endif