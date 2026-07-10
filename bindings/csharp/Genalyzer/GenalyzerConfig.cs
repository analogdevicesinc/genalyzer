// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Genalyzer
{
    /// <summary>
    /// Managed wrapper for the simplified-beta C API
    /// (<c>cgenalyzer_simplified_beta.h</c>).
    ///
    /// Encapsulates the opaque <c>gn_config</c> pointer and exposes all
    /// configuration, generation, processing, and analysis calls as instance
    /// methods.  Implements <see cref="IDisposable"/> to ensure the native
    /// config struct is freed.
    /// </summary>
    public sealed class GenalyzerConfig : IDisposable
    {
        // The opaque gn_config pointer (a pointer-to-pointer in C)
        private IntPtr _handle = IntPtr.Zero;
        private bool   _disposed;

        // The native config struct RETAINS the tone-array pointers passed to
        // gn_config_set_tone_* / gn_config_gen_tone and dereferences them later
        // during waveform generation.  We therefore pin the arrays in
        // long-lived GCHandles owned by this instance (rather than relying on
        // transient P/Invoke marshaling, which would leave a dangling pointer
        // once the call returns) and free them in Dispose.
        private GCHandle _toneFreqPin;
        private GCHandle _toneAmplPin;
        private GCHandle _tonePhasePin;

        /// <summary>Creates an empty configuration object.</summary>
        public GenalyzerConfig() { }

        // ---------------------------------------------------------------
        // IDisposable
        // ---------------------------------------------------------------

        /// <summary>Frees the native configuration structure.</summary>
        public void Dispose()
        {
            if (!_disposed)
            {
                if (_handle != IntPtr.Zero)
                    NativeMethods.gn_config_free(ref _handle);
                FreeTonePins();
                _disposed = true;
            }
            GC.SuppressFinalize(this);
        }

        ~GenalyzerConfig() => Dispose();

        /// <summary>
        /// Pins <paramref name="arr"/> in <paramref name="slot"/> (freeing any
        /// array previously pinned there) and returns the stable address to
        /// hand to the native config, which retains it past the call.
        /// </summary>
        private static IntPtr PinTone(ref GCHandle slot, double[] arr)
        {
            if (arr == null) throw new ArgumentNullException(nameof(arr));
            if (slot.IsAllocated) slot.Free();
            slot = GCHandle.Alloc(arr, GCHandleType.Pinned);
            return slot.AddrOfPinnedObject();
        }

        /// <summary>Releases the pinned tone-array handles, if allocated.</summary>
        private void FreeTonePins()
        {
            if (_toneFreqPin.IsAllocated)  _toneFreqPin.Free();
            if (_toneAmplPin.IsAllocated)  _toneAmplPin.Free();
            if (_tonePhasePin.IsAllocated) _tonePhasePin.Free();
        }

        // ---------------------------------------------------------------
        // Individual setters / getters
        // ---------------------------------------------------------------

        /// <summary>Sets the tone type (real cosine / real sine / complex exp).</summary>
        public void SetToneType(ToneType ttype)
            => Util.Check(NativeMethods.gn_config_set_ttype((int)ttype, ref _handle));

        /// <summary>Sets the number of sample points.</summary>
        public void SetNpts(int npts)
            => Util.Check(NativeMethods.gn_config_set_npts(
                (UIntPtr)npts, ref _handle));

        /// <summary>Gets the number of sample points.</summary>
        public int GetNpts()
        {
            Util.Check(NativeMethods.gn_config_get_npts(
                out UIntPtr npts, ref _handle));
            return (int)npts;
        }

        /// <summary>Sets the sample rate (S/s).</summary>
        public void SetSampleRate(double sampleRate)
            => Util.Check(NativeMethods.gn_config_set_sample_rate(
                sampleRate, ref _handle));

        /// <summary>Gets the sample rate (S/s).</summary>
        public double GetSampleRate()
        {
            Util.Check(NativeMethods.gn_config_get_sample_rate(
                out double rate, ref _handle));
            return rate;
        }

        /// <summary>Sets the data rate (S/s).</summary>
        public void SetDataRate(double dataRate)
            => Util.Check(NativeMethods.gn_config_set_data_rate(
                dataRate, ref _handle));

        /// <summary>Sets the frequency shift.</summary>
        public void SetShiftFreq(double shiftFreq)
            => Util.Check(NativeMethods.gn_config_set_shift_freq(
                shiftFreq, ref _handle));

        /// <summary>Sets the number of tones to generate.</summary>
        public void SetNumTones(int numTones)
            => Util.Check(NativeMethods.gn_config_set_num_tones(
                (UIntPtr)numTones, ref _handle));

        /// <summary>Sets the tone frequency array.</summary>
        /// <remarks>The array is pinned for the lifetime of this object (or
        /// until replaced by a later call) because the native config retains
        /// the pointer for use during tone generation.</remarks>
        public void SetToneFreq(double[] toneFreq)
            => Util.Check(NativeMethods.gn_config_set_tone_freq(
                PinTone(ref _toneFreqPin, toneFreq), ref _handle));

        /// <summary>Sets the tone amplitude array.</summary>
        /// <remarks>The array is pinned for the lifetime of this object (or
        /// until replaced by a later call); see <see cref="SetToneFreq"/>.</remarks>
        public void SetToneAmpl(double[] toneAmpl)
            => Util.Check(NativeMethods.gn_config_set_tone_ampl(
                PinTone(ref _toneAmplPin, toneAmpl), ref _handle));

        /// <summary>Sets the tone phase array.</summary>
        /// <remarks>The array is pinned for the lifetime of this object (or
        /// until replaced by a later call); see <see cref="SetToneFreq"/>.</remarks>
        public void SetTonePhase(double[] tonePhase)
            => Util.Check(NativeMethods.gn_config_set_tone_phase(
                PinTone(ref _tonePhasePin, tonePhase), ref _handle));

        /// <summary>Sets the full-scale range.</summary>
        public void SetFsr(double fsr)
            => Util.Check(NativeMethods.gn_config_set_fsr(fsr, ref _handle));

        /// <summary>Sets the quantization resolution.</summary>
        public void SetQres(int qres)
            => Util.Check(NativeMethods.gn_config_set_qres(qres, ref _handle));

        /// <summary>Sets the input-referred RMS noise.</summary>
        public void SetNoiseRms(double noiseRms)
            => Util.Check(NativeMethods.gn_config_set_noise_rms(
                noiseRms, ref _handle));

        /// <summary>Sets the code format.</summary>
        public void SetCodeFormat(CodeFormat fmt)
            => Util.Check(NativeMethods.gn_config_set_code_format(
                (int)fmt, ref _handle));

        /// <summary>Sets the FFT size.</summary>
        public void SetNfft(int nfft)
            => Util.Check(NativeMethods.gn_config_set_nfft(
                (UIntPtr)nfft, ref _handle));

        /// <summary>Gets the FFT size.</summary>
        public int GetNfft()
        {
            Util.Check(NativeMethods.gn_config_get_nfft(
                out UIntPtr nfft, ref _handle));
            return (int)nfft;
        }

        /// <summary>Sets the number of FFT averages.</summary>
        public void SetFftNavg(int navg)
            => Util.Check(NativeMethods.gn_config_set_fft_navg(
                (UIntPtr)navg, ref _handle));

        /// <summary>Sets the window function.</summary>
        public void SetWindow(Window win)
            => Util.Check(NativeMethods.gn_config_set_win((int)win, ref _handle));

        /// <summary>Sets the fundamental single-side bins.</summary>
        public void SetSsbFund(int ssb)
            => Util.Check(NativeMethods.gn_config_set_ssb_fund(ssb, ref _handle));

        /// <summary>Sets the rest single-side bins.</summary>
        public void SetSsbRest(int ssb)
            => Util.Check(NativeMethods.gn_config_set_ssb_rest(ssb, ref _handle));

        /// <summary>Sets the maximum harmonic order.</summary>
        public void SetMaxHarmOrder(int order)
            => Util.Check(NativeMethods.gn_config_set_max_harm_order(
                order, ref _handle));

        /// <summary>Sets the DNL analysis signal type.</summary>
        public void SetDnlaSignalType(DnlSignal signalType)
            => Util.Check(NativeMethods.gn_config_set_dnla_signal_type(
                (int)signalType, ref _handle));

        /// <summary>Sets the INL analysis line-fit type.</summary>
        public void SetInlaFit(InlLineFit fit)
            => Util.Check(NativeMethods.gn_config_set_inla_fit(
                (int)fit, ref _handle));

        /// <summary>Sets the ramp start value.</summary>
        public void SetRampStart(double start)
            => Util.Check(NativeMethods.gn_config_set_ramp_start(
                start, ref _handle));

        /// <summary>Sets the ramp stop value.</summary>
        public void SetRampStop(double stop)
            => Util.Check(NativeMethods.gn_config_set_ramp_stop(
                stop, ref _handle));

        /// <summary>Gets the code-density array size computed by the config.</summary>
        public int GetCodeDensitySize()
        {
            Util.Check(NativeMethods.gn_config_get_code_density_size(
                out UIntPtr sz, ref _handle));
            return (int)sz;
        }

        // ---------------------------------------------------------------
        // Compound configurators
        // ---------------------------------------------------------------

        /// <summary>
        /// Configures all tone-generation parameters in one call.
        /// </summary>
        public void ConfigGenTone(ToneType ttype, int npts, double sampleRate,
            double[] toneFreq, double[] toneAmpl, double[] tonePhase)
        {
            if (toneFreq.Length != toneAmpl.Length ||
                toneFreq.Length != tonePhase.Length)
                throw new ArgumentException(
                    "toneFreq, toneAmpl, and tonePhase must have equal length.");
            // Pin all three arrays for the lifetime of this object; the native
            // config retains the pointers and dereferences them later in
            // GenRealTone / GenComplexTone.
            IntPtr freqPtr  = PinTone(ref _toneFreqPin,  toneFreq);
            IntPtr amplPtr  = PinTone(ref _toneAmplPin,  toneAmpl);
            IntPtr phasePtr = PinTone(ref _tonePhasePin, tonePhase);
            Util.Check(NativeMethods.gn_config_gen_tone(
                (int)ttype, (UIntPtr)npts, sampleRate,
                (UIntPtr)toneFreq.Length,
                freqPtr, amplPtr, phasePtr,
                ref _handle));
        }

        /// <summary>Configures ramp generation parameters.</summary>
        public void ConfigGenRamp(int npts, double rampStart, double rampStop)
            => Util.Check(NativeMethods.gn_config_gen_ramp(
                (UIntPtr)npts, rampStart, rampStop, ref _handle));

        /// <summary>Configures quantization parameters.</summary>
        public void ConfigQuantize(int npts, double fsr, int qres,
            double qnoise = 0.0)
            => Util.Check(NativeMethods.gn_config_quantize(
                (UIntPtr)npts, fsr, qres, qnoise, ref _handle));

        /// <summary>Configures histogram / NLA parameters.</summary>
        public void ConfigHistzNla(int npts, int qres)
            => Util.Check(NativeMethods.gn_config_histz_nla(
                (UIntPtr)npts, qres, ref _handle));

        /// <summary>Configures FFT parameters.</summary>
        public void ConfigFftz(int npts, int qres, int navg, int nfft,
            Window win)
            => Util.Check(NativeMethods.gn_config_fftz(
                (UIntPtr)npts, qres, (UIntPtr)navg, (UIntPtr)nfft,
                (int)win, ref _handle));

        /// <summary>
        /// Configures Fourier analysis with a fixed tone frequency.
        /// </summary>
        public void ConfigFa(double fixedToneFreq)
            => Util.Check(NativeMethods.gn_config_fa(fixedToneFreq, ref _handle));

        /// <summary>
        /// Configures Fourier analysis using automatic tone detection.
        /// </summary>
        public void ConfigFaAuto(byte ssbWidth)
            => Util.Check(NativeMethods.gn_config_fa_auto(ssbWidth, ref _handle));

        // ---------------------------------------------------------------
        // Waveform generation
        // ---------------------------------------------------------------

        /// <summary>Generates a ramp waveform based on the current configuration.</summary>
        public double[] GenRamp()
        {
            Util.Check(NativeMethods.gn_gen_ramp(out IntPtr ptr, ref _handle));
            return MarshalDoubleArray(ptr, GetNpts());
        }

        /// <summary>Generates a real tone waveform based on the current configuration.</summary>
        public double[] GenRealTone()
        {
            Util.Check(NativeMethods.gn_gen_real_tone(out IntPtr ptr, ref _handle));
            return MarshalDoubleArray(ptr, GetNpts());
        }

        /// <summary>
        /// Generates a complex (I/Q) tone waveform based on the current
        /// configuration.
        /// </summary>
        public (double[] I, double[] Q) GenComplexTone()
        {
            Util.Check(NativeMethods.gn_gen_complex_tone(
                out IntPtr ptrI, out IntPtr ptrQ, ref _handle));
            int n = GetNpts();
            return (MarshalDoubleArray(ptrI, n), MarshalDoubleArray(ptrQ, n));
        }

        // ---------------------------------------------------------------
        // Processing
        // ---------------------------------------------------------------

        /// <summary>
        /// Quantizes the supplied double waveform using the current
        /// configuration and returns a 32-bit integer array.
        /// </summary>
        public int[] Quantize(double[] input)
        {
            Util.Check(NativeMethods.gn_quantize(
                out IntPtr ptr, input, ref _handle));
            return MarshalInt32Array(ptr, GetNpts());
        }

        /// <summary>
        /// Computes the FFT of the quantized I/Q waveforms.
        /// <paramref name="inQ"/> may be null for a real-only FFT.
        /// </summary>
        public double[] Fftz(int[] inI, int[]? inQ = null)
        {
            Util.Check(NativeMethods.gn_fftz(
                out IntPtr ptr, inI, inQ ?? Array.Empty<int>(), ref _handle));
            int nfft = GetNfft();
            return MarshalDoubleArray(ptr, 2 * nfft);
        }

        /// <summary>
        /// Computes the histogram of the quantized waveform.
        /// </summary>
        public ulong[] Histz(int[] qwf)
        {
            Util.Check(NativeMethods.gn_histz(
                out IntPtr ptr, out UIntPtr histLen, qwf, ref _handle));
            return MarshalUInt64Array(ptr, (int)histLen);
        }

        /// <summary>Computes the DNL from a histogram array.</summary>
        public double[] Dnlz(ulong[] hist)
        {
            Util.Check(NativeMethods.gn_dnlz(
                out IntPtr ptr, out UIntPtr len, hist, ref _handle));
            return MarshalDoubleArray(ptr, (int)len);
        }

        /// <summary>Computes the INL from a DNL array.</summary>
        public double[] Inlz(double[] dnl)
        {
            Util.Check(NativeMethods.gn_inlz(
                out IntPtr ptr, out UIntPtr len, dnl, ref _handle));
            return MarshalDoubleArray(ptr, (int)len);
        }

        // ---------------------------------------------------------------
        // High-level analysis (get all results)
        // ---------------------------------------------------------------

        /// <summary>
        /// Runs waveform analysis on a quantized 32-bit waveform and returns
        /// all results.
        /// </summary>
        public Dictionary<string, double> GetWfaResults(int[] qwf)
        {
            Util.Check(NativeMethods.gn_get_wfa_results(
                out IntPtr rkeys, out IntPtr rvalues,
                out UIntPtr size, qwf, ref _handle));
            return MarshalResults(rkeys, rvalues, (int)size);
        }

        /// <summary>
        /// Runs histogram analysis and returns all results.
        /// </summary>
        public Dictionary<string, double> GetHaResults(ulong[] hist)
        {
            Util.Check(NativeMethods.gn_get_ha_results(
                out IntPtr rkeys, out IntPtr rvalues,
                out UIntPtr size, hist, ref _handle));
            return MarshalResults(rkeys, rvalues, (int)size);
        }

        /// <summary>
        /// Runs DNL analysis and returns all results.
        /// </summary>
        public Dictionary<string, double> GetDnlaResults(double[] dnl)
        {
            Util.Check(NativeMethods.gn_get_dnla_results(
                out IntPtr rkeys, out IntPtr rvalues,
                out UIntPtr size, dnl, ref _handle));
            return MarshalResults(rkeys, rvalues, (int)size);
        }

        /// <summary>
        /// Runs INL analysis and returns all results.
        /// </summary>
        public Dictionary<string, double> GetInlaResults(double[] inl)
        {
            Util.Check(NativeMethods.gn_get_inla_results(
                out IntPtr rkeys, out IntPtr rvalues,
                out UIntPtr size, inl, ref _handle));
            return MarshalResults(rkeys, rvalues, (int)size);
        }

        /// <summary>
        /// Runs Fourier analysis and returns a single named metric.
        /// </summary>
        public double GetFaSingleResult(double[] fftIlv, string metricName)
        {
            Util.Check(NativeMethods.gn_get_fa_single_result(
                out double rvalue, metricName, fftIlv, ref _handle));
            return rvalue;
        }

        /// <summary>
        /// Runs Fourier analysis and returns all results.
        /// </summary>
        public Dictionary<string, double> GetFaResults(double[] fftIlv)
        {
            Util.Check(NativeMethods.gn_get_fa_results(
                out IntPtr rkeys, out IntPtr rvalues,
                out UIntPtr size, fftIlv, ref _handle));
            return MarshalResults(rkeys, rvalues, (int)size);
        }

        // ---------------------------------------------------------------
        // Private marshalling helpers
        // ---------------------------------------------------------------

        private static double[] MarshalDoubleArray(IntPtr ptr, int length)
        {
            if (ptr == IntPtr.Zero || length <= 0) return Array.Empty<double>();
            var arr = new double[length];
            Marshal.Copy(ptr, arr, 0, length);
            return arr;
        }

        private static int[] MarshalInt32Array(IntPtr ptr, int length)
        {
            if (ptr == IntPtr.Zero || length <= 0) return Array.Empty<int>();
            var arr = new int[length];
            Marshal.Copy(ptr, arr, 0, length);
            return arr;
        }

        private static ulong[] MarshalUInt64Array(IntPtr ptr, int length)
        {
            if (ptr == IntPtr.Zero || length <= 0) return Array.Empty<ulong>();
            var arr = new ulong[length];
            // Marshal.Copy does not have a ulong overload; copy via long.
            var longs = new long[length];
            Marshal.Copy(ptr, longs, 0, length);
            for (int i = 0; i < length; i++)
                arr[i] = (ulong)longs[i];
            return arr;
        }

        private static Dictionary<string, double> MarshalResults(
            IntPtr rkeys, IntPtr rvalues, int count)
        {
            var dict = new Dictionary<string, double>(count, StringComparer.Ordinal);
            if (count <= 0 || rkeys == IntPtr.Zero || rvalues == IntPtr.Zero)
                return dict;

            var rawValues = new double[count];
            Marshal.Copy(rvalues, rawValues, 0, count);

            for (int i = 0; i < count; i++)
            {
                // rkeys is a char** - read each pointer then the string it
                // points to.  Decode as UTF-8 via the shared helper so all
                // native-string reads use one consistent encoding.
                IntPtr strPtr = Marshal.ReadIntPtr(rkeys, i * IntPtr.Size);
                string key = Util.PtrToStringUtf8(strPtr);
                dict[key] = rawValues[i];
            }
            return dict;
        }
    }
}
