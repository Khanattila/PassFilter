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

#include "PFVapoursynth.h"
#include "PFDefault.h"
#include "shared/common.h"
#include "shared/startchar.h"

#ifdef _MSC_VER
#    define strcasecmp _stricmp
#endif

#ifdef VAPOURSYNTH_H

//////////////////////////////////////////
// VapourSynthFunctions
inline bool PFVapoursynth::equals(const VSVideoInfo *v, const VSVideoInfo *w) {
    return 
        v->width == w->width && 
        v->height == w->height && 
        v->fpsNum == w->fpsNum &&
        v->fpsDen == w->fpsDen && 
        v->numFrames == w->numFrames && 
        v->format == w->format;
}

//////////////////////////////////////////
// VapourSynthInit
static void VS_CC VapourSynthPluginViInit(VSMap *in, VSMap *out, void **instanceData, 
    VSNode *node, VSCore *core, const VSAPI *vsapi) {

    PFVapoursynth *d = (PFVapoursynth*) * instanceData;
    vsapi->setVideoInfo(d->vi, 1, node);
}

//////////////////////////////////////////
// VapourSynthGetFrame
static const VSFrameRef *VS_CC VapourSynthPluginGetFrame(int n, int activationReason, void **instanceData,
    void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {

    PFVapoursynth *d = (PFVapoursynth*) * instanceData;
    if (activationReason == arInitial) {
        vsapi->requestFrameFilter(n, d->node, frameCtx);
    } else if (activationReason == arAllFramesReady) {
        // Variables
        const VSFrameRef *src = vsapi->getFrameFilter(n, d->node, frameCtx);
        const VSFormat *fi = d->vi->format;
        VSFrameRef *dst = nullptr;
        
        /* Just a template */
 
        // Info
        if (d->info) {
            uint8_t *frm = vsapi->getWritePtr(dst, 0);
            int pitch = vsapi->getStride(dst, 0);
            DrawString(frm, pitch, 0, 0, "PassFilter");
            DrawString(frm, pitch, 0, 1, " Version " VERSION);
            DrawString(frm, pitch, 0, 2, " Copyright(C) Khanattila");
        }

        // Return
        return dst;
    }
    return 0;
}

//////////////////////////////////////////
// VapourSynthFree
static void VS_CC VapourSynthPluginFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
    PFVapoursynth *d = (PFVapoursynth*) instanceData;
    fftwf_destroy_plan(d->dct2);
    fftwf_destroy_plan(d->dct3);
    fftwf_free(d->data0);
    fftwf_free(d->data1);
    vsapi->freeNode(d->node);
    free(d);
}

//////////////////////////////////////////
// VapourSynthCreate
static void VS_CC VapourSynthPluginCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {

    // Check source clip
    PFVapoursynth d;
    d.node = vsapi->propGetNode(in, "clip", 0, 0);
    d.vi = vsapi->getVideoInfo(d.node);

    // Create a new filter and return a reference to it
    PFVapoursynth *data = (PFVapoursynth*) malloc(sizeof(d));
    if (data) *data = d;
    else {
        vsapi->setError(out, "pflt.PFVapoursynth: fatal error!\n (malloc fail)");
        vsapi->freeNode(d.node);
        return;
    }
    vsapi->createFilter(in, out, "PFVapoursynth", VapourSynthPluginViInit, VapourSynthPluginGetFrame, VapourSynthPluginFree,
        fmParallelRequests, 0, data, core);
}


//////////////////////////////////////////
// VapourSynthPluginInit
VS_EXTERNAL_API(void) VapourSynthPluginInit(VSConfigPlugin configFunc, VSRegisterFunction registerFunc, VSPlugin *plugin) {
    configFunc("com.Khanattila.PFVapoursynth", "pflt", "PFVapoursynth for VapourSynth", VAPOURSYNTH_API_VERSION, 1, plugin);
    registerFunc("LowPassFilter", "clip:clip;cutoff_frequency:float:opt;info:int:opt", VapourSynthPluginCreate, nullptr, plugin);
}

#endif //__VAPOURSYNTH_H__