/** Configuration data wrappers
 */

#include "Config.h"

typedef char str20[20];
typedef ConfigString<str20> ConfigString20 ;

extern ConfigU8		configVersion ;
extern ConfigMac	mac ;
extern ConfigIpAddress	staticIpAddress ;
extern ConfigString20	name ;
extern ConfigIpAddress	ntpServer1 ;
extern ConfigU8		pulseMake ;
extern ConfigU8		pulseBreak ;
extern ConfigU8		timeZone ;
extern ConfigU8		dstMode ;
extern ConfigString20	tzName ;

extern ConfigIpAddress	ipAddress ;
extern ConfigTime	Utc ;
extern ConfigTime	clock ;
extern ConfigTime	local ;

void ShowConfig() ;
