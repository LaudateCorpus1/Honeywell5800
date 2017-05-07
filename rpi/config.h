#ifndef __CONFIG_H__
#define __CONFIG_H__

// Fully qualified filename for device state file
#define C_STATE_FILE "/tmp/deviceState.json"

// Number of seconds after which an event should be reported, even if
// the state hasn't changed. This is required to capture keyfob presses.
#define C_REPEAT_TIME 2

// Preferred strftime Date and Time string format
#define C_DATETIME_FORMAT "%F %T %z"

//  System command to execute when an event occurs. Null string to do nothing.
#define C_UPDATE_CMD ""

//#define C_UPDATE_CMD "cat /tmp/deviceState.json"

// Example for smartthings
//#define C_UPDATE_CMD "curl -H 'Authorization: Bearer 12345678-1234-1234-1234-1234567890ab' -H 'Content-Type: application/json' -X PUT -d '@/var/www/html/deviceState.json' https://graph.api.smartthings.com:443/api/smartapps/installations/12345678-1234-1234-1234-1234567890ab/event&"

#endif
