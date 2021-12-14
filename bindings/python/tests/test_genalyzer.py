import sys

sys.path.insert(1, "../")
import genalyzer
import glob
import pytest
import os

test_dir = "../../../tests/test_vectors/"
loc = os.path.dirname(__file__)
test_gen_tone_files = [
    f for f in glob.glob(os.path.join(loc, test_dir, "test_gen_tone_*.txt"))
]
test_quantize_files = [
    f for f in glob.glob(os.path.join(loc, test_dir, "test_quantize_*.txt"))
]
test_gen_ramp_files = [
    f
    for f in glob.glob(
        os.path.join(loc, test_dir, "test_gen_ramp_[^and_quantize_]*.txt")
    )
]
test_gen_ramp_and_quantize_files = [
    f
    for f in glob.glob(os.path.join(loc, test_dir, "test_gen_ramp_and_quantize_*.txt"))
]
test_rfft_input_files = [
    f for f in glob.glob(os.path.join(loc, test_dir, "test_rfft_input_*.txt"))
]
test_fft_input_files = [
    f for f in glob.glob(os.path.join(loc, test_dir, "test_fft_input_*.txt"))
]


def get_test_config(f):
    config_dict = {}
    for line in f:
        if "----------" in line:
            break
        else:
            (key, val) = line.split("=")
            (val, new_line) = val.split("\n")
            config_dict[key] = float(val)

    return config_dict


@pytest.mark.parametrize("filename", test_gen_tone_files)
def test_gen_tone(filename):
    with open(filename, "r") as f:
        config_dict = get_test_config(f)
        c = genalyzer.config_tone_gen(config_dict)
        awf = genalyzer.gen_tone(c)
        assert len(awf) != 0, "the list is non empty"


@pytest.mark.parametrize("filename", test_quantize_files)
def test_quantize(filename):
    with open(filename, "r") as f:
        config_dict = get_test_config(f)
        c = genalyzer.config_tone_gen(config_dict)
        awf = genalyzer.gen_tone(c)
        qwf = genalyzer.quantize(c, awf)
        assert len(qwf) != 0, "the list is non empty"


@pytest.mark.parametrize("filename", test_gen_ramp_files)
def test_gen_ramp(filename):
    with open(filename, "r") as f:
        config_dict = get_test_config(f)
        c = genalyzer.config_ramp_nl_meas(config_dict)
        awf = genalyzer.gen_ramp(c)
        assert len(awf) != 0, "the list is non empty"


@pytest.mark.parametrize("filename", test_rfft_input_files)
def test_rfft(filename):
    with open(filename, "r") as f:
        config_dict = get_test_config(f)
        c = genalyzer.config_tone_meas(config_dict)
        awf = genalyzer.gen_tone(c)
        qwf = genalyzer.quantize(c, awf)
        out_i, out_q = genalyzer.rfft(c, qwf)
        assert len(out_i) != 0, "the list is non empty"
        assert len(out_q) != 0, "the list is non empty"


@pytest.mark.parametrize("filename", test_fft_input_files)
def test_fft(filename):
    with open(filename, "r") as f:
        config_dict = get_test_config(f)
        c = genalyzer.config_tone_meas(config_dict)
        awf = genalyzer.gen_tone(c)
        qwf = genalyzer.quantize(c, awf)
        qwf_i = [qwf[i] for i in range(len(qwf)) if i % 2 == 0]
        qwf_q = [qwf[i] for i in range(len(qwf)) if i % 2 != 0]
        out_i, out_q = genalyzer.fft(c, qwf_i, qwf_q)
        assert len(out_i) != 0, "the list is non empty"
        assert len(out_q) != 0, "the list is non empty"


@pytest.mark.parametrize("filename", test_quantize_files)
def test_metric_t(filename):
    with open(filename, "r") as f:
        config_dict = get_test_config(f)
        c = genalyzer.config_tone_gen(config_dict)
        awf = genalyzer.gen_tone(c)
        qwf = genalyzer.quantize(c, awf)
        res = genalyzer.metric_t(c, qwf, "SFDR")
        # assert err_code != 22, "invalid argument"


@pytest.mark.parametrize("filename", test_fft_input_files)
def test_fft_metric_f(filename):
    with open(filename, "r") as f:
        config_dict = get_test_config(f)
        c = genalyzer.config_tone_meas(config_dict)
        awf = genalyzer.gen_tone(c)
        qwf = genalyzer.quantize(c, awf)
        qwf_i = [qwf[i] for i in range(len(qwf)) if i % 2 == 0]
        qwf_q = [qwf[i] for i in range(len(qwf)) if i % 2 != 0]
        fft_i, fft_q = genalyzer.fft(c, qwf_i, qwf_q)
        fft_data = [val for pair in zip(fft_i, fft_q) for val in pair]
        res = genalyzer.metric_f(c, fft_data, "SFDR")
        # assert err_code != 22, "invalid argument"


@pytest.mark.parametrize("filename", test_rfft_input_files)
def test_rfft_metric_f(filename):
    with open(filename, "r") as f:
        config_dict = get_test_config(f)
        c = genalyzer.config_tone_meas(config_dict)
        awf = genalyzer.gen_tone(c)
        qwf = genalyzer.quantize(c, awf)
        rfft_i, rfft_q = genalyzer.rfft(c, qwf)
        rfft_data = [val for pair in zip(rfft_i, rfft_q) for val in pair]
        res = genalyzer.metric_f(c, rfft_data, "SFDR")
        # assert err_code != 22, "invalid argument"
