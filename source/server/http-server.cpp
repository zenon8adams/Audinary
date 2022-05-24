#include <memory>
#include <httpserver.hpp>
#include "server/router.hpp"
#include "../../include/server/http-server.hpp"

#include <syslog.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <string>

#define MAXCLIENT 5

void failureExit( const char *msg, const char *detail)
{
    syslog( LOG_ERR, "%s: %s\n", msg, detail);

    closelog();
    exit( EXIT_FAILURE);
}

void failureExit( const char *msg)
{
    failureExit( msg, strerror( errno));
}

void handleClient( int sock)
{
    char buffer[ BUFSIZ];

    ssize_t num_bytes_received = recv( sock, buffer, BUFSIZ, 0);
    if( num_bytes_received < 0)
        failureExit( "recv() failed");

    const char *delim = " \t\n\r";
    char *param_ptr, *arg_ptr,
            *bp = strtok_r( buffer, delim, &arg_ptr);

    if( strcmp( bp, "GET") != 0)
        failureExit( "Query method", "Only GET method is supported");

    bp = strtok_r( NULL, delim, &arg_ptr);
    if( *bp != '/')
        failureExit( "Param", "Expected a '/'");
    if( *++bp == '?')
        ++bp;
    std::unordered_map<std::string, std::string> params;
    while( ( bp = strtok_r( bp, "&", &param_ptr)) != NULL)
    {
        const char *sep = strchr( bp, '=');
        std::string key( (const char *)bp, sep), value( sep + 1, sep + strlen( sep));
        bp = NULL;
        params[ key] = value;
    }

    bp = strtok_r( NULL, delim, &arg_ptr);

    if( strcmp( bp, "HTTP/1.0") != 0 && strcmp( bp, "HTTP/1.1") != 0)
        failureExit( "Protocol", "Only HTTP/1.0 or HTTP/1.1 are supported!");

    if( num_bytes_received > 0)
    {
        auto json_response = Router::generateResponse( params);
        json_response.insert( 0, "HTTP/1.0 200 OK\nContent-Type: application/json\n"
                                 "Content-Length: " + std::to_string( json_response.size()) + "\n\n");
        ssize_t num_bytes_sent = send( sock, json_response.data(), json_response.size(), 0);
        if( num_bytes_sent < 0 || num_bytes_sent != ( ssize_t)json_response.size())
            failureExit( "send() failed");
    }

    close( sock);
}

void serve( const int serv_port)
{
    int sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( sock < 0)
        failureExit( "socket() failed");

    sockaddr_in serv_addr;
    memset( &serv_addr, 0, sizeof( serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl( INADDR_ANY);
    serv_addr.sin_port = htons( serv_port);

    if( bind( sock, ( sockaddr *)&serv_addr, sizeof( serv_addr)) < 0)
        failureExit( "bind() failed");

    if( listen( sock, MAXCLIENT) < 0)
        failureExit( "listen() failed");

    for( ;;)
    {
        sockaddr_in client_addr;
        socklen_t client_addr_length = sizeof( client_addr);

        int client_sock = accept( sock, ( sockaddr *)&client_addr, &client_addr_length);
        if( client_sock < 0)
            failureExit( "accept() failed");

        handleClient( client_sock);
    }
}