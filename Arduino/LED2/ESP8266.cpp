
#include "ESP8266.h"

#define LOG_OUTPUT_DEBUG            (1)
#define LOG_OUTPUT_DEBUG_PREFIX     (1)

#define logDebug(arg)\
  do {\
    if (LOG_OUTPUT_DEBUG)\
    {\
      if (LOG_OUTPUT_DEBUG_PREFIX)\
      {\
        Serial.print("[LOG Debug: ");\
        Serial.print((const char*)__FILE__);\
        Serial.print(",");\
        Serial.print((unsigned int)__LINE__);\
        Serial.print(",");\
        Serial.print((const char*)__FUNCTION__);\
        Serial.print("] ");\
      }\
      Serial.print(arg);\
    }\
  } while(0)

#ifdef ESP8266_USE_SOFTWARE_SERIAL
ESP8266::ESP8266(SoftwareSerial &uart, uint32_t baud): m_puart(&uart)
{

}
#else
ESP8266::ESP8266(HardwareSerial &uart, uint32_t baud): m_puart(&uart)
{
  m_puart->begin(baud);
  rx_empty();
}
#endif

bool ESP8266::autoSetBaud(uint32_t baudRateSet)
{
  rx_empty();
  long time0 = millis();
  long baudRateArray[] = {9600, 19200, 57600, 115200}; //These are the optional default baudrates
  const int attempts = 5;
  bool baudFlag = 0;

#ifndef ESP8266_USE_SOFTWARE_SERIAL
  baudRateSet = 115200;                         //for hardware serial set to highest baudrate
#endif

  for (int j = 0 ; j < attempts ; j++) {                    //attempt to connect to esp over each baudrate
    for (int i = 0; i < sizeof(baudRateArray) ; i++)        //check for current esp baudrate
    {
      m_puart->begin(baudRateArray[i]);

      m_puart->println("AT");
      delay(20);
      while (m_puart->available()) {
        String inData = m_puart->readStringUntil('\n');
        if (inData.indexOf("OK") != -1) {       //if OK received, this is the current baudrate of the ESP
          baudFlag = 1;
          delay(15);
          break;
        }

      }
      if (baudFlag)
        break;
    }
    // ESP current BaudRate was found, now try to set it to 9600
    if (baudFlag) {
      baudFlag = 0;
      for (int j = 0; j < attempts; j++)               //at the found baudrate,
      {
        m_puart->print("AT+CIOBAUD=");
        m_puart->println(baudRateSet);
        delay(20);
        while (m_puart->available()) {
          String inData = m_puart->readStringUntil('\n');
          if (inData.indexOf("OK") != -1 || inData.indexOf("AT") != -1) {
            baudFlag = 1;
            m_puart->begin(baudRateSet);
            delay(100);
            return 1;
          }

        }
        if (baudFlag)
          break;
      }
    }

    if (baudFlag)
      break;
  }
  return 0;

}

//when using software serial BaudRate should be lower than 115200. 9600 works reliably
bool ESP8266::init(const String &ssid, const String &pwd, uint32_t baudRateSet)
{
  if (autoSetBaud(baudRateSet))
  {
    Serial.println("Baudrate set success");
  }
  else
  {
    Serial.println("Baudrate set failed");
    return false;
  }

  //Setting operation mode to Station + SoftAP
  if (setOprToStationSoftAP())
  {
    Serial.println("Station + softAP - OK");
  }
  else
  {
    Serial.println("Station + softAP - Error, Reset Board!");
    return false;
  }

  if (joinAP(ssid, pwd))
  {
    Serial.print("Joining AP successful, ");
    Serial.println(getLocalIP().c_str());
  }
  else
  {
    Serial.println("Join AP failure, Reset Board!");
    return false;
  }

  if (disableMUX())
  {
    Serial.println("Single Mode OK");
  }
  else
  {
    Serial.println("Single Mode Error, Reset Board!");
    return false;
  }
  return true;
}



bool ESP8266::restart(void)
{
  unsigned long start;
  if (eATRST()) {
    delay(2000);
    start = millis();
    while (millis() - start < 3000) {
      if (eAT()) {
        delay(1500); /* Waiting for stable */
        return true;
      }
      delay(100);
    }
  }
  return false;
}


bool ESP8266::setOprToStationSoftAP(void)
{
  uint8_t mode;
  if (!qATCWMODE(&mode)) {
    return false;
  }
  if (mode == 3) {
    return true;
  } else {
    if (sATCWMODE(3) && restart()) {
      return true;
    } else {
      return false;
    }
  }
}

bool ESP8266::joinAP(String ssid, String pwd)
{
  return sATCWJAP(ssid, pwd);
}


String ESP8266::getLocalIP(void)
{

  String inData;

  rx_empty();
  m_puart->println("AT+CIFSR");
  delay(50);
  while (m_puart->available() > 0) {
    inData = m_puart->readStringUntil('\n');
    if (inData.indexOf("IP") != -1) {
      delay(100);
      return ("IP: " + inData.substring( inData.indexOf("IP") + 4, inData.length() - 2 ));
    }
  }
  return "Couldn't get IP adress";

}


bool ESP8266::disableMUX(void)
{
  rx_empty();
#ifdef ESP8266_USE_SOFTWARE_SERIAL
  String inData;
  m_puart->println("AT+CIPMUX=0");
  delay(50);
  while (m_puart->available() > 0) {
    inData = m_puart->readStringUntil('\n');
    if (inData.indexOf("OK") != -1) {
      delay(100);
      return true;
    }
  }
  return false;
#else
  return sATCIPMUX(0);
#endif

}

bool ESP8266::createTCP(String addr, uint32_t port)
{
  return sATCIPSTARTSingle("TCP", addr, port);
}


/*----------------------------------------------------------------------------*/
/* +IPD,<id>,<len>:<data> */
/* +IPD,<len>:<data> */


void ESP8266::rx_empty(void)
{
  while (m_puart->available() > 0) {
    m_puart->read();
  }
}

String ESP8266::recvString(String target, uint32_t timeout)
{
  String data;
  char a;
  unsigned long start = millis();
  while (millis() - start < timeout) {
    while (m_puart->available() > 0) {
      a = m_puart->read();
      if (a == '\0') continue;
      data += a;
    }
    if (data.indexOf(target) != -1) {
      break;
    }
  }
  return data;
}

String ESP8266::recvString(String target1, String target2, uint32_t timeout)
{
  String data;
  char a;
  unsigned long start = millis();
  while (millis() - start < timeout) {
    while (m_puart->available() > 0) {
      a = m_puart->read();
      if (a == '\0') continue;
      data += a;
    }
    if (data.indexOf(target1) != -1) {
      break;
    } else if (data.indexOf(target2) != -1) {
      break;
    }
  }
  return data;
}

String ESP8266::recvString(String target1, String target2, String target3, uint32_t timeout)
{
  String data;
  char a;
  unsigned long start = millis();
  while (millis() - start < timeout) {
    while (m_puart->available() > 0) {
      a = m_puart->read();
      if (a == '\0') continue;
      data += a;
    }
    if (data.indexOf(target1) != -1) {
      break;
    } else if (data.indexOf(target2) != -1) {
      break;
    } else if (data.indexOf(target3) != -1) {
      break;
    }
  }
  return data;
}

bool ESP8266::recvFind(String target, uint32_t timeout)
{
  String data_tmp;
  data_tmp = recvString(target, timeout);
  if (data_tmp.indexOf(target) != -1) {
    return true;
  }
  return false;
}

bool ESP8266::recvFindAndFilter(String target, String begin, String end, String & data, uint32_t timeout)
{
  String data_tmp;
  data_tmp = recvString(target, timeout);
  if (data_tmp.indexOf(target) != -1) {
    int32_t index1 = data_tmp.indexOf(begin);
    int32_t index2 = data_tmp.indexOf(end);
    if (index1 != -1 && index2 != -1) {
      index1 += begin.length();
      data = data_tmp.substring(index1, index2);
      return true;
    }
  }
  data = "";
  return false;
}

bool ESP8266::eAT(void)
{
  rx_empty();
  m_puart->println("AT");
  return recvFind("OK");
}

bool ESP8266::eATRST(void)
{
  rx_empty();
  m_puart->println("AT+RST");
  return recvFind("OK");
}



bool ESP8266::qATCWMODE(uint8_t *mode)
{
  String str_mode;
  bool ret;
  if (!mode) {
    return false;
  }
  rx_empty();
  m_puart->println("AT+CWMODE?");
  ret = recvFindAndFilter("OK", "+CWMODE:", "\r\n\r\nOK", str_mode);
  if (ret) {
    *mode = (uint8_t)str_mode.toInt();
    return true;
  } else {
    return false;
  }
}

bool ESP8266::sATCWMODE(uint8_t mode)
{
  String data;
  rx_empty();
  m_puart->print("AT+CWMODE=");
  m_puart->println(mode);

  data = recvString("OK", "no change");
  if (data.indexOf("OK") != -1 || data.indexOf("no change") != -1) {
    return true;
  }
  return false;
}

bool ESP8266::sATCWJAP(String ssid, String pwd)
{
  String data;
  rx_empty();
  m_puart->print("AT+CWJAP=\"");
  m_puart->print(ssid);
  m_puart->print("\",\"");
  m_puart->print(pwd);
  m_puart->println("\"");

  data = recvString("OK", "FAIL", 10000);
  if (data.indexOf("OK") != -1) {
    return true;
  }
  return false;
}


bool ESP8266::sATCIPSTARTSingle(String type, String addr, uint32_t port)
{
  String data;
  rx_empty();
  m_puart->print("AT+CIPSTART=\"");
  m_puart->print(type);
  m_puart->print("\",\"");
  m_puart->print(addr);
  m_puart->print("\",");
  m_puart->println(port);

  data = recvString("OK", "ERROR", "ALREADY CONNECT", 500);
  if (data.indexOf("OK") != -1 || data.indexOf("ALREADY CONNECT") != -1) {
    return true;
  }
  return false;
}



bool ESP8266::sATCIPMUX(uint8_t mode)
{
  String data;

  rx_empty();
  delay(100);
  m_puart->print("AT+CIPMUX=");
  m_puart->println(mode);

  data = recvString("OK", "Link is builded");
  if (data.indexOf("OK") != -1) {
    delay(100);
    return true;
  }
  return false;
}




bool ESP8266::sendSingle(const char* url)
{
  rx_empty();
  m_puart->print("AT+CIPSEND=");
  m_puart->println(strlen(url));
  if (recvFind(">", 500)) {
    rx_empty();
    m_puart->print(url);


    return recvFind("SEND OK", 500);
  }
  else
    return false;
}


int ESP8266::recvSingle(uint8_t *buffer, int bufferLen)
{
  int i = 0;
  int bodyFlag = 1;
#ifndef ESP8266_USE_SOFTWARE_SERIAL
String inData = "";
#endif

  unsigned long start = millis();
  while (millis() - start < 500) {
    while (m_puart->available() > 0 && i < bufferLen)
    {
      //when using software serial due to buffer issues read incoming string char by char
#ifdef ESP8266_USE_SOFTWARE_SERIAL
      char c = m_puart->read();
      buffer[i++] = c;
#else
      inData += m_puart->readStringUntil('\n');

#endif
    }

    if (i == bufferLen && m_puart->available()) {
      Serial.println(F("buffer is full!"));
      return i - 1;
    }
  }

  return i - 1;
}

int ESP8266::httpGet(char* request)
{
  Serial.println(request);

  if (createTCP("fcm.googleapis.com", 80))
  {
    Serial.println(F("create tcp - OK"));
  }
  else
  {
    Serial.println(F("create tcp - ERROR"));    
    return "";
  }

  if (!sendSingle(request))
  {
    Serial.print(F("not sent"));
    //return "";
  }
    else
  {
    Serial.print(F("sent hurray!"));
  }

  int len = recvSingle(m_responseBuffer, MAX_BUFFER_SIZE);
  Serial.println((char*)m_responseBuffer);

  return len;
}

















