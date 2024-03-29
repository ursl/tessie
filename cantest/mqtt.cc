#include <unistd.h>
#include <iostream>
#include <cstring>

#include <mosquittopp.h>

using namespace std;
using namespace mosqpp;

bool gConnected(false);

// -- tessie mosquitto client class
class tMosq : public mosquittopp {
  // struct mosquitto_message{
  // 	uint16_t mid;
  // 	char *topic;
  // 	uint8_t *payload;
  // 	uint32_t payloadlen;
  // 	int qos;
  // 	bool retain;
  // };
public:
  tMosq(const char *id, const char *topic, const char *host, int port);
  ~tMosq();
  bool   send_message(const char *message);
  string rec_message(const char *topic);
  int    fPublished;

private:
  const char *fHost;
  const char *fId;
  const char *fTopic;
  int        fPort;
  int        fKeepalive;
  
  void on_connect(int rc);
  void on_disconnect(int rc);
  void on_publish(int mid);
  void on_subscribe();
  void on_message(const struct mosquitto_message *message);
};

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
    gConnected = true; 
  } else {
    cout << ">> tMosq - Impossible to connect with server(" << rc << ")" << endl;
    gConnected = false; 
  }
}

void tMosq::on_publish(int mid) {
  cout << ">> tMosq - Message (" << mid << ") succeed to be published " << endl;
  fPublished = true;
}



// ----------------------------------------------------------------------
int main(int argc, char *argv[]) {
  int send(0);
  string msg(""), host("coldbox01"), topic("test"), id("0");
  
  // -- command line arguments
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-h"))  {host = string(argv[++i]); }
    if (!strcmp(argv[i], "-i"))  {id = string(argv[++i]); }
    if (!strcmp(argv[i], "-s"))  {msg  = string(argv[++i]); send = 1; }
    if (!strcmp(argv[i], "-t"))  {topic = string(argv[++i]); }
  } 

  mosqpp::lib_init();

  class tMosq *tmosq;;
  int rc;
  
  tmosq = new tMosq(id.c_str(), topic.c_str(), host.c_str(), 1883);
  
  while(1) {
    rc = tmosq->loop();
    if (rc) {
      tmosq->reconnect();
    } else {
      break;
    }
  }

  cout << "connected. " << endl;
  if (1 == send) {
    // -- wait for connection 
    while(1){
      if (gConnected) break;
      usleep(2000);
    }
    cout << "send message ->" << msg.c_str() << "<-" << endl;
    while (1) {
      tmosq->send_message(msg.c_str());
      usleep(50000);
      if (tmosq->fPublished) break;
    }

  } else {
    cout << "waiting to receive " << endl;
    while(1){
     
    }
  }
  
  
  
  mosqpp::lib_cleanup();

  return 0;
}
