#ifndef GENALYZER_IMPL_ENUM_MAPS_HPP
#define GENALYZER_IMPL_ENUM_MAPS_HPP

#include "enum_map.hpp"
#include "enums.hpp"
#include "utils.hpp"

namespace genalyzer_impl {

    int enum_value(const str_t& enumeration, const str_t& enumerator);

    template<typename E>
    E get_enum(int);

    const enum_map analysis_type_map ("AnalysisType", {
        { to_int(AnalysisType::DNL)       , "DNL"       },
        { to_int(AnalysisType::Fourier)   , "Fourier"   },
        { to_int(AnalysisType::Histogram) , "Histogram" },
        { to_int(AnalysisType::INL)       , "INL"       },
        { to_int(AnalysisType::Waveform)  , "Waveform"  }}
    );

    const enum_map code_format_map ("CodeFormat", {
        { to_int(CodeFormat::OffsetBinary)   , "OffsetBinary"   },
        { to_int(CodeFormat::TwosComplement) , "TwosComplement" }}
    );

    const enum_map dnl_signal_map ("DnlSignal", {
        { to_int(DnlSignal::Ramp) , "Ramp" },
        { to_int(DnlSignal::Tone) , "Tone" }}
    );

    const enum_map fa_comp_tag_map ("FACompTag", {
        { to_int(FACompTag::DC)       , "DC"       },
        { to_int(FACompTag::Signal)   , "Signal"   },
        { to_int(FACompTag::HD)       , "HD"       },
        { to_int(FACompTag::IMD)      , "IMD"      },
        { to_int(FACompTag::ILOS)     , "ILOS"     },
        { to_int(FACompTag::ILGT)     , "ILGT"     },
        { to_int(FACompTag::CLK)      , "CLK"      },
        { to_int(FACompTag::UserDist) , "UserDist" },
        { to_int(FACompTag::Noise)    , "Noise"    }}
    );

    const enum_map fa_ssb_map ("FASsb", {
        { to_int(FASsb::Default) , "Default" },
        { to_int(FASsb::DC)      , "DC"      },
        { to_int(FASsb::Signal)  , "Signal"  },
        { to_int(FASsb::WO)      , "WO"      }}
    );

    const enum_map freq_axis_format_map ("FreqAxisFormat", {
        { to_int(FreqAxisFormat::Bins) , "Bins" },
        { to_int(FreqAxisFormat::Freq) , "Freq" },
        { to_int(FreqAxisFormat::Norm) , "Norm" }}
    );

    const enum_map freq_axis_type_map ("FreqAxisType", {
        { to_int(FreqAxisType::DcCenter) , "DcCenter" },
        { to_int(FreqAxisType::DcLeft)   , "DcLeft"   },
        { to_int(FreqAxisType::Real)     , "Real"     }}
    );

    const enum_map inl_line_fit_map ("InlLineFit", {
        { to_int(InlLineFit::NoFit)   , "NoFit"   },
        { to_int(InlLineFit::BestFit) , "BestFit" },
        { to_int(InlLineFit::EndFit)  , "EndFit"  }}
    );

    const enum_map rfft_scale_map ("RfftScale", {
        { to_int(RfftScale::DbfsDc)  , "DbfsDc"  },
        { to_int(RfftScale::DbfsSin) , "DbfsSin" },
        { to_int(RfftScale::Native)  , "Native"  }}
    );

    const enum_map window_map ("Window", {
        { to_int(Window::NoWindow)       , "NoWindow"       },
        { to_int(Window::BlackmanHarris) , "BlackmanHarris" },
        { to_int(Window::Hann)           , "Hann"           }}
    );
    
} // namespace genalyzer_impl

namespace genalyzer_impl {

    const enum_map fa_comp_type_map ("FACompType", {
        { to_int(FACompType::DC)        , "DC"        },
        { to_int(FACompType::FixedTone) , "FixedTone" },
        { to_int(FACompType::MaxTone)   , "MaxTone"   },
        { to_int(FACompType::WOTone)    , "WOTone"    }}
    );

    const enum_map fa_result_map ("FAResult", {
        { to_int(FAResult::AnalysisType)   , "analysistype"   },
        { to_int(FAResult::SignalType)     , "signaltype"     },
        { to_int(FAResult::NFFT)           , "nfft"           },
        { to_int(FAResult::DataSize)       , "datasize"       },
        { to_int(FAResult::FBin)           , "fbin"           },
        { to_int(FAResult::FData)          , "fdata"          },
        { to_int(FAResult::FSample)        , "fsample"        },
        { to_int(FAResult::FShift)         , "fshift"         },
        { to_int(FAResult::FSNR)           , "fsnr"           },
        { to_int(FAResult::SNR)            , "snr"            },
        { to_int(FAResult::SINAD)          , "sinad"          },
        { to_int(FAResult::SFDR)           , "sfdr"           },
        { to_int(FAResult::ABN)            , "abn"            },
        { to_int(FAResult::NSD)            , "nsd"            },
        { to_int(FAResult::CarrierIndex)   , "carrierindex"   },
        { to_int(FAResult::MaxSpurIndex)   , "maxspurindex"   },
        { to_int(FAResult::AB_Width)       , "ab_width"       },
        { to_int(FAResult::AB_I1)          , "ab_i1"          },
        { to_int(FAResult::AB_I2)          , "ab_i2"          },
        { to_int(FAResult::AB_NBins)       , "ab_nbins"       },
        { to_int(FAResult::AB_RSS)         , "ab_rss"         },
        { to_int(FAResult::Signal_NBins)   , "signal_nbins"   },
        { to_int(FAResult::Signal_RSS)     , "signal_rss"     },
        { to_int(FAResult::CLK_NBins)      , "clk_nbins"      },
        { to_int(FAResult::CLK_RSS)        , "clk_rss"        },
        { to_int(FAResult::HD_NBins)       , "hd_nbins"       },
        { to_int(FAResult::HD_RSS)         , "hd_rss"         },
        { to_int(FAResult::ILOS_NBins)     , "ilos_nbins"     },
        { to_int(FAResult::ILOS_RSS)       , "ilos_rss"       },
        { to_int(FAResult::ILGT_NBins)     , "ilgt_nbins"     },
        { to_int(FAResult::ILGT_RSS)       , "ilgt_rss"       },
        { to_int(FAResult::IMD_NBins)      , "imd_nbins"      },
        { to_int(FAResult::IMD_RSS)        , "imd_rss"        },
        { to_int(FAResult::UserDist_NBins) , "userdist_nbins" },
        { to_int(FAResult::UserDist_RSS)   , "userdist_rss"   },
        { to_int(FAResult::THD_NBins)      , "thd_nbins"      },
        { to_int(FAResult::THD_RSS)        , "thd_rss"        },
        { to_int(FAResult::ILV_NBins)      , "ilv_nbins"      },
        { to_int(FAResult::ILV_RSS)        , "ilv_rss"        },
        { to_int(FAResult::Dist_NBins)     , "dist_nbins"     },
        { to_int(FAResult::Dist_RSS)       , "dist_rss"       },
        { to_int(FAResult::Noise_NBins)    , "noise_nbins"    },
        { to_int(FAResult::Noise_RSS)      , "noise_rss"      },
        { to_int(FAResult::NAD_NBins)      , "nad_nbins"      },
        { to_int(FAResult::NAD_RSS)        , "nad_rss"        }}
    );

    const enum_map fa_tone_result_map ("FAToneResult", {
        { to_int(FAToneResult::OrderIndex) , "orderindex" },
        { to_int(FAToneResult::Tag)        , "tag"        },
        { to_int(FAToneResult::Freq)       , "freq"       },
        { to_int(FAToneResult::FFinal)     , "ffinal"     },
        { to_int(FAToneResult::FWAvg)      , "fwavg"      },
        { to_int(FAToneResult::I1)         , "i1"         },
        { to_int(FAToneResult::I2)         , "i2"         },
        { to_int(FAToneResult::NBins)      , "nbins"      },
        { to_int(FAToneResult::InBand)     , "inband"     },
        { to_int(FAToneResult::Mag)        , "mag"        },
        { to_int(FAToneResult::Mag_dBFS)   , "mag_dbfs"   },
        { to_int(FAToneResult::Mag_dBc)    , "mag_dbc"    },
        { to_int(FAToneResult::Phase)      , "phase"      },
        { to_int(FAToneResult::Phase_c)    , "phase_c"    }}
    );
    
    const enum_map object_type_map ("ObjectType", {
        { to_int(ObjectType::FourierAnalysis) , "FourierAnalysis" }}
    );
    
} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_ENUM_MAPS_HPP