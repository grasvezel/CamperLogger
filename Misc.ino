/********************************************************************************************\
  Time stuff
  \*********************************************************************************************/
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000  (946684800UL) // the time at the start of y2k
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

struct  timeStruct {
  uint8_t Second;
  uint8_t Minute;
  uint8_t Hour;
  uint8_t Wday;   // day of week, sunday is day 1
  uint8_t Day;
  uint8_t Month;
  uint8_t Year;   // offset from 1970;
} tm;

uint32_t prevMillis = 0;
uint32_t sysTime = 0;
uint32_t nextSyncTime = 0;
uint32_t syncInterval = 3600;  // time sync will be attempted after this many seconds
uint32_t errSyncInterval = 30;

void addLog(byte level, String line) {
  if(level > logLevel) {
    return;
  }
  if(sysTime > 1000) {
    line = formattedDate() + " " + formattedTime() + " CPU" + String(xPortGetCoreID()) + " " + line + "\n";
  } else {
    line = "00-00-0000 00:00:00 CPU" + String(xPortGetCoreID()) + " " + line + "\n";
  }
  Serial.print(line);
}

void addLogNoTime(byte level, String line) {
  if(level > logLevel) {
    return;
  }
  line = "                    CPU" + String(xPortGetCoreID()) + " " + line + "\n";
  Serial.print(line);
}

void formatIP(const IPAddress& ip, char (&strIP)[20]) {
  sprintf_P(strIP, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
}
String formatIP(const IPAddress& ip) {
  char strIP[20];
  formatIP(ip, strIP);
  return String(strIP);
}

/********************************************************************************************\
  Status LED
\*********************************************************************************************/
#define PWMRANGE 1024
#define STATUS_PWM_NORMALVALUE (PWMRANGE>>2)
#define STATUS_PWM_NORMALFADE (PWMRANGE>>8)
#define STATUS_PWM_TRAFFICRISE (PWMRANGE>>1)
void statusLED(boolean traffic) {
  static int gnStatusValueCurrent = -1;
  static long int gnLastUpdate = millis();

  if (PIN_STATUS_LED == -1)
    return;

  int nStatusValue = gnStatusValueCurrent;

  if (traffic)
  {
    nStatusValue += STATUS_PWM_TRAFFICRISE; //ramp up fast
  }
  else
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      long int delta = timePassedSince(gnLastUpdate);
      if (delta>0 || delta<0 )
      {
        nStatusValue -= STATUS_PWM_NORMALFADE; //ramp down slowly
        nStatusValue = std::max(nStatusValue, STATUS_PWM_NORMALVALUE);
        gnLastUpdate=millis();
      }
    }
    //AP mode is active
    else if (WifiIsAP())
    {
      nStatusValue = ((millis()>>1) & PWMRANGE) - (PWMRANGE>>2); //ramp up for 2 sec, 3/4 luminosity
    }
    //Disconnected
    else
    {
      nStatusValue = (millis()>>1) & (PWMRANGE>>2); //ramp up for 1/2 sec, 1/4 luminosity
    }
  }

  nStatusValue = constrain(nStatusValue, 0, PWMRANGE);

  if (gnStatusValueCurrent != nStatusValue)
  {
    gnStatusValueCurrent = nStatusValue;

    long pwm = nStatusValue * nStatusValue; //simple gamma correction
    pwm >>= 10;
    analogWriteESP32(PIN_STATUS_LED, pwm);
  }
}

/********************************************************************************************\
  Unsigned long Timer timeOut check
\*********************************************************************************************/

// Return the time difference as a signed value, taking into account the timers may overflow.
// Returned timediff is between -24.9 days and +24.9 days.
// Returned value is positive when "next" is after "prev"
long timeDiff(unsigned long prev, unsigned long next)
{
  long signed_diff = 0;
  // To cast a value to a signed long, the difference may not exceed half the ULONG_MAX
  const unsigned long half_max_unsigned_long = 2147483647u; // = 2^31 -1
  if (next >= prev) {
    const unsigned long diff = next - prev;
    if (diff <= half_max_unsigned_long) {
      // Normal situation, just return the difference.
      // Difference is a positive value.
      signed_diff = static_cast<long>(diff);
    } else {
      // prev has overflow, return a negative difference value
      signed_diff = static_cast<long>((ULONG_MAX - next) + prev + 1u);
      signed_diff = -1 * signed_diff;
    }
  } else {
    // next < prev
    const unsigned long diff = prev - next;
    if (diff <= half_max_unsigned_long) {
      // Normal situation, return a negative difference value
      signed_diff = static_cast<long>(diff);
      signed_diff = -1 * signed_diff;
    } else {
      // next has overflow, return a positive difference value
      signed_diff = static_cast<long>((ULONG_MAX - prev) + next + 1u);
    }
  }
  return signed_diff;
}

void analogWriteESP32(int pin, int value)
{
  // find existing channel if this pin has been used before
  int8_t ledChannel = -1;
  for(byte x = 0; x < 16; x++)
    if (ledChannelPin[x] == pin)
      ledChannel = x;

  if(ledChannel == -1) // no channel set for this pin
    {
      for(byte x = 0; x < 16; x++) // find free channel
        if (ledChannelPin[x] == -1)
          {
            int freq = 5000;
            ledChannelPin[x] = pin;  // store pin nr
            ledcSetup(x, freq, 10);  // setup channel
            ledcAttachPin(pin, x);   // attach to this pin
            ledChannel = x;
            break;
          }
    }
  ledcWrite(ledChannel, value);
}

unsigned long getNtpTime()
{
  if (WiFi.status() != WL_CONNECTED) {
    return 0;
  }
  addLogNoTime(LOG_LEVEL_DEBUG_MORE, "NTP  : Opening UDP to send NTP request");
  WiFiUDP udp;
  udp.begin(123);
  for (byte x = 1; x < 4; x++)
  {
    String log = F("NTP  : NTP sync request: ");
    log += x;
    addLogNoTime(LOG_LEVEL_DEBUG_MORE, log);

    const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
    byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
    
    IPAddress timeServerIP;
    WiFi.hostByName(NTP_SERVER, timeServerIP);

    log = F("NTP  : NTP send to ");
    log += formatIP(timeServerIP);
    addLogNoTime(LOG_LEVEL_DEBUG_MORE, log);

    while (udp.parsePacket() > 0) ; // discard any previously received packets

    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    udp.beginPacket(timeServerIP, 123); //NTP requests are to port 123
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();

    uint32_t beginWait = millis();
    while (!timeOutReached(beginWait + 1500)) {
      int size = udp.parsePacket();
      if (size >= NTP_PACKET_SIZE) {
        udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
        unsigned long secsSince1900;
        // convert four bytes starting at location 40 to a long integer
        secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
        secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
        secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
        secsSince1900 |= (unsigned long)packetBuffer[43];
        log = F("NTP  : NTP replied in ");
        log += timePassedSince(beginWait);
        log += F(" ms");
        addLogNoTime(LOG_LEVEL_DEBUG_MORE, log);
        udp.stop();
        return secsSince1900 - 2208988800UL + TIME_ZONE * SECS_PER_MIN;
      }
    }
    log = F("NTP  : No reply");
    delay(1000);
    addLogNoTime(LOG_LEVEL_DEBUG_MORE, log);
  }
  udp.stop();
  return 0;
}

// Check if a certain timeout has been reached.
boolean timeOutReached(unsigned long timer) {
  const long passed = timePassedSince(timer);
  return passed >= 0;
}

long timePassedSince(unsigned long timestamp) {
  return timeDiff(timestamp, millis());
}

unsigned long now() {
  // calculate number of seconds passed since last call to now()
  const long msec_passed = timePassedSince(prevMillis);
  const long seconds_passed = msec_passed / 1000;
  sysTime += seconds_passed;
  prevMillis += seconds_passed * 1000;
  if (nextSyncTime <= sysTime) {
    // nextSyncTime & sysTime are in seconds
    addLogNoTime(LOG_LEVEL_DEBUG_MORE, "NTP  : Calling getNtpTime()");
    unsigned long  t = getNtpTime();
    addLogNoTime(LOG_LEVEL_DEBUG, "NTP  : Time received: " + String(t));
    if (t != 0) {
      setTime(t + Settings.DST * SECS_PER_HOUR);
    } else {
      nextSyncTime = sysTime + errSyncInterval;
    }
  }
  breakTime(sysTime, tm);
  return (unsigned long)sysTime;
}

void initTime() {
  nextSyncTime = 0;
  now();
}

void setTime(unsigned long t) {
  sysTime = (uint32_t)t;
  nextSyncTime = (uint32_t)t + syncInterval;
  prevMillis = millis();  // restart counting from now (thanks to Korman for this fix)
}

void breakTime(unsigned long timeInput, struct timeStruct &tm) {
  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;
  const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  time = (uint32_t)timeInput;
  tm.Second = time % 60;
  time /= 60; // now it is minutes
  tm.Minute = time % 60;
  time /= 60; // now it is hours
  tm.Hour = time % 24;
  time /= 24; // now it is days
  tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1

  year = 0;
  days = 0;
  while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm.Year = year; // year is offset from 1970

  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0

  days = 0;
  month = 0;
  monthLength = 0;
  for (month = 0; month < 12; month++) {
    if (month == 1) { // february
      if (LEAP_YEAR(year)) {
        monthLength = 29;
      } else {
        monthLength = 28;
      }
    } else {
      monthLength = monthDays[month];
    }

    if (time >= monthLength) {
      time -= monthLength;
    } else {
      break;
    }
  }
  tm.Month = month + 1;  // jan is month 1
  tm.Day = time + 1;     // day of month
}

int weekday()
{  return tm.Wday;
}

String formattedTime() {
  now();
  String timeString;
  String tm_hour;
  String tm_minute;
  String tm_second;
  if(tm.Hour < 10)
    timeString = "0";
  timeString += String(tm.Hour) + ":";
  if(tm.Minute < 10)
    timeString += "0";
  timeString += String(tm.Minute) + ":";
  if(tm.Second < 10)
    timeString += "0";
  timeString += String(tm.Second);
  return timeString;
}

String formattedDate() {
  now();
  int day = tm.Day;
  int month = tm.Month;
  int year = tm.Year;
  String timeString = "";
  if(day < 10) {
    timeString += "0";
  }
  timeString += String(day) + "-";
  if(month < 10) {
     timeString += "0";
  }
  timeString += String(month) + "-";
  timeString += String(1970 + year);
  return timeString;
}

/********************************************************************************************\
* Save a byte to RTC memory
\*********************************************************************************************/
#define RTC_BASE 65 // system doc says user area starts at 64, but it does not work (?)
void saveToRTC(byte Par1)
{
  byte buf[3] = {0xAA, 0x55, 0};
  buf[2] = Par1;
//  system_rtc_mem_write(RTC_BASE, buf, 3);
}


/********************************************************************************************\
* Read a byte from RTC memory
\*********************************************************************************************/
boolean readFromRTC(byte* data)
{
  byte buf[3] = {0, 0, 0};
//  system_rtc_mem_read(RTC_BASE, buf, 3);
  if (buf[0] == 0xAA && buf[1] == 0x55)
  {
    *data = buf[2];
    return true;
  }
  return false;
}
