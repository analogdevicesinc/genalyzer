classdef (Sealed) genalyzer < handle
    properties (Constant) % C Library Interface
        header_name = 'cgenalyzer_advanced.h';
        nixbin_name = 'libgenalyzer_plus_plus.so';
        winbin_name = 'genalyzer_plus_plus.dll';
        lib_alias = 'gn';
    end
    properties (Constant) % Enumerations
        CodeFormatOffsetBinary   = 0
        CodeFormatTwosComplement = 1
        DnlSignalRamp            = 0
        DnlSignalTone            = 1
        FaCompTagDC              = 0
        FaCompTagSignal          = 1
        FaCompTagHD              = 2
        FaCompTagIMD             = 3
        FaCompTagILOS            = 4
        FaCompTagILGT            = 5
        FaCompTagCLK             = 6
        FaCompTagUserDist        = 7
        FaCompTagNoise           = 8
        FaSsbDefault             = 0
        FaSsbDC                  = 1
        FaSsbSignal              = 2
        FaSsbWO                  = 3
        FreqAxisFormatBins       = 0
        FreqAxisFormatFreq       = 1
        FreqAxisFormatNorm       = 2
        FreqAxisTypeDcCenter     = 0
        FreqAxisTypeDcLeft       = 1
        FreqAxisTypeReal         = 2
        InlLineFitBestFit        = 0
        InlLineFitEndFit         = 1
        InlLineFitNoFit          = 2
        RfftScaleDbfsDc          = 0
        RfftScaleDbfsSin         = 1
        RfftScaleNative          = 2
        WindowBlackmanHarris     = 0
        WindowHann               = 1
        WindowNoWindow           = 2
    end
    properties (Access=private, Constant)
        AnalysisTypeDnl       = 0
        AnalysisTypeFourier   = 1
        AnalysisTypeHistogram = 2
        AnalysisTypeInl       = 3
        AnalysisTypeWaveform  = 4
    end
    methods (Static) % C Library Load/Unload
        function load()
            if not(libisloaded(genalyzer.lib_alias))
                if ispc
                    loadlibrary(genalyzer.winbin_name, ...
                                genalyzer.header_name, ...
                                'alias', genalyzer.lib_alias);
                elseif isunix
                    loadlibrary(genalyzer.nixbin_name, ...
                                genalyzer.header_name, ...
                                'alias', genalyzer.lib_alias);
                else
                    disp(['Cannot load library: ''', computer, ''' is not supported'])
                end
                calllib('gn', 'gn_set_string_termination', false);
            end
        end
        function unload()
            if (libisloaded(genalyzer.lib_alias))
                unloadlibrary(genalyzer.lib_alias)
            end
        end
    end
    methods (Access=private, Static) % Helpers
        function raise_exception_on_failure(result)
            if 0 == nargin
                result = 1;
            end
            if result
                size = 0;
                [~, size] = calllib('gn', 'gn_error_string_size', size);
                msg = blanks(size);
                [result, msg] = calllib('gn', 'gn_error_string', msg, numel(msg));
                if result
                    msg = 'An error was reported, but the error message could not be retrieved.';
                end
                throw(MException('genalyzer:error', msg))
            end
        end
        function type = check_array_1d(X, types)
            assert(isa(types, 'cell'), 'Expected ''types'' to be a cell array')
            types_str = '{';
            for i = 1:numel(types)
                if isa(X, types{i})
                    type = class(X);
                    break
                end
                types_str = strcat(types_str, types{i});
                if numel(types) == i
                    types_str = strcat(types_str, '}');
                    msg = ['Expected type in ' types_str ', got ' class(X)];
                    throw(MException('genalyzer:error', msg))
                end
                types_str = strcat(types_str, ', ');
            end
            assert(isrow(X), 'Expected row vector')
        end
        function [keys, values] = get_analysis_containers(analysis_type)
            size = 0;
            [result, size] = calllib('gn', 'gn_analysis_results_size', size, analysis_type);
            genalyzer.raise_exception_on_failure(result)
            key_sizes = zeros(1, size, 'uint64');
            [result, key_sizes] = calllib('gn', 'gn_analysis_results_key_sizes', ...
                key_sizes, size, analysis_type);
            genalyzer.raise_exception_on_failure(result)
            keys = cell(size, 1);
            values = zeros(1, size);
            for r = 1:size
                keys{r} = blanks(key_sizes(r));
            end
        end
        function [keyarr, valarr] = get_key_value_arrays(result_map)
            assert(isa(result_map, 'containers.Map'), 'Expected ''containers.Map''')
            size = result_map.Count;
            keyarr = keys(result_map);
            valcellarr = values(result_map);
            valarr = zeros(1, size);
            for k = 1:size
                valarr(k) = valcellarr{k};
            end
        end
        function boxes = get_annot_boxes(axis_type, datasize, bottom, height, x1, x2, xscalar)
            boxes = {};
            lbound = 0;
            rbound = datasize - 1;
            if genalyzer.FreqAxisTypeDcCenter == axis_type
                lbound = lbound - floor(datasize / 2);
                rbound = rbound - floor(datasize / 2);
                if rbound < x1
                    x1 = x1 - datasize;
                end
                if rbound < x2
                    x2 = x2 - datasize;
                end
            end
            if x1 <= x2  % 1 box, x1->x2
                left = (x1 - 0.5) * xscalar;
                width = ((x2 - x1) + 1) * xscalar;
                boxes = [boxes, {{[left, left + width, left + width, left], ...
                                  [bottom, bottom, bottom + height, bottom + height]}}];
            else  % x2 < x1: 2 boxes, x1->rbound, lbound->x2
                if x1 < rbound
                    left = (x1 - 0.5) * xscalar;
                    width = rbound * xscalar - left;
                    boxes = [boxes, {{[left, left + width, left + width, left], ...
                                  [bottom, bottom, bottom + height, bottom + height]}}];
                end
                if lbound < x2
                    left = lbound * xscalar;
                    width = (x2 + 0.5) * xscalar - left;
                    boxes = [boxes, {{[left, left + width, left + width, left], ...
                                  [bottom, bottom, bottom + height, bottom + height]}}];
                end
            end
        end
    end
    methods (Static) % API Utilities
        function ver = version()
            size = 0;
            [~, size] = calllib('gn', 'gn_version_string_size', size);
            ver = blanks(size);
            [result, ver] = calllib('gn', 'gn_version_string', ver, numel(ver));
            genalyzer.raise_exception_on_failure(result)
        end
    end
    methods (Static) % Array Operations
        function Y = abs(X)
            genalyzer.check_array_1d(X, {'double'});
            Y = zeros(1, numel(X) / 2, 'double');
            [result, Y] = calllib('gn', 'gn_abs', Y, numel(Y), X, numel(X));
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = angle(X)
            genalyzer.check_array_1d(X, {'double'});
            Y = zeros(1, numel(X) / 2, 'double');
            [result, Y] = calllib('gn', 'gn_angle', Y, numel(Y), X, numel(X));
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = db(X)
            genalyzer.check_array_1d(X, {'double'});
            Y = zeros(1, numel(X) / 2, 'double');
            [result, Y] = calllib('gn', 'gn_db', Y, numel(Y), X, numel(X));
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = db10(X)
            genalyzer.check_array_1d(X, {'double'});
            Y = zeros(1, numel(X), 'double');
            [result, Y] = calllib('gn', 'gn_db10', Y, numel(Y), X, numel(X));
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = db20(X)
            genalyzer.check_array_1d(X, {'double'});
            Y = zeros(1, numel(X), 'double');
            [result, Y] = calllib('gn', 'gn_db20', Y, numel(Y), X, numel(X));
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = norm(X)
            genalyzer.check_array_1d(X, {'double'});
            Y = zeros(1, numel(X) / 2, 'double');
            [result, Y] = calllib('gn', 'gn_norm', Y, numel(Y), X, numel(X));
            genalyzer.raise_exception_on_failure(result)
        end
    end
    methods (Static) % Code Density
        function X = code_axis(n, fmt)
            if nargin < 2
                fmt = genalyzer.CodeFormatTwosComplement;
            end
            size = 0;
            [result, size] = calllib('gn', 'gn_code_density_size', size, n, fmt);
            genalyzer.raise_exception_on_failure(result)
            X = zeros(1, size, 'double');
            [result, X] = calllib('gn', 'gn_code_axis', X, numel(X), n, fmt);
            genalyzer.raise_exception_on_failure(result)
        end
        function X = code_axisx(min_code, max_code)
            size = 0;
            [result, size] = calllib('gn', 'gn_code_densityx_size', size, min_code, max_code);
            genalyzer.raise_exception_on_failure(result)
            X = zeros(1, size, 'double');
            [result, X] = calllib('gn', 'gn_code_axisx', X, numel(X), min_code, max_code);
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = dnl(X, signal_type)
            if nargin < 2
                signal_type = genalyzer.DnlSignalTone;
            end
            genalyzer.check_array_1d(X, {'uint64'});
            Y = zeros(1, numel(X), 'double');
            [result, Y] = calllib('gn', 'gn_dnl', Y, numel(Y), X, numel(X), signal_type);
            genalyzer.raise_exception_on_failure(result)
        end
        function M = dnl_analysis(X)
        % DNL_ANALYSIS  Returns DNL analysis metrics
        %   M = DNL_ANALYSIS(X)
        %
        % Parameters
        % ----------
        % X : double array of DNL data
        %
        % Returns
        % -------
        % M : containers.Map with KeyType 'char' and ValueType 'double'
        %
        %   ====================================================================================
        %     Key             |  Description
        %   ====================================================================================
        %     min             |  Minumum value
        %     max             |  Maximum value
        %     avg             |  Average value
        %     rms             |  RMS value
        %     min_index       |  Index of first occurence of minimum value
        %     max_index       |  Index of first occurence of maximum value
        %     first_nm_index  |  Index of first non-missing code
        %     last_nm_index   |  Index of last non-missing code
        %     nm_range        |  Non-missing code range (1 + (last_nm_index - first_nm_index))
        %   ====================================================================================

            genalyzer.check_array_1d(X, {'double'});
            [keys, values] = genalyzer.get_analysis_containers(genalyzer.AnalysisTypeDnl);
            [result, keys, values] = calllib('gn', 'gn_dnl_analysis', ...
                keys, numel(keys), values, numel(values), X, numel(X));
            genalyzer.raise_exception_on_failure(result)
            M = containers.Map(keys, values);
        end
        function Y = hist(X, n, fmt)
            if nargin < 2
                fmt = genalyzer.CodeFormatTwosComplement;
            end
            genalyzer.check_array_1d(X, {'int16', 'int32', 'int64'});
            size = 0;
            [result, size] = calllib('gn', 'gn_code_density_size', size, n, fmt);
            genalyzer.raise_exception_on_failure(result)
            Y = zeros(1, size, 'uint64');
            if isa(X, 'int16')
                [result, Y] = calllib('gn', 'gn_hist16', Y, numel(Y), X, numel(X), n, fmt, false);
            elseif isa(X, 'int32')
                [result, Y] = calllib('gn', 'gn_hist32', Y, numel(Y), X, numel(X), n, fmt, false);
            else
                [result, Y] = calllib('gn', 'gn_hist64', Y, numel(Y), X, numel(X), n, fmt, false);
            end
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = histx(X, min, max)
            genalyzer.check_array_1d(X, {'int16', 'int32', 'int64'});
            size = 0;
            [result, size] = calllib('gn', 'gn_code_densityx_size', size, min, max);
            genalyzer.raise_exception_on_failure(result)
            Y = zeros(1, size, 'uint64');
            if isa(X, 'int16')
                [result, Y] = calllib('gn', 'gn_histx16', Y, numel(Y), X, numel(X), min, max, false);
            elseif isa(X, 'int32')
                [result, Y] = calllib('gn', 'gn_histx32', Y, numel(Y), X, numel(X), min, max, false);
            else
                [result, Y] = calllib('gn', 'gn_histx64', Y, numel(Y), X, numel(X), min, max, false);
            end
            genalyzer.raise_exception_on_failure(result)
        end
        function M = hist_analysis(X)
        % HIST_ANALYSIS  Returns histogram analysis metrics
        %   M = HIST_ANALYSIS(X)
        %
        % Parameters
        % ----------
        % X : uint64 array of histogram data
        %
        % Returns
        % -------
        % M : containers.Map with KeyType 'char' and ValueType 'double'
        %
        %   ================================================================================
        %     Key             |  Description
        %   ================================================================================
        %     sum             |  Sum of all histogram bins
        %     first_nz_index  |  First non-zero bin
        %     last_nz_index   |  Last non-zero bin
        %     nz_range        |  Non-zero bin range (1 + (last_nz_index - first_nz_index))
        %   ================================================================================

            genalyzer.check_array_1d(X, {'uint64'});
            [keys, values] = genalyzer.get_analysis_containers(genalyzer.AnalysisTypeHistogram);
            [result, keys, values] = calllib('gn', 'gn_hist_analysis', ...
                keys, numel(keys), values, numel(values), X, numel(X));
            genalyzer.raise_exception_on_failure(result)
            M = containers.Map(keys, values);
        end
        function Y = inl(X, fit)
            if nargin < 2
                fit = genalyzer.InlLineFitBestFit;
            end
            genalyzer.check_array_1d(X, {'double'});
            Y = zeros(1, numel(X), 'double');
            [result, Y] = calllib('gn', 'gn_inl', Y, numel(Y), X, numel(X), fit);
            genalyzer.raise_exception_on_failure(result)
        end
        function M = inl_analysis(X)
        % INL_ANALYSIS  Returns INL analysis metrics
        %   M = INL_ANALYSIS(X)
        %
        % Parameters
        % ----------
        % X : double array of INL data
        %
        % Returns
        % -------
        % M : containers.Map with KeyType 'char' and ValueType 'double'
        %
        %   ===========================================================
        %	  Key        |  Description
        %   ===========================================================
        %     min        |  Minumum value
        %     max        |  Maximum value
        %     min_index  |  Index of first occurence of minimum value
        %     max_index  |  Index of first occurence of maximum value
        %   ===========================================================

            genalyzer.check_array_1d(X, {'double'});
            [keys, values] = genalyzer.get_analysis_containers(genalyzer.AnalysisTypeInl);
            [result, keys, values] = calllib('gn', 'gn_inl_analysis', ...
                keys, numel(keys), values, numel(values), X, numel(X));
            genalyzer.raise_exception_on_failure(result)
            M = containers.Map(keys, values);
        end
    end
    methods (Static) % Fourier Analysis
        function M = fft_analysis(cfg_id, X, nfft, axis_type)
        % FFT_ANALYSIS  Returns Fourier analysis metrics
        %   M = FFT_ANALYSIS(cfg_id, X, nfft, axis_type)
        %
        % Parameters
        % ----------
        % cfg_id    : configuration identifier; can be one of the following
        %               1. Key associated with a configuration object in the Manager
        %               2. Filename of a saved configuration
        % X         : double array of complex or magnitude FFT data
        % nfft      : FFT size
        % axis_type : If X contains I/Q data, each tone's ffinal result is aliased accordingly:
        %               FreqAxisTypeDcCenter : [-fdata/2, fdata/2)
        %               FreqAxisTypeDcLeft   : [0, fdata)
        %             If X contains Real data, this parameter has no effect.
        %
        % Details
        % -------
        % The type of analysis (Real or Complex) and the type of data (Complex or Magnitude) is
        % detected automatically based on numel(X) and nfft.  Consider the following examples:
        %
        %     NFFT  |  numel(X)  |  Analysis Type  |  Data Type
        %   --------+------------+-----------------+-------------
        %     1024  |      2048  |  Complex        |  Complex
        %     1024  |      1024  |  Complex        |  Magnitude
        %     1024  |      1026  |  Real           |  Complex
        %     1024  |       513  |  Real           |  Magnitude
        %
        % The tone results 'phase' and 'phase_c' are always included in the results, but are only
        % meaningful when the data type is Complex and when the given tone comprises exactly one
        % bin.  Otherwise, 'phase' and 'phase_c' equal 0.0.
        %
        % Returns
        % -------
        % M : containers.Map with KeyType 'char' and ValueType 'double'
        %
        %   ===================================================================================
        %     Key                   |  Description                                 |  Units
        %   ===================================================================================
        %     signaltype            |  Signal type: 0=Real, 1=Complex              |
        %     nfft                  |  FFT size                                    |
        %     datasize              |  Data size                                   |
        %     fbin                  |  Frequency bin size                          |  Hz
        %     fdata                 |  Data rate                                   |  S/s
        %     fsample               |  Sample rate                                 |  S/s
        %     fshift                |  Shift frequency                             |  Hz
        %     fsnr                  |  Full-scale-to-noise ratio (a.k.a. "SNRFS")  |  dB
        %     snr                   |  Signal-to-noise ratio                       |  dB
        %     sinad                 |  Signal-to-noise-and-distortion ratio        |  dB
        %     sfdr                  |  Spurious-free dynamic range                 |  dB
        %     abn                   |  Average bin noise                           |  dBFS
        %     nsd                   |  Noise spectral density                      |  dBFS/Hz
        %     carrierindex          |  Order index of the Carrier tone             |
        %     maxspurindex          |  Order index of the MaxSpur tone             |
        %     ab_width              |  Analysis band width                         |  Hz
        %     ab_i1                 |  Analysis band first index                   |
        %     ab_i2                 |  Analysis band last index                    |
        %     {PREFIX}_nbins        |  Number of bins associated with PREFIX       |
        %     {PREFIX}_rss          |  Root-sum-square associated with PREFIX      |
        %     {TONEKEY}:orderindex  |  Tone order index                            |
        %     {TONEKEY}:freq        |  Tone frequency                              |  Hz
        %     {TONEKEY}:ffinal      |  Tone final frequency                        |  Hz
        %     {TONEKEY}:fwavg       |  Tone weighted-average frequency             |  Hz
        %     {TONEKEY}:i1          |  Tone first index                            |
        %     {TONEKEY}:i2          |  Tone last index                             |
        %     {TONEKEY}:nbins       |  Tone number of bins                         |
        %     {TONEKEY}:inband      |  0: tone is in-band; 1: tone is out-of-band  |
        %     {TONEKEY}:mag         |  Tone magnitude                              |
        %     {TONEKEY}:mag_dbfs    |  Tone magnitude relative to full-scale       |  dBFS
        %     {TONEKEY}:mag_dbc     |  Tone magnitude relative to carrier          |  dBc
        %     {TONEKEY}:phase       |  Tone phase                                  |  rad
        %     {TONEKEY}:phase_c     |  Tone phase relative to carrier              |  rad
        %   ===================================================================================

            if nargin < 4
                axis_type = genalyzer.FreqAxisTypeDcLeft;
            end
            genalyzer.check_array_1d(X, {'double'});
            size = 0;
            [result, size] = calllib('gn', 'gn_fft_analysis_results_size', size, cfg_id, numel(X), nfft);
            genalyzer.raise_exception_on_failure(result)
            key_sizes = zeros(1, size, 'uint64');
            [result, key_sizes] = calllib('gn', 'gn_fft_analysis_results_key_sizes', ...
                key_sizes, size, cfg_id, numel(X), nfft);
            genalyzer.raise_exception_on_failure(result)
            keys = cell(size, 1);
            values = zeros(1, size);
            for r = 1:size
                keys{r} = blanks(key_sizes(r));
            end
            [result, keys, values] = calllib('gn', 'gn_fft_analysis', ...
                keys, numel(keys), values, numel(values), cfg_id, X, numel(X), nfft, axis_type);
            genalyzer.raise_exception_on_failure(result)
            M = containers.Map(keys, values);
        end
        % Fourier Analysis Configuration
        function fa_analysis_band(obj_key, center, width)
            if isa(center, 'char') && isa(width, 'char')
                result = calllib('gn', 'gn_fa_analysis_band_e', obj_key, center, width);
            else
                result = calllib('gn', 'gn_fa_analysis_band', obj_key, center, width);
            end
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_clk(obj_key, X, as_noise)
            if nargin < 3
                as_noise = false;
            end
            assert(isrow(X), 'Expected row vector')
            if isfloat(X)
                assert(isequal(X, int32(X)), 'Expected integer values')
            end
            X = int32(X);
            result = calllib('gn', 'gn_fa_clk', obj_key, X, numel(X), as_noise);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_conv_offset(obj_key, enable)
            result = calllib('gn', 'gn_fa_conv_offset', obj_key, enable);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_create(obj_key)
            result = calllib('gn', 'gn_fa_create', obj_key);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_dc(obj_key, as_dist)
            result = calllib('gn', 'gn_fa_dc', obj_key, as_dist);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_fdata(obj_key, f)
            if isa(f, 'char')
                result = calllib('gn', 'gn_fa_fdata_e', obj_key, f);
            else
                result = calllib('gn', 'gn_fa_fdata', obj_key, f);
            end
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_fixed_tone(obj_key, comp_key, tag, freq, ssb)
            if nargin < 5
                ssb = -1;
            end
            if isa(freq, 'char')
                result = calllib('gn', 'gn_fa_fixed_tone_e', obj_key, comp_key, tag, freq, ssb);
            else
                result = calllib('gn', 'gn_fa_fixed_tone', obj_key, comp_key, tag, freq, ssb);
            end
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_fsample(obj_key, f)
            if isa(f, 'char')
                result = calllib('gn', 'gn_fa_fsample_e', obj_key, f);
            else
                result = calllib('gn', 'gn_fa_fsample', obj_key, f);
            end
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_fshift(obj_key, f)
            if isa(f, 'char')
                result = calllib('gn', 'gn_fa_fshift_e', obj_key, f);
            else
                result = calllib('gn', 'gn_fa_fshift', obj_key, f);
            end
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_fund_images(obj_key, enable)
            result = calllib('gn', 'gn_fa_fund_images', obj_key, enable);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_hd(obj_key, n)
            result = calllib('gn', 'gn_fa_hd', obj_key, n);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_ilv(obj_key, X, as_noise)
            if nargin < 3
                as_noise = false;
            end
            assert(isrow(X), 'Expected row vector')
            if isfloat(X)
                assert(isequal(X, int32(X)), 'Expected integer values')
            end
            X = int32(X);
            result = calllib('gn', 'gn_fa_ilv', obj_key, X, numel(X), as_noise);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_imd(obj_key, n)
            result = calllib('gn', 'gn_fa_imd', obj_key, n);
            genalyzer.raise_exception_on_failure(result)
        end
        function S = fa_load(filename, obj_key)
            if nargin < 2
                obj_key = '';
            end
            size = 0;
            [result, size] = calllib('gn', 'gn_fa_load_key_size', size, filename, obj_key);
            genalyzer.raise_exception_on_failure(result)
            S = blanks(size);
            [result, S] = calllib('gn', 'gn_fa_load', S, numel(S), filename, obj_key);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_max_tone(obj_key, comp_key, tag, ssb)
            if nargin < 4
                ssb = -1;
            end
            result = calllib('gn', 'gn_fa_max_tone', obj_key, comp_key, tag, ssb);
            genalyzer.raise_exception_on_failure(result)
        end
        function S = fa_preview(cfg_id, cplx)
            if nargin < 2
                cplx = false;
            end
            size = 0;
            [result, size] = calllib('gn', 'gn_fa_preview_size', size, cfg_id, cplx);
            genalyzer.raise_exception_on_failure(result)
            S = blanks(size);
            [result, S] = calllib('gn', 'gn_fa_preview', S, numel(S), cfg_id, cplx);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_quad_errors(obj_key, enable)
            result = calllib('gn', 'gn_fa_quad_errors', obj_key, enable);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_remove_comp(obj_key, comp_key)
            result = calllib('gn', 'gn_fa_remove_comp', obj_key, comp_key);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_reset(obj_key)
            result = calllib('gn', 'gn_fa_reset', obj_key);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_ssb(obj_key, group, n)
            result = calllib('gn', 'gn_fa_ssb', obj_key, group, n);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_ssb_dc(obj_key, n)
            disp('fa_ssb_dc(obj_key, n) is deprecated; use fa_ssb(obj_key, FaSsbDC, n)')
            result = calllib('gn', 'gn_fa_ssb', obj_key, genalyzer.FaSsbDC, n);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_ssb_def(obj_key, n)
            disp('fa_ssb_def(obj_key, n) is deprecated; use fa_ssb(obj_key, FaSsbDefault, n)')
            result = calllib('gn', 'gn_fa_ssb', obj_key, genalyzer.FaSsbDefault, n);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_ssb_wo(obj_key, n)
            disp('fa_ssb_wo(obj_key, n) is deprecated; use fa_ssb(obj_key, FaSsbWO, n)')
            result = calllib('gn', 'gn_fa_ssb', obj_key, genalyzer.FaSsbWO, n);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_var(obj_key, name, value)
            result = calllib('gn', 'gn_fa_var', obj_key, name, value);
            genalyzer.raise_exception_on_failure(result)
        end
        function fa_wo(obj_key, n)
            result = calllib('gn', 'gn_fa_wo', obj_key, n);
            genalyzer.raise_exception_on_failure(result)
        end
        % Fourier Analysis Results
        function M = fa_annotations(result_map, axis_type, axis_fmt)
            if nargin < 2
                axis_type = genalyzer.FreqAxisTypeDcLeft;
            end
            if nargin < 3
                axis_fmt = genalyzer.FreqAxisFormatFreq;
            end
            
            if 1.0 == result_map('signaltype')
                if genalyzer.FreqAxisTypeReal == axis_type
                    axis_type = genalyzer.FreqAxisTypeDcLeft;
                end
            else
                axis_type = genalyzer.FreqAxisTypeReal;
            end
            
            datasize = result_map('datasize');
            fbin = result_map('fbin');
            if genalyzer.FreqAxisFormatBins == axis_fmt
                xscalar = 1 / fbin;
            elseif genalyzer.FreqAxisFormatNorm == axis_fmt
                xscalar = 1 / result_map('fdata');
            else
                xscalar = 1.0;
            end
            
            tone_keys = containers.Map('KeyType', 'int32', 'ValueType', 'char');
            for k = keys(result_map)
                kk = k{1};
                pos = strfind(kk, ':orderindex');
                if 1 == numel(pos)
                    tk = strtok(kk, ':');
                    oi = result_map(kk);
                    tone_keys(oi) = tk;
                end
            end
            
            %
            % Labels = {x, y, str}
            %
            label_ymap = containers.Map('KeyType', 'double', 'ValueType', 'double');
            label_smap = containers.Map('KeyType', 'double', 'ValueType', 'char');
            for k = 0 : tone_keys.Count - 1
                tk = tone_keys(k);
                x = result_map(strcat(tk, ':ffinal')) * xscalar;
                y = result_map(strcat(tk, ':mag_dbfs'));
                if isKey(label_ymap, x)
                    if label_ymap(x) < y
                        label_ymap(x) = y;
                    end
                    label_smap(x) = [label_smap(x), char(10), tk];
                else
                    label_ymap(x) = y;
                    label_smap(x) = tk;
                end
            end
            labels = {};
            for k = keys(label_ymap)
                kk = k{1};
                labels = [labels, {{kk, label_ymap(kk), label_smap(kk)}}];
            end
            %
            % Lines = {[x1, x2], [y1, y2]}
            %
            lines = {};
            if isKey(result_map, 'abn')
                abn = result_map('abn');
                x = result_map('fdata') * xscalar;
                lines{1} = {[-x, x], [abn, abn]};
            end
            %
            % Analysis Band Boxes = {[xll, xlr, xur, xul], [yll, ylr, yur, yul]]
            %
            ab_boxes = {};
            if result_map('ab_nbins') < datasize
                bottom = -300;
                height = 600;
                x1     = result_map('ab_i2') + 1;  % "invert" the analysis band
                x2     = result_map('ab_i1') - 1;  % to draw *excluded* spectrum
                new_boxes = genalyzer.get_annot_boxes(...
                    axis_type, datasize, bottom, height, x1, x2, fbin*xscalar);
                ab_boxes = [ab_boxes, new_boxes];
            end
            %
            % Tone Boxes = {[xll, xlr, xur, xul], [yll, ylr, yur, yul]]
            %
            tone_boxes = {};
            for k = 0 : tone_keys.Count - 1
                tk = tone_keys(k);
                bottom = -300;
                height = result_map(strcat(tk, ':mag_dbfs')) - bottom;
                x1     = result_map(strcat(tk, ':i1'));
                x2     = result_map(strcat(tk, ':i2'));
                new_boxes = genalyzer.get_annot_boxes(...
                    axis_type, datasize, bottom, height, x1, x2, fbin*xscalar);
                tone_boxes = [tone_boxes, new_boxes];
            end
            
            M = containers.Map({'labels', 'lines', 'ab_boxes', 'tone_boxes'}, ...
                               { labels,   lines,   ab_boxes,   tone_boxes});
        end
        function S = fa_result_string(result_map, result_key)
            [keys, values] = genalyzer.get_key_value_arrays(result_map);
            size = 0;
            [result, size] = calllib('gn', 'gn_fa_result_string_size', size, ...
                keys, numel(keys), values, numel(values), result_key);
            genalyzer.raise_exception_on_failure(result)
            S = blanks(size);
            [result, S] = calllib('gn', 'gn_fa_result_string', S, size, ...
                keys, numel(keys), values, numel(values), result_key);
            genalyzer.raise_exception_on_failure(result)
        end
    end
    methods (Static) % Fourier Transforms
        function Y = fft(X, varargin)
            % 1. fft(iq, navg=1, nfft=0, window=Window.NoWindow)
            %    Computes the FFT of interleaved normalized samples.  Data type of iq is 'double'.
            % 
            % 2. fft(i, q, navg=1, nfft=0, window=Window.NoWindow)
            %    Computes the FFT of split normalized samples.  Data type of i and q is 'double'.
            % 
            % 3. fft(iq, n, navg=1, nfft=0, window=Window.NoWindow, fmt=CodeFormat.TwosComplement)
            %    Computes the FFT of interleaved quantized samples.  Data type of iq is 'int16', 'int32', or
            %    'int64'.  Requires the second argument, n, which specifies code width, i.e., quantizer
            %    resolution.
            % 
            % 4. fft(i, q, n, navg=1, nfft=0, window=Window.NoWindow, fmt=CodeFormat.TwosComplement)
            %    Computes the FFT of split quantized samples.  Data type of i and q is 'int16', 'int32', or
            %    'int64'.  Requires the third argument, n, which represents code width, i.e., quantizer
            %    resolution.
            % 
            % Parameters
            % ----------
            % X : Input array, the dtype determines the interpretation of varargin.
            % 
            % Returns
            % -------
            % The discrete Fourier transform as an array of interleaved Real and Imaginary values.
            %
            type = genalyzer.check_array_1d(X, {'double', 'int16', 'int32', 'int64'});
            Xi = X;
            Xq = zeros(1, 0, type);
            base_index = 1;
            if isfloat(X) % normalized samples
                if 4 < nargin || ...                                % 4 < nargin implies split Xi and Xq
                   1 < nargin && numel(Xi) == numel(varargin{1})    % definitely not split if 1 == nargin
                    % infer split Xi and Xq; in MATLAB, cannot distinguish scalar from array
                    Xq = varargin{1};
                    base_index = 2;
                end
                if base_index < nargin
                    navg = varargin{base_index};
                else
                    navg = 1;
                end
                if base_index + 1 < nargin
                    nfft = varargin{base_index + 1};
                else
                    nfft = 0;
                end
                if base_index + 2 < nargin
                    win = varargin{base_index + 2};
                else
                    win = genalyzer.WindowNoWindow;
                end
            else % quantized samples
                if 6 < nargin || ...                                % 6 < nargin implies split Xi and Xq
                   2 < nargin && numel(Xi) == numel(varargin{1})    % definitely not split if 2 == nargin
                    % infer split Xi and Xq; in MATLAB, cannot distinguish scalar from array
                    Xq = varargin{1};
                    base_index = 2;
                end
                if base_index < nargin
                    n = varargin{base_index};
                else
                    throw(MException('Missing required parameter, n, code width'))
                end
                if base_index + 1 < nargin
                    navg = varargin{base_index + 1};
                else
                    navg = 1;
                end
                if base_index + 2 < nargin
                    nfft = varargin{base_index + 2};
                else
                    nfft = 0;
                end
                if base_index + 3 < nargin
                    win = varargin{base_index + 3};
                else
                    win = genalyzer.WindowNoWindow;
                end
                if base_index + 4 < nargin
                    fmt = varargin{base_index + 4};
                else
                    fmt = genalyzer.CodeFormatTwosComplement;
                end
            end
            size = 0;
            navg = max(0, navg);
            nfft = max(0, nfft);
            [result, size] = calllib('gn', 'gn_fft_size', size, numel(Xi), numel(Xq), navg, nfft);
            genalyzer.raise_exception_on_failure(result)
            Y = zeros(1, size, 'double');
            if isa(X, 'int16')
                [result, Y] = calllib('gn', 'gn_fft16', Y, numel(Y), Xi, numel(Xi), Xq, numel(Xq), n, navg, nfft, win, fmt);
            elseif isa(X, 'int32')
                [result, Y] = calllib('gn', 'gn_fft32', Y, numel(Y), Xi, numel(Xi), Xq, numel(Xq), n, navg, nfft, win, fmt);
            elseif isa(X, 'int64')
                [result, Y] = calllib('gn', 'gn_fft64', Y, numel(Y), Xi, numel(Xi), Xq, numel(Xq), n, navg, nfft, win, fmt);
            else
                [result, Y] = calllib('gn', 'gn_fft', Y, numel(Y), Xi, numel(Xi), Xq, numel(Xq), navg, nfft, win);
            end
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = rfft(X, varargin)
            % 1. rfft(a, navg=1, nfft=0, window=Window.NoWindow, scale=RfftScale.DbfsSin)
            %    Computes the FFT of real normalized samples.
            %
            % 2. rfft(a, n, navg=1, nfft=0, window=Window.NoWindow, fmt=CodeFormat.TwosComplement, scale=RfftScale.DbfsSin)
            %    Computes the FFT of real quantized samples.  Requires the second argument, n, which specifies
            %    code width, i.e., quantizer resolution.
            %
            % Parameters
            % ----------
            % X : Input array; the data type determines the interpretation of varargin.
            % varargin :
            %   class(a) = 'float64' :
            %       varargin{1} : navg
            %       varargin{2} : nfft
            %       varargin{3} : window
            %       varargin{4} : scale
            %   class(a) = 'int16', 'int32', 'int64' :
            %       varargin{1} : n
            %       varargin{2} : navg
            %       varargin{3} : nfft
            %       varargin{4} : window
            %       varargin{5} : fmt
            %       varargin{6} : scale
            %
            % Returns
            % -------
            % The discrete Fourier transform as an array of interleaved Real and Imaginary values.
            %
            genalyzer.check_array_1d(X, {'double', 'int16', 'int32', 'int64'});
            if isfloat(X) % normalized samples
                if nargin < 2
                    navg = 1;
                else
                    navg = varargin{1};
                end
                if nargin < 3
                    nfft = 0;
                else
                    nfft = varargin{2};
                end
                if nargin < 4
                    win = genalyzer.WindowNoWindow;
                else
                    win = varargin{3};
                end
                if nargin < 5
                    scale = genalyzer.RfftScaleDbfsSin;
                else
                    scale = varargin{4};
                end
            else % quantized samples
                if nargin < 2
                    throw(MException('Missing required parameter, n, code width'))
                else
                    n = varargin{1};
                end
                if nargin < 3
                    navg = 1;
                else
                    navg = varargin{2};
                end
                if nargin < 4
                    nfft = 0;
                else
                    nfft = varargin{3};
                end
                if nargin < 5
                    win = genalyzer.WindowNoWindow;
                else
                    win = varargin{4};
                end
                if nargin < 6
                    fmt = genalyzer.CodeFormatTwosComplement;
                else
                    fmt = varargin{5};
                end
                if nargin < 7
                    scale = genalyzer.RfftScaleDbfsSin;
                else
                    scale = varargin{6};
                end
            end
            size = 0;
            navg = max(0, navg);
            nfft = max(0, nfft);
            [result, size] = calllib('gn', 'gn_rfft_size', size, numel(X), navg, nfft);
            genalyzer.raise_exception_on_failure(result)
            Y = zeros(1, size, 'double');
            if isa(X, 'int16')
                [result, Y] = calllib('gn', 'gn_rfft16', Y, numel(Y), X, numel(X), n, navg, nfft, win, fmt, scale);
            elseif isa(X, 'int32')
                [result, Y] = calllib('gn', 'gn_rfft32', Y, numel(Y), X, numel(X), n, navg, nfft, win, fmt, scale);
            elseif isa(X, 'int64')
                [result, Y] = calllib('gn', 'gn_rfft64', Y, numel(Y), X, numel(X), n, navg, nfft, win, fmt, scale);
            else
                [result, Y] = calllib('gn', 'gn_rfft', Y, numel(Y), X, numel(X), navg, nfft, win, scale);
            end
            genalyzer.raise_exception_on_failure(result)
        end
    end
    methods (Static) % Fourier Utilities
        function X = alias(fs, freq, axis_type)
            X = 0.0;
            [result, X] = calllib('gn', 'gn_alias', X, fs, freq, axis_type);
            genalyzer.raise_exception_on_failure(result)
        end
        function X = coherent(nfft, fs, freq)
            X = 0.0;
            [result, X] = calllib('gn', 'gn_coherent', X, nfft, fs, freq);
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = fftshift(X)
            genalyzer.check_array_1d(X, {'double'});
            Y = zeros(1, numel(X), 'double');
            [result, Y] = calllib('gn', 'gn_fftshift', Y, numel(Y), X, numel(X));
            genalyzer.raise_exception_on_failure(result)
        end
        function X = freq_axis(nfft, axis_type, fs, axis_fmt)
            if nargin < 3
                fs = 1.0;
            end
            if nargin < 4
                axis_fmt = genalyzer.FreqAxisFormatFreq;
            end
            size = 0;
            [result, size] = calllib('gn', 'gn_freq_axis_size', size, nfft, axis_type);
            genalyzer.raise_exception_on_failure(result)
            X = zeros(1, size, 'double');
            [result, X] = calllib('gn', 'gn_freq_axis', X, numel(X), nfft, axis_type, fs, axis_fmt);
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = ifftshift(X)
            genalyzer.check_array_1d(X, {'double'});
            Y = zeros(1, numel(X), 'double');
            [result, Y] = calllib('gn', 'gn_ifftshift', Y, numel(Y), X, numel(X));
            genalyzer.raise_exception_on_failure(result)
        end
    end
    methods (Static) % Manager
        function mgr_clear()
            calllib('gn', 'gn_mgr_clear');
        end
        function X = mgr_compare(obj_key1, obj_key2)
            X = false;
            [result, X] = calllib('gn', 'gn_mgr_compare', X, obj_key1, obj_key2);
            genalyzer.raise_exception_on_failure(result)
        end
        function X = mgr_contains(obj_key)
            X = false;
            [result, X] = calllib('gn', 'gn_mgr_contains', X, obj_key);
            genalyzer.raise_exception_on_failure(result)
        end
        function mgr_remove(obj_key)
            calllib('gn', 'gn_mgr_remove', obj_key);
        end
        function S = mgr_save(obj_key, filename)
            if nargin < 2
                filename = '';
            end
            size = 0;
            [result, size] = calllib('gn', 'gn_mgr_save_filename_size', size, obj_key, filename);
            genalyzer.raise_exception_on_failure(result)
            S = blanks(size);
            [result, S] = calllib('gn', 'gn_mgr_save', S, numel(S), obj_key, filename);
            genalyzer.raise_exception_on_failure(result)
        end
        function X = mgr_size()
            X = 0;
            [~, X] = calllib('gn', 'gn_mgr_size', X);
        end
        function S = mgr_to_string(obj_key)
            if nargin < 1
                obj_key = '';
            end
            size = 0;
            [result, size] = calllib('gn', 'gn_mgr_to_string_size', size, obj_key);
            genalyzer.raise_exception_on_failure(result)
            S = blanks(size);
            [result, S] = calllib('gn', 'gn_mgr_to_string', S, numel(S), obj_key);
            genalyzer.raise_exception_on_failure(result)
        end
        function S = mgr_type(obj_key)
            size = 0;
            [result, size] = calllib('gn', 'gn_mgr_type_size', size, obj_key);
            genalyzer.raise_exception_on_failure(result)
            S = blanks(size);
            [result, S] = calllib('gn', 'gn_mgr_type', S, numel(S), obj_key);
            genalyzer.raise_exception_on_failure(result)
        end
    end
    methods (Static) % Signal Processing
        function Y = downsample(X, ratio, interleaved)
            type = genalyzer.check_array_1d(X, {'double', 'int16', 'int32', 'int64'});
            if nargin < 3
                interleaved = false;
            end
            size = 0;
            [result, size] = calllib('gn', 'gn_downsample_size', size, numel(X), ratio, interleaved);
            genalyzer.raise_exception_on_failure(result)
            Y = zeros(1, size, type);
            if isa(X, 'int16')
                [result, Y] = calllib('gn', 'gn_downsample16', Y, numel(Y), X, numel(X), ratio, interleaved);
            elseif isa(X, 'int32')
                [result, Y] = calllib('gn', 'gn_downsample32', Y, numel(Y), X, numel(X), ratio, interleaved);
            elseif isa(X, 'int64')
                [result, Y] = calllib('gn', 'gn_downsample64', Y, numel(Y), X, numel(X), ratio, interleaved);
            else
                [result, Y] = calllib('gn', 'gn_downsample', Y, numel(Y), X, numel(X), ratio, interleaved);
            end
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = fshift(X, varargin)
            % 1. fshift(iq, fs, fshift_)
            %    Performs frequency shift on interleaved normalized samples.  Data type of iq is 'double'.
            % 
            % 2. fshift(i, q, fs, fshift_)
            %    Performs frequency shift on split normalized samples.  Data type of i and q is 'double'.
            % 
            % 3. fshift(iq, n, fs, fshift_, fmt=CodeFormat.TwosComplement)
            %    Performs frequency shift on interleaved quantized samples.  Data type of iq is 'int16',
            %    'int32', or 'int64'.
            % 
            % 4. fshift(i, q, n, fs, fshift_, fmt=CodeFormat.TwosComplement)
            %    Performs frequency shift on split quantized samples.  Data type of i and q is 'int16',
            %    'int32', or 'int64'.
            % 
            % Parameters
            % ----------
            % X : Input array; the data type determines the interpretation of varargin.
            % 
            % Returns
            % -------
            % Frequency shifted version of X as an array of interleaved Real and Imaginary values.
            % The data type is the same as that of X.
            %
            type = genalyzer.check_array_1d(X, {'double', 'int16', 'int32', 'int64'});
            Xi = X;
            Xq = zeros(1, 0, type);
            base_index = 1;
            if isfloat(X) % normalized samples
                if nargin < 3
                    throw(MException('Missing required parameters'))
                end
                if 3 < nargin % Xi and Xq are split
                    Xq = varargin{1};
                    genalyzer.check_array_1d(Xq, {type});
                    base_index = 2;
                end
                fs = varargin{base_index};
                fshift_ = varargin{base_index + 1};
            else % quantized samples
                if nargin < 4
                    throw(MException('Missing required parameters'))
                end
                if 4 < nargin && isa(varargin{1}, type) && isrow(varargin{1}) && numel(Xi) == numel(varargin{1})
                    % infer split Xi and Xq
                    Xq = varargin{1};
                    base_index = 2;
                end
                n = varargin{base_index};
                fs = varargin{base_index + 1};
                fshift_ = varargin{base_index + 2};
                if base_index + 3 < nargin
                    fmt = varargin{base_index + 3};
                else
                    fmt = genalyzer.CodeFormatTwosComplement;
                end
            end
            size = 0;
            [result, size] = calllib('gn', 'gn_fshift_size', size, numel(Xi), numel(Xq));
            genalyzer.raise_exception_on_failure(result)
            Y = zeros(1, size, type);
            if isa(X, 'int16')
                [result, Y] = calllib('gn', 'gn_fshift16', Y, numel(Y), Xi, numel(Xi), Xq, numel(Xq), n, fs, fshift_, fmt);
            elseif isa(X, 'int32')
                [result, Y] = calllib('gn', 'gn_fshift32', Y, numel(Y), Xi, numel(Xi), Xq, numel(Xq), n, fs, fshift_, fmt);
            elseif isa(X, 'int64')
                [result, Y] = calllib('gn', 'gn_fshift64', Y, numel(Y), Xi, numel(Xi), Xq, numel(Xq), n, fs, fshift_, fmt);
            else
                [result, Y] = calllib('gn', 'gn_fshift', Y, numel(Y), Xi, numel(Xi), Xq, numel(Xq), fs, fshift_);
            end
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = normalize(X, n, fmt)
            if nargin < 3
                fmt = genalyzer.CodeFormatTwosComplement;
            end
            genalyzer.check_array_1d(X, {'int16', 'int32', 'int64'});
            Y = zeros(1, numel(X), 'double');
            if isa(X, 'int16')
                [result, Y] = calllib('gn', 'gn_normalize16', Y, numel(Y), X, numel(X), n, fmt);
            elseif isa(X, 'int32')
                [result, Y] = calllib('gn', 'gn_normalize32', Y, numel(Y), X, numel(X), n, fmt);
            else
                [result, Y] = calllib('gn', 'gn_normalize64', Y, numel(Y), X, numel(X), n, fmt);
            end
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = polyval(X, C)
            genalyzer.check_array_1d(X, {'double'});
            genalyzer.check_array_1d(C, {'double'});
            Y = zeros(1, numel(X), 'double');
            [result, Y] = calllib('gn', 'gn_polyval', Y, numel(Y), X, numel(X), C, numel(C));
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = quantize16(X, fsr, n, noise, fmt)
            if nargin < 4
                noise = 0.0;
            end
            if nargin < 5
                fmt = genalyzer.CodeFormatTwosComplement;
            end
            genalyzer.check_array_1d(X, {'double'});
            Y = zeros(1, numel(X), 'int16');
            [result, Y] = calllib('gn', 'gn_quantize16', Y, numel(Y), X, numel(X), fsr, n, noise, fmt);
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = quantize32(X, fsr, n, noise, fmt)
            if nargin < 4
                noise = 0.0;
            end
            if nargin < 5
                fmt = genalyzer.CodeFormatTwosComplement;
            end
            genalyzer.check_array_1d(X, {'double'});
            Y = zeros(1, numel(X), 'int32');
            [result, Y] = calllib('gn', 'gn_quantize32', Y, numel(Y), X, numel(X), fsr, n, noise, fmt);
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = quantize64(X, fsr, n, noise, fmt)
            if nargin < 4
                noise = 0.0;
            end
            if nargin < 5
                fmt = genalyzer.CodeFormatTwosComplement;
            end
            genalyzer.check_array_1d(X, {'double'});
            Y = zeros(1, numel(X), 'int64');
            [result, Y] = calllib('gn', 'gn_quantize64', Y, numel(Y), X, numel(X), fsr, n, noise, fmt);
            genalyzer.raise_exception_on_failure(result)
        end
        function Y = quantize(X, fsr, n, noise, fmt)
            if nargin < 4
                noise = 0.0;
            end
            if nargin < 5
                fmt = genalyzer.CodeFormatTwosComplement;
            end
            if n < 16 || 16 == n && genalyzer.CodeFormatTwosComplement == fmt
                Y = genalyzer.quantize16(X, fsr, n, noise, fmt);
            else
                Y = genalyzer.quantize32(X, fsr, n, noise, fmt);
            end
        end
    end
    methods (Static) % Waveforms
        function X = cos(size, fs, ampl, freq, phase, td, tj)
            if nargin < 5
                phase = 0.0;
            end
            if nargin < 6
                td = 0.0;
            end
            if nargin < 7
                tj = 0.0;
            end
            X = zeros(1, size, 'double');
            [result, X] = calllib('gn', 'gn_cos', X, numel(X), fs, ampl, freq, phase, td, tj);
            genalyzer.raise_exception_on_failure(result)
        end
        function X = gaussian(size, mean, sd)
            X = zeros(1, size, 'double');
            [result, X] = calllib('gn', 'gn_gaussian', X, numel(X), mean, sd);
            genalyzer.raise_exception_on_failure(result)
        end
        function X = ramp(size, start, stop, noise)
            if nargin < 4
                noise = 0.0;
            end
            X = zeros(1, size, 'double');
            [result, X] = calllib('gn', 'gn_ramp', X, numel(X), start, stop, noise);
            genalyzer.raise_exception_on_failure(result)
        end
        function X = sin(size, fs, ampl, freq, phase, td, tj)
            if nargin < 5
                phase = 0.0;
            end
            if nargin < 6
                td = 0.0;
            end
            if nargin < 7
                tj = 0.0;
            end
            X = zeros(1, size, 'double');
            [result, X] = calllib('gn', 'gn_sin', X, numel(X), fs, ampl, freq, phase, td, tj);
            genalyzer.raise_exception_on_failure(result)
        end
        function M = wf_analysis(X)
        % WF_ANALYSIS  Returns waveform analysis metrics
        %   M = WF_ANALYSIS(X)
        %
        % Parameters
        % ----------
        % X : array of one of the following types: double, int16, int32, int64
        %
        % Returns
        % -------
        % M : containers.Map with KeyType 'char' and ValueType 'double'
        %
        %   ===========================================================
        %	  Key        |  Description
        %   ===========================================================
        %     min        |  Minumum value
        %     max        |  Maximum value
        %     mid        |  Middle value ((max + min) / 2)
        %     range      |  Range (max - min)
        %     avg        |  Average value
        %     rms        |  RMS value
        %     rmsac      |  RMS value with DC removed
        %     min_index  |  Index of first occurence of minimum value
        %     max_index  |  Index of first occurence of maximum value
        %   ===========================================================

            genalyzer.check_array_1d(X, {'double', 'int16', 'int32', 'int64'});
            [keys, values] = genalyzer.get_analysis_containers(genalyzer.AnalysisTypeWaveform);
            if isa(X, 'int16')
                [result, keys, values] = calllib('gn', 'gn_wf_analysis16', ...
                    keys, numel(keys), values, numel(values), X, numel(X));
            elseif isa(X, 'int32')
                [result, keys, values] = calllib('gn', 'gn_wf_analysis32', ...
                    keys, numel(keys), values, numel(values), X, numel(X));
            elseif isa(X, 'int64')
                [result, keys, values] = calllib('gn', 'gn_wf_analysis64', ...
                    keys, numel(keys), values, numel(values), X, numel(X));
            else
                [result, keys, values] = calllib('gn', 'gn_wf_analysis', ...
                    keys, numel(keys), values, numel(values), X, numel(X));
            end
            genalyzer.raise_exception_on_failure(result)
            M = containers.Map(keys, values);
        end
    end
end