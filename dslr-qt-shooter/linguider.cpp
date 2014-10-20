#include "linguider.h"
#include <QtNetwork/QLocalSocket>
#include <stdint.h>
#include <QDebug>

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
    for(int i=0; i<sizeof(command_header); i++) {
        qDebug() << "header_data[" << i << "]: " << int(header_data[i]);
        data.push_back(header_data[i]);
    }

    qDebug() << "data: " << data;
    qDebug() << "write: " << socket.write(data);
    socket.flush();
    socket.waitForReadyRead();
    QByteArray answer = socket.read(1024);
    qDebug() << "Answer: size=" << answer.size() << "; " << answer;
    for(int i=0; i<answer.size(); i++) {
        qDebug() << "answer[" << i << "]: " << int(answer[i]);
    }
}
