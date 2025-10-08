"""Helper functions for pyadi-iio devices."""

import iio
import numpy as np

import genalyzer as gn


def fft(
    interface,
    data,
    navg=1,
    window=gn.Window.NO_WINDOW,
    axis_type=gn.FreqAxisType.DC_CENTER,
    axis_fmt=gn.FreqAxisFormat.FREQ,
):
    """Perform FFT based off pyadi-iio interface.

    This function performs an FFT on data acquired from a pyadi-iio device and
    generate the scaled FFT result in dB along with the corresponding frequency axis.
    The dB scaled data is shifted if the axis_type is set to DC_CENTER.
    The function supports both real and complex data. For complex data, the input
    numpy array should have a complex dtype (np.complex64 or np.complex128).
    For real data, the input numpy array should have an integer dtype (e.g., np.int16,
    np.int32, etc.) matching the data format of the device channel.
    The function also supports multiple channels if the pyadi-iio device has more
    than one enabled channel. In this case, the input data should be a list of numpy
    arrays, one for each enabled channel.

    Args:
        ``interface``: pyadi-iio device instance.

        ``data``: numpy array or list of numpy arrays (for multiple channels).

        ``navg``: number of averages.

        ``window``: window type from gn.Window enum.

        ``axis_type``: frequency axis type from gn.FreqAxisType enum.

        ``axis_fmt``: frequency axis format from gn.FreqAxisFormat enum.

    Returns:
        ``fft_out``: FFT output as a numpy array or list of numpy arrays (for multiple channels).

        ``fft_out_db``: dB scaled FFT output as a numpy array or list of numpy arrays (for multiple channels).

        ``fft_freq_out``: frequency axis as a numpy array or list of numpy arrays (for multiple channels).
    """
    # Checks
    assert hasattr(
        interface, "_rxadc"
    ), "Non standard pyadi-iio device. interface must have _rxadc attribute"

    assert isinstance(
        interface._rxadc, iio.Device
    ), "interface must be an iio.Device as _rxadc attribute"
    if not isinstance(data, (np.ndarray, list)) and not all(
        isinstance(d, np.ndarray) for d in data
    ):
        raise ValueError("data must be a numpy array or a list of numpy arrays")
    assert navg > 0 and isinstance(navg, int), "navg must be a positive integer"

    # Check data meets channels
    if len(interface.rx_enabled_channels) > 1:
        if not isinstance(data, list):
            raise ValueError("data must be a list when multiple channels are enabled")
        if len(data) != len(interface.rx_enabled_channels):
            raise ValueError("data length must match number of enabled channels")
    elif isinstance(data, list):
        raise ValueError("data must be a numpy array when a single channel is enabled")
    elif len(interface.rx_enabled_channels) == 0:
        raise ValueError("No enabled channels found in interface")

    if hasattr(interface, "rx_sample_rate"):
        fs = int(interface.rx_sample_rate)
    elif hasattr(interface, "sample_rate"):
        fs = int(interface.sample_rate)
    else:
        raise ValueError("Sample rate not found in interface")

    fft_out = {}
    fft_out_db = {}
    fft_freq_out = {}

    for i, rx_ch in enumerate(interface.rx_enabled_channels):
        if isinstance(rx_ch, int):
            channel = interface._rxadc.channels[rx_ch]
        elif isinstance(rx_ch, str):
            channel = interface._rxadc.find_channel(rx_ch, False)
        else:
            raise ValueError("rx_channel must be int or str")

        if not channel:
            raise ValueError(
                f"Channel {rx_ch} not found in device {interface._rxadc.name}"
            )

        df = channel.data_format
        fmt = ("i" if df.is_signed is True else "u") + str(df.length // 8)
        fmt = f">{fmt}" if df.is_be else fmt

        qres = df.bits

        fmt = np.dtype(fmt)

        assert fmt in [
            np.dtype("int16"),
            np.dtype("int32"),
        ], "Unsupported data format"

        code_fmt = gn.CodeFormat.TWOS_COMPLEMENT  # Binary scaling not supported for now

        rx_data = data[i] if isinstance(data, list) else data

        is_complex = rx_data.dtype in [np.complex64, np.complex128]
        nfft = len(rx_data) // (navg)
        assert navg * nfft == len(rx_data), "data length must be multiple of navg"

        if is_complex:
            x_re = np.array(rx_data.real).astype(fmt)
            x_im = np.array(rx_data.imag).astype(fmt)
            fft_cplx = gn.fft(x_re, x_im, qres, navg, nfft, window, code_fmt)
        else:
            x = np.array(rx_data).astype(fmt)
            fft_cplx = gn.fft_real(x, qres, navg, nfft, window, code_fmt)

        # Frequency axis and dB
        freq_axis = gn.freq_axis(nfft, axis_type, fs, axis_fmt)
        fft_db = gn.db(fft_cplx)
        if gn.FreqAxisType.DC_CENTER == axis_type:
            fft_db = gn.fftshift(fft_db)

        fft_out[rx_ch] = fft_cplx
        fft_out_db[rx_ch] = fft_db
        fft_freq_out[rx_ch] = freq_axis

    if len(fft_out) == 1:
        first = list(fft_out.keys())[0]
        fft_out = fft_out[first]
        fft_out_db = fft_out_db[first]
        fft_freq_out = fft_freq_out[first]

    return fft_out, fft_out_db, fft_freq_out
