/** Configuration data wrappers
 *
 * These fields are customizable and reportable data for 
 * different installations.  Some are expected never to change.
 * Some are expected to change frequently. Do not change the
 * meaning of any existing values.  Add new fields to the end
 * of this list instead and deprecate old values.  This will
 * help ensure EEPROM does not hold meaningless old data.
 */

#include "Arduino.h"
#include "IPAddress.h"

enum ConfigError {
	ConfigError_None               =  0 ,

	ConfigError_ParseExpectedDigit =  1 ,
	ConfigError_ParseTooShort      =  2 ,
	ConfigError_ParseTooLong       =  3 ,
	ConfigError_ParseExpectedDot   =  4 ,
	ConfigError_ParseExpectedColon =  5 ,
	ConfigError_ParseInvalid       =  6 ,
	ConfigError_ParseExpectedHexDigit =  7 ,


	ConfigError_DataInvalid        = 20 ,
	ConfigError_BufferTooShort     = 21 ,
} ;

class ConfigItem {
	public:
		ConfigItem( const char * name , void * data , uint8_t len ) ;

		/** Parse a string into this config value */
		virtual ConfigError Parse( const char * str ) = 0 ;

		/** Display value as a string */
		virtual ConfigError ToString( char * buf , size_t len ) = 0 ;

		/** Emit data as binary data with length 'len' */
		uint8_t *GetData( ) { return (uint8_t *) data ; }
		uint8_t GetLen( ) { return len ; }

		const char * GetName() { return name ; }

	protected:
		const char * 	name ;
		void * 		data ;
		uint8_t 	len ;
		uint8_t		parseIndex ;
} ;

class ConfigU8 : public ConfigItem
{
	public:
		ConfigU8( const char * name ) : ConfigItem( name, &n, 1 ) { }

		ConfigError Parse( const char * str ) ;
		ConfigError ToString( char * buf , size_t len ) ;

		uint8_t value() { return n ; }

	protected:
		uint8_t n ;
} ;

class ConfigIpAddress : public ConfigItem
{
	public:
		ConfigIpAddress( const char * name ) : ConfigItem( name, address, 4 ) { }

		ConfigError Parse( const char * str ) ;
		ConfigError ToString( char * buf , size_t size ) ;

		IPAddress value() ;
	protected:
		uint8_t address[4] ;

};

class ConfigTime : public ConfigItem
{
	public:
		ConfigTime( const char * name ) : ConfigItem( name, clock, sizeof(clock) ) { }

		ConfigError Parse( const char * str ) ;
		ConfigError ToString( char * buf , size_t size ) ;

		int minutes() ;
	protected:
		int8_t clock[3] ;
};

class ConfigMac : public ConfigItem
{
	public:
		ConfigMac( const char * name ) : ConfigItem( name, mac, sizeof(mac) ) { }

		ConfigError Parse( const char * str ) ;
		ConfigError ToString( char * buf , size_t size ) ;

		const uint8_t * value() { return mac ; }
	protected:
		uint8_t mac[6] ;
};

template< class T >
class ConfigString : public ConfigItem {
	public:
		ConfigString( const char * name ) : ConfigItem( name, str, sizeof(T) ) { }

		ConfigError Parse( const char * str ) {
			if ( strlen(str) >= len ) return ConfigError_ParseTooLong ;
			strcpy( this->str, str ) ;
			return ConfigError_None ;
		}
		ConfigError ToString( char * buf , size_t size ) {
			if ( strlen(str) >= size ) return ConfigError_BufferTooShort ;
			strcpy( buf , str ) ;
			return ConfigError_None ;
		}

		const char * value() { return str ; }
	protected:
		T str;

};
