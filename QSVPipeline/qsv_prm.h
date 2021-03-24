﻿// -----------------------------------------------------------------------------------------
// QSVEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2011-2016 rigaya
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
// --------------------------------------------------------------------------------------------

#ifndef _QSV_PRM_H_
#define _QSV_PRM_H_

#include <climits>
#include <vector>
#include "rgy_version.h"
#include "mfxcommon.h"
#include "mfxvp8.h"
#include "mfxvp9.h"
#include "mfxstructures.h"
#include "convert_csp.h"
#include "rgy_caption.h"
#include "rgy_simd.h"
#include "rgy_prm.h"

#define QSVENCC_ABORT_EVENT _T("QSVEncC_abort_%u")

enum {
    MFX_DEINTERLACE_NONE        = 0,
    MFX_DEINTERLACE_NORMAL      = 1,
    MFX_DEINTERLACE_IT          = 2, //inverse telecine, to 24fps
    MFX_DEINTERLACE_BOB         = 3,
    MFX_DEINTERLACE_IT_MANUAL   = 4, //inverse telecine, manual select
    MFX_DEINTERLACE_AUTO_SINGLE = 5,
    MFX_DEINTERLACE_AUTO_DOUBLE = 6,
};

enum {
    MVC_DISABLED          = 0x0,
    MVC_ENABLED           = 0x1,
    MVC_VIEWOUTPUT        = 0x2,    // 2 output bitstreams
};

enum MemType {
    SYSTEM_MEMORY = 0x00,
    VA_MEMORY     = 0x01,
    D3D9_MEMORY   = 0x01,
    D3D11_MEMORY  = 0x02,
    HW_MEMORY     = D3D9_MEMORY | D3D11_MEMORY,
};

static MemType operator|(MemType a, MemType b) {
    return (MemType)((uint32_t)a | (uint32_t)b);
}

static MemType operator|=(MemType& a, MemType b) {
    a = a | b;
    return a;
}

static MemType operator&(MemType a, MemType b) {
    return (MemType)((uint32_t)a & (uint32_t)b);
}

static MemType operator&=(MemType& a, MemType b) {
    a = (MemType)((uint32_t)a & (uint32_t)b);
    return a;
}

enum {
    FPS_CONVERT_NONE = 0,
    FPS_CONVERT_MUL2,
    FPS_CONVERT_MUL2_5,
};

enum {
    QSV_VPP_SUB_SIMPLE = 0,
    QSV_VPP_SUB_COMPLEX,
};

struct VppDenoise {
    bool enable;
    int strength; // 0 - 100

    VppDenoise();
    ~VppDenoise() {};
};

struct VppMCTF {
    bool enable;
    int strength; // 0 - 20

    VppMCTF();
    ~VppMCTF() {};
};

struct VppDetailEnhance {
    bool enable;
    int strength; // 0 - 100

    VppDetailEnhance();
    ~VppDetailEnhance() {};
};

enum class VppType : int {
    VPP_NONE,

    MFX_COLORSPACE,
    MFX_CROP,
    MFX_ROTATE,
    MFX_MIRROR,
    MFX_DEINTERLACE,
    MFX_MCTF,
    MFX_DENOISE,
    MFX_RESIZE,
    MFX_DETAIL_ENHANCE,
    MFX_IMAGE_STABILIZATION,
    MFX_FPS_CONV,
    MFX_COPY,

    MFX_MAX,

    CL_MIN = MFX_MAX,

    CL_CROP,
    CL_AFS,
    CL_NNEDI,
    CL_YADIF,
    CL_COLORSPACE,
    CL_DELOGO,
    CL_TRANSFORM,

    CL_DENOISE_PMD,
    CL_DENOISE_KNN,
    CL_DENOISE_SMOOTH,

    CL_RESIZE,

    CL_SUBBURN,

    CL_UNSHARP,
    CL_EDGELEVEL,
    CL_WARPSHARP,

    CL_TWEAK,

    CL_DEBAND,

    CL_PAD,

    CL_MAX,
};

enum class VppFilterType { FILTER_NONE, FILTER_MFX, FILTER_OPENCL };

static VppFilterType getVppFilterType(VppType vpptype) {
    if (vpptype == VppType::VPP_NONE) return VppFilterType::FILTER_NONE;
    if (vpptype < VppType::MFX_MAX) return VppFilterType::FILTER_MFX;
    if (vpptype < VppType::CL_MAX) return VppFilterType::FILTER_OPENCL;
    return VppFilterType::FILTER_NONE;
}

struct sVppParams {
    bool bEnable;             //use vpp

    bool bUseResize;          //use vpp resizer
    int resizeInterp;
    int resizeMode;

    int deinterlace;      //set deinterlace mode
    int telecinePattern;

    int imageStabilizer;  //MFX_IMAGESTAB_MODE_UPSCALE, MFX_IMAGESTAB_MODE_BOXED
    int fpsConversion;    //FPS_CONVERT_xxxx

    int rotate;
    bool halfTurn;
    int mirrorType;

    bool useProAmp;          //not supported

    VppDenoise denoise;
    VppMCTF mctf;
    VppDetailEnhance detail;

    sVppParams();
    ~sVppParams() {};
};

struct sInputParams {
    VideoInfo input;              //入力する動画の情報
    RGYParamCommon common;
    RGYParamControl ctrl;
    RGYParamVpp vpp;
    sVppParams vppmfx;

    int nEncMode;      // RateControl
    int nTargetUsage;  // Quality
    uint32_t CodecId;       // H.264 only for this
    int CodecProfile;
    int CodecLevel;
    int outputDepth;
    RGY_CHROMAFMT outputCsp;
    int nIdrInterval;  // Idr frame interval to I frame, not supported
    int nGOPLength;    // (Max) GOP Length
    bool bopenGOP;      // if false, GOP_CLOSED is set
    bool bforceGOPSettings; // if true, GOP_STRICT is set
    int nBframes;      // set sequential Bframes num, -1 is auto.
    int nRef;          // set ref frames num.
    int nBitRate;
    int nMaxBitrate;
    int VBVBufsize;
    int nQPI;          // QP for I frames
    int nQPP;          // QP for P frames
    int nQPB;          // QP for B frames
    int nQPMin[3];
    int nQPMax[3];
    int nAVBRAccuarcy;    // param for AVBR algorithm, for API v1.3
    int nAVBRConvergence; // param for AVBR algorithm, for API v1.3

    int         nICQQuality;
    int         nQVBRQuality;

    int        nSlices;       // number of slices, 0 is auto

    uint32_t ColorFormat;   //YV12 or NV12

    uint32_t memType;       //use d3d surface

    int nInputBufSize; //input buf size

    int        nPAR[2]; //PAR比
    bool       bCAVLC;  //CAVLC
    int        nInterPred;
    int        nIntraPred;
    bool       bRDO;
    int        nMVPrecision;
    std::pair<int,int> MVSearchWindow;

    int        MVC_flags;
    int        nBluray;

    bool       bMBBRC;
    bool       extBRC;
    bool       extBrcAdaptiveLTR;

    int        nLookaheadDepth;
    int        nTrellis;

    int        nAsyncDepth;
    int        nOutputBufSizeMB;

    bool       bBPyramid;
    bool       bAdaptiveI;
    bool       bAdaptiveB;
    int        nLookaheadDS;

    bool       bDisableTimerPeriodTuning;

    bool       bIntraRefresh;
    bool       bNoDeblock;

    int        nWinBRCSize;

    int        nMVCostScaling;
    bool       bDirectBiasAdjust;
    bool       bGlobalMotionAdjust;
    bool       bUseFixedFunc;

    int        nSessionThreads;
    int        nSessionThreadPriority;

    int        nVP8Sharpness;

    int        nWeightP;
    int        nWeightB;
    int        nFadeDetect;

    uint32_t   nFallback;
    bool       bOutputAud;
    bool       bOutputPicStruct;
    int16_t    pQPOffset[8];

    int        nRepartitionCheck;
    int8_t     padding[2];

    int        hevc_ctu;
    int        hevc_sao;
    int        hevc_tskip;
    int        hevc_tier;

    tstring    pythonPath;

    bool       bBenchmark;
    mfxU32     nBenchQuality; //ベンチマークの対象

    sInputParams();
    ~sInputParams();
};

enum {
    MFX_CODEC_RAW = MFX_MAKEFOURCC('R', 'A', 'W', ' '),
};

const CX_DESC list_codec[] = {
    { _T("h264"),     MFX_CODEC_AVC   },
    { _T("hevc"),     MFX_CODEC_HEVC  },
    { _T("mpeg2"),    MFX_CODEC_MPEG2 },
    { _T("vc-1"),     MFX_CODEC_VC1   },
    { _T("vp8"),      MFX_CODEC_VP8   },
    { _T("raw"),      MFX_CODEC_RAW   },
    { NULL, 0 }
};

const CX_DESC list_rc_mode[] = {
    { _T("CBR"),     MFX_RATECONTROL_CBR    },
    { _T("VBR"),     MFX_RATECONTROL_VBR    },
    { _T("CQP"),     MFX_RATECONTROL_CQP    },
    { _T("AVBR"),    MFX_RATECONTROL_AVBR   },
    { _T("LA"),      MFX_RATECONTROL_LA     },
    { _T("LA-EXT"),  MFX_RATECONTROL_LA_EXT },
    { _T("LA-HRD"),  MFX_RATECONTROL_LA_HRD },
    { _T("LA-ICQ"),  MFX_RATECONTROL_LA_ICQ },
    { _T("ICQ"),     MFX_RATECONTROL_ICQ    },
    { _T("QVBR"),    MFX_RATECONTROL_QVBR   },
    { _T("VCM"),     MFX_RATECONTROL_VCM    },
    { NULL, 0 }
};

const CX_DESC list_avc_profile[] = {
    { _T("auto"),     0                        },
    { _T("Baseline"), MFX_PROFILE_AVC_BASELINE },
    { _T("Main"),     MFX_PROFILE_AVC_MAIN     },
    { _T("High"),     MFX_PROFILE_AVC_HIGH     },
    { NULL, 0 }
};

const CX_DESC list_hevc_profile[] = {
    { _T("auto"),     0                       },
    { _T("main"),     MFX_PROFILE_HEVC_MAIN   },
    { _T("main10"),   MFX_PROFILE_HEVC_MAIN10 },
    { _T("main444"),  MFX_PROFILE_HEVC_REXT   },
    { NULL, 0 }
};

const CX_DESC list_hevc_tier[] = {
    { _T("main"),   MFX_TIER_HEVC_MAIN },
    { _T("high"),   MFX_TIER_HEVC_HIGH },
    { NULL, 0 }
};

const CX_DESC list_mpeg2_profile[] = {
    { _T("auto"),     0                        },
    { _T("Simple"),   MFX_PROFILE_MPEG2_SIMPLE },
    { _T("Main"),     MFX_PROFILE_MPEG2_MAIN   },
    { _T("High"),     MFX_PROFILE_MPEG2_HIGH   },
    { NULL, 0 }
};

const CX_DESC list_vc1_profile[] = {
    { _T("auto"),     0                        },
    { _T("Simple"),   MFX_PROFILE_VC1_SIMPLE   },
    { _T("Main"),     MFX_PROFILE_VC1_MAIN     },
    { _T("Advanced"), MFX_PROFILE_VC1_ADVANCED },
    { NULL, 0 }
};

const CX_DESC list_vp8_profile[] = {
    { _T("auto"),     0 },
    { _T("0"),        MFX_PROFILE_VP8_0 },
    { _T("1"),        MFX_PROFILE_VP8_1 },
    { _T("2"),        MFX_PROFILE_VP8_2 },
    { _T("3"),        MFX_PROFILE_VP8_3 },
    { NULL, 0 }
};

const CX_DESC list_vp9_profile[] = {
    { _T("auto"),     0 },
    { _T("0"),        MFX_PROFILE_VP9_0 },
    { _T("1"),        MFX_PROFILE_VP9_1 },
    { _T("2"),        MFX_PROFILE_VP9_2 },
    { _T("3"),        MFX_PROFILE_VP9_3 },
    { NULL, 0 }
};

const CX_DESC list_output_depth[] = {
    { _T("8"),     8 },
    { _T("10"),   10 },
    { _T("12"),   12 },
    { NULL, 0 }
};

const CX_DESC list_output_csp[] = {
    { _T("i420"), RGY_CHROMAFMT_YUV420 },
    { _T("i422"), RGY_CHROMAFMT_YUV422 },
    { _T("i444"), RGY_CHROMAFMT_YUV444 },
    { NULL, 0 }
};

const CX_DESC list_interlaced_mfx[] = {
    { _T("progressive"),     MFX_PICSTRUCT_PROGRESSIVE },
    { _T("interlaced(tff)"), MFX_PICSTRUCT_FIELD_TFF   },
    { _T("interlaced(bff)"), MFX_PICSTRUCT_FIELD_BFF   },
    { NULL, 0 }
};

const CX_DESC list_deinterlace[] = {
    { _T("none"),      MFX_DEINTERLACE_NONE        },
    { _T("normal"),    MFX_DEINTERLACE_NORMAL      },
    { _T("it"),        MFX_DEINTERLACE_IT          },
    { _T("bob"),       MFX_DEINTERLACE_BOB         },
#if ENABLE_ADVANCED_DEINTERLACE
    { _T("it-manual"), MFX_DEINTERLACE_IT_MANUAL   },
    { _T("auto"),      MFX_DEINTERLACE_AUTO_SINGLE },
    { _T("auto-bob"),  MFX_DEINTERLACE_AUTO_DOUBLE },
#endif
    { NULL, 0 }
};

const CX_DESC list_telecine_patterns[] = {
    { _T("32"),     MFX_TELECINE_PATTERN_32 },
    { _T("2332"),   MFX_TELECINE_PATTERN_2332 },
    { _T("repeat"), MFX_TELECINE_PATTERN_FRAME_REPEAT },
    { _T("41"),     MFX_TELECINE_PATTERN_41 },
    { NULL, 0 }
};

const CX_DESC list_avc_level[] = {
    { _T("auto"), 0                 },
    { _T("1"),    MFX_LEVEL_AVC_1   },
    { _T("1b"),   MFX_LEVEL_AVC_1b  },
    { _T("1.1"),  MFX_LEVEL_AVC_11  },
    { _T("1.2"),  MFX_LEVEL_AVC_12  },
    { _T("1.3"),  MFX_LEVEL_AVC_13  },
    { _T("2"),    MFX_LEVEL_AVC_2   },
    { _T("2.1"),  MFX_LEVEL_AVC_21  },
    { _T("2.2"),  MFX_LEVEL_AVC_22  },
    { _T("3"),    MFX_LEVEL_AVC_3   },
    { _T("3.1"),  MFX_LEVEL_AVC_31  },
    { _T("3.2"),  MFX_LEVEL_AVC_32  },
    { _T("4"),    MFX_LEVEL_AVC_4   },
    { _T("4.1"),  MFX_LEVEL_AVC_41  },
    { _T("4.2"),  MFX_LEVEL_AVC_42  },
    { _T("5"),    MFX_LEVEL_AVC_5   },
    { _T("5.1"),  MFX_LEVEL_AVC_51  },
    { _T("5.2"),  MFX_LEVEL_AVC_52  },
    { _T("6"),    MFX_LEVEL_AVC_6   },
    { _T("6.1"),  MFX_LEVEL_AVC_61  },
    { _T("6.2"),  MFX_LEVEL_AVC_62  },
    { NULL, 0 }
};
const CX_DESC list_hevc_level[] = {
    { _T("auto"), 0                 },
    { _T("1"),    MFX_LEVEL_HEVC_1   },
    { _T("2"),    MFX_LEVEL_HEVC_2   },
    { _T("2.1"),  MFX_LEVEL_HEVC_21  },
    { _T("3"),    MFX_LEVEL_HEVC_3   },
    { _T("3.1"),  MFX_LEVEL_HEVC_31  },
    { _T("4"),    MFX_LEVEL_HEVC_4   },
    { _T("4.1"),  MFX_LEVEL_HEVC_41  },
    { _T("5"),    MFX_LEVEL_HEVC_5   },
    { _T("5.1"),  MFX_LEVEL_HEVC_51  },
    { _T("5.2"),  MFX_LEVEL_HEVC_52  },
    { _T("6"),    MFX_LEVEL_HEVC_6   },
    { _T("6.1"),  MFX_LEVEL_HEVC_61  },
    { _T("6.2"),  MFX_LEVEL_HEVC_62  },
    { NULL, 0 }
};
const CX_DESC list_mpeg2_level[] = {
    { _T("auto"),     0                        },
    { _T("low"),      MFX_LEVEL_MPEG2_LOW      },
    { _T("main"),     MFX_LEVEL_MPEG2_MAIN     },
    { _T("high"),     MFX_LEVEL_MPEG2_HIGH     },
    { _T("high1440"), MFX_LEVEL_MPEG2_HIGH1440 },
    { NULL, 0 }
};
const CX_DESC list_vc1_level[] = {
    { _T("auto"),     0                    },
    { _T("low"),      MFX_LEVEL_VC1_LOW    },
    { _T("median"),   MFX_LEVEL_VC1_MEDIAN },
    { _T("high"),     MFX_LEVEL_VC1_HIGH   },
    { NULL, 0 }
};
const CX_DESC list_vc1_level_adv[] = {
    { _T("auto"),  0               },
    { _T("0"),     MFX_LEVEL_VC1_0 },
    { _T("1"),     MFX_LEVEL_VC1_1 },
    { _T("2"),     MFX_LEVEL_VC1_2 },
    { _T("3"),     MFX_LEVEL_VC1_3 },
    { _T("4"),     MFX_LEVEL_VC1_4 },
    { NULL, 0 }
};
const CX_DESC list_vp8_level[] = {
    { _T("auto"),     0                    },
    { NULL, 0 }
};
const CX_DESC list_vp9_level[] = {
    { _T("auto"),     0                    },
    { NULL, 0 }
};
const CX_DESC list_avc_trellis[] = {
    { _T("Auto"),           MFX_TRELLIS_UNKNOWN },
    { _T("off"),            MFX_TRELLIS_OFF },
    { _T("for I frames"),   MFX_TRELLIS_I   },
    { _T("for I,P frames"), MFX_TRELLIS_I | MFX_TRELLIS_P },
    { _T("for All frames"), MFX_TRELLIS_I | MFX_TRELLIS_P | MFX_TRELLIS_B },
    { NULL, 0 }
};
const CX_DESC list_avc_trellis_for_options[] = {
    { _T("auto"), MFX_TRELLIS_UNKNOWN },
    { _T("off"),  MFX_TRELLIS_OFF },
    { _T("i"),    MFX_TRELLIS_I   },
    { _T("ip"),   MFX_TRELLIS_I | MFX_TRELLIS_P },
    { _T("all"),  MFX_TRELLIS_I | MFX_TRELLIS_P | MFX_TRELLIS_B },
    { _T("ipb"),  MFX_TRELLIS_I | MFX_TRELLIS_P | MFX_TRELLIS_B },
    { _T("ib"),   MFX_TRELLIS_I | MFX_TRELLIS_B },
    { _T("p"),    MFX_TRELLIS_P },
    { _T("pb"),   MFX_TRELLIS_P | MFX_TRELLIS_B },
    { _T("b"),    MFX_TRELLIS_B },
    { NULL, 0 }
};

const CX_DESC list_lookahead_ds[] = {
    { _T("auto"),   MFX_LOOKAHEAD_DS_UNKNOWN },
    { _T("slow"),   MFX_LOOKAHEAD_DS_OFF     },
    { _T("medium"), MFX_LOOKAHEAD_DS_2x      },
    { _T("fast"),   MFX_LOOKAHEAD_DS_4x      },
    { NULL, 0 }
};

const CX_DESC list_mv_cost_scaling[] = {
    { _T("default"), -1 },
    { _T("0"),        0 },
    { _T("1"),        1 },
    { _T("2"),        2 },
    { _T("3"),        3 },
    { _T("4"),        4 },
    { NULL, 0 }
};

const CX_DESC list_priority[] = {
    { _T("low"),    MFX_PRIORITY_LOW    },
    { _T("normal"), MFX_PRIORITY_NORMAL },
    { _T("high"),   MFX_PRIORITY_HIGH   },
};

enum {
    QSV_AUD_ENC_NONE = -1,
    QSV_AUD_ENC_COPY = 0,
    QSV_AUD_ENC_AAC,
    QSV_AUD_ENC_MP3,
    QSV_AUD_ENC_MP2,
    QSV_AUD_ENC_VORBIS,
    QSV_AUD_ENC_WAV,
    QSV_AUD_ENC_OPUS,
    QSV_AUD_ENC_AC3,
};

static inline const CX_DESC *get_level_list(int CodecID) {
    switch (CodecID) {
        case MFX_CODEC_AVC:     return list_avc_level;
        case MFX_CODEC_MPEG2:   return list_mpeg2_level;
        case MFX_CODEC_VC1:     return list_vc1_level;
        case MFX_CODEC_HEVC:    return list_hevc_level;
        case MFX_CODEC_VP8:     return list_vp8_level;
        case MFX_CODEC_VP9:     return list_vp9_level;
        case MFX_CODEC_RAW:     return list_empty;
        case MFX_CODEC_CAPTURE: return list_empty;
        default:                return list_empty;
    }
}

static inline const CX_DESC *get_profile_list(int CodecID) {
    switch (CodecID) {
        case MFX_CODEC_AVC:     return list_avc_profile;
        case MFX_CODEC_MPEG2:   return list_mpeg2_profile;
        case MFX_CODEC_VC1:     return list_vc1_profile;
        case MFX_CODEC_HEVC:    return list_hevc_profile;
        case MFX_CODEC_VP8:     return list_vp8_profile;
        case MFX_CODEC_VP9:     return list_vp9_profile;
        case MFX_CODEC_RAW:     return list_empty;
        case MFX_CODEC_CAPTURE: return list_empty;
        default:                return list_empty;
    }
}

//表示用
const CX_DESC list_quality[] = {
    { _T(" 1 - best quality"), MFX_TARGETUSAGE_BEST_QUALITY },
    { _T(" 2 - higher"),       2                            },
    { _T(" 3 - high quality"), 3                            },
    { _T(" 4 - balanced"),     MFX_TARGETUSAGE_BALANCED     },
    { _T(" 5 - fast"),         5                            },
    { _T(" 6 - faster"),       6                            },
    { _T(" 7 - fastest"),      MFX_TARGETUSAGE_BEST_SPEED   },
    { NULL, 0 }
};

//オプション用
const CX_DESC list_quality_for_option[] = {
    { _T("best"),     MFX_TARGETUSAGE_BEST_QUALITY },
    { _T("higher"),   2                            },
    { _T("high"),     3                            },
    { _T("balanced"), MFX_TARGETUSAGE_BALANCED     },
    { _T("fast"),     5                            },
    { _T("faster"),   6                            },
    { _T("fastest"),  MFX_TARGETUSAGE_BEST_SPEED   },
    { NULL, 0 }
};

const CX_DESC list_mv_presicion[] = {
    { _T("Auto"),     MFX_MVPRECISION_UNKNOWN    },
    { _T("full-pel"), MFX_MVPRECISION_INTEGER    },
    { _T("half-pel"), MFX_MVPRECISION_HALFPEL    },
    { _T("Q-pel"),    MFX_MVPRECISION_QUARTERPEL },
    { NULL, 0 }
};

const CX_DESC list_pred_block_size[] = {
    { _T("Auto"),          MFX_BLOCKSIZE_UNKNOWN    },
    { _T("16x16"),         MFX_BLOCKSIZE_MIN_16X16  },
    { _T("8x8"),           MFX_BLOCKSIZE_MIN_8X8    },
    { _T("4x4"),           MFX_BLOCKSIZE_MIN_4X4    },
    { NULL, 0 }
};

const CX_DESC list_hevc_ctu[] = {
    { _T("auto"), 0 },
    { _T("16"), 16 },
    { _T("32"), 32 },
    { _T("64"), 64 },
    { NULL, 0 }
};

const CX_DESC list_hevc_sao[] = {
    { _T("auto"),   MFX_SAO_UNKNOWN },
    { _T("none"),   MFX_SAO_DISABLE },
    { _T("luma"),   MFX_SAO_ENABLE_LUMA },
    { _T("chroma"), MFX_SAO_ENABLE_CHROMA },
    { _T("all"),    (uint32_t)MFX_SAO_ENABLE_LUMA | (uint32_t)MFX_SAO_ENABLE_CHROMA },
    { NULL, 0 }
};

const CX_DESC list_vpp_image_stabilizer[] = {
    { _T("none"),    0 },
    { _T("upscale"), MFX_IMAGESTAB_MODE_UPSCALE },
    { _T("box"),     MFX_IMAGESTAB_MODE_BOXING  },
    { NULL, 0 }
};

const CX_DESC list_vpp_fps_conversion[] = {
    { _T("off"),  0 },
    { _T("x2"),   FPS_CONVERT_MUL2 },
    { _T("x2.5"), FPS_CONVERT_MUL2_5  },
    { NULL, 0 }
};

const CX_DESC list_vpp_sub_shaping[] = {
    { _T("simple"),  QSV_VPP_SUB_SIMPLE  },
    { _T("complex"), QSV_VPP_SUB_COMPLEX },
    { NULL, 0 }
};

const CX_DESC list_vpp_rotate_angle[] = {
    { _T("0"),     MFX_ANGLE_0    },
    { _T("90"),    MFX_ANGLE_90   },
    { _T("180"),   MFX_ANGLE_180  },
    { _T("270"),   MFX_ANGLE_270  },
    { NULL, 0 }
};

const CX_DESC list_vpp_mirroring[] = {
    { _T("n"), MFX_MIRRORING_DISABLED   },
    { _T("h"), MFX_MIRRORING_HORIZONTAL },
    { _T("v"), MFX_MIRRORING_VERTICAL   },
    { NULL, 0 }
};

/*
const CX_DESC list_vpp_scaling_quality[] = {
    { _T("auto"),   MFX_SCALING_MODE_DEFAULT  },
    { _T("simple"), MFX_SCALING_MODE_LOWPOWER },
    { _T("fine"),   MFX_SCALING_MODE_QUALITY  },
    { NULL, 0 }
};
*/

//define defaults
const int QSV_DEFAULT_REF = 0;
const int QSV_DEFAULT_GOP_LEN = 0;
const int QSV_DEFAULT_ICQ = 23;
const int QSV_DEFAULT_QVBR = 23;
const int QSV_DEFAULT_QPI = 24;
const int QSV_DEFAULT_QPP = 26;
const int QSV_DEFAULT_QPB = 27;
const int QSV_BFRAMES_AUTO = -1;
const int QSV_DEFAULT_H264_BFRAMES = 3;
const int QSV_DEFAULT_HEVC_BFRAMES = 3;
const int QSV_DEFAULT_QUALITY = MFX_TARGETUSAGE_BALANCED;
const int QSV_DEFAULT_INPUT_BUF_SW = 1;
const int QSV_DEFAULT_INPUT_BUF_HW = 3;
const int QSV_INPUT_BUF_MIN = 1;
const int QSV_INPUT_BUF_MAX = 16;
const int QSV_DEFAULT_CONVERGENCE = 90;
const int QSV_DEFAULT_ACCURACY = 500;
const int QSV_DEFAULT_FORCE_GOP_LEN = 1;
const int QSV_DEFAULT_OUTPUT_BUF_MB = 8;
const uint32_t QSV_DEFAULT_BENCH = (1 << 1) | (1 << 4) | (1 << 7);

const int QSV_DEFAULT_VQP_STRENGTH = 10;
const int QSV_DEFAULT_VQP_SENSITIVITY = 50;
const int QSV_DEFAULT_SC_SENSITIVITY = 80;

const int QSV_DEFAULT_ASYNC_DEPTH = 4;
const int QSV_ASYNC_DEPTH_MAX = 64;
const int QSV_SESSION_THREAD_MAX = 64;

const int QSV_LOOKAHEAD_DEPTH_MIN = 10;
const int QSV_LOOKAHEAD_DEPTH_MAX = 100;

const uint32_t QSV_DEFAULT_AUDIO_IGNORE_DECODE_ERROR = 10;

const int QSV_DEFAULT_VPP_DELOGO_DEPTH = 128;

const int QSV_DEFAULT_PERF_MONITOR_INTERVAL = 500;

const int QSV_VPP_DENOISE_MIN = 0;
const int QSV_VPP_DENOISE_MAX = 100;
const int QSV_VPP_MCTF_AUTO = 0;
const int QSV_VPP_MCTF_MIN = 1;
const int QSV_VPP_MCTF_MAX = 20;
const int QSV_VPP_DETAIL_ENHANCE_MIN = 0;
const int QSV_VPP_DETAIL_ENHANCE_MAX = 100;

#endif //_QSV_PRM_H_
