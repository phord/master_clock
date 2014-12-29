/** Trigger a one-shot NTP time-sync */
void triggerNtp() ;

/** Initialize the NTP sync daemon */
void NtpSetup() ;

/** Service the NTP daemon.  Must be called frequently. */
void NtpService() ;
