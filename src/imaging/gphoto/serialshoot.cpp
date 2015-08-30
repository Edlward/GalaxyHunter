/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Marco Gulino <email>
 *
 * Adapted from setSerialSignal v0.1 9/13/01
 * www.embeddedlinuxinterfacing.com
 *
 *
 * The original location of this source is
 * http://www.embeddedlinuxinterfacing.com/chapters/06/setSerialSignal.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "gphoto/serialshoot.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <iostream>
#include <string.h>
#include <sstream>

using namespace std;

class SerialShoot::Private {
public:
  Private(const string &port) : port(port) {}
  const std::string port;
  
  void setStatus(function<void()> f);
  int fd;
  struct termios tio;
  int status;
  bool shooting = false;
};

SerialShoot::SerialShoot(const std::string& port) : d(new Private{port})
{
  shoot();
}

SerialShoot::~SerialShoot()
{
  stop();
}

void SerialShoot::shoot()
{
  cerr << "Starting shoot on port " << d->port << endl;
  d->setStatus([=]{ d->status &= ~TIOCM_DTR; d->status |= TIOCM_RTS; });
  d->shooting = true;
}

void SerialShoot::Private::setStatus(function< void() > f)
{
  fd = open(port.c_str(), O_RDWR); // TODO: check for >= 0
  if(fd < 0) {
    stringstream ss;
    ss << "errno: " << errno;
    char *errdesc = strerror(errno);
    ss << ": " << errdesc;
    cerr << "Error opening " << port << ": " << ss.str();
  }
  tcgetattr(fd, &tio);          /* get the termio information */
  tio.c_cflag &= ~HUPCL;        /* clear the HUPCL bit */
  tcsetattr(fd, TCSANOW, &tio); /* set the termio information */

  ioctl(fd, TIOCMGET, &status); /* get the serial port status */
  f();
  ioctl(fd, TIOCMSET, &status); /* set the serial port status */
  close(fd);
}


void SerialShoot::stop()
{
  if(! d->shooting)
    return;
  cerr << "Ending shoot on port " << d->port << endl;
  d->shooting = false;
  d->setStatus([=]{ d->status &= ~TIOCM_RTS; d->status |= TIOCM_DTR; });
}

