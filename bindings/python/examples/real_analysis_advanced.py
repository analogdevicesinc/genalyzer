
def main():
    import genalyzer as gn
    #print("Library path: {}".format(gn.advanced._genalyzer._libpath))
    #print("Version: {}\n".format(gn.advanced.__version__))

    #
    # Setup
    #
    plot        = True
    navg        = 2
    nfft        = 1024 * 256
    fs          = 1e9
    fsr         = 2.0
    ampl_dbfs   = -1.0
    freq        = 10e6
    phase       = 0.110
    td          = 0.0
    tj          = 0.0
    poco        = [0.0, 1.0, 0.0, 0.003]        # distortion polynomial coefficients
    qres        = 12                            # quantizer resolution
    qnoise_dbfs = -63.0                         # quantizer noise
    code_fmt    = gn.advanced.CodeFormat.TWOS_COMPLEMENT
    sig_type    = gn.advanced.DnlSignal.TONE
    inl_fit     = gn.advanced.InlLineFit.BEST_FIT
    rfft_scale  = gn.advanced.RfftScale.DBFS_SIN
    window      = gn.advanced.Window.NO_WINDOW

    ########################################################################

    npts = navg * nfft
    ampl = (fsr / 2) * 10**(ampl_dbfs / 20)
    qnoise = 10**(qnoise_dbfs / 20)
    ssb_fund = 4
    ssb_rest = 3
    if gn.advanced.Window.NO_WINDOW == window:
        freq = gn.advanced.coherent(nfft, fs, freq)
        ssb_fund = 0
        ssb_rest = 0

    #
    # Signal generation and processing
    #
    awf = gn.advanced.cos(npts, fs, ampl, freq, phase, td, tj)
    awf = gn.advanced.polyval(awf, poco)
    qwf = gn.advanced.quantize(awf, fsr, qres, qnoise, code_fmt)
    hist = gn.advanced.hist(qwf, qres, code_fmt)
    dnl = gn.advanced.dnl(hist, sig_type)
    inl = gn.advanced.inl(dnl, inl_fit)
    fft_cplx = gn.advanced.rfft(qwf, qres, navg, nfft, window, code_fmt, rfft_scale)

    #
    # Fourier analysis configuration
    #
    key = 'fa'
    gn.advanced.mgr_remove(key)
    gn.advanced.fa_create(key)
    gn.advanced.fa_analysis_band(key, "fdata*0.0", "fdata*1.0")
    gn.advanced.fa_fixed_tone(key, 'A', gn.advanced.FaCompTag.SIGNAL, freq, ssb_fund)
    # gn.advanced.fa_max_tone(key, 'A', gn.advanced.FaCompTag.SIGNAL, ssb_fund)
    gn.advanced.fa_hd(key, 3)
    gn.advanced.fa_ssb(key, gn.advanced.FaSsb.DEFAULT, ssb_rest)
    gn.advanced.fa_ssb(key, gn.advanced.FaSsb.DC, -1)
    gn.advanced.fa_ssb(key, gn.advanced.FaSsb.SIGNAL, -1)
    gn.advanced.fa_ssb(key, gn.advanced.FaSsb.WO, -1)
    gn.advanced.fa_fsample(key, fs)
    print(gn.advanced.fa_preview(key, False))
    # fn = gn.advanced.mgr_save(key)

    #
    # Analysis
    #
    wf_results   = gn.advanced.wf_analysis(qwf)
    hist_results = gn.advanced.hist_analysis(hist)
    dnl_results  = gn.advanced.dnl_analysis(dnl)
    inl_results  = gn.advanced.inl_analysis(inl)
    fft_results  = gn.advanced.fft_analysis(key, fft_cplx, nfft)

    #
    # Print results
    #
    print("\nWaveform Analysis Results:")
    for k, v in wf_results.items():
        print("{:10s}{:16.6f}".format(k, v))
    print("\nHistogram Analysis Results:")
    for k, v in hist_results.items():
        print("{:10s}{:16.6f}".format(k, v))
    print("\nDNL Analysis Results:")
    for k, v in dnl_results.items():
        print("{:10s}{:16.6f}".format(k, v))
    print("\nINL Analysis Results:")
    for k, v in inl_results.items():
        print("{:10s}{:16.6f}".format(k, v))
    print("\nFourier Analysis Results:")
    for k in ['fsnr', 'dc:mag_dbfs', 'A:freq', 'A:ffinal', 'A:mag_dbfs', 'A:phase']:
        print("{:20s}{:20.6f}".format(k, fft_results[k]))

    #
    # Plot
    #
    if plot:
        import matplotlib.pyplot as pl
        from matplotlib.patches import Rectangle as MPRect
        code_axis = gn.advanced.code_axis(qres, code_fmt)
        freq_axis = gn.advanced.freq_axis(nfft, gn.advanced.FreqAxisType.REAL, fs)
        fft_db = gn.advanced.db(fft_cplx)
        fig = pl.figure(1)
        fig.clf()
        pl.subplot2grid((4, 2), (0, 0))
        pl.plot(qwf)
        pl.subplot2grid((4, 2), (1, 0))
        pl.plot(code_axis, hist)
        pl.subplot2grid((4, 2), (0, 1))
        pl.plot(code_axis, dnl)
        pl.subplot2grid((4, 2), (1, 1))
        pl.plot(code_axis, inl)
        fftax = pl.subplot2grid((4, 2), (2, 0), rowspan=2, colspan=2)
        pl.plot(freq_axis, fft_db)
        pl.grid(True)
        pl.xlim(freq_axis[0], freq_axis[-1])
        pl.ylim(-140.0, 20.0)
        annots = gn.advanced.fa_annotations(fft_results)
        for x, y, label in annots["labels"]:
            pl.annotate(label, xy=(x, y), ha='center', va='bottom')
        for line in annots["lines"]:
            pl.axline((line[0], line[1]), (line[2], line[3]), c='pink')
        for box in annots["ab_boxes"]:
            fftax.add_patch(MPRect((box[0], box[1]), box[2], box[3],
                ec='lightgray', fc='gainsboro', fill=True, hatch='x'))
        for box in annots["tone_boxes"]:
            fftax.add_patch(MPRect((box[0], box[1]), box[2], box[3],
                ec='pink', fc='pink', fill=True, hatch='x'))

        pl.tight_layout()
        pl.show()


if __name__ == "__main__":
    main()