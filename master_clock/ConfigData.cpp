/** Configuration data wrappers
 *
 * These fields are customizable and reportable data for 
 * different installations.  Some are expected never to change.
 * Some are expected to change frequently. Do not change the
 * meaning of any existing values.  Add new fields to the end
 * of this list instead and deprecate old values.  This will
 * help ensure EEPROM does not hold meaningless old data.
 */

#include "ConfigData.h"
#include "console.h"
//___________________________________________________________________________
//                                                                CONFIG DATA

ConfigU8	configVersion("ConfigVersion") ;	///< Read-only config data version
ConfigMac	mac("MAC") ;				///< Fixed MAC address
ConfigIpAddress	staticIpAddress("StaticIpAddress");	///< Static IP address (disables DHCP)
ConfigString20	name("Name");				///< Name of this master clock
ConfigIpAddress	ntpServer1("NTPServer");		///< IP address of NTP server to poll
ConfigU8	pulseMake("PulseOn");			///< length of high pulse (units=10ms)
ConfigU8	pulseBreak("PulseOff");			///< length of low pulse (units=10ms)
ConfigU8	timeZone("TimeZone");			///< offset from GMT (hours)
ConfigU8	dstMode("DstMode");			///< Daylight Saving time adjustment (0=none, 1=US, etc.)
ConfigString20	tzName("tzName");			///< 'E'=eastern, 'C'=central, etc.

ConfigIpAddress ipAddress("IpAddress");			///< Discovered IP address
ConfigTime	Utc("UtcTime");				///< UTC time discovered from NTP server
ConfigTime	clock("ClockTime");			///< Time currently displayed on clock
ConfigTime	local("LocalTime");			///< Local time derived from UTC, TimeZone, and DstMode

ConfigItem *configData[] = {
	&configVersion,
	&mac,
	&staticIpAddress,
	&name,
	&ntpServer1,
	&pulseMake,
	&pulseBreak,
	&timeZone,
	&dstMode,
	&tzName,
	&clock
} ;

ConfigItem *ephemeralData[] = {
	&Utc,
	&local
} ;

#define ARRAY_SIZE(x)  (sizeof(x) / sizeof(x[0]))

void ShowItem( ConfigItem &ci )
{
	p( ci.GetName() ) ;
	p( " = " ) ;
	char buf[100];
	ConfigError err = ci.ToString( buf , sizeof(buf) ) ;
	if ( err ) p("ERR(%u)" , err ) ;
	else       p( "%s" , buf ) ;
	p( "\n" ) ;
}

void ShowConfig()
{
	for (unsigned int i = 0 ; i < ARRAY_SIZE(configData) ; i++ )
		ShowItem(*configData[i]);
	for (unsigned int i = 0 ; i < ARRAY_SIZE(ephemeralData) ; i++ )
		ShowItem(*ephemeralData[i]);
}
