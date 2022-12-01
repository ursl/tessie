#ifndef TMOSQ_HH
#define TMOSQ_HH

#include <string>
#include <queue>

#include <mosquittopp.h>

// struct mosquitto_message{
// 	uint16_t mid;
// 	char *topic;
// 	uint8_t *payload;
// 	uint32_t payloadlen;
// 	int qos;
// 	bool retain;
// };

// -- tessie mosquitto client class
class tMosq : public mosqpp::mosquittopp {
public:
  tMosq(const char *id, const char *topic, const char *host, int port);
  ~tMosq();
  bool   send_message(const char *message);
  //  std::string rec_message(const char *topic);
  int    fPublished;
  bool   fConnected;
  std::string getMessage();
  int    getNMessages() {return fNMessages;}

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

  // -- messages counter
  int                     fNMessages;
  std::queue<std::string> fMessages;
};

#endif // TMOSQ_HH
