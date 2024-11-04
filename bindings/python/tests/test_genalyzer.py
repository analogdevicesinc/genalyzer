import os, glob, json, pytest, genalyzer


test_dir = os.path.join(*["..", "..", "..", "tests", "test_vectors"])
loc = os.path.dirname(__file__)
test_gen_real_tone_files = [
    f for f in glob.glob(os.path.join(loc, test_dir, "test_gen_real_tone_*.json"))
]
test_gen_complex_tone_files = [
    f for f in glob.glob(os.path.join(loc, test_dir, "test_gen_complex_tone_*.json"))
]
test_quantize_real_tone_files = [
    f for f in glob.glob(os.path.join(loc, test_dir, "test_quantize_real_tone_*.json"))
]
test_quantize_complex_tone_files = [
    f
    for f in glob.glob(os.path.join(loc, test_dir, "test_quantize_complex_tone_*.json"))
]
test_fft_tone_files = [
    f for f in glob.glob(os.path.join(loc, test_dir, "test_fft_tone_*.json"))
]
test_gen_wave_data = [
    f for f in glob.glob(os.path.join(loc, test_dir, "test_gen_wave_data_*.json"))
]


@pytest.mark.parametrize("filename", test_gen_real_tone_files)
def test_gen_real_tone(filename):
    with open(filename) as f:
        data = json.load(f)
        if data["num_tones"] == 1:
            freq_list = [data["freq"]]
            scale_list = [data["scale"]]
            phase_list = [data["phase"]]
        else:
            freq_list = data["freq"]
            scale_list = data["scale"]
            phase_list = data["phase"]

        c = genalyzer.simplified_beta.simplified_beta.config_gen_tone(
            data["wf_type"],
            data["npts"],
            data["fs"],
            data["num_tones"],
            freq_list,
            scale_list,
            phase_list,
        )
        awf = genalyzer.simplified_beta.simplified_beta.gen_real_tone(c)
        genalyzer.simplified_beta.simplified_beta.config_free(c)
        assert len(awf) != 0, "the list is non empty"


@pytest.mark.parametrize("filename", test_gen_complex_tone_files)
def test_gen_complex_tone(filename):
    with open(filename) as f:
        data = json.load(f)
        if data["num_tones"] == 1:
            freq_list = [data["freq"]]
            scale_list = [data["scale"]]
            phase_list = [data["phase"]]
        else:
            freq_list = data["freq"]
            scale_list = data["scale"]
            phase_list = data["phase"]

        c = genalyzer.simplified_beta.simplified_beta.config_gen_tone(
            data["wf_type"],
            data["npts"],
            data["fs"],
            data["num_tones"],
            freq_list,
            scale_list,
            phase_list,
        )
        awfi, awfq = genalyzer.simplified_beta.simplified_beta.gen_complex_tone(c)
        genalyzer.simplified_beta.simplified_beta.config_free(c)
        assert len(awfi) != 0, "the list is non empty"
        assert len(awfq) != 0, "the list is non empty"


@pytest.mark.parametrize("filename", test_quantize_real_tone_files)
def test_quantize_real_tone(filename):
    with open(filename) as f:
        data = json.load(f)
        awf = data["test_vec"]
        c = genalyzer.simplified_beta.simplified_beta.config_quantize(
            data["npts"], data["fsr"], data["qres"], data["qnoise"]
        )
        qwf = genalyzer.simplified_beta.simplified_beta.quantize(awf, c)
        genalyzer.simplified_beta.simplified_beta.config_free(c)
        assert qwf != 0, "the list is non empty"


@pytest.mark.parametrize("filename", test_quantize_complex_tone_files)
def test_quantize_complex_tone(filename):
    with open(filename) as f:
        data = json.load(f)
        awfi = data["test_vec_i"]
        awfq = data["test_vec_q"]
        c = genalyzer.simplified_beta.simplified_beta.config_quantize(
            data["npts"], data["fsr"], data["qres"], data["qnoise"]
        )
        qwfi = genalyzer.simplified_beta.simplified_beta.quantize(awfi, c)
        qwfq = genalyzer.simplified_beta.quantize(awfq, c)
        genalyzer.simplified_beta.simplified_beta.config_free(c)
        assert qwfi != 0, "the list is non empty"
        assert qwfq != 0, "the list is non empty"


@pytest.mark.parametrize("filename", test_fft_tone_files)
def test_fft(filename):
    with open(filename) as f:
        data = json.load(f)
        qwfi = data["test_vecq_i"]
        qwfi = [int(i) for i in qwfi]
        qwfq = data["test_vecq_q"]
        qwfq = [int(i) for i in qwfq]
        c = genalyzer.simplified_beta.simplified_beta.config_fftz(
            data["npts"], data["qres"], data["navg"], data["nfft"], data["win"] - 1
        )
        out = genalyzer.simplified_beta.simplified_beta.fftz(qwfi, qwfq, c)
        genalyzer.simplified_beta.simplified_beta.config_free(c)
        assert out != 0, "the list is non empty"


@pytest.mark.parametrize("filename", test_fft_tone_files)
def test_get_fa_results(filename):
    with open(filename) as f:
        data = json.load(f)
        if data["num_tones"] == 1:
            freq_list = [data["freq"]]
        else:
            freq_list = data["freq"]

        qwfi = data["test_vecq_i"]
        qwfi = [int(i) for i in qwfi]
        qwfq = data["test_vecq_q"]
        qwfq = [int(i) for i in qwfq]
        c = genalyzer.simplified_beta.simplified_beta.config_fftz(
            data["npts"], data["qres"], data["navg"], data["nfft"], data["win"] - 1
        )
        fft_out_i, fft_out_q = genalyzer.simplified_beta.simplified_beta.fftz(qwfi, qwfq, c)
        fft_out = [val for pair in zip(fft_out_i, fft_out_q) for val in pair]
        genalyzer.simplified_beta.simplified_beta.config_set_sample_rate(data["fs"], c)
        genalyzer.simplified_beta.simplified_beta.config_fa(freq_list[0], c)
        fa_results = genalyzer.simplified_beta.simplified_beta.get_fa_results(fft_out, c)
        genalyzer.simplified_beta.simplified_beta.config_free(c)
        assert bool(fa_results), "the dict is non empty"


@pytest.mark.parametrize("filename", test_fft_tone_files)
def test_get_fa_single_result(filename):
    with open(filename) as f:
        data = json.load(f)
        if data["num_tones"] == 1:
            freq_list = [data["freq"]]
        else:
            freq_list = data["freq"]

        qwfi = data["test_vecq_i"]
        qwfi = [int(i) for i in qwfi]
        qwfq = data["test_vecq_q"]
        qwfq = [int(i) for i in qwfq]
        c = genalyzer.simplified_beta.config_fftz(
            data["npts"], data["qres"], data["navg"], data["nfft"], data["win"] - 1
        )
        fft_out_i, fft_out_q = genalyzer.simplified_beta.fftz(qwfi, qwfq, c)
        fft_out = [val for pair in zip(fft_out_i, fft_out_q) for val in pair]
        genalyzer.simplified_beta.config_set_sample_rate(data["fs"], c)
        genalyzer.simplified_beta.config_fa(freq_list[0], c)
        sfdr = genalyzer.simplified_beta.get_fa_single_result("sfdr", fft_out, c)
        genalyzer.simplified_beta.config_free(c)
        assert sfdr != 0, "the value is non zero"


@pytest.mark.parametrize("filename", test_quantize_real_tone_files)
def test_get_ha_results(filename):
    with open(filename) as f:
        data = json.load(f)
        qwf = data["test_vecq"]
        qwf = [int(i) for i in qwf]
        c = genalyzer.simplified_beta.config_quantize(
            data["npts"], data["fsr"], data["qres"], data["qnoise"]
        )
        hist_qwf = genalyzer.simplified_beta.histz(qwf, c)
        ha_results = genalyzer.simplified_beta.get_ha_results(hist_qwf, c)
        genalyzer.simplified_beta.config_free(c)
        assert bool(ha_results), "the dict is non empty"


@pytest.mark.parametrize("filename", test_quantize_real_tone_files)
def test_get_wfa_results(filename):
    with open(filename) as f:
        data = json.load(f)
        qwf = data["test_vecq"]
        qwf = [int(i) for i in qwf]
        c = genalyzer.simplified_beta.config_quantize(
            data["npts"], data["fsr"], data["qres"], data["qnoise"]
        )
        wfa_results = genalyzer.simplified_beta.get_wfa_results(qwf, c)
        genalyzer.simplified_beta.config_free(c)
        assert bool(wfa_results), "the dict is non empty"


@pytest.mark.parametrize("filename", test_gen_wave_data)
def test_gen_wf_data(filename):
    with open(filename) as f:
        data = json.load(f)
        wavegen = genalyzer.helpers.WaveformGen(
            data["npts"],
            data["freq"],
            data["code_fmt"],
            data["res"],
            data["v_ref_n"],
            data["v_ref_p"],
            data["v_min"],
            data["v_max"],
        )
        wf_sine = wavegen.gen_sine_wave()
        wf_cosine = wavegen.gen_cosine_wave()
        wf_tri = wavegen.gen_triangular_wave()
        wf_square = wavegen.gen_square_wave()
        wf_pwm = wavegen.gen_pwm_wave(data["duty_cyc"])
        assert len(wf_sine) != 0, "the list is empty"
        assert len(wf_cosine) != 0, "the list is empty"
        assert len(wf_tri) != 0, "the list is empty"
        assert len(wf_square) != 0, "the list is empty"
        assert len(wf_pwm) != 0, "the list is empty"
