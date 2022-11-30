#include "tMosq.hh"

#include <iostream>
#include <cstring>

using namespace std;

tMosq::tMosq(const char *id,const char *topic, const char *host, int port) : mosquittopp(id) {
  fKeepalive = 60;    // Basic configuration setup for tMosq class
  fId    = id;
  fPort  = port;
  fHost  = host;
  fTopic = topic;
  connect_async(fHost, fPort, fKeepalive);
  loop_start();       // Start thread managing connection / publish / subscribe
};

tMosq::~tMosq() {
  loop_stop();           // Kill the thread
  mosqpp::lib_cleanup(); // Mosquitto library cleanup
}

bool tMosq::send_message(const char *message) {
  // Send message - depending on QoS, mosquitto lib managed re-submission this the thread
  // * NULL : Message Id (int *) this allow to latter get status of each message
  // * topic : topic to be used
  // * length of the message
  // * message
  // * qos (0,1,2)
  // * retain (boolean) - indicates if message is retained on broker or not
  // Should return MOSQ_ERR_SUCCESS
  int ret = publish(NULL, fTopic, strlen(message), message, 1, false);
  return (ret == MOSQ_ERR_SUCCESS);
}

void tMosq::on_message(const struct mosquitto_message *message) {
  cout << "on_message: ->" << message->topic << "<-" << endl;
  string smsg("");
  if (!strcmp(message->topic, "test")){
    char *buffer = new char[message->payloadlen+1];
    memcpy(buffer, message->payload, message->payloadlen*sizeof(char));
    smsg = string(buffer);
    smsg = string((char*)message->payload);
  }
  cout << "received message, len = " << message->payloadlen << " ->" << smsg << "<-" << endl;
}

void tMosq::on_subscribe() {
  cout << "Subscription succeeded." << endl;
}

void tMosq::on_disconnect(int rc) {
  cout << ">> tMosq - disconnection(" << rc << ")" << endl;
}

void tMosq::on_connect(int rc) {
  if (0 == rc) {
    cout << ">> tMosq - connected with server" << endl;
    cout << ">> subscribing to \"test\" " << endl;
    subscribe(NULL, "test");
    cout << ">> subscribed to \"test\" " << endl;
    fConnected = true;
  } else {
    cout << ">> tMosq - Impossible to connect with server(" << rc << ")" << endl;
    fConnected = false;
  }
}

void tMosq::on_publish(int mid) {
  cout << ">> tMosq - Message (" << mid << ") succeed to be published " << endl;
  fPublished = true;
}

