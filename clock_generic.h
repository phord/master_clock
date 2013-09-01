extern int a , b ;

extern int tick ;
void ticker() ;

// User I/O
void sendString( const char * str ) ;
char readKey();

// Raise/lower digital IO pins
void sendSignal( int a, int b) ;

// Run the clock loop
void service() ;
