/*
Author: samiul ahmed joy
Github Link: https://github.com/samiulahmedjoy/planemanagement
Library Used: ESP Mail Client(available on arduino ide)
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>
#include <IRremote.hpp>
//#include <Wire.h>

#define SENSOR_PIN A0 // Analog sensor pin for moisture sensor
// #define IR_PIN D6 // DEMO FOR IR PIN, replace this with your sensor pin
// #define RELAY_PIN D2 // If you have relay for other functions

// IR remote keys
// const unsigned short int send_value=89; // use this if you implement IR remote

// Analog value gets writen on this
unsigned int analog_value;

// Minimum threshold value
const unsigned int threshold_value=800;

// Email message prefix
char email_message[62]="Your plant needs water, because the value is below threshold "

// A value for keeping score of button press
unsigned short int b=0;

// Value to copy subject_message to subject
char subject[25];

// Email Subject limited to 24 characters, change if you want more chars
char subject_message[25] = "This is a subject";

// Value to copy text message on, limited to 29 chars
char textMsg[75];

// value assign for conversion of analogvalue
char analog_value_conversion[9];

#define WIFI_SSID "YOURSSID"
#define WIFI_PASSWORD "YOURPASSWORD1234"

// Using gmail as smtp host
#define SMTP_HOST "smtp.gmail.com"

// port 465 used for gmail in this case 
#define SMTP_PORT esp_mail_smtp_port_465

// The login credentials
#define AUTHOR_EMAIL "espmail@gmail.com"
#define AUTHOR_PASSWORD "12345678changeme"

// Recipient email address
#define RECIPIENT_EMAIL "emailaddress@gmail.com"

// Declare the global used SMTPSession object for SMTP transport
SMTPSession smtp;

// Callback function to get the Email sending status
void smtpCallback(SMTP_Status status);

void setup() {
	// Serial debugging is enabled
	Serial.begin(115200);

	// IrReceiver.begin(IR_PIN, 0);
	// include a sensor pin as INPUT too for example
	pinMode(SENSOR_PIN, INPUT);
	// pinMode(RELAY_PIN, OUTPUT);

	// WiFi connection wait check
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	Serial.print("Connecting to Wi-Fi");

	// Wifi Status Check Loop
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(300);
	}
	Serial.println();
	Serial.print("Connected with IP: ");
	Serial.println(WiFi.localIP());
	Serial.println();

	/*  Set the network reconnection option */
	MailClient.networkReconnect(true);

	// Setting network debugging option to false for now
	smtp.debug(0);
}

// Email Sending function
void Send_Email(void) {
	/* Set the callback function to get the sending results */
	smtp.callback(smtpCallback);

	// Initializing config to struct Session_Config
	Session_Config config;

	/* Set the session config */
	config.server.host_name = SMTP_HOST;
	config.server.port = SMTP_PORT;
	config.login.email = AUTHOR_EMAIL;
	config.login.password = AUTHOR_PASSWORD;

	config.login.user_domain = F("127.0.0.1");
	// Enabling secure ssl/tls mode
	config.secure.mode = esp_mail_secure_mode_ssl_tls;

	// NTP server setup
	config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
	config.time.gmt_offset = 6;
	config.time.day_light_offset = 0;

	// Initializing message to struct SMTP_Message
	SMTP_Message message;

	/* Set the message headers */
	message.sender.name = F("ESPMAIL");
	message.sender.email = AUTHOR_EMAIL;

	message.subject = subject;

	message.addRecipient(F("Recepient Value"), RECIPIENT_EMAIL);

	message.text.content = textMsg;

	message.text.transfer_encoding = "base64"; // recommend for non-ASCII words in message.

	message.text.charSet = F("utf-8"); // recommend for non-ASCII words in message.

	message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

	/* Set the custom message header */
	message.addHeader(F("Message-ID: <abcde.fghij@gmail.com>"));

	// NTP timeout in seconds
	smtp.setTCPTimeout(1000);

	/* Connect to the server */
	if (!smtp.connect(&config)) {
		MailClient.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
		return;
	}

	if (!smtp.isLoggedIn()) {
		Serial.println("Not yet logged in.");
	}
	else {
		if (smtp.isAuthenticated())
			Serial.println("Successfully logged in.");
		else
			Serial.println("Connected with no Auth.");
	}

	/* Start sending Email and close the session */
	if (!MailClient.sendMail(&smtp, &message))
		MailClient.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());

	// Let the user know/indicate that email has been sent
	digitalWrite(LED_BUILTIN, HIGH);
	delay(1000);
	digitalWrite(LED_BUILTIN, LOW);
}

void loop(void) {

	// Assign analog read value to analog_value
	analog_value=analogRead(SENSOR_PIN);

	// if the analog value is less then threshold, then send email;
	if(analog_value < threshold_value) {
		// conversion of ints/chars to strings
		sprintf(subject, "%s", subject_message);
		sprintf(analog_value_conversion, "%d", analog_value);
		// Appending values to textMsg
		strcat(textMsg, email_message);
		strcat(textMsg, analog_value_conversion);
		// Call Send_email function
		Send_Email();
		// Add a delay of 5 minutes
		delay(300000)
	}
	// Overall delay of the running program
	delay(30000);
}

/* If you want to implement IR remote, this section is for that
	if (IrReceiver.decode()) {

		switch(IrReceiver.decodedIRData.command) {
			case send_value:
				// Conversion of subject_value to string
				sprintf(subject, "%s", subject_message);
				// Conversion of remote value to string
				sprintf(threshold_value_conversion, "%d", send_value);
				// copy the email message to textMsg
				strcat(textMsg, email_message);
				// append the conversion sensor value to the textMsg value
				strcat(textMsg, threshold_value_conversion);
				// Run the send_email function
				Send_Email();
				b=1;
				break ;
//			case wifi_off_val:
//				WiFi.mode(WIFI_OFF);
//				b=0
//				break ;
			default:
				// if the email client is running/active
				// Give a small assurance by blinking light
				if (b==1) {
					digitalWrite(LED_BUILTIN, HIGH);
					delay(1000);
					digitalWrite(LED_BUILTIN, LOW);
				}
				b=0;
			break ;
		}
	}
	IrReceiver.resume();
	delay(2000);
} */

/* Callback function to get the Email sending status. This is for debugging mostly */
void smtpCallback(SMTP_Status status) {
	/* Print the current status */
	Serial.println(status.info());

	/* Print the sending result */
	if (status.success()) {
		// MailClient.printf used in the examples is for format printing via debug Serial port
		// that works for all supported Arduino platform SDKs e.g. SAMD, ESP32 and ESP8266.
		// In ESP8266 and ESP32, you can use Serial.printf directly.

		Serial.println("----------------");
		MailClient.printf("Message sent success: %d\n", status.completedCount());
		MailClient.printf("Message sent failed: %d\n", status.failedCount());
		Serial.println("----------------\n");

		for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
			/* Get the result item */
			SMTP_Result result = smtp.sendingResult.getItem(i);

			// In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
			// your device time was synched with NTP server.
			// Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
			// You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)

			MailClient.printf("Message No: %d\n", i + 1);
			MailClient.printf("Status: %s\n", result.completed ? "success" : "failed");
			MailClient.printf("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
			MailClient.printf("Recipient: %s\n", result.recipients.c_str());
			MailClient.printf("Subject: %s\n", result.subject.c_str());
		}
		Serial.println("----------------\n");
		// You need to clear sending result as the memory usage will grow up.
		smtp.sendingResult.clear();
	}
}
