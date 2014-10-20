#include "linguider.h"
#include <QtNetwork/QUdpSocket>

LinGuider::LinGuider(QObject *parent) :
    QObject(parent)
{
    QUdpSocket socket;
}
