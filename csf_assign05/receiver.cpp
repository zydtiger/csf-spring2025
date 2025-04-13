#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  Connection conn;
  Message msg;

  conn.connect(server_hostname, server_port);

  // send rlogin and join messages (expect a response from
  //       the server for each one)
  if (!conn.send(Message(TAG_RLOGIN, username))) {
    std::cerr << "Receiver login failed to send!" << std::endl;
    return -1;
  }
  if (!conn.receive(msg)) {
    std::cerr << "Failed to receive message from server!" << std::endl;
    return -1;
  }
  if (msg.tag != TAG_OK) {
    std::cerr <<  msg.data << std::endl;
    return -1;
  }

  if (!conn.send(Message(TAG_JOIN, room_name))) {
    std::cerr << "Join room request failed to send!" << std::endl;
    return -1;
  }
  if (!conn.receive(msg)) {
    std::cerr << "Failed to receive message from server!" << std::endl;
    return -1;
  }
  if (msg.tag != TAG_OK) {
    std::cerr << msg.data << std::endl;
    return -1;
  }
  
  // loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)
  while (true) {
    if (!conn.receive(msg)) {
      std::cerr << "Failed to receive message from server!" << std::endl;
      return -1;
    }

    if (msg.tag == TAG_DELIVERY) {
      size_t colon_pos = msg.data.find(":");
      std::string sender_info = msg.data.substr(colon_pos+1);
      colon_pos = sender_info.find(":");
      std::string sender_name = sender_info.substr(0, colon_pos);
      std::string message = sender_info.substr(colon_pos+1);

      std::cout << sender_name << ": " << message << std::endl;
    } else {
      std::cerr << "Unexpected message: " << msg.to_string();
    }
  }

  return 0;
}
