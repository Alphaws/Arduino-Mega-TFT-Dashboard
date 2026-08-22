#include <Arduino.h>

// Dynamic Weather Forecast Parser & Rolling Dynamic Clock Hours for Mega ADK
#define LCD_RS 38
#define LCD_WR 39
#define LCD_CS 40
#define LCD_RST 41

static const uint8_t font5x7[96][5] PROGMEM = {
  {0x00,0x00,0x00,0x00,0x00}, {0x00,0x00,0x5F,0x00,0x00}, {0x00,0x07,0x00,0x07,0x00}, {0x14,0x7F,0x14,0x7F,0x14},
  {0x24,0x2A,0x7F,0x2A,0x12}, {0x23,0x13,0x08,0x64,0x62}, {0x36,0x49,0x55,0x22,0x50}, {0x00,0x05,0x03,0x00,0x00},
  {0x00,0x1C,0x22,0x41,0x00}, {0x00,0x41,0x22,0x1C,0x00}, {0x14,0x08,0x3E,0x08,0x14}, {0x08,0x08,0x3E,0x08,0x08},
  {0x00,0x50,0x30,0x00,0x00}, {0x08,0x08,0x08,0x08,0x08}, {0x00,0x60,0x60,0x00,0x00}, {0x20,0x10,0x08,0x04,0x02},
  {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00}, {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31},
  {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39}, {0x3C,0x4A,0x4A,0x4A,0x30}, {0x01,0x71,0x09,0x05,0x03},
  {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x49,0x3E}, {0x00,0x36,0x36,0x00,0x00}, {0x00,0x56,0x36,0x00,0x00},
  {0x08,0x14,0x22,0x41,0x00}, {0x14,0x14,0x14,0x14,0x14}, {0x00,0x41,0x22,0x14,0x08}, {0x02,0x01,0x51,0x09,0x06},
  {0x3E,0x41,0x5D,0x55,0x1E}, {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36}, {0x3E,0x41,0x41,0x41,0x22},
  {0x7F,0x41,0x41,0x22,0x1C}, {0x7F,0x49,0x49,0x49,0x41}, {0x7F,0x09,0x09,0x09,0x01}, {0x3E,0x41,0x49,0x49,0x7A},
  {0x7F,0x08,0x08,0x08,0x7F}, {0x00,0x41,0x7F,0x41,0x00}, {0x20,0x40,0x41,0x3F,0x01}, {0x7F,0x08,0x14,0x22,0x41},
  {0x7F,0x40,0x40,0x40,0x40}, {0x7F,0x02,0x0C,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F}, {0x3E,0x41,0x41,0x41,0x3E},
  {0x7F,0x09,0x09,0x09,0x06}, {0x3E,0x41,0x51,0x21,0x5E}, {0x7F,0x09,0x19,0x29,0x46}, {0x46,0x49,0x49,0x49,0x31},
  {0x01,0x01,0x7F,0x01,0x01}, {0x3F,0x40,0x40,0x40,0x3F}, {0x1F,0x20,0x40,0x20,0x1F}, {0x3F,0x40,0x38,0x40,0x3F},
  {0x63,0x14,0x08,0x14,0x63}, {0x07,0x08,0x70,0x08,0x07}, {0x61,0x51,0x49,0x45,0x43}
};

void LCD_Write_Bus(unsigned char high, unsigned char low) {
  PORTA = high; PORTC = low;
  digitalWrite(LCD_WR, LOW); digitalWrite(LCD_WR, HIGH);
}

void LCD_Write_COM(unsigned char cmd) {
  digitalWrite(LCD_RS, LOW); LCD_Write_Bus(0x00, cmd);
}

void LCD_Write_DATA(unsigned char data) {
  digitalWrite(LCD_RS, HIGH); LCD_Write_Bus(0x00, data);
}

void LCD_Set_Window(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_Write_COM(0x2A);
  LCD_Write_DATA(x1 >> 8); LCD_Write_DATA(x1 & 0xFF);
  LCD_Write_DATA(x2 >> 8); LCD_Write_DATA(x2 & 0xFF);
  LCD_Write_COM(0x2B);
  LCD_Write_DATA(y1 >> 8); LCD_Write_DATA(y1 & 0xFF);
  LCD_Write_DATA(y2 >> 8); LCD_Write_DATA(y2 & 0xFF);
  LCD_Write_COM(0x2C);
}

void LCD_Fill_Rect(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color) {
  LCD_Set_Window(x1, y1, x2, y2);
  uint32_t total = (uint32_t)(x2 - x1 + 1) * (y2 - y1 + 1);
  for (uint32_t i = 0; i < total; i++) {
    digitalWrite(LCD_RS, HIGH);
    PORTA = color >> 8; PORTC = color & 0xFF;
    digitalWrite(LCD_WR, LOW); digitalWrite(LCD_WR, HIGH);
  }
}

void LCD_Draw_Pixel(unsigned int x, unsigned int y, unsigned int color) {
  LCD_Set_Window(x, y, x, y);
  digitalWrite(LCD_RS, HIGH);
  PORTA = color >> 8; PORTC = color & 0xFF;
  digitalWrite(LCD_WR, LOW); digitalWrite(LCD_WR, HIGH);
}

void LCD_Draw_Char(unsigned int x, unsigned int y, char c, unsigned int color, unsigned int bg, uint8_t size) {
  if (c < 32 || c > 126) c = '?';
  uint8_t idx = c - 32;
  for (int8_t i = 0; i < 5; i++) {
    uint8_t line = pgm_read_byte(&font5x7[idx][i]);
    for (int8_t j = 0; j < 8; j++) {
      if (line & 0x01) {
        if (size == 1) LCD_Draw_Pixel(x + i, y + j, color);
        else LCD_Fill_Rect(x + i * size, y + j * size, x + (i + 1) * size - 1, y + (j + 1) * size - 1, color);
      }
      line >>= 1;
    }
  }
}

void LCD_Draw_String(unsigned int x, unsigned int y, const char *str, unsigned int color, unsigned int bg, uint8_t size) {
  while (*str) {
    LCD_Draw_Char(x, y, *str, color, bg, size);
    x += 6 * size;
    str++;
  }
}

void LCD_Init_Full() {
  digitalWrite(LCD_CS, LOW);
  digitalWrite(LCD_RST, HIGH); delay(10);
  digitalWrite(LCD_RST, LOW); delay(20);
  digitalWrite(LCD_RST, HIGH); delay(120);
  LCD_Write_COM(0x01); delay(120);
  LCD_Write_COM(0x36); LCD_Write_DATA(0x28);
  LCD_Write_COM(0x3A); LCD_Write_DATA(0x55);
  LCD_Write_COM(0x11); delay(120);
  LCD_Write_COM(0x29); delay(20);
}

void Log_To_TFT_Footer(const char *msg, unsigned int color = 0xFFE0) {
  LCD_Fill_Rect(0, 195, 319, 239, 0x0015);
  LCD_Draw_String(10, 210, msg, color, 0x0015, 1);
}

int hours = 0;
int minutes = 0;
int seconds = 0;
int year = 2026;
int month = 1;
int day = 1;

bool isSynced = false;

char liveTemp[12] = "19 C";
char liveDesc[15] = "ZAPOR";
char liveHum[12] = "64%";

void Draw_Forecast_Cards() {
  // Card 1: MOST (Current)
  LCD_Fill_Rect(10, 90, 80, 185, 0x01E0);
  LCD_Draw_String(22, 98, "MOST", 0xFFFF, 0x01E0, 1);
  LCD_Draw_String(20, 115, liveTemp, 0xFFE0, 0x01E0, 2);
  LCD_Draw_String(18, 145, liveDesc, 0xFFFF, 0x01E0, 1);
  LCD_Draw_String(22, 165, liveHum, 0x07FF, 0x01E0, 1);

  // Dynamic Hours calculation based on current hours (+3h, +6h, +9h)
  char h1Buf[8], h2Buf[8], h3Buf[8];
  snprintf(h1Buf, sizeof(h1Buf), "%02d:00", (hours + 3) % 24);
  snprintf(h2Buf, sizeof(h2Buf), "%02d:00", (hours + 6) % 24);
  snprintf(h3Buf, sizeof(h3Buf), "%02d:00", (hours + 9) % 24);

  // Card 2: +3 Hours
  LCD_Fill_Rect(86, 90, 156, 185, 0x02E0);
  LCD_Draw_String(98, 98, h1Buf, 0xFFFF, 0x02E0, 1);
  LCD_Draw_String(96, 115, "21 C", 0xFFE0, 0x02E0, 2);
  LCD_Draw_String(92, 145, "FELHOS", 0xFFFF, 0x02E0, 1);
  LCD_Draw_String(98, 165, "75%", 0x07FF, 0x02E0, 1);

  // Card 3: +6 Hours
  LCD_Fill_Rect(162, 90, 232, 185, 0x03E0);
  LCD_Draw_String(174, 98, h2Buf, 0xFFFF, 0x03E0, 1);
  LCD_Draw_String(172, 115, "24 C", 0xFFE0, 0x03E0, 2);
  LCD_Draw_String(170, 145, "NAPOS", 0xFFFF, 0x03E0, 1);
  LCD_Draw_String(174, 165, "60%", 0x07FF, 0x03E0, 1);

  // Card 4: +9 Hours
  LCD_Fill_Rect(238, 90, 309, 185, 0x02E0);
  LCD_Draw_String(250, 98, h3Buf, 0xFFFF, 0x02E0, 1);
  LCD_Draw_String(248, 115, "27 C", 0xFFE0, 0x02E0, 2);
  LCD_Draw_String(244, 145, "MELEG", 0xFFFF, 0x02E0, 1);
  LCD_Draw_String(250, 165, "52%", 0x07FF, 0x02E0, 1);
}

void Draw_Forecast_UI() {
  LCD_Fill_Rect(0, 0, 319, 239, 0x000F);

  // Header Box
  LCD_Fill_Rect(0, 0, 319, 35, 0x001A);
  LCD_Draw_String(15, 8, "BUDAPEST IDOJARAS (LIVE)", 0xFFFF, 0x001A, 2);

  // Clock Bar
  LCD_Fill_Rect(10, 42, 309, 82, 0x1800);

  Draw_Forecast_Cards();

  Log_To_TFT_Footer("VARAKOZAS SERIAL1 WEMOS ADATRA...", 0xFFE0);
}

void Parse_Sync_Command(String input) {
  if (input.indexOf("SYNC:") != -1) {
    int idx = input.indexOf("SYNC:");
    hours = input.substring(idx + 5, idx + 7).toInt();
    minutes = input.substring(idx + 8, idx + 10).toInt();
    seconds = input.substring(idx + 11, idx + 13).toInt();
    year = input.substring(idx + 14, idx + 18).toInt();
    month = input.substring(idx + 19, idx + 21).toInt();
    day = input.substring(idx + 22, idx + 24).toInt();
    isSynced = true;

    char buf[50];
    snprintf(buf, sizeof(buf), "UTOLSO FRISSITES: %02d:%02d:%02d (WIFI)", hours, minutes, seconds);
    Log_To_TFT_Footer(buf, 0x07E0);
    Draw_Forecast_Cards();
  }

  if (input.indexOf("WEATHER:") != -1) {
    int idx = input.indexOf("WEATHER:");
    String data = input.substring(idx + 8);
    int p1 = data.indexOf(',');
    int p2 = data.indexOf(',', p1 + 1);

    if (p1 != -1 && p2 != -1) {
      String t = data.substring(0, p1);
      String d = data.substring(p1 + 1, p2);
      String h = data.substring(p2 + 1);

      t.toCharArray(liveTemp, sizeof(liveTemp));
      d.toCharArray(liveDesc, sizeof(liveDesc));
      h.toCharArray(liveHum, sizeof(liveHum));

      Draw_Forecast_Cards();
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  DDRA = 0xFF; DDRC = 0xFF;
  pinMode(LCD_RS, OUTPUT); pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_CS, OUTPUT); pinMode(LCD_RST, OUTPUT);

  LCD_Init_Full();
  Draw_Forecast_UI();
}

void loop() {
  static uint32_t last_tick = 0;

  if (Serial1.available()) {
    String msg = Serial1.readStringUntil('\n');
    Parse_Sync_Command(msg);
  }

  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    Parse_Sync_Command(msg);
  }

  if (millis() - last_tick >= 1000) {
    last_tick = millis();

    seconds++;
    if (seconds >= 60) {
      seconds = 0; minutes++;
      if (minutes >= 60) {
        minutes = 0; hours++;
        if (hours >= 24) { hours = 0; day++; }
        Draw_Forecast_Cards(); // Refresh rolling forecast card hours hourly
      }
    }

    char timeBuffer[25];
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d  %04d.%02d.%02d", hours, minutes, seconds, year, month, day);

    // Refresh Clock Bar
    LCD_Fill_Rect(15, 50, 304, 75, 0x1800);
    LCD_Draw_String(35, 55, timeBuffer, 0xFFFF, 0x1800, 2);
  }
}
