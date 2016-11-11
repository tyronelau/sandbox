#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#include "base/async_pipe.h"
#include "base/atomic.h"
#include "base/event_loop.h"
#include "base/log.h"
#include "base/packer.h"
#include "base/packet.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

using agora::base::async_pipe_reader;
using agora::base::async_pipe_writer;
using agora::base::event_loop;
using agora::base::timer_event;
using agora::base::packer;
using agora::base::unpacker;

using agora::base::pipe_read_listener;
using agora::base::pipe_write_listener;

enum {PROMPT_URI = 2};

DECLARE_PACKET_1(prompt, PROMPT_URI, std::string, content);

class event_writer : private pipe_write_listener {
 public:
  explicit event_writer(int fd);
  virtual ~event_writer();

  int run();
 private:
  virtual bool on_error(async_pipe_writer *writer, int events);
 private:
  void on_timer();

  static void timer_callback(int fd, void *context);
 private:
  int fd_;
  int count_;

  event_loop loop_;
  async_pipe_writer *writer_;
};

event_writer::event_writer(int fd) {
  fd_ = fd;
  count_ = 0;

  writer_ = NULL;
}

event_writer::~event_writer() {
  delete writer_;
}

int event_writer::run() {
  timer_event *timer = loop_.add_timer(50, timer_callback, this);
  (void)timer;

  writer_ = new async_pipe_writer(&loop_, fd_, this);

  loop_.run();
  return 0;
}

void event_writer::timer_callback(int fd, void *context) {
  (void)fd;

  event_writer *writer = reinterpret_cast<event_writer *>(context);
  writer->on_timer();
}

void event_writer::on_timer() {
  count_ = (count_ + 100) % 65535;
  if (count_ < 100)
    count_ = 100;

  count_ = (count_ / 5) * 5 + 1;

  prompt pkt;
  pkt.content.resize(count_);

  char *p = &pkt.content[0];
  char *q = p;

  for (int i = 0; i < count_ / 5; ++i) {
    char str[16];
    snprintf(str, 16, "%05d", i);

    q = p + i * 5;
    memcpy(q, str, 5);
  }

  pkt.content.back() = '\n';

  writer_->write_packet(pkt);
}

bool event_writer::on_error(async_pipe_writer *writer, int events) {
  (void)writer;
  cerr << "Error in writer: " << events;

  loop_.stop();
  return true;
}

class event_reader : private pipe_read_listener {
 public:
  explicit event_reader(int fd);
  virtual ~event_reader();

  int run();
 private:
  virtual bool on_receive_packet(async_pipe_reader *reader, unpacker &pkr,
      uint16_t uri);

  virtual bool on_error(async_pipe_reader *reader, int events);
 private:
  int fd_;

  event_loop loop_;
  async_pipe_reader *reader_;
};

event_reader::event_reader(int fd) {
  fd_ = fd;
  reader_ = NULL;
}

event_reader::~event_reader() {
  delete reader_;
}

int event_reader::run() {
  reader_ = new async_pipe_reader(&loop_, fd_, this);

  loop_.run();
  return 0;
}

bool event_reader::on_receive_packet(async_pipe_reader *reader, unpacker &pkr,
    uint16_t uri) {
  (void)reader;

  assert(uri = PROMPT_URI);
  prompt pkt;
  pkt.unmarshall(pkr);

  cout << "Prompt: " << pkt.content.size() << endl;

  return true;
}

bool event_reader::on_error(async_pipe_reader *reader, int events) {
  (void) reader;
  cerr << "Error in reader: " << events;

  loop_.stop();
  return true;
}

int main(int argc, char * const argv[]) {
  (void)argc;
  (void)argv;

  int reader_fds[2];
  if (pipe(reader_fds) != 0) {
    cerr << "Failed to create a pipe for read: "
        << strerror(errno) << endl;
    return -errno;
  }

  pid_t pid = fork();
  if (pid < 0) {
    cerr << "Failed to fork the process: " << strerror(errno) << endl;
    return -errno;
  } else if (pid == 0) {
    close(reader_fds[0]);
    event_writer writer(reader_fds[1]);
    writer.run();
  } else {
    close(reader_fds[1]);
    event_reader reader(reader_fds[0]);
    reader.run();
  }

  return 0;
}
