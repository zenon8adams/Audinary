#if defined( __WIN32) || defined( __MSVC)
    #error "Sorry, your platform is unsuported."
#endif

#include <httpserver.hpp>
#include <syslog.h>
#include <model/beeper.hpp>
#include <model/textual-time.hpp>
#include <sys/stat.h>
#include <iostream>
#include <libgen.h>

#include "server/http-server.hpp"
#include "include/encoder/huffman-coding.hpp"

#define PRODUCTION

void serve()
{
    httpserver::webserver clock_server = httpserver::create_webserver( 3800);

    ClockServerResource server_resource;
    clock_server.register_resource( "/", &server_resource);
    clock_server.start( true);

    closelog();
}

int main( int ac, char *av[])
{

#if defined(PRODUCTION)

    pid_t pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);

    close( STDIN_FILENO);

    openlog("Audinary", LOG_PID | LOG_NDELAY | LOG_CONS | LOG_PERROR, LOG_DAEMON);
    setlogmask( LOG_UPTO( LOG_DEBUG));

    pid_t sid = setsid();
    if (sid < 0) {
        syslog(LOG_ERR, "Unable to get session id");
        exit(EXIT_FAILURE);
    }

    if (chdir("/") < 0) {
        syslog( LOG_ERR, "Execution of chdir() failed");

        closelog();
        exit( EXIT_FAILURE);
    }

    syslog( LOG_INFO, "Daemon started successfully!");

    try {

        extern char _binary_beep_start[];
        extern char _binary_beep_end[];

        extern char _binary_i18n_start[];
        extern char _binary_i18n_end[];

        size_t i18n_bytes = ( char *)&_binary_i18n_end - ( char *)&_binary_i18n_start;
        HuffmanCoding decoder( HuffmanCoding::GeneratorMode::DECODE);
        TextualTime::instance().setRefrenceLocalization( decoder.retrieve( ( const char *)&_binary_i18n_start, i18n_bytes));

        size_t beep_bytes = ( char *)&_binary_beep_end - ( char *)&_binary_beep_start;
        Beeper::instance()->installBeeper( ( const char *)&_binary_beep_start, beep_bytes);

        serve();

        closelog();
    }
    catch (const std::exception &error) {
        syslog( LOG_ERR, "%s", error.what());

        closelog();
        exit(EXIT_FAILURE);
    }

#elif defined(DEVELOPMENT)

//    extern char _binary_beep_start[];
//    extern char _binary_beep_end[];
//
//    extern char _binary_i18n_start[];
//    extern char _binary_i18n_end[];
//
//    size_t i18n_bytes = ( char *)&_binary_i18n_end - ( char *)&_binary_i18n_start;
//
//    HuffmanCoding decoder( HuffmanCoding::GeneratorMode::DECODE);
//
//    TextualTime::instance().setRefrenceLocalization( decoder.retrieve( ( const char *)&_binary_i18n_start, i18n_bytes));
//
//    size_t beep_bytes = ( char *)&_binary_beep_end - ( char *)&_binary_beep_start;
//    Beeper::instance()->installBeeper( ( const char *)&_binary_beep_start, beep_bytes);
//
//    serve();

//        Beeper::instance()->installBeeper(av[ 1]);
        HuffmanCoding decoder( HuffmanCoding::GeneratorMode::DECODE);
        TextualTime::instance().setRefrenceLocalization( decoder.retrieve( av[ 1]));
//        std::cout << decoder.retrieve( av[ 2]);
        serve();
//
//        HuffmanCoding encoder(HuffmanCoding::GeneratorMode::ENCODE, av[1]);
//
//        char *path = strdup( av[ 1]);
//        char *dirmem  = strdup( av[ 1]);
//        char *dir     = dirname( dirmem);
//        char *filename = basename( path);
//        char *end = strchr( filename, '.');
//        std::string patch( dir);
//
//        patch.append( "/");
//        patch.append( filename, end);
//        patch.append( ".loc");
//
//        std::cout << patch;
//
//        encoder.save( patch.c_str());
//
//        free( path);
//        free( dirmem);


/*    std::string currentTime;
    while( !( currentTime = TextualTime::get()).empty())
        std::ofstream("english.txt", std::ios::out | std::ios::app) << currentTime <<'\n';*/

#endif

}