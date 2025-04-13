#include "message.h"

std::string stripend(std::string s) {
  size_t end = s.length();
  if (end > 0 && s[end-1] == '\n') end--;
  if (end > 0 && s[end-1] == '\r') end--; // clean up cases for \r\n
  return s.substr(0, end);
}

Message::Message(const std::string &tag, const std::string &rawdata)
  : tag(tag) {
  data = stripend(rawdata);
}

Message::Message(const std::string &msg) {
  size_t colon_pos = msg.find(':');

  if (colon_pos == std::string::npos) {
    throw std::runtime_error("Message is invalid!");
  }

  tag = msg.substr(0, colon_pos);
  data = stripend(msg.substr(colon_pos + 1));
}
