//#include <AdafruitIO_MQTT.h>
//#include <AdafruitIO_Ethernet.h>
//#include <AdafruitIO_FONA.h>
//#include <AdafruitIO_Definitions.h>
//#include <AdafruitIO_Time.h>
//#include <AdafruitIO_Group.h>
//#include <AdafruitIO_Data.h>
//#include <AdafruitIO_Dashboard.h>

#include <AdafruitIO.h>
#include <AdafruitIO_WiFi.h>
#include <AdafruitIO_Feed.h>
#include <Arduino.h>
#include <U8x8lib.h>
//#include "Free_Fonts.h" // Include the header file attached to this sketch
//#include "config.h" // Adafruit IO credentials

// Setup m5stick Pins
#define LedPin 19
// #define IrPin 17
#define BuzzerPin 26
#define BtnPin 35

// Setup Sensor Pins
#define PM25 13  // G13 to Pin 2 (PM2.5)
#define PM1 25  // G25 to Pin 4 (PM1.0)

// Setup Time
unsigned long starttime;
unsigned long sampletime_ms = 30000;//sample 30s ;


// Setup the Grove DustSensor for PM2.5 / G13
// int PM25 = 13;
unsigned long duration25;
unsigned long lowpulseoccupancy25 = 0;
float ratio25 = 0;
float concentration25 = 0;

// Setup the Grove DustSensor for PM1.0 / G25
// int PM1 = 25;
unsigned long duration1;
unsigned long lowpulseoccupancy1 = 0;
float ratio1 = 0;
float concentration1 = 0;


// Setup Adafruit IO Credentials
/************************ Adafruit IO Config *******************************/
// visit io.adafruit.com if you need to create an account,
// or if you need your Adafruit IO key.
//#define IO_USERNAME    "your_username"
//#define IO_KEY         "your_AIO_key"


/******************************* WIFI **************************************/
// Setup Wifi Credentials
//#define WIFI_SSID       "your_ssid"
//#define WIFI_PASS       "your_pass"


// Setup Adafruit IO Wifi Settings
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

/***************************** IO FEED ************************************/
// Setup the Adafruit IO feed for the Gove Dust Sensor
// Input your feed key in between the " " (Quotation Marks)
// 

// PM 2.5 Pin 4 on PPD42NS Dust Sensor - G13 on m5stick
AdafruitIO_Feed *PM25FeedConcentration = io.feed("m5stick_pm25_conc");
AdafruitIO_Feed *PM25FeedLPO = io.feed("m5stick_pm25_lpo");
AdafruitIO_Feed *PM25FeedRatio = io.feed("m5stick_pm25_ratio");

// PM 1.0 Pin 2 on PPD42NS Dust Sensor - G25 on m5stick
AdafruitIO_Feed *PM1FeedConcentration = io.feed("m5stick_pm1_conc");
AdafruitIO_Feed *PM1FeedLPO = io.feed("m5stick_pm1_lpo");
AdafruitIO_Feed *PM1FeedRatio = io.feed("m5stick_pm1_ratio");

// Setup U8x8 Screen on the m5stick
// U8x8 constructor for your display
U8X8_SH1107_64X128_4W_HW_SPI u8x8(14, /* dc=*/ 27, /* reset=*/ 33);

// Create a U8x8log object
U8X8LOG u8x8log;

// Define the dimension of the U8x8log window
#define U8LOG_WIDTH 16
#define U8LOG_HEIGHT 8

// Allocate static memory for the U8x8log window
uint8_t u8log_buffer[U8LOG_WIDTH*U8LOG_HEIGHT];


void setup() 
{
// Startup U8x8
  u8x8.begin();
  
  // Set a suitable font. This font will be used for U8x8log
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
  // Start U8x8log, connect to U8x8, set the dimension and assign the static memory
  u8x8log.begin(u8x8, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
  
  // Set the U8x8log redraw mode
  u8x8log.setRedrawMode(1);    // 0: Update screen with newline, 1: Update screen for every char  
    Serial.begin(9600);

    // Display Start Message
    Serial.println((String)"Starting Sensor");
    u8x8log.println((String)"Starting Sensor");
     
    // Set Up Serial and Pin Mode for Dust Sensor
    Serial.begin(9600);
    pinMode(PM25,INPUT);
    pinMode(PM1,INPUT);
    starttime = millis();//get the current time;
  
    // Connect to io.adafruit.com
    Serial.print("Connecting to Adafruit IO");
    u8x8log.println("Connecting to Adafruit IO");
    io.connect();

    // Wait for a connection
    while (io.status() < AIO_CONNECTED)
    {
      Serial.print(".");
      u8x8log.print(".");
            
      delay(2500);
    }

    // we are connected to Adafruit IO
    Serial.println();
    Serial.println(io.statusText());
    u8x8log.println(io.statusText());

    // (Optional) Send Default Value of 10
    //io.run();
    //concentrationFeed->save(10);

    // Set Initial Screen
    u8x8log.println((String)"Reading"); 
    
    // Display Zero Reading on Serial
    Serial.println((String)"L25: "+lowpulseoccupancy25+" R25: "+ratio25+" C25: "+concentration25+" ");
    Serial.println((String)"L1: "+lowpulseoccupancy1+" R1: "+ratio1+" C1: "+concentration1+" ");


    // Display Zero Reading on m5stick screen
    u8x8log.println((String)"L25: "+lowpulseoccupancy25+" ");
    u8x8log.println((String)"R25: "+ratio25+" ");
    u8x8log.println((String)"C25: "+concentration25+" "); 
    u8x8log.println((String)"L1: "+lowpulseoccupancy1+" ");
    u8x8log.println((String)"R1: "+ratio1+" ");
    u8x8log.println((String)"C1: "+concentration1+" "); 

    
    delay(5000);
    
}

void loop() 
{
    // Run Adafruit IO
    io.run();

    // Start Reading from Grove Dust Sensor
    duration25 = pulseIn(PM25, LOW);
    duration1 = pulseIn(PM1, LOW);
    lowpulseoccupancy25 = lowpulseoccupancy25+duration25;
    lowpulseoccupancy1 = lowpulseoccupancy1+duration1;
    
    if ((millis()-starttime) > sampletime_ms)//if the sampel time == 30s
    {
        
        // PM2.5 Readings
        ratio25 = lowpulseoccupancy25/(sampletime_ms*10.0);  // Integer percentage 0=>100
        concentration25 = 1.1*pow(ratio25,3)-3.8*pow(ratio25,2)+520*ratio25+0.62; // using spec sheet curve
        // Serial.print(lowpulseoccupancy25);
        // Serial.print(",");
        // Serial.print(ratio25);
        // Serial.print(",");
        // Serial.println(concentration25);

        // PM1.0 Readings
        ratio1 = lowpulseoccupancy1/(sampletime_ms*10.0);  // Integer percentage 0=>100
        concentration1 = 1.1*pow(ratio1,3)-3.8*pow(ratio1,2)+520*ratio1+0.62; // using spec sheet curve
        // Serial.print(lowpulseoccupancy1);
        // Serial.print(",");
        // Serial.print(ratio1);
        // Serial.print(",");
        // Serial.println(concentration1);


        // Send values to serial connection:
        Serial.println((String)"L25: "+lowpulseoccupancy25+" R25: "+ratio25+" C25: "+concentration25+" ");
        Serial.println((String)"L1: "+lowpulseoccupancy1+" R1: "+ratio1+" C1: "+concentration1+" ");

        // Clear the Screen
        u8x8.clearDisplay();

        delay(500);

        // Print the readings on the m5stick screen for PM25
        u8x8log.println((String)"P2.5 LRC");
        u8x8log.println((String)""+lowpulseoccupancy25+" ");
        u8x8log.println((String)""+ratio25+" ");
        u8x8log.println((String)""+concentration25+" ");        

        // Print the readings on the m5stick screen for PM1.0
        u8x8log.println((String)"P1.0 LRC");
        u8x8log.println((String)""+lowpulseoccupancy1+" ");
        u8x8log.println((String)""+ratio1+" ");
        u8x8log.println((String)""+concentration1+" ");        


        // send data to Adafruit IO feeds
        PM25FeedLPO->save(lowpulseoccupancy25);
        PM25FeedRatio->save(ratio25);
        PM25FeedConcentration->save(concentration25);
        PM1FeedLPO->save(lowpulseoccupancy1);
        PM1FeedRatio->save(ratio1);
        PM1FeedConcentration->save(concentration1);

        // Confirm Post on serial
        Serial.println((String)"Sent: "+concentration25);
        Serial.println((String)"Sent: "+concentration1);

        
        delay(5000);
        
        lowpulseoccupancy25 = 0;
        lowpulseoccupancy1 = 0;
        starttime = millis();
        
        // clear screen for the next loop:
        // lcd.clear();
    }

}
