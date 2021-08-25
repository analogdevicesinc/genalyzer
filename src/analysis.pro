## -----------------------------------------------------------------------------
## @file $URL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/analysis.pro $
## @version $Revision: 12767 $
## @date $Date: 2020-08-24 16:38:41 -0400 (Mon, 24 Aug 2020) $
## @author $Author: pderouni $
## @brief Project qmake file for building the analysis module.
## -----------------------------------------------------------------------------

include(../../mkspecs/lib.pri)

TARGET = analysis

CONFIG += qt shared

QT = core

INCLUDEPATH += \
    $$(BOOST_INCLUDE_PATH) \
    $$(EIGEN_INCLUDE_PATH) \
    $$(FFTW_INCLUDE_PATH)

LIBS += -L$$(FFTW_LIBRARY_PATH)
LIBS += -llibfftw3-3

DEFINES += \
    ICD_DO_NOT_INCLUDE_BOOST \
    _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

QMAKE_CXXFLAGS_WARN_ON += -wd4251

icd_rebug {
    DEFINES += ICD_ANALYSIS_DEBUG_BUILD
}

HEADERS += \
    abstract_object.hpp \
    abstract_signal_data.hpp \
    abstract_time_data.hpp \
    abstract_freq_data.hpp \
    analysis.hpp \
    analysis_results.hpp \
    array_math.hpp \
    basic_quantizer.hpp \
    checks.hpp \
    code_density.hpp \
    constants.hpp \
    cplx_freq_data.hpp \
    cplx_time_data.hpp \
    dft.hpp \
    enum_map.hpp \
    enum_maps.hpp \
    enums.hpp \
    es_map.hpp \
    fft_analysis.hpp \
    fft_analysis2.hpp \
    fft_component.hpp \
    file_utils.hpp \
    formatted_data.hpp \
    io_data.hpp \
    io_functions.hpp \
    io_item.hpp \
    io_map.hpp \
    io_null.hpp \
    io_object.hpp \
    io_vector.hpp \
    mag_freq_data.hpp \
    math_expression.hpp \
    math_expression_tokens.hpp \
    matrix.hpp \
    processes.hpp \
    quantarch.hpp \
    quantizer.hpp \
    quantizer_arch.hpp \
    quantizer_data.hpp \
    quantizer_inst.hpp \
    quantizer_process.hpp \
    quantizer_stage.hpp \
    real_time_data.hpp \
    real_to_string.hpp \
    sampling_params.hpp \
    sparse_array_mask.hpp \
    spectral_component.hpp \
    text_utils.hpp \
    type_aliases.hpp \
    types.hpp \
    types_and_constants.hpp \
    value_of_slbw.hpp \
    var_data.hpp \
    var_item.hpp \
    var_map.hpp \
    var_vector.hpp \
    waveforms.hpp

SOURCES += \
    abstract_object.cpp \
    abstract_signal_data.cpp \
    abstract_freq_data.cpp \
    abstract_time_data.cpp \
    array_math.cpp \
    code_density.cpp \
    cplx_freq_data.cpp \
    cplx_time_data.cpp \
    dft.cpp \
    fft_analysis.cpp \
    fft_analysis2.cpp \
    fft_component.cpp \
    formatted_data.cpp \
    io_data.cpp \
    io_item.cpp \
    io_map.cpp \
    io_null.cpp \
    io_object.cpp \
    io_vector.cpp \
    mag_freq_data.cpp \
    math_expression.cpp \
    math_expression_tokens.cpp \
    processes.cpp \
    quantarch.cpp \
    quantizer.cpp \
    quantizer_data.cpp \
    quantizer_process.cpp \
    quantizer_stage.cpp \
    real_time_data.cpp \
    real_to_string.cpp \
    sparse_array_mask.cpp \
    text_utils.cpp \
    value_of_slbw.cpp \
    var_item.cpp \
    var_map.cpp \
    var_vector.cpp \
    waveforms.cpp
