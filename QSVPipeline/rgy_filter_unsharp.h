﻿// -----------------------------------------------------------------------------------------
// NVEnc by rigaya
// -----------------------------------------------------------------------------------------
//
// The MIT License
//
// Copyright (c) 2014-2016 rigaya
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// ------------------------------------------------------------------------------------------

#include "rgy_filter.h"
#include "rgy_prm.h"
#include <array>

class RGYFilterParamUnsharp : public RGYFilterParam {
public:
    VppUnsharp unsharp;
    RGYFilterParamUnsharp() : unsharp() {};
    virtual ~RGYFilterParamUnsharp() {};
    virtual tstring print() const override { return unsharp.print(); };
};

class RGYFilterUnsharp : public RGYFilter {
public:
    RGYFilterUnsharp(shared_ptr<RGYOpenCLContext> context);
    virtual ~RGYFilterUnsharp();
    virtual RGY_ERR init(shared_ptr<RGYFilterParam> pParam, shared_ptr<RGYLog> pPrintMes) override;
protected:
    virtual RGY_ERR run_filter(const FrameInfo *pInputFrame, FrameInfo **ppOutputFrames, int *pOutputFrameNum, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event) override;
    virtual void close() override;

    virtual RGY_ERR setWeight(unique_ptr<RGYCLBuf> &pGaussWeightBuf, int radius, float sigma);
    virtual RGY_ERR procPlane(FrameInfo *pOutputPlane, const FrameInfo *pInputPlane, const RGYCLBuf *gaussWeightBuf, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event);
    virtual RGY_ERR procFrame(FrameInfo *pOutputPlane, const FrameInfo *pInputPlane, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event);

    bool m_bInterlacedWarn;
    unique_ptr<RGYOpenCLProgram> m_unsharp;
    unique_ptr<RGYCLBuf> m_pGaussWeightBufY;
    unique_ptr<RGYCLBuf> m_pGaussWeightBufUV;
    unique_ptr<RGYCLFrame> m_srcImage;
};
