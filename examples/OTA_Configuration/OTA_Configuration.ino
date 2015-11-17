/*
 * Web-based OTA Configuration
 *
 *    Input: Web Interface (http)
 *    Output: OTA Configuration Packets
 *
 * Author: Greg Scull/Mat Mrosko
 * Updated: October 18, 2015- Greg Scull
 * Updated: May 21, 2014 - Mat Mrosko, Materdaddy, rfpixelcontrol@matmrosko.com
 * Updated: October 13, 2014 - Mat Mrosko, Materdaddy, rfpixelcontrol@matmrosko.com
 * 		MAC address in default sketch wouldn't work with certain "smart" switches.
 * 		Jon Chuchla gave useful information regarding mac addresses here:
 * 		http://diychristmas.org/vb1/showthread.php?2760-Can-t-ping-komby-sandwich&p=31396&viewfull=1#post31396
 * Updated: February 19th, 2014 - Mat Mrosko, Materdaddy, rfpixelcontrol@matmrosko.com
 *		Feedback on chosen default MAC address from DMCole proved that certain network
 *		equipment isn't happy with the 0x12, 0x34 ... type MAC address and will not
 *		allow network traffic through as expected.  Using an address that fits the
 *		format described by Jon, but recommended by Arduino seems to work on both the
 *		older network equipment in use by DMCole as well as Materdaddy's equipment the
 *		October 2014 changes were tested on.
 * Updated: March 14, 2014 - Dave Cole, dmcole@dmcole.com.
 *		Corrected URL to MAC address description in Required Configuration section
 *
 * License:
 *    Users of this software agree to hold harmless the creators and
 *    contributors of this software.  By using this software you agree that
 *    you are doing so at your own risk, you could kill yourself or someone
 *    else by using this software and/or modifying the factory controller.
 *    By using this software you are assuming all legal responsibility for
 *    the use of the software and any hardware it is used on.
 *
 *    The Commercial Use of this Software is Prohibited.
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
//Include extra util.h for htonl 
#include "../../Ethernet/src/utility/util.h"

#include "OTAConfig.h"
#include "packets.h"
#include "printf.h"
#include "RF24Wrapper.h"
#include "WebServer.h"


/********************* START OF REQUIRED CONFIGURATION ***********************/
// NRF_TYPE Description: http://learn.komby.com/wiki/58/configuration-settings#NRF_TYPE
// Valid Values: RF1, MINIMALIST_SHIELD, WM_2999_NRF, RFCOLOR_2_4
#define NRF_TYPE                        MINIMALIST_SHIELD

// MAC Address Description: http://learn.komby.com/wiki/58/configuration-settings#MAC-Address
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// IP Address Description: http://learn.komby.com/wiki/58/configuration-settings#IP-Address
static uint8_t ip[] =  { 192, 168, 1, 99 };
// USE_DHCP Description: http://learn.komby.com/wiki/58/configuration-settings#USE_DHCP
#define USE_DHCP                        0
/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// DATA_RATE Description: http://learn.komby.com/wiki/58/configuration-settings#DATA_RATE
// Valid Values: RF24_250KBPS, RF24_1MBPS
// NOTE Receivers programmed after October 2015 should leave the DATA_RATE at RF24_250KBPS,  Receivers programmed prior to October 2015 ( beta 4 code) need to set this data rate
//to the data rate of the target receiver.
#define DATA_RATE                       RF24_250KBPS
/******************* END OF NON-OTA CONFIGURATION SECTION ********************/

/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/
// DEBUG Description: http://learn.komby.com/wiki/58/configuration-settings#DEBUG
//#define DEBUG                           1

// PACKET_SEND_DELAY Description: http://learn.komby.com/wiki/58/configuration-settings#PACKET_SEND_DELAY
#define PACKET_SEND_DELAY               1000
/********************* END OF ADVANCED SETTINGS SECTION **********************/


#define PIXEL_TYPE                      NONE
#define RF_WRAPPER                      1
//Include this after all configuration variables are set
#include "RFShowControlConfig.h"


WebServer webserver("", 80);
#define MAX_LOGICAL_CONTROLLERS         5

void doRFSend(WebServer &server);

// no-cost stream operator as described at
// http://sundial.org/arduino/?page_id=119
//
template<class T> inline Print &operator <<(Print &obj, T arg)
{
  obj.print(arg);
  return obj;
}

// Various blocks 'o html-o

P(htmlHead) =
  "<!DOCTYPE html>"
  "<html"
  "<head>"
  "<title>RF Config</title>"
  "<style type=\"text/css\">"
  "body {font: 100%/1.4 Verdana, Arial, Helvetica, sans-serif;background-color: #4E5869;margin: 0;padding: 0;color: #000;}"
  "ul, ol, dl { padding: 0; margin: 0;}"
  "h1, h2, h3, h4, h5, h6, p {margin-top: 0;padding-right: 15px;padding-left: 15px;}"
  "a img { border: none;}"
  "a:link { color:#414958;text-decoration: underline; }"
  "a:visited {color: #4E5869;text-decoration: underline;}"
  "a:hover, a:active, a:focus {text-decoration: none;}"
  ".container {width: 90%;max-width: 1260px;min-width: 520px;background-color: #FFF;margin: 0 auto; }"
  ".content {padding: 10px 0;}"
  ".content ul, .content ol { padding: 0 15px 15px 40px; }"
  ".fltrt {float: right; margin-left: 8px;}"
  ".fltlft {float: left;margin-right: 8px;}"
  ".clearfloat {clear:both;height:0;font-size: 1px;line-height: 0px;}"
  ".container .content h1 { color: #00F;}"
  "</style></head>";

P(redirect) = "<h1>Success!</h1><form action=\"/logical\" method=\"post\">"
        "<div class = class=\"container\" id=\"buttons\">"
        "<input type=\"submit\" name=\"doneinit\" value=\"Return\">"
        "<input type=\"submit\" name=\"doneinit\" value=\"Resend\">"
        "</div> </form> ";
P(dcontain) = "<div class=\"container\"> <div class=\"content\">";
P(contain) = "<p class=\"container\">";

P(writeInit) = "Writing Payload For init packet";
P(writeLogical) = "Writing Payload For logical packet ";

uint8_t cfg_pkt[32];

// packets for each logical controller
uint8_t lcfg_pkt[MAX_LOGICAL_CONTROLLERS][32];


int iterations;
int num_logcont;  //number of logical controllers

void configInit(WebServer &server, WebServer::ConnectionType type ,char *url_tail, bool tail_complete)
{
  int i;

  P(initHtml) =
    "<body>"
    "<script>"
    "function addFields(){"
    "var number = document.getElementById(\"member\").value; var container = document.getElementById(\"container\");"
    "while (container.hasChildNodes()) {container.removeChild(container.lastChild); }"
    "for (i=0;i<number;i++){container.appendChild(document.createTextNode((i+1)+ \"-> \"));"
    "var dd = document.createElement(\"select\");dd.name = \"xname\"+(i+1); dd.id = \"Xid\";"
    "dd.options[dd.length] = new Option(\"LED\", \"LED\"); dd.options[dd.length] = new Option(\"DMX\", \"DMX\");"
    "dd.options[dd.length] = new Option(\"Renard\", \"Renard\");"
    "container.appendChild(dd);  container.appendChild(document.createTextNode(\"# of Nodes \"));"
    "var input = document.createElement(\"input\"); input.name = \"nodes\"+(i+1); input.type = \"text\";"
    "container.appendChild(input); container.appendChild(document.createTextNode(\"Start Addr. \" ));"
    "var input2 = document.createElement(\"input\"); input2.name = \"start\"+(i+1); input2.type = \"text\";"
    "container.appendChild(input2); container.appendChild(document.createTextNode(\"Rate \" ));"
    "var input3 = document.createElement(\"input\"); input3.name = \"zrate\"+(i+1); input3.type = \"text\";"
    "container.appendChild(input3); container.appendChild(document.createElement(\"br\")); } }"
    "</script>"
    "<div class=\"container\">"
    "<div class=\"content\">"
    "<h1>RF Configurator</h1>"
    "<p class=\"container\">"
    //change Intro text below
    "<i>RFShowControl OTA Programming</i></p>"
    "<p class=\"container\">&nbsp;</p>"
    "<form id=\"form1\" name=\"form1\" method=\"post\" action=\"/init\">"
    "<p>Controller ID: <input type=\"text\" name=\"controller\" id=\"controller-id\" maxlength=\"3\"/> </p>"
    "<p>Listen Rate:"
    "<input type=\"radio\" name=\"radio\" id=\"250K\" value=\"0\" checked/> <label for=\"250K\">250 kbps</label>"
    "<input type=\"radio\" name=\"radio\" id=\"1M\" value=\"1\" /> <label for=\"1M\">1.0 Mbps</label>"
    "<input type=\"radio\" name=\"radio\" id=\"2M\" value=\"2\" /> <label for=\"2M\">2.0 Mbps</label>"
    "</p>"
    "<p>Listen Channel :"
    "<label for=\"channel\"></label> <input type=\"text\" name=\"channel\" id=\"channel\" maxlength=\"3\" />"
    "</p><div>"
    "<input type=\"text\" id=\"member\" name=\"member\" value=\"\">"
    "Number of Logical Controllers: (max.5)<br/><br/>"
    "<a href=\"#\" id=\"filldetails\" onclick=\"addFields()\">Enumerate Logical Controllers</a><br />"
    "<div id=\"container\"/></div> <br />"
    "<div><p><input type=\"submit\" value=\"Send the Config Packet\"></p>"
    "</div> </p>"
    "</form>"
    "<p class=\"container\">&nbsp;</p>"
    "<p class=\"container\">&nbsp;</p>"
    "<!-- end .content --></div> <!-- end .container --></div>";

  server.httpSuccess();
//send out header
  server.printP(htmlHead);
//send out initial form, jump to http:xx.xx.xx.xx/init for processing
  server.printP(initHtml);

  server << "</body></html>";
}

//initCMD -- called after receiving main parameters.
void initCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{

  struct ControllerInitInfo *c_hdr = (struct ControllerInitInfo*) cfg_pkt;

  server.httpSuccess(); //send out acknowledge


  if (type == WebServer::POST)
  {
    bool repeat;
    char name[16], value[16];
    int speed;
    int status;
    uint8_t lcont;


    //clear config message array -- this is the RF packet for configuration; will be filled as data is extraced from the POST

    memset (cfg_pkt,0,32);
    for (int i = 0; i <MAX_LOGICAL_CONTROLLERS; i++)
    {
      memset(lcfg_pkt[i],0,32);
    }

//processing init page
    do
    {
      // get next server post data.  Sequences will be name, value for each entry)
      repeat = server.readPOSTparam(name, 16, value, 16);

#ifdef DEBUG
      for (int q = 0; q<16; q++)
      {
        Serial.print(name[q]);
      }
      Serial.println();
      for (int q = 0; q<16; q++)
      {
        Serial.print(value[q]);
      }
      Serial.println();
#endif


      if (!(strcmp(name, "controller"))) // controller id
      {
        c_hdr->controllerID = htonl(strtoul(value, NULL, 0));
      }
      else if (!(strcmp(name, "radio"))) // radio speed
      {
        speed = (int)strtoul(value, NULL, 0);
        switch (speed)
        {
        case 0:
          c_hdr->rfListenRate = RF24_250KBPS;
          break;
        case 1:
          c_hdr->rfListenRate = RF24_1MBPS;
          break;
        case 2:
          c_hdr->rfListenRate = RF24_2MBPS;
          break;
        }
      }
      else if (!(strcmp(name, "member"))) // logical controller
      {
        c_hdr->numLogicalControllers = (uint8_t)strtoul(value, NULL, 0);
        num_logcont = (int)c_hdr->numLogicalControllers ;

        struct LogicalInitInfo *tmphdr = (struct LogicalInitInfo*) lcfg_pkt;
        for (int q=0; q <num_logcont; q++)
        {
          tmphdr->controllerID=c_hdr->controllerID;
          tmphdr->logicalControllerNumber=(uint8_t)q;
          tmphdr++;
        }
      }
      else if (!(strcmp(name, "channel"))) // channel do
      {
        c_hdr->rfListenChannel =(uint8_t)strtoul(value, NULL, 0);
      }

      //set up rest of packet
      c_hdr->packetType = CONTROLLERINFOINIT;
      // initial controller packet is ready to go, time to parse data for individual logical controllers.
      // Individual elements all have names with 5 letters followed by a number; we extract the number from position 6 of the name
      //

      //first initialize each logical packet being used with logical blick and controller ID


      // now fill up the packet
      if (name[0]=='x' || name[0]=='s' || name[0]=='n'|| name[0]=='z') // must be logical node parameter, so process here
      {
        // extract the array number
        lcont = (int)(name[5] & 0x0f);

#ifdef DEBUG
        Serial.print ("lcont value: ");
        Serial.println(lcont, DEC);
#endif

        // set the overlayed structure to point at the logical controller buffer we are placing the element in;
        struct LogicalInitInfo *l_hdr = (struct LogicalInitInfo*) lcfg_pkt;
        for (int zz=1; zz < lcont; zz++)l_hdr++;


        //load values into packet
        switch (name[0])
        {
        case 'x': // Type: DMX, Renard, LEDs
          if (!(strcmp(value, "DMX")))
          {
            l_hdr->packetType=LOGICALCONTROLLER_SERIAL;
            l_hdr->outputFormat=DMX_OUTPUT;
          }
          else if (!(strcmp(value, "Renard")))
          {
            l_hdr->packetType=LOGICALCONTROLLER_SERIAL;
            l_hdr->outputFormat=RENARD_OUTPUT;
          }
          else if (!(strcmp(value, "LED")))
          {
            l_hdr->packetType=LOGICALCONTROLLER_LED;
          }
          break;

        case 's':
          l_hdr->startChannel=htonl((uint32_t)strtoul(value, NULL, 0));
          break;

        case 'n':
          l_hdr->numChannels=htonl((uint32_t)strtoul(value, NULL, 0));
          break;

        case 'z':
          l_hdr->baudRate=htonl((uint32_t)strtoul(value, NULL, 0));
          break;
        }
      }
    }
    while (repeat);

    //all data is in place, so time to send to the radio and update the console
    doRFSend(server);
  }
}

// got all the parameters at this point -- time to send to RF and update the screen with status info
void sendRFPacket(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
//web connection link to doRFsend
{
  doRFSend(server);
}
//send the configuration (held in global variables) to the radio and update the web page

void doRFSend(WebServer &server)
{
  //update HTML Page with status

  //send out header
  server.printP(htmlHead);
  server.printP(dcontain);
  server.printP(contain);

  // send on the radio
#ifdef DEBUG
  printf("config array\n");
  for (int i=0; i < 16; i++)
  {
    Serial.print (cfg_pkt[i], HEX);
    Serial.print (" ");
  }
  Serial.println();
  for (int i=0; i < 16; i++)
  {
    Serial.print (cfg_pkt[16+i], HEX);
    Serial.print (" ");
  }
  Serial.println();

  //dump logical packets

  printf("lconfig array\n");
  for (int xyzzy = 0; xyzzy < MAX_LOGICAL_CONTROLLERS; xyzzy++)
  {
    for (int i=0; i < 16; i++)
    {
      Serial.print (lcfg_pkt[xyzzy][i], HEX);
      Serial.print (" ");
    }
    Serial.println();
    for (int i=0; i < 16; i++)
    {
      Serial.print (lcfg_pkt[xyzzy][16+i], HEX);
      Serial.print (" ");
    }
    Serial.println();
    Serial.println();
  }
  printf("Writing Payload For init packet\n");
 #endif
  server << "<body>";
  server.printP(contain);

  server.printP(writeInit);
  server << "<br/>";
//todo komby research if this is acceptable/test me
//  while (radio.get_status() & 0x01) ; // wait for FIFO spot
//radio.write_payload( &cfg_pkt, 32 );
  radio.writeFast(&cfg_pkt, 32, true);
  delayMicroseconds(5000);
  delay (PACKET_SEND_DELAY); //wait for printf to complete on receiver
  char *fred = (char *)lcfg_pkt;
  for (int zz = 0; zz < num_logcont; zz++)
  {

#ifdef DEBUG
    printf("Writing Payload For logical controller packet");
#endif
    server.printP(writeLogical);
    server << zz;
    server << "<br/>";

#ifdef DEBUG
    Serial.println(zz);
#endif
  //  while (radio.get_status() & 0x01) ; // wait for FIFO spot
    radio.writeFast( fred, 32, false);
    fred+=32;
    delayMicroseconds(5000);
    delay (PACKET_SEND_DELAY); //wait for printf to complete on receiver
  }

  server << "</p></div></div>";

  server.printP(dcontain);
  server.printP(contain);
  server.printP(redirect);
  server << "</p></div></div>";
  server << "</body></html>";

}

void initsuccess(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  P(successHTMLold) = "<body><h1 style=\"text-align: center\">Success!</h1> <form action= \"/\">"
            "<input type=\"submit\" value=\"Return to Main Page\">"
            "<form action= \"/\"> \"<input type=\"submit\" value=\"Eat Me\">"
            "</form>";
  P(successHTMLxxx) = "<body> <form action=\"action\" method=\"get\"><input type=\"text\" name=\"abc\">"
            "<div id=\"buttons\">"
            "<input type=\"submit\" name=\"next\" value=\"Next\">"
            "<input type=\"submit\" name=\"prev\" value=\"Prev\">"
            "<div style=\"clear:both\"></div></div></form>";

  P(successHTML) = "<body> <h1 style=\"text-align: center\">Success!</h1><form action=\"newpage\" method=\"get\">"
           "<div id=\"buttons\">"
           "<input type=\"submit\" name=\"next\" value=\"Next\">"
           "<input type=\"submit\" name=\"prev\" value=\"Prev\">"
           "</form>";
  char name[16], value[16];

  server.httpSuccess();
  //send out header
  server.printP(htmlHead);
  //send out html success message
  server.printP(successHTML);
  server << "</body></html>";
}

void rawCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  char name[16], value[16];
  boolean repeat;
  int holdme;

  server.httpSuccess();
  //name will consist of dat followed by two digits.  We'll use the init array only for sending
  //
  //first zero out the packet -- just in case
  memset (cfg_pkt,0,32);

  // now indicate zero logical controllers, so a call to sendRF will only send one packet
  num_logcont=0;

  if (type == WebServer::POST)
  {
    do
    {
      repeat = server.readPOSTparam(name, 16, value, 16);
      // validate it's a good 'un

      if(name[1]=='x')
      {
        // extract number
        holdme=(int)strtoul(&name[2], NULL, 10);
        //get index pointer

        cfg_pkt[holdme] = (uint8_t)strtoul(value, NULL, 16);


#ifdef DEBUG
        for (int q = 0; q<16; q++)
        {
          Serial.print(name[q]);
        }
        Serial.print(" holdme is ");
        Serial.print(holdme, DEC);
        Serial.print(" ");
        Serial.println();
        for (int q = 0; q<16; q++)
        {
          Serial.print(value[q]);
        }
        Serial.print(" config packet ");
        Serial.print(holdme, DEC);
        Serial.print(" is ");
        Serial.print(cfg_pkt[holdme], HEX);
        Serial.println();
#endif
      }
    }
    while (repeat);


    doRFSend(server);
  }
}

void logicalInit(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  char name[16], value[16];
  boolean repeat;

  do
  {
    repeat = server.readPOSTparam(name, 16, value, 16);

    if (!(strcmp(value, "Resend")))
    {
      server.httpSeeOther("/sendrf");
    }
    if (!(strcmp(value, "Return")))
    {
      server.httpSeeOther("/");
    }
  }
  while (repeat);
}

void setup(void)
{
#ifdef DEBUG
  Serial.begin(115200);
  printf_begin();
  printf("Startup\n");
#endif

#if (USE_DHCP == 1)
  if (Ethernet.begin(mac) == 0) {
#ifndef DEBUG
	Serial.begin(115200);
#endif
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;/*ever*/;)
      ;
  }
#else
  Ethernet.begin(mac, ip);
#endif

  delay(1000);
  webserver.begin();
#ifdef DEBUG
  printf("Webserver up at %s\n", Ethernet.localIP());
#endif

  radio.Initialize( radio.TRANSMITTER, pipes, RF_NODE_CONFIGURATION_CHANNEL, DATA_RATE);
  delayMicroseconds(5000);

#ifdef DEBUG
  radio.printDetails();
#endif

  webserver.setDefaultCommand(&configInit);
  webserver.addCommand("init", &initCmd);
  webserver.addCommand("initsuccess", &initsuccess);
  webserver.addCommand("logical", &logicalInit);
  webserver.addCommand("sendrf", &sendRFPacket);
}

void loop(void)
{
  // process incoming connections one at a time forever
  webserver.processConnection();

  // if you wanted to do other work based on a connecton, it would go here
}