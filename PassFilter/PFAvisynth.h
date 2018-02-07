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

#ifndef PF_AVISYNTH_H
#define PF_AVISYNTH_H

#ifdef _WIN32
#    include <avisynth.h>
#endif

#include <fftw3.h>

#ifdef __AVISYNTH_6_H__
struct PFAvisynth : public GenericVideoFilter {
private:
    const double cutoff_frequency;
    const bool info;
    const int filter;
    float *data0, *data1;
    fftwf_plan dct2, dct3;
    bool equals(VideoInfo *v, VideoInfo *w);
public:
    PFAvisynth(PClip _child, const double _cutoff_frequency, const bool _info, const int _filter, IScriptEnvironment *env);
    ~PFAvisynth();
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env);
};
#endif //__AVISYNTH_6_H__

#endif //__PF_AVISYNTH_H__