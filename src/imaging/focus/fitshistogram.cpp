/***************************************************************************
                          fitshistogram.cpp  -  FITS Historgram
                          -----------------
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

#include "fitshistogram.h"
#include "fitsimage.h"
#include <cmath>

void FITSHistogram::constructHistogram(int hist_width, int hist_height)
{
    int id;
    double fits_w=0, fits_h=0;
    float *buffer = _image_data->getImageBuffer();
 
    _image_data->getSize(&fits_w, &fits_h);
    _image_data->getMinMax(&fits_min, &fits_max);
 
    int pixel_range = (int) (fits_max - fits_min);
 
    #ifdef HIST_LOG
    qDebug() << "fits MIN: " << fits_min << " - fits MAX: " << fits_max << " - pixel range: " << pixel_range;
    #endif
 
    if (hist_width > histArray.size())
        histArray.resize(hist_width);
 
    cumulativeFreq.resize(histArray.size());
 
    for (int i=0; i < hist_width; i++)
    {
        histArray[i] = 0;
        cumulativeFreq[i] = 0;
    }
 
    binWidth = ((double) hist_width / (double) pixel_range);
 
    #ifdef HIST_LOG
    qDebug() << "Hist Array is now " << hist_width << " wide..., pixel range is " << pixel_range << " Bin width is " << binWidth << endl;
    #endif
 
    if (binWidth == 0 || buffer == NULL)
        return;
 
    for (int i=0; i < fits_w * fits_h ; i++)
    {
        id = (int) round((buffer[i] - fits_min) * binWidth);
 
        if (id >= hist_width)
            id = hist_width - 1;
        else if (id < 0)
            id=0;
 
        histArray[id]++;
    }
 
    // Cumuliative Frequency
    for (int i=0; i < histArray.size(); i++)
        for (int j=0; j <= i; j++)
            cumulativeFreq[i] += histArray[j];
 
    int maxIntensity=0;
    int maxFrequency=histArray[0];
    for (int i=0; i < hist_width; i++)
    {
        if (histArray[i] > maxFrequency)
        {
            maxIntensity = i;
            maxFrequency = histArray[i];
        }
    }
 
 
    double median=0;
    int halfCumulative = cumulativeFreq[histArray.size() - 1]/2;
    for (int i=0; i < histArray.size(); i++)
    {
        if (cumulativeFreq[i] > halfCumulative)
        {
            median = i * binWidth + fits_min;
            break;
 
        }
    }
 
    _image_data->setMedian(median);
    JMIndex = (double) maxIntensity / (double) hist_width;
 
    #ifdef HIST_LOG
    qDebug() << "maxIntensity " << maxIntensity <<  " JMIndex " << JMIndex << endl;
    #endif
 
    // Normalize histogram height. i.e. the maximum value will take the whole height of the widget
    histFactor = ((double) hist_height) / ((double) findMax(hist_width));
 
    histogram_height = hist_height;
    histogram_width = hist_width;
 
    //updateBoxes(fits_min, fits_max);
 
    //ui->histFrame->update();
}

FITSHistogram::FITSHistogram()
{

}

int FITSHistogram::findMax(int hist_width)
{
    double max = -1e9;
 
    for (int i=0; i < hist_width; i++)
        if (histArray[i] > max) max = histArray[i];
 
    return ((int) max);
}
