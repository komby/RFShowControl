/* Web_Demo.pde -- sample code for Webduino server library */

/*
 * To use this demo,  enter one of the following USLs into your browser.
 * Replace "host" with the IP address assigned to the Arduino.
 *
 * http://host/
 * http://host/json
 *
 * This URL brings up a display of the values READ on digital pins 0-9
 * and analog pins 0-5.  This is done with a call to defaultCmd.
 *
 *
 * http://host/form
 *
 * This URL also brings up a display of the values READ on digital pins 0-9
 * and analog pins 0-5.  But it's done as a form,  by the "formCmd" function,
 * and the digital pins are shown as radio buttons you can change.
 * When you click the "Submit" button,  it does a POST that sets the
 * digital pins,  re-reads them,  and re-displays the form.
 *
 */
#define  DEBUG

#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Wrapper.h>
#include "printf.h"
#include <OTAConfig.h>
#include <Arduino.h>
#include <util.h>
#include <SPI.h>
#include <Ethernet.h>
#include "WebServer.h"
#include "packets.h"
#include <EEPROM.h>


//
// below is the configuration info most likely to change
//
#define PACKET_SEND_DELAY 1000   // delay between packets.  Increase if using debug prints on receiver.
#define PREFIX ""


//What Speed do you want to use to transmit?
//Valid Values:   RF24_250KBPS, RF24_1MBPS
#define DATA_RATE RF24_250KBPS

//Use channel 100
#define TRANSMIT_CHANNEL 125

//Definitions: http://learn.komby.com/wiki/46/rfpixelcontrol-nrf_type-definitions-explained
#define NRF_TYPE MINIMALIST_SHIELD

//#define NRF_TYPE RF1_1_3

// Ethernet controller MAC address...
// must be unique...can generate with:
// http://www.miniwebtool.com/mac-address-generator/
//
RF24Wrapper radio(9, 8);

static uint8_t mac[] = { 0x5B, 0xD0, 0x00, 0xEA, 0x80, 0x84 };
// CHANGE THIS TO MATCH YOUR HOST NETWORK
static uint8_t ip[] = { 192, 168, 1, 99 };
// set up the radio.   change pins for particular board

WebServer webserver(PREFIX, 80);
//radio packet buffers - one for controller;
#define MAX_LOGICAL_CONTROLLERS 5

//additional constants
//
//

//RF info

#define RF_DELAY 2000
#define RF_NODE_CONFIGURATION_CHANNEL 125

const uint64_t pipes[2] = {
	0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL
};
// static HTML info (global) below

// no-cost stream operator as described at
// http://sundial.org/arduino/?page_id=119
//
template<class T>
inline Print &operator <<(Print &obj, T arg)
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
    "a img {	border: none;}"
    "a:link {	color:#414958;text-decoration: underline; }"
    "a:visited {color: #4E5869;text-decoration: underline;}"
    "a:hover, a:active, a:focus {text-decoration: none;}"
    ".container {width: 90%;max-width: 1260px;min-width: 520px;background-color: #FFF;margin: 0 auto; }"
    ".content {padding: 10px 0;}"
    ".content ul, .content ol { padding: 0 15px 15px 40px; }"
    ".fltrt {float: right; margin-left: 8px;}"
    ".fltlft {float: left;margin-right: 8px;}"
    ".clearfloat {clear:both;height:0;font-size: 1px;line-height: 0px;}"
    ".container .content h1 {	color: #00F;}"
    "</style></head>";

P(redirect) = "<h1>Success!</h1><form action=\"/logical\" method=\"post\">"
              "<div class = class=\"container\" id=\"buttons\">"
              "<input type=\"submit\"  name=\"doneinit\" value=\"Return\">"
              "<input type=\"submit\"  name=\"doneinit\" value=\"Resend\">"
            //  "<input type=\"submit\"  name=\"doneinit\" value=\"Raw\">"
              "</div> </form> ";
P(dcontain) = "<div class=\"container\"> <div class=\"content\">";
P(contain) = "<p class=\"container\">";

P(writeInit) = "Writing Payload For init packet";
P(writeLogical) = "Writing Payload For logical packet ";

//P(rawinput)=
 //   "<h1>Raw input</h1>"
   // "<form id=\"rawinput\" name=\"rawinput\" method=\"post\" action=\"/rawpost\">"
   // "<table width=\"95%\" border=\"0\" cellpadding=\"2\">"
   // "<tr><td width=\"19%\">byte     0  - 15	:</td>"
   // "<td width=\"81%\">"
 //"<input type=\"text\" name=\"0x00\" id=\"D00\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x01\" id=\"D01\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x02\" id=\"D02\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x03\" id=\"D03\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x04\" id=\"D04\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x05\" id=\"D05\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x06\" id=\"D06\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x07\" id=\"D07\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x08\" id=\"D08\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x09\" id=\"D09\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x10\" id=\"D10\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x11\" id=\"D11\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x12\" id=\"D12\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x13\" id=\"D13\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x14\" id=\"D14\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x15\" id=\"D15\"  size=\"2\" />"
//
 //"</td></tr><tr><td><br />byte 16- 31:</td><td>"
//
 //"<input type=\"text\" name=\"0x16\" id=\"D16\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x17\" id=\"D17\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x18\" id=\"D18\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x19\" id=\"D19\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x20\" id=\"D20\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x21\" id=\"D21\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x22\" id=\"D22\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x23\" id=\"D23\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x24\" id=\"D24\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x25\" id=\"D25\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x26\" id=\"D26\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x27\" id=\"D27\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x28\" id=\"D28\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x29\" id=\"D29\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x30\" id=\"D30\"  size=\"2\" />"
 //"<input type=\"text\" name=\"0x31\" id=\"D31\"  size=\"2\" />"
    //"<input type=\"text\" name=\"0x00\" id=\"D00\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x01\" id=\"D01\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x02\" id=\"D02\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x03\" id=\"D03\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x04\" id=\"D04\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x05\" id=\"D05\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x06\" id=\"D06\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x07\" id=\"D07\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x08\" id=\"D08\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x09\" id=\"D09\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x10\" id=\"D10\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x11\" id=\"D11\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x12\" id=\"D12\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x13\" id=\"D13\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x14\" id=\"D14\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x15\" id=\"D15\" maxlength=\"2\" size=\"2\" />"
//
    //"</td></tr><tr><td><br />byte 16- 31:</td><td>"
//
    //"<input type=\"text\" name=\"0x16\" id=\"D16\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x17\" id=\"D17\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x18\" id=\"D18\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x19\" id=\"D19\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x20\" id=\"D20\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x21\" id=\"D21\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x22\" id=\"D22\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x23\" id=\"D23\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x24\" id=\"D24\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x25\" id=\"D25\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x26\" id=\"D26\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x27\" id=\"D27\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x28\" id=\"D28\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x29\" id=\"D29\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x30\" id=\"D30\" maxlength=\"2\" size=\"2\" />"
    //"<input type=\"text\" name=\"0x31\" id=\"D31\" maxlength=\"2\" size=\"2\" />"

   // "</td></tr></table><input type=\"submit\" value=\"Send the Raw Packet\"/></form><p>&nbsp;</p><p></p><p>"
    //"</p>";

uint8_t cfg_pkt[32];

// packets for each logical controller
uint8_t lcfg_pkt[MAX_LOGICAL_CONTROLLERS][32];


int iterations;
int num_logcont;    //number of logical controllers


//void rawInit(WebServer &server, WebServer::ConnectionType type ,char *url_tail, bool tail_complete)
//{
    //server.httpSuccess();
////send out header
    //server.printP(htmlHead);
    //server << "<body>";
    //server.printP(dcontain);
    //server.printP(rawinput);
//
    //server << "</div></div></body></html>";
//}
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
        "container.appendChild(dd);	container.appendChild(document.createTextNode(\"# of Nodes \"));"
        "var input = document.createElement(\"input\");  input.name = \"nodes\"+(i+1);  input.type = \"text\";"
        "container.appendChild(input); container.appendChild(document.createTextNode(\"Start Addr. \" ));"
        "var input2 = document.createElement(\"input\");  input2.name = \"start\"+(i+1); input2.type = \"text\";"
        "container.appendChild(input2); container.appendChild(document.createTextNode(\"Rate \" ));"
        "var input3 = document.createElement(\"input\");   input3.name = \"zrate\"+(i+1); input3.type = \"text\";"
        "container.appendChild(input3);  container.appendChild(document.createElement(\"br\")); }  }"
        "</script>"
        "<div class=\"container\">"
        "<div class=\"content\">"
        "<h1>RF Configurator</h1>"
        "<p class=\"container\">"
        //change Intro text below
        "<i>RFPixelControl OTA Programming</i></p>"
        //
        "<p class=\"container\">&nbsp;</p>"
        "<form id=\"form1\" name=\"form1\" method=\"post\" action=\"/init\">"
        "<p>Controller ID:  <input type=\"text\" name=\"controller\" id=\"controller-id\" maxlength=\"3\"/> </p>"
        "<p>Listen Rate:"
        "<input type=\"radio\" name=\"radio\" id=\"250K\" value=\"0\" checked/> <label for=\"250K\">250 Khz</label>"
        "<input type=\"radio\" name=\"radio\" id=\"1M\" value=\"1\" /> <label for=\"1M\">1.0 Mhz</label>"
        "<input type=\"radio\" name=\"radio\" id=\"2M\" value=\"2\" /> <label for=\"2M\">2.0 Mhz</label>"
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
            for (int q = 0; q<16; q++) Serial.print(name[q]);
            Serial.println();
            for (int q = 0; q<16; q++) Serial.print(value[q]);
            Serial.println();
#endif


            if (!(strcmp(name, "controller")))   // controller id
            {

                c_hdr->controllerID = htonl(strtoul(value, NULL, 0));
                //cfg_pkt[IDX_CONTROLLER_ID]  = (uint8_t)strtoul(value, NULL, 0);
            }
            else if (!(strcmp(name, "radio")))   // radio speed
            {
                speed = (int)strtoul(value, NULL, 0);
                switch (speed)
                {
                case 0:
                    c_hdr->rfListenRate  = RF24_250KBPS;
                    //cfg_pkt[IDX_RF_LISTEN_RATE] = RF24_250KBPS;
					break;
                case 1:
                    c_hdr->rfListenRate  = RF24_1MBPS;
					break;
                case 2:
                    c_hdr->rfListenRate  = RF24_2MBPS;
					break;
                }
            }
            else if (!(strcmp(name, "member")))   // logical controller
            {
                c_hdr->numLogicalControllers = (uint8_t)strtoul(value, NULL, 0);
                num_logcont = (int)c_hdr->numLogicalControllers ;

                struct LogicalInitInfo *tmphdr  = (struct LogicalInitInfo*) lcfg_pkt;
                for (int q=0; q <num_logcont; q++)
                {
				     
                    tmphdr->controllerID=c_hdr->controllerID;
                    tmphdr->logicalControllerNumber=(uint8_t)q;
                    tmphdr++;
                }
            }
            else if (!(strcmp(name, "channel")))   // channel do
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
            if (name[0]=='x' || name[0]=='s' || name[0]=='n'|| name[0]=='z')  // must be logical node parameter, so process here
            {
                // extract the array number
                lcont = (int)(name[5] & 0x0f);

//#ifdef DEBUG
                Serial.print ("lcont value: ");
                Serial.println(lcont, DEC);
//#endif

                // set the overlayed structure to point at the logical controller buffer we are placing the element in;
                struct LogicalInitInfo *l_hdr = (struct LogicalInitInfo*) lcfg_pkt;
                for (int zz=1; zz < lcont; zz++)l_hdr++;

                // load logical controller number into packet
                // l_hdr->logicalControllerNumber=(uint8_t)lcont;


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
    else
        ;
}

// got all the parameters at this point -- time to send to RF and update the screen with status info

void sendRFPacket(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
//web connection link to doRFsend //
{
    doRFSend(server);
}
//send the configuration (held in global variables) to the radio and update the web page
//

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

    while (radio.get_status() & 0x01) ;  // wait for FIFO spot
    radio.write_payload( &cfg_pkt, 32 );
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

        Serial.println(zz);
        while (radio.get_status() & 0x01) ;  // wait for FIFO spot
        radio.write_payload( fred, 32 );
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
                        "<input type=\"submit\"  name=\"next\" value=\"Next\">"
                        "<input type=\"submit\"  name=\"prev\" value=\"Prev\">"
                        "<div style=\"clear:both\"></div></div></form>";

    P(successHTML) = "<body> <h1 style=\"text-align: center\">Success!</h1><form action=\"newpage\" method=\"get\">"
                     "<div id=\"buttons\">"
                     "<input type=\"submit\"  name=\"next\" value=\"Next\">"
                     "<input type=\"submit\"  name=\"prev\" value=\"Prev\">"
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


//#ifdef DEBUG
                for (int q = 0; q<16; q++) Serial.print(name[q]);
                Serial.print(" holdme is ");
                Serial.print(holdme, DEC);
                Serial.print(" ");
                Serial.println();
                for (int q = 0; q<16; q++) Serial.print(value[q]);
                Serial.print(" config packet ");
                Serial.print(holdme, DEC);
                Serial.print(" is ");
                Serial.print(cfg_pkt[holdme], HEX);
                Serial.println();
//#endif
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



void setup()
{

    Serial.begin(57600);
    printf_begin();
    printf("Startup\n");

    Ethernet.begin(mac, ip);
    delay(1000);
    webserver.begin();
    printf("Webserver up\n");
    
	radio.Initialize( radio.TRANSMITTER, pipes, RF_NODE_CONFIGURATION_CHANNEL, DATA_RATE);
//radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);

    delayMicroseconds(5000);
    radio.printDetails();
    webserver.setDefaultCommand(&configInit);
    webserver.addCommand("init", &initCmd);
    webserver.addCommand("initsuccess", &initsuccess);
    webserver.addCommand("logical", &logicalInit);
    webserver.addCommand("sendrf", &sendRFPacket);
   
}

void loop()
{
    // process incoming connections one at a time forever
    webserver.processConnection();

    // if you wanted to do other work based on a connecton, it would go here
}
