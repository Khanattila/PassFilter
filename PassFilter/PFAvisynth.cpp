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

#include "PFAvisynth.h"
#include "PFDefault.h"
#include "shared/common.h"
#include "shared/startchar.h"

#include <algorithm>
#include <cmath>

#ifdef _MSC_VER
#    define strcasecmp _stricmp
#endif

#ifdef __AVISYNTH_6_H__

//////////////////////////////////////////
// AviSynthFunctions
inline bool PFAvisynth::equals(VideoInfo *v, VideoInfo *w) {
    return 
        v->width == w->width && 
        v->height == w->height && 
        v->fps_numerator == w->fps_numerator &&
        v->fps_denominator == w->fps_denominator && 
        v->num_frames == w->num_frames &&
        v->pixel_type == w->pixel_type &&
        v->sample_type == w->sample_type;
}

//////////////////////////////////////////
// AviSynthInit
PFAvisynth::PFAvisynth(PClip _child, const double _cutoff_frequency, const bool _info, const int _filter, 
    IScriptEnvironment *env) : GenericVideoFilter(_child), cutoff_frequency(_cutoff_frequency), info(_info), filter(_filter) {

    // Check AviSynth Version
    env->CheckVersion(5);

    // Check source clip
    if (vi.IsRGB() || !vi.IsPlanar())
        env->ThrowError("PassFilter: Only planar YUV or YUVA are supported!");

    // Checks user value
    if (cutoff_frequency < 0)
        env->ThrowError("PassFilter: 'cutoff_frequency' must be greater than 0!");
    if ((info && vi.IsRGB()) || (info && vi.BitsPerComponent() != 8))
        env->ThrowError("PassFilter: 'info' requires Gray8 or YUV-P8 color space!");

    // Allocates FFTW data
    data0 = fftwf_alloc_real(vi.width * vi.height);
    data1 = fftwf_alloc_real(vi.width * vi.height);
    dct2 = fftwf_plan_r2r_2d(vi.width, vi.height, data0, data1, FFTW_REDFT10, FFTW_REDFT10, FFTW_ESTIMATE);
    dct3 = fftwf_plan_r2r_2d(vi.width, vi.height, data1, data0, FFTW_REDFT01, FFTW_REDFT01, FFTW_ESTIMATE);

}

//////////////////////////////////////////
// AviSynthGetFrame
PVideoFrame __stdcall PFAvisynth::GetFrame(int n, IScriptEnvironment* env) {
    // Variables
    PVideoFrame src_frame = child->GetFrame(n, env);
    PVideoFrame dst_frame = env->NewVideoFrame(vi);
    float exp_cutoff = (float) exp(cutoff_frequency);
    
    // Write data   
    const uint8_t *srcY = src_frame->GetReadPtr(PLANAR_Y);
    int src_pitch = src_frame->GetPitch(PLANAR_Y);
    for (int y = 0; y < vi.height; y++) {
        for (int x = 0; x < vi.width; x++) {
            data0[y * vi.width + x] = srcY[y * src_pitch + x];
        }
    }

    // DCT2
    fftwf_execute(dct2);

    // Filter
    for (int e = 0; e < (vi.width * vi.height); e++) {
        data1[e] = std::min(exp_cutoff, data1[e]);
    }

    // DCT3
    fftwf_execute(dct3);

    // Read data
    uint8_t *dstY = dst_frame->GetWritePtr(PLANAR_Y);
    int dst_pitch = dst_frame->GetPitch(PLANAR_Y);
    for (int y = 0; y < vi.height; y++) {
        for (int x = 0; x < vi.width; x++) {
            dstY[y * dst_pitch + x] = (uint8_t) data0[y * vi.width + x];
        }
    }

    // Copy chroma
    if (!vi.IsY8()) {
        env->BitBlt(dst_frame->GetWritePtr(PLANAR_U), dst_frame->GetPitch(PLANAR_U), src_frame->GetReadPtr(PLANAR_U),
            src_frame->GetPitch(PLANAR_U), src_frame->GetRowSize(PLANAR_U), src_frame->GetHeight(PLANAR_U));
        env->BitBlt(dst_frame->GetWritePtr(PLANAR_V), dst_frame->GetPitch(PLANAR_V), src_frame->GetReadPtr(PLANAR_V),
            src_frame->GetPitch(PLANAR_V), src_frame->GetRowSize(PLANAR_V), src_frame->GetHeight(PLANAR_V));
    }

    // Info
    if (info) {
        uint8_t *frm = dst_frame->GetWritePtr(PLANAR_Y);
        int pitch = dst_frame->GetPitch(PLANAR_Y);
        DrawString(frm, pitch, 0, 0, "PassFilter");
        DrawString(frm, pitch, 0, 1, " Version " VERSION);
        DrawString(frm, pitch, 0, 2, " Copyright(C) Khanattila");
    }

    // Return
    return dst_frame;
}

//////////////////////////////////////////
// AviSynthFree
PFAvisynth::~PFAvisynth() {
    fftwf_destroy_plan(dct2);
    fftwf_destroy_plan(dct3);
    fftwf_free(data0);
    fftwf_free(data1);
}

//////////////////////////////////////////
// AviSynthCreate
AVSValue __cdecl AviSynthPluginCreateLowPass(AVSValue args, void* user_data, IScriptEnvironment* env) {
    return new PFAvisynth(
        args[0].AsClip(), 
        args[1].AsFloat(DFT_cutoff_frequency),
        args[2].AsBool(DFT_info), 
        PF_LOW_PASS, env);
}

//////////////////////////////////////////
// AviSynthPluginInit
const AVS_Linkage *AVS_linkage = 0;
extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(
    IScriptEnvironment* env, const AVS_Linkage * const vectors) {

    AVS_linkage = vectors;
    env->AddFunction("LowPassFilter", "c[cutoff_frequency]f[info]b", AviSynthPluginCreateLowPass, 0);
    if (env->FunctionExists("SetFilterMTMode")) {
        static_cast<IScriptEnvironment2*>(env)->SetFilterMTMode("LowPassFilter", MT_MULTI_INSTANCE, true);
    }
    return "PassFilter for AviSynth";
}

#endif //__AVISYNTH_6_H__