#ifndef LINGUIDER_H
#define LINGUIDER_H

#include <QObject>
#include <memory>

class LinGuider : public QObject
{
    Q_OBJECT
public:
    explicit LinGuider(QObject *parent = 0);
    virtual ~LinGuider();
    bool is_connected() const;
    QString version() const;
    QString dither() const;
    void connect();
signals:

public slots:
  
private:
  class Private;
  friend class Private;
  std::unique_ptr<Private> const d;
};

#endif // LINGUIDER_H
