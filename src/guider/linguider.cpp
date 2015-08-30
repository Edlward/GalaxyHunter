#include "linguider.h"
#include <QtNetwork/QLocalSocket>
#include <stdint.h>
#include <QDebug>
#include <algorithm>

struct command_header {
    uint16_t signature;
    uint16_t command;
    uint32_t data_length;
    command_header(uint16_t command, uint32_t data_length) : signature(2), command(command), data_length(data_length) {}
    command_header() = default;
} __attribute__ ((packed));

struct get_version : public command_header {
    get_version() : command_header(1, 0) {}
};
struct dither : public command_header {
    dither() : command_header(4, 0) {}
};

template<class T> struct command {
  T header;
  QByteArray data;
};

class LinGuider::Private {
public:
  Private() = default;
  QLocalSocket socket;
  template<class T> command<T> send_command(command<T> c);
};

LinGuider::LinGuider(QObject *parent) :
    QObject(parent), d(new Private)
{
}

LinGuider::~LinGuider()
{
}

void LinGuider::connect()
{
  qDebug() << __PRETTY_FUNCTION__;
  if(!is_connected())
    d->socket.connectToServer("/tmp/lg_ss");
}


template<class T> command<T> LinGuider::Private::send_command(command<T> c)
{
  QByteArray buffer;
  char *header = reinterpret_cast<char*>(&c.header);
  std::copy(header, header+sizeof(c.header), std::back_inserter(buffer));
  socket.write(buffer);
  socket.waitForBytesWritten();
  socket.waitForReadyRead();
  buffer = socket.read(sizeof(c.header));
  std::copy(buffer.begin(), buffer.end(), header);
  c.data = socket.read(c.header.data_length);
  return c;
}

QString LinGuider::dither() const
{
  if(!is_connected()) {
    return {};
  }
  auto response = d->send_command<::dither>({});
  return {response.data};
}

QString LinGuider::version() const
{
  if(!is_connected()) {
    return {};
  }
  auto version_response = d->send_command<get_version>({});
  return {version_response.data};
}

bool LinGuider::is_connected() const
{
  return d->socket.isOpen();
}

