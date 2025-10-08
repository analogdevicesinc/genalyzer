# Example of Fourier analysis using Genalyzer and an ADI SDR
# Requires: genalyzer, pyadi-iio, matplotlib, numpy
def main():

    import time

    import numpy as np
    import genalyzer as gn

    from importlib.util import find_spec
    if find_spec("adi") is None or find_spec("iio") is None:
        print("pyadi-iio must be installed to run this example")
        return

    import adi

    #
    # Setup Example parameters
    #
    plot = True
    navg = 8
    nfft = 2**16
    fshift = 0e6
    tone_freq = 70e6
    uri = "ip:192.168.86.35"
    axis_fmt = gn.FreqAxisFormat.FREQ
    axis_type = gn.FreqAxisType.DC_CENTER
    window = gn.Window.BLACKMAN_HARRIS
    # Single sideband parameters (width in bins)
    ssb_fund = 4
    ssb_rest = 3
    if gn.Window.NO_WINDOW == window:
        ssb_fund = 0
        ssb_rest = 0

    # Setup SDR
    sdr = adi.adrv9009_zu11eg(uri)
    fs = int(sdr.rx_sample_rate)
    sdr.rx_lo = int(1e9)
    sdr.tx_lo = int(1e9)
    sdr.rx_buffer_size = nfft * navg
    sdr.rx_enabled_channels = [0]

    # Start transmission
    sdr.dds_single_tone(tone_freq, 0.9, 0)
    time.sleep(2)

    # Acquisition and processing
    for i in range(16):
        x = sdr.rx()

    fft_cplx, fft_db, fft_freq_out = gn.pai.fft(sdr, x, navg, window)

    #
    # Fourier analysis configuration
    #
    key = "fa"
    gn.mgr_remove(key)
    gn.fa_create(key)
    gn.fa_analysis_band(key, "fdata*0.0", "fdata*1.0")
    gn.fa_fixed_tone(key, "A", gn.FaCompTag.SIGNAL, tone_freq, ssb_fund)
    gn.fa_conv_offset(key, 0.0 != fshift)
    gn.fa_hd(key, 3)
    gn.fa_ssb(key, gn.FaSsb.DEFAULT, ssb_rest)
    gn.fa_ssb(key, gn.FaSsb.DC, -1)
    gn.fa_ssb(key, gn.FaSsb.SIGNAL, -1)
    gn.fa_ssb(key, gn.FaSsb.WO, -1)
    gn.fa_fdata(key, fs)
    gn.fa_fsample(key, fs)
    gn.fa_fshift(key, fshift)
    print(gn.fa_preview(key, True))

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

        # freq_axis = gn.freq_axis(nfft, axis_type, fs, axis_fmt)
        # fft_db = gn.db(fft_cplx)
        fig = pl.figure(1)
        fig.clf()
        pl.plot(fft_freq_out, fft_db)
        pl.grid(True)
        pl.xlim(fft_freq_out[0], fft_freq_out[-1])
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
