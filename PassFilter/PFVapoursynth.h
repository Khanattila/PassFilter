/*
*    This file is part of PassFilter,
*    Copyright(C) 2018 Edoardo Brunetti.
*
*    PassFilter is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    PassFilter is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with PassFilter. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PF_VAPOURSYNTH_H
#define PF_VAPOURSYNTH_H

#include <VapourSynth.h>
#include <VSHelper.h>
#include <fftw3.h>

#ifdef VAPOURSYNTH_H
typedef struct PFVapoursynth {
public:
    VSNodeRef *node;
    const VSVideoInfo *vi;
    double cutoff_frequency;
    bool info;
    int filter;
    float *data0, *data1;
    fftwf_plan dct2, dct3;
    bool equals(const VSVideoInfo *v, const VSVideoInfo *w);
} PFVapoursynth;
#endif //__VAPOURSYNTH_H__

#endif //__PF_VAPOURSYNTH_H__ */