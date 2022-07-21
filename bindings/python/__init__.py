"""
Data Converter Analysis Library
===============================

ENUMERATIONS

    CodeFormat
    DNLSignal
    FaCompTag
    FreqAxisFormat
    FreqAxisType
    InlLineFit
    RfftScale
    Window

FUNCTIONS

    Array Operations
    ----------------
    abs
    angle
    db
    db10
    db20
    norm

    Code Density
    ------------
    code_axis
    code_axisx
    dnl
    dnl_analysis
    hist
    histx
    hist_analysis
    inl
    inl_analysis

    Fourier Analysis
    ----------------
    fft_analysis

        Fourier Analysis Configuration
        ------------------------------
        fa_analysis_band
        fa_clk
        fa_conv_offset
        fa_create
        fa_dc
        fa_fdata
        fa_fixed_tone
        fa_fsample
        fa_fshift
        fa_fund_images
        fa_hd
        fa_ilv
        fa_imd
        fa_load
        fa_max_tone
        fa_preview
        fa_quad_errors
        fa_remove_comp
        fa_reset
        fa_ssb
        fa_ssb_dc       DEPRECATED
        fa_ssb_def      DEPRECATED
        fa_ssb_wo       DEPRECATED
        fa_var
        fa_wo

        Fourier Analysis Results
        ------------------------
        fa_annotations
        fa_result_string

    Fourier Transforms
    ------------------
    fft
    rfft

    Fourier Utilities
    ------------------
    alias
    coherent
    fftshift
    freq_axis
    ifftshift

    Manager
    -------
    mgr_clear
    mgr_compare
    mgr_contains
    mgr_remove
    mgr_save
    mgr_size
    mgr_to_string
    mgr_type

    Signal Processing
    -----------------
    downsample
    fshift
    normalize
    polyval
    quantize
    quantize16
    quantize32
    quantize64

    Waveforms
    -------------------
    cos
    gaussian
    ramp
    sin
    wf_analysis


"""

from ._dcanalysis import *

__version__ = _dcanalysis._version_string()