#include "linguider.h"
#include <QtNetwork/QLocalSocket>
#include <stdint.h>
#include <QDebug>
#include <algorithm>

struct command_header {
    uint16_t signature;
    uint16_t command;
    uint32_t data_length;
} __attribute__ ((packed));

LinGuider::LinGuider(QObject *parent) :
    QObject(parent)
{
    QLocalSocket socket;
    socket.connectToServer("/tmp/lg_ss");
    qDebug() << "socket: " << socket.state();
    command_header header{2, 1, 0 };
    QByteArray data;
    char *header_data = reinterpret_cast<char*>(&header);
    std::copy(header_data, header_data+sizeof(header), std::back_inserter(data));
    socket.write(data);
    socket.waitForBytesWritten();
    socket.waitForReadyRead();
    QByteArray answer = socket.read(sizeof(header));
    std::copy(answer.begin(), answer.end(), header_data);
    answer = socket.read(header.data_length);
    qDebug() << "Answer: signature=" << header.signature << ", command=" << header.command << ", data_length=" << header.data_length;
    qDebug() << "answer: " << answer;
}
