//================================================================================
// TelnetServer

/* Library imports */
#include <SPI.h>
#include <Ethernet.h>

#include "clock_generic.h"
#include "ConfigData.h"

EthernetServer telnet_server(23);  // create a server at port 80
EthernetClient telnet_client ;

void setupTelnetServer()
{
    telnet_server.begin();           // start to listen for clients
}

int TelnetRead()
{
    int key = -1 ;
    if ( telnet_client )
        if ( telnet_client.available() )
            key = telnet_client.read();
    return key ;
}

int TelnetWrite( const char * str )
{
    return telnet_server.write( (uint8_t *) str , (size_t) strlen(str) );
}

void serviceTelnetServer()
{
    if ( telnet_client && ! telnet_client.connected() )
        telnet_client.stop() ;

    if ( ! telnet_client )
    {
        telnet_client = telnet_server.available() ;

        if ( telnet_client ) ShowConfig() ;
    }
}
