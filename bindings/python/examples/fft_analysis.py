def main():
    import numpy as np
    import genalyzer as gn

    # print("Library path: {}".format(gn._genalyzer._libpath))
    # print("Version: {}\n".format(gn.__version__))

    #
    # Setup
    #
    plot = True
    navg = 2
    nfft = 1024 * 16
    fs = 1e9
    fdata = fs / 1
    fshift = 0e6
    fsr = 2.0
    ampl_dbfs = -1.0
    freq = 70e6
    phase = 0.110
    td = 0.0
    tj = 0.0
    qpe = (np.pi / 2) * 1e-5  # quadrature phase error
    poco = [0.0, 1.0, 0.0, 0.003]  # distortion polynomial coefficients
    qres = 12  # quantizer resolution
    qnoise_dbfs = -63.0  # quantizer noise
    code_fmt = gn.CodeFormat.TWOS_COMPLEMENT
    axis_fmt = gn.FreqAxisFormat.FREQ
    axis_type = gn.FreqAxisType.DC_CENTER
    window = gn.Window.NO_WINDOW

    ########################################################################

    dsr = int(fs / fdata)
    npts = navg * nfft * dsr
    ampl = (fsr / 2) * 10 ** (ampl_dbfs / 20)
    qnoise = 10 ** (qnoise_dbfs / 20)
    ssb_fund = 4
    ssb_rest = 3
    if gn.Window.NO_WINDOW == window:
        freq = gn.coherent(nfft, fdata, freq)
        fbin = fdata / nfft
        fshift = round(fshift / fbin) * fbin
        ssb_fund = 0
        ssb_rest = 0

    #
    # Signal generation and processing
    #
    awfi = gn.cos(npts, fs, ampl, freq, phase, td, tj)
    awfq = gn.sin(npts, fs, ampl, freq, phase + qpe, td, tj)
    awfi = gn.polyval(awfi, poco)
    awfq = gn.polyval(awfq, poco)
    qwfi = gn.quantize(awfi, fsr, qres, qnoise, code_fmt)
    qwfq = gn.quantize(awfq, fsr, qres, qnoise, code_fmt)
    xwf = gn.fshift(qwfi, qwfq, qres, fs, fshift, code_fmt)
    xwf = gn.downsample(xwf, dsr, True)
    fft_cplx = gn.fft(xwf, qres, navg, nfft, window, code_fmt)

    #
    # Fourier analysis configuration
    #
    key = "fa"
    gn.mgr_remove(key)
    gn.fa_create(key)
    gn.fa_analysis_band(key, "fdata*0.0", "fdata*1.0")
    gn.fa_fixed_tone(key, "A", gn.FaCompTag.SIGNAL, freq, ssb_fund)
    # gn.fa_max_tone(key, 'A', gn.FaCompTag.SIGNAL, ssb_fund)
    gn.fa_conv_offset(key, 0.0 != fshift)
    gn.fa_hd(key, 3)
    gn.fa_ssb(key, gn.FaSsb.DEFAULT, ssb_rest)
    gn.fa_ssb(key, gn.FaSsb.DC, -1)
    gn.fa_ssb(key, gn.FaSsb.SIGNAL, -1)
    gn.fa_ssb(key, gn.FaSsb.WO, -1)
    gn.fa_fdata(key, fdata)
    gn.fa_fsample(key, fs)
    gn.fa_fshift(key, fshift)
    print(gn.fa_preview(key, True))
    # fn = gn.mgr_save(key)

    #
    # Fourier analysis execution
    #
    results = gn.fft_analysis(key, fft_cplx, nfft, axis_type)
    carrier = gn.fa_result_string(results, "carrierindex")
    maxspur = gn.fa_result_string(results, "maxspurindex")

    #
    # Print results
    #
    for k in [
        "fsnr",
        "sfdr",
        "dc:mag_dbfs",
        "A:freq",
        "A:ffinal",
        "A:mag_dbfs",
        "A:phase",
        "-3A:mag_dbc",
    ]:
        print("{:20s}{:20.6f}".format(k, results[k]))
    print("{:20s}{:20s}".format("Carrier", carrier))
    print("{:20s}{:20s}".format("MaxSpur", maxspur))

    #
    # Plot
    #
    if plot:
        import matplotlib.pyplot as pl
        from matplotlib.patches import Rectangle as MPRect

        freq_axis = gn.freq_axis(nfft, axis_type, fdata, axis_fmt)
        fft_db = gn.db(fft_cplx)
        if gn.FreqAxisType.DC_CENTER == axis_type:
            fft_db = gn.fftshift(fft_db)
        fig = pl.figure(1)
        fig.clf()
        pl.plot(freq_axis, fft_db)
        pl.grid(True)
        pl.xlim(freq_axis[0], freq_axis[-1])
        pl.ylim(-140.0, 20.0)
        annots = gn.fa_annotations(results, axis_type, axis_fmt)
        for x, y, label in annots["labels"]:
            pl.annotate(label, xy=(x, y), ha="center", va="bottom")
        for line in annots["lines"]:
            pl.axline((line[0], line[1]), (line[2], line[3]), c="pink")
        for box in annots["ab_boxes"]:
            fig.axes[0].add_patch(
                MPRect(
                    (box[0], box[1]),
                    box[2],
                    box[3],
                    ec="lightgray",
                    fc="gainsboro",
                    fill=True,
                    hatch="x",
                )
            )
        for box in annots["tone_boxes"]:
            fig.axes[0].add_patch(
                MPRect(
                    (box[0], box[1]),
                    box[2],
                    box[3],
                    ec="pink",
                    fc="pink",
                    fill=True,
                    hatch="x",
                )
            )
        pl.show()


if __name__ == "__main__":
    main()
