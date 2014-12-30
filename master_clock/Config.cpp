/** Configuration data wrappers
 */

#include "Config.h"

//_________________________________________________________________________
//                                                                   STATIC

static ConfigError ParseUInt( const char *& str , unsigned int &n ) ;
static ConfigError ParseUIntHex( const char *& str , unsigned int &n ) ;
static ConfigError CharToString( char *& buf , size_t &len , char ch ) ;
static ConfigError UintToString( char *& buf , size_t &len , unsigned int n , const char * fmt ) ;
static ConfigError ByteArrayToString( char * buf , size_t size , byte *data , size_t len , char delim , const char * fmt) ;

//_________________________________________________________________________
//                                                               ConfigItem
ConfigItem::ConfigItem( const char * name , void * data , uint8_t len ) : name(name), data(data), len(len)
{
	for (int i = 0 ; i < len ; i++ ) ((uint8_t *)data)[i] = 0;
}
//_____________________________________
//_________________________________________________________________________
//                                                                 ConfigU8
ConfigError ConfigU8::Parse( const char * str ) {
	unsigned int n = 0;
	ConfigError err = ParseUInt( str , n );
	if ( err ) return err ;
	if ( *str ) return ConfigError_ParseExpectedDigit ;
	if ( n > 255 ) return ConfigError_ParseTooLong ;

	this->n = (uint8_t)n;
	return ConfigError_None ;
}
//_____________________________________
ConfigError ConfigU8::ToString( char * buf , size_t len ) {
	return UintToString( buf , len , n , "%u" ) ;
}
//_____________________________________
//_________________________________________________________________________
//                                                          ConfigIpAddress
ConfigError ConfigMac::Parse( const char * str )
{
	uint8_t tmp[len];
	ConfigError err = ConfigError_None ;
	for (int i = 0 ; !err && i < len ; i++ )
	{
		unsigned int n = 0;
		err = ParseUIntHex( str , n );
		if ( err ) return err ;
		if ( i<len-1 && *str++ != ':') return ConfigError_ParseExpectedColon ;
		if ( n > 255 ) return ConfigError_ParseInvalid ;
		tmp[i] = uint8_t(n);
	}
	if ( *str ) return ConfigError_ParseTooLong ;

	for (int i = 0 ; !err && i < len ; i++ ) mac[i] = tmp[i] ;
	return ConfigError_None ;
}
//_____________________________________
ConfigError ConfigMac::ToString( char * buf , size_t size )
{
	return ByteArrayToString( buf, size , mac , len , ':' , "%02X" ) ;
}
//_____________________________________
//_________________________________________________________________________
//                                                          ConfigIpAddress
ConfigError ConfigIpAddress::Parse( const char * str )
{
	uint8_t tmp[len];
	ConfigError err = ConfigError_None ;
	for (int i = 0 ; !err && i < len ; i++ )
	{
		unsigned int n = 0;
		err = ParseUInt( str , n );
		if ( err ) return err ;
		if ( i<len-1 && *str++ != '.') return ConfigError_ParseExpectedDot ;
		if ( n > 255 ) return ConfigError_ParseInvalid ;
		tmp[i] = uint8_t(n);
	}
	if ( *str ) return ConfigError_ParseTooLong ;

	for (int i = 0 ; !err && i < len ; i++ ) address[i] = tmp[i] ;
	return ConfigError_None ;
}
//_____________________________________
ConfigError ConfigIpAddress::ToString( char * buf , size_t size )
{
	return ByteArrayToString( buf, size , address , len , '.' , "%u" ) ;
}
//_____________________________________
IPAddress ConfigIpAddress::value()
{
       return IPAddress(address[0], address[1], address[2], address[3]) ;
}
//_____________________________________________________________________
//                                                           ConfigTime
//_____________________________________
ConfigError ConfigTime::Parse( const char * str )
{
	return ConfigError_DataInvalid ; // TODO
}
//_____________________________________
ConfigError ConfigTime::ToString( char * buf , size_t size )
{
	return ByteArrayToString( buf, size , (byte *)clock , len , ':' , "%02" ) ;
}
//_____________________________________
//_____________________________________________________________________
//                                                              HELPERS
/** Parse a string into an integer */
static ConfigError ParseUInt( const char *& str , unsigned int &n )
{
	n = 0;
	if ( ! *str ) return ConfigError_ParseTooShort ;
	if ( ! isdigit(*str) ) return ConfigError_ParseExpectedDigit ;
	for ( ; isdigit(*str) ; str++ ) {
		unsigned int overflow = n ;
		n=n*10 + *str - '0' ;
		if ( n < overflow ) return ConfigError_ParseTooLong ;
	}
	return ConfigError_None ;
}
//_____________________________________
/** Parse a hex string into an integer */
static ConfigError ParseUIntHex( const char *& str , unsigned int &n )
{
	n = 0;
	if ( ! *str ) return ConfigError_ParseTooShort ;
	if ( ! isxdigit(*str) ) return ConfigError_ParseExpectedHexDigit ;
	for ( ; isxdigit(*str) ; str++ ) {
		unsigned int overflow = n ;
		n=(n<<4) + *str ;
		if ( isdigit(*str) )      n -= '0' ;
		else if ( isupper(*str) ) n -= 'A' - 10 ;
		else                      n -= 'a' - 10 ;
		if ( n < overflow ) return ConfigError_ParseTooLong ;
	}
	return ConfigError_None ;
}
//_____________________________________
/** Emit a character */
static ConfigError CharToString( char *& buf , size_t &len , char ch )
{
	if ( len < 2 ) return ConfigError_BufferTooShort ;
	*buf++ = ch ;
	*buf = 0 ;
	--len ;
	return ConfigError_None ;
}
//_____________________________________
static ConfigError ByteArrayToString( char * buf , size_t size , byte *data , size_t len , char delim , const char * fmt)
{
	ConfigError err = ConfigError_None ;
	for (size_t i = 0 ; !err && i < len ; i++ ) {
		if ( i && delim ) err = CharToString( buf , size , delim ) ;
		if ( ! err )      err = UintToString( buf , size , data[i] , fmt ) ;
	}
	return err ;
}
//_____________________________________
/** Emit an unsigned int as string */
static ConfigError UintToString( char *& buf , size_t &len , unsigned int n , const char * fmt )
{
	int count = snprintf(buf , len, fmt, n) ;
	if ( count >= int(len) ) return ConfigError_BufferTooShort ;
	buf += count ;
	len -= count ;
	return ConfigError_None ;
}
//_____________________________________
