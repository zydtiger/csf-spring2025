#include "guard.h"
#include "message.h"
#include "message_queue.h"
#include "user.h"
#include "room.h"

#include <stdexcept>

Room::Room(const std::string &room_name)
  : room_name(room_name) {
  if (pthread_mutex_init(&lock, NULL) != 0) {
    throw std::runtime_error("Failed to initialize mutex in Room!");
  }
}

Room::~Room() {
  pthread_mutex_destroy(&lock);
}

void Room::add_member(User *user) {
  Guard guard(lock);
  members.insert(user);
}

void Room::remove_member(User *user) {
  Guard guard(lock);
  members.erase(user);
}

void Room::broadcast_message(const std::string &sender_username, const std::string &message_text) {
  Guard guard(lock);
  std::string data = room_name + ":" + sender_username + ":" + message_text;
  Message *msg = new Message(TAG_DELIVERY, data);
  for (User *user : members) {
    user->mqueue.enqueue(msg);
  }
}
