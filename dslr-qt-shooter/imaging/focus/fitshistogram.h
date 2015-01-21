/***************************************************************************
                          fitshistogram.h  -  FITS Historgram
                          ---------------
    begin                : Thu Mar 4th 2004
    copyright            : (C) 2004 by Jasem Mutlaq
    email                : mutlaqja@ikarustech.com
 ***************************************************************************/
 
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Marco Gulino <marco.gulino@bhuman.it>
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

#ifndef FITSHISTOGRAM_H
#define FITSHISTOGRAM_H
#include <QVarLengthArray>
class FITSImage;
class FITSHistogram

{
public:
  static const int INITIAL_MAXIMUM_WIDTH = 500;
  FITSHistogram();
  void constructHistogram(int hist_width, int hist_height);
  void setImage( FITSImage *image_data) { _image_data = image_data; }
  double getJMIndex() { return JMIndex; }
  
private:
   FITSImage *_image_data;
   double JMIndex;
   double fits_min, fits_max;
   QVarLengthArray<int, INITIAL_MAXIMUM_WIDTH> histArray;
   QVarLengthArray<int, INITIAL_MAXIMUM_WIDTH> cumulativeFreq;
   double binWidth;
   double histFactor;
   int  findMax(int hist_width);
   int histogram_height, histogram_width;
};

#endif // FITSHISTOGRAM_H
