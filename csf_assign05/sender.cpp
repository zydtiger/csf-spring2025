#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  Connection conn;
  Message msg;

  conn.connect(server_hostname, server_port);

  // Sender login
  if (!conn.send(Message(TAG_SLOGIN, username))) {
    std::cerr << "Sender login failed to send!" << std::endl;
    return -1;
  }

  // Wait til server responds
  if (!conn.receive(msg)) {
    std::cerr << "Failed to receive message from server!" << std::endl;
    return -1;
  }
  if (msg.tag != TAG_OK) {
    std::cerr << msg.data << std::endl;
    return -1;
  }

  // Loop reading commands from user, sending messages to
  // server as appropriate
  std::string line;
  bool joined_room = false;
  while (true) {
    printf("> ");
    fflush(stdout);

    getline(std::cin, line);
    line = stripend(line);

    // Join room
    if (line.length() >= 5 && line.substr(0, 5) == "/join") {
      size_t space_pos = line.find(' ');
      if (space_pos == std::string::npos) {
        std::cerr << "Room name cannot be empty!" << std::endl;
        continue;
      }
      std::string room_name = line.substr(space_pos+1);
      if (room_name.empty()) {
        std::cerr << "Room name cannot be empty!" << std::endl;
        continue;
      }

      if (joined_room) { // leave room first
        if (!conn.send(Message(TAG_LEAVE, ""))) {
          std::cerr << "Leave room request failed to send!" << std::endl;
          continue;
        }
        if (!conn.receive(msg)) {
          std::cerr << "Failed to receive message from server!" << std::endl;
          continue;
        }
        if (msg.tag != TAG_OK) {
          std::cerr << msg.data << std::endl;
          continue;
        }
      }

      if (!conn.send(Message(TAG_JOIN, room_name))) {
        std::cerr << "Join room request failed to send!" << std::endl;
        continue;
      }
      if (!conn.receive(msg)) {
        std::cerr << "Failed to receive message from server!" << std::endl;
        continue;
      }
      if (msg.tag != TAG_OK) {
        std::cerr << msg.data << std::endl;
        continue;
      }

      joined_room = true;
    }

    // Leave room
    else if (line == "/leave") {
      if (!conn.send(Message(TAG_LEAVE, ""))) {
        std::cerr << "Leave room request failed to send!" << std::endl;
        continue;
      }
      if (!conn.receive(msg)) {
        std::cerr << "Failed to receive message from server!" << std::endl;
        continue;
      }
      if (msg.tag != TAG_OK) {
        std::cerr << msg.data << std::endl;
        continue;
      }

      joined_room = false;
    }

    // Quit
    else if (line == "/quit") {
      if (!conn.send(Message(TAG_QUIT, ""))) {
        std::cerr << "Quit request failed to send!" << std::endl;
        continue;
      }
      if (!conn.receive(msg)) {
        std::cerr << "Failed to receive message from server!" << std::endl;
        continue;
      }
      if (msg.tag != TAG_OK) {
        std::cerr << msg.data << std::endl;
        continue;
      }

      break;
    }

    // Send to room
    else if (joined_room) {
      if (!conn.send(Message(TAG_SENDALL, line))) {
        std::cerr << "Message to room failed to send!" << std::endl;
        continue;
      }
      if (!conn.receive(msg)) {
        std::cerr << "Failed to receive message from server!" << std::endl;
        continue;
      }
      if (msg.tag != TAG_OK) {
        std::cerr << msg.data << std::endl;
        continue;
      }
    }

    else {
      std::cerr << "You must use `/join [room_name]` first!" << std::endl;
    }
  }

  conn.close();
  return 0;
}
