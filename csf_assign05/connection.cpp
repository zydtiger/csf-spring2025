#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  rio_readinitb(&m_fdbuf, m_fd);
}

void Connection::connect(const std::string &hostname, int port) {
  m_fd = open_clientfd(hostname.c_str(), std::to_string(port).c_str());
  
  if (m_fd < 0) {
    m_last_result = EOF_OR_ERROR;
    return;
  }
  
  rio_readinitb(&m_fdbuf, m_fd);
  m_last_result = SUCCESS;
}

Connection::~Connection() {
  if (is_open()) {
    close();
  }
}

bool Connection::is_open() const {
  return m_fd >= 0;
}

void Connection::close() {
  if (is_open()) {
    Close(m_fd);
    m_fd = -1;
  }
}

bool Connection::send(const Message &msg) {
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if (!is_open()) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  std::string message = msg.to_string();
  ssize_t bytes_written = rio_writen(m_fd, message.c_str(), message.length());
  
  if (bytes_written != static_cast<ssize_t>(message.length())) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  
  m_last_result = SUCCESS;
  return true;
}

bool Connection::receive(Message &msg) {
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if (!is_open()) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  char buf[Message::MAX_LEN + 1];
  ssize_t bytes_read = rio_readlineb(&m_fdbuf, buf, Message::MAX_LEN);
  
  if (bytes_read <= 0) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  buf[bytes_read] = '\0';
  std::string line(buf);
 
  try {
    msg = Message(line);
  } catch (std::runtime_error &) {
    m_last_result = INVALID_MSG;
    return false;
  }

  m_last_result = SUCCESS;
  return true;
}