/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Marco Gulino <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "file2image.h"
#include <QImage>
#include <QFileInfo>
#include <Magick++.h>
#include <iostream>

File2Image::File2Image(QImage &image) : _image(image)
{

}

File2Image::~File2Image()
{

}

QImage& File2Image::load(const QString& file,  const QString &fileTypeHint)
{
  _image.load(file);
  if(!_image.isNull())
    return _image;
  Magick::Image image(QString("%1:%2").arg(fileTypeHint).arg(file).toStdString());
  Magick::Blob output;
  image.magick("PNG");
  image.write(&output);
  _image.loadFromData(reinterpret_cast<const uint8_t*>(output.data()), output.length());

  return _image;
}

