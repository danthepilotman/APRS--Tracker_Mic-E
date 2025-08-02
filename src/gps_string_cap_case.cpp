#include"gps_string_cap_case.h"


gps_NMEA my_gps( Serial, 9600 );



void gps_NMEA::get_GPS_Data() {
  char NMEA_data[NMEA_DATA_MAX_SIZE];
  bool GGA_good = false;
  bool GSA_good = false;
  bool RMC_good = false;

#ifndef DEBUG
  memset(&gps_data, 0, sizeof(gps_data));
#endif

  while (!GGA_good || !GSA_good || !RMC_good) {
    get_NMEA_Sentence(NMEA_data);

    if (strncmp(NMEA_data, "$GPGGA", 6) == 0 && !GGA_good) {
      GGA_good = parse_GGA(NMEA_data);
      continue;
    }

    if (strncmp(NMEA_data, "$GPGSA", 6) == 0 && !GSA_good) {
      GSA_good = parse_GSA(NMEA_data);
      continue;
    }

    if (strncmp(NMEA_data, "$GPRMC", 6) == 0 && !RMC_good) {
      RMC_good = parse_RMC(NMEA_data);

#ifdef USE_OLED
      if (!gps_data.fix) {
        oled.setCursor(0, 0);
        for (uint8_t i = 0; i < NMEA_DATA_MAX_SIZE && NMEA_data[i]; i++)
          oled.print(NMEA_data[i]);
        oled.clearToEOP();
      }
#endif
    }

#ifdef USE_WDT
    wdt_reset();
#endif
  }
}

void gps_NMEA::get_NMEA_Sentence(char* NMEA_data) {
  while (true) {
    digitalWrite(GPS_VALID_PIN, HIGH);

    while (!my_gps.gpsSerial.available());
    NMEA_data[0] = my_gps.gpsSerial.read();

    if (NMEA_data[0] != '$')
      continue;

    for (uint8_t i = 1; i < NMEA_DATA_MAX_SIZE; i++) {
      while (!my_gps.gpsSerial.available());
      NMEA_data[i] = my_gps.gpsSerial.read();
      if (NMEA_data[i] == '\n') {
        NMEA_data[i] = '\0';
        break;
      }
    }

    digitalWrite(GPS_VALID_PIN, LOW);

#ifdef DEBUG
    my_gps.gpsSerial.print("NMEA_data: ");
    my_gps.gpsSerial.println(NMEA_data);
#endif

    if (gps_NMEA::xsum_Check(NMEA_data)) {
#ifdef DEBUG
      my_gps.gpsSerial.println(F("Good checksum"));
#endif
      break;
    }
  }
}

bool gps_NMEA::parse_GGA(const char* NMEA_data) {
  const char* ptr = next_field(NMEA_data);
  ptr = next_field(ptr);
  ptr = next_field(ptr); ptr = next_field(ptr);
  ptr = next_field(ptr); ptr = next_field(ptr);

  if (!is_Empty(ptr)) {
   gps_data.fixquality = atoi(ptr);
   gps_data.fix =gps_data.fixquality > 0;
  }

  if (!gps_data.fix) return false;

  ptr = next_field(ptr);
  if (!is_Empty(ptr))gps_data.satellites = atoi(ptr);

  ptr = next_field(ptr);
  ptr = next_field(ptr);

  if (!is_Empty(ptr))gps_data.altitude = int16_t(round(atof(ptr)));

  return true;
}

bool gps_NMEA::parse_RMC(const char* NMEA_data) {
  const char* ptr = next_field(NMEA_data);
  parse_Time(ptr);

  ptr = next_field(ptr);
  parse_Fix(ptr);
  if (!gps_data.fix) return false;

  ptr = next_field(ptr);
  parse_Coord(ptr);

  ptr = next_field(ptr);
  gps_data.NorS = *ptr;

  ptr = next_field(ptr);
  parse_Coord(ptr);

  ptr = next_field(ptr);
  gps_data.EorW = *ptr;

  ptr = next_field(ptr);
  if (!is_Empty(ptr))my_gps.gps_data.speed = (unsigned int)(round(atof(ptr)));

  ptr = next_field(ptr);
  if (!is_Empty(ptr))gps_data.course = (unsigned int)(round(atof(ptr)));

  ptr = next_field(ptr);
  if (!is_Empty(ptr)) {
  unsigned long fulldate = atol(ptr);
   my_gps.gps_data.day = fulldate / 10000;
   my_gps.gps_data.month = (fulldate % 10000) / 100;
   my_gps.gps_data.year = fulldate % 100;
  }

  return true;
}

bool gps_NMEA::parse_GSA(const char* NMEA_data) {
  const char* ptr = next_field(NMEA_data);
  ptr = next_field(ptr);

  if (!is_Empty(ptr)) {
   gps_data.fixquality_3d = atoi(ptr);
    return true;
  }

  return false;
}

char* gps_NMEA::next_field(const char* ptr) {
  char* next = strchr(ptr, ',');
  return (next) ? (next + 1) : NULL;
}

bool gps_NMEA::is_Empty(const char* pStart) {
  return (pStart == NULL || *pStart == ',' || *pStart == '*');
}

bool gps_NMEA::parse_Time(const char* ptr) {
  if (!is_Empty(ptr)) {
    unsigned long time = atol(ptr);
   gps_data.minute = (time % 10000) / 100;
   gps_data.seconds = (time % 100);
    return true;
  }
  return false;
}

bool gps_NMEA::parse_Fix(const char* ptr) {
  if (!is_Empty(ptr)) {
    if (ptr[0] == 'A') 
      gps_data.fix = true;
    else if (ptr[0] == 'V')
      gps_data.fix = false;
    
    else return false;
    
    return true;
  }
  
  return false;
}

bool gps_NMEA::parse_Coord(const char* coord) {
  if (!coord || *coord == '\0') return false;

  const char* e = strchr(coord, '.');
  if (!e) return false;

  ptrdiff_t int_len = e - coord;
  if (int_len != 4 && int_len != 5) return false;

  char digits[9] = {0};
  if (int_len == 5)
    memcpy(digits, e - 5, 5);
  else
    memcpy(digits + 1, e - 4, 4);
  memcpy(digits + 5, e + 1, 4);

  for (int i = (int_len == 5 ? 0 : 1); i < 8; ++i) {
    if (!isdigit(digits[i])) return false;
    digits[i] -= '0';
  }

  if (int_len == 5) {
   my_gps.gps_data.lon_DD = 100 * digits[0] + 10 * digits[1] + digits[2];
   my_gps.gps_data.lon_MM = 10 * digits[3] + digits[4];
   my_gps.gps_data.lon_hh = 10 * digits[5] + digits[6];
   my_gps.gps_data.lon_mm = 10 * digits[7] + digits[8];
  } else {
   my_gps.gps_data.lat_DD = 10 * digits[1] + digits[2];
   my_gps.gps_data.lat_MM = 10 * digits[3] + digits[4];
   my_gps.gps_data.lat_hh = 10 * digits[5] + digits[6];
   my_gps.gps_data.lat_mm = 10 * digits[7] + digits[8];
  }

  return true;
}

uint8_t gps_NMEA::hex_Str_To_Int(const char* chk_sum_in) {
  uint8_t integer;
  uint8_t delta;

  if (isalpha(chk_sum_in[0])) delta = 'A' - 0xA;
  else delta = '0';

  integer = (chk_sum_in[0] - delta) << 4;

  if (isalpha(chk_sum_in[1])) delta = 'A' - 0xA;
  else delta = '0';

  integer |= (chk_sum_in[1] - delta);
  return integer;
}

bool gps_NMEA::xsum_Check(const char* NMEA_data) {
  uint8_t computed_check_sum = 0;
  char* ptr = strchr(NMEA_data, '*');
  if (!ptr || !*(ptr + 1) || !*(ptr + 2)) return false;

  ptr++;
  uint8_t received_check_sum = hex_Str_To_Int(ptr);

  for (uint8_t i = 1; i < NMEA_DATA_MAX_SIZE && NMEA_data[i] != '*'; i++)
    computed_check_sum ^= NMEA_data[i];

  return computed_check_sum == received_check_sum;
}

