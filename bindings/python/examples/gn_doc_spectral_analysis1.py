import numpy as np
import genalyzer.advanced as gn
import pprint
import matplotlib.pyplot as pl
from matplotlib.patches import Rectangle as MPRect
from tabulate import tabulate

#
# Signal 
#
npts = 30000 # number of points in the signal
freq = 300000 # tone frequency
phase = 0.0 # tone phase
ampl_dbfs = -1.0 # amplitude of the tone in dBFS
qnoise_dbfs = -60.0  # quantizer noise in dBFS
fsr = 2.0 # full-scale range of I/Q components of the complex tone
ampl = (fsr / 2) * 10 ** (ampl_dbfs / 20) # amplitude of the tone in linear scale
qnoise = 10 ** (qnoise_dbfs / 20) # quantizer noise in linear scale

#
# FFT configuration
#
navg = 1 # number of FFT averages
nfft = int(npts/navg) # FFT-order
qres = 12  # data resolution
code_fmt = gn.CodeFormat.TWOS_COMPLEMENT # integer data format
window = gn.Window.NO_WINDOW # window function to apply
ssb_fund = 0 # number of single-side bins
axis_type = gn.FreqAxisType.DC_CENTER # axis type
fs = 3e6 # sample-rate of the data
axis_fmt = gn.FreqAxisFormat.FREQ # axis-format
num_harmonics = 3 # number of harmonics to analyze
ssb_rest = 0 # default number of single-side bins for non-signal components
ssb_dc = 0 # number of single-side bins for the DC-component
ssb_wo = 0 # number of single-side bins for the WO-component

#
# Genarate signal for analysis
#
awfi = gn.cos(npts, fs, ampl, freq, phase)
awfq = gn.sin(npts, fs, ampl, freq, phase)
qwfi = gn.quantize(awfi, fsr, qres, qnoise, code_fmt)
qwfq = gn.quantize(awfq, fsr, qres, qnoise, code_fmt)

#
# Compute FFT
#
fft_cplx = gn.fft(qwfi, qwfq, qres, navg, nfft, window, code_fmt)

#
# Fourier analysis configuration
#
test_label = "fa"
gn.fa_create(test_label)
signal_component_label = 'A'
gn.fa_max_tone(test_label, signal_component_label, gn.FaCompTag.SIGNAL, ssb_fund)
gn.fa_fsample(test_label, fs)
gn.fa_hd(test_label, num_harmonics)
gn.fa_ssb(test_label, gn.FaSsb.DEFAULT, ssb_rest)
gn.fa_ssb(test_label, gn.FaSsb.DC, ssb_dc)
gn.fa_ssb(test_label, gn.FaSsb.WO, ssb_wo)

#
# Fourier analysis execution
#
results = gn.fft_analysis(test_label, fft_cplx, nfft, axis_type)

#
# Print results and plot
#
freq_axis = gn.freq_axis(nfft, axis_type, fs, axis_fmt)
fft_db = gn.db(fft_cplx)
if gn.FreqAxisType.DC_CENTER == axis_type:
    fft_db = gn.fftshift(fft_db)

annots = gn.fa_annotations(results, axis_type, axis_fmt)
print('annots["labels"]: ')
labels_head = ('frequency (Hz)', 'magnitude (dBFs)', 'component label')
labels_table = tabulate(annots["labels"], headers=labels_head, tablefmt="grid")
print(labels_table, "\n")

print('annots["tone_boxes"]: ')
c1 = [x[0] for x in annots["tone_boxes"]]
c2 = [x[2] for x in annots["tone_boxes"]]
tone_boxes_head = ('box left boundary (Hz)', 'width (Hz)')
tone_boxes_table = tabulate(map(list, zip(*(c1, c2))), headers=tone_boxes_head, tablefmt="grid")
print(tone_boxes_table, "\n")

print('+----------------+')
print("results dictionary")
print('+----------------+')
pprint.pprint(results)    

# plot
toneDC_Hz = annots["labels"][0][0]
toneDC_bin = toneDC_Hz/(fs/nfft)
toneDC_mag = fft_db[int(toneDC_bin+0.5*nfft)]
toneA_Hz = annots["labels"][1][0]
toneA_bin = toneA_Hz/(fs/nfft)    
toneA_mag = fft_db[int(toneA_bin+0.5*nfft)]
toneA_im_Hz = annots["labels"][2][0]
toneA_im_bin = toneA_im_Hz/(fs/nfft)
toneA_im_mag = fft_db[int(toneA_im_bin+0.5*nfft)]
tone2A_Hz = annots["labels"][3][0]
tone2A_bin = tone2A_Hz/(fs/nfft)
tone2A_mag = fft_db[int(tone2A_bin+0.5*nfft)]
tone2A_im_Hz = annots["labels"][4][0]
tone2A_im_bin = tone2A_im_Hz/(fs/nfft)
tone2A_im_mag = fft_db[int(tone2A_im_bin+0.5*nfft)]
tone3A_im_Hz = annots["labels"][5][0]
tone3A_im_bin = tone3A_im_Hz/(fs/nfft)
tone3A_im_mag = fft_db[int(tone3A_im_bin+0.5*nfft)]
toneWO_Hz = annots["labels"][6][0]
toneWO_bin = toneWO_Hz/(fs/nfft)
toneWO_mag = fft_db[int(toneWO_bin+0.5*nfft)]

sfdr = results["sfdr"]
nsd = results["nsd"]
scale_MHz = 1e-6
fig = pl.figure(1)
fig.clf()
pl.plot(freq_axis*scale_MHz, fft_db)
pl.grid(True)
pl.xlabel('frequency (MHz)')
pl.ylabel('magnitude (dBFs)')
pl.xlim(freq_axis[0]*scale_MHz, freq_axis[-1]*scale_MHz)
pl.ylim(-140.0, 20.0)
for x, y, label in annots["labels"]:
    if label == 'dc':
        pl.annotate(label+": ["+f"{toneDC_Hz:.2f}"+" ,"+f"{toneDC_mag:.2f}"+"]", 
                    xy=(x*scale_MHz, toneDC_mag), 
                    xytext=(x*scale_MHz, -10), 
                    color = 'red',
                    horizontalalignment="center",
                    arrowprops=dict(arrowstyle='->',color='red',lw=1))
    elif label == 'A':
        pl.annotate(label+": ["+f"{toneA_Hz:.2f}"+" ,"+f"{toneA_mag:.2f}"+"]", 
                    xy=(x*scale_MHz, toneA_mag), 
                    xytext=(x*scale_MHz, 10), 
                    color = 'black',
                    horizontalalignment="center",
                    arrowprops=dict(arrowstyle='->',color='black',lw=1))
    elif label == '-A':
        pl.annotate(label+": ["+f"{toneA_im_Hz:.2f}"+" ,"+f"{toneA_im_mag:.2f}"+"]", 
                    xy=(x*scale_MHz, toneA_im_mag), 
                    xytext=(x*scale_MHz, -20), 
                    color = 'black',
                    horizontalalignment="center",
                    arrowprops=dict(arrowstyle='->',color='black',lw=1))            
    elif label == '2A':
        pl.annotate(label+": ["+f"{tone2A_Hz:.2f}"+" ,"+f"{tone2A_mag:.2f}"+"]", 
                    xy=(x*scale_MHz, tone2A_mag), 
                    xytext=(x*scale_MHz, -40), 
                    color = 'green',
                    horizontalalignment="center",
                    arrowprops=dict(arrowstyle='->',color='green',lw=1))
    elif label == '-2A':
        pl.annotate(label+": ["+f"{tone2A_im_Hz:.2f}"+" ,"+f"{tone2A_im_mag:.2f}"+"]", 
                    xy=(x*scale_MHz, tone2A_im_mag), 
                    xytext=(x*scale_MHz, -40), 
                    color = 'green',
                    horizontalalignment="center",
                    arrowprops=dict(arrowstyle='->',color='green',lw=1))
    elif label == '-3A':
        pl.annotate(label+": ["+f"{tone3A_im_Hz:.2f}"+" ,"+f"{tone3A_im_mag:.2f}"+"]", 
                    xy=(x*scale_MHz, tone3A_im_mag), 
                    xytext=(x*scale_MHz, -60), 
                    color = 'magenta',
                    horizontalalignment="center",
                    arrowprops=dict(arrowstyle='->',color='magenta',lw=1))        
    elif label == 'wo':
        pl.annotate(label+": ["+f"{toneWO_Hz:.2f}"+" ,"+f"{toneWO_mag:.2f}"+"]", 
                    xy=(x*scale_MHz, toneWO_mag), 
                    xytext=(x*scale_MHz, -80), 
                    color = 'magenta',
                    horizontalalignment="center",
                    arrowprops=dict(arrowstyle='->',color='magenta',lw=1))
    else:
        pl.annotate(label, xy=(x*scale_MHz, y), ha="center", va="bottom")
pl.axhline(y = toneA_mag, color = 'k', linestyle = '-')
pl.axhline(y = toneWO_mag, color = 'k', linestyle = '-')
pl.annotate('', 
            xy=(1.25,toneWO_mag), 
            xytext=(1.25,toneA_mag),
            arrowprops=dict(arrowstyle='<->',color='black',lw=1))
pl.annotate('SFDR'+": "+f"{sfdr:.2f}"+' dB',
            xy=(1.25, -40), 
            xytext=(1.25, -40), 
            verticalalignment="center",
            rotation=270)
pl.axhline(y = nsd, color = 'r', linestyle = '-')
pl.annotate('NSD'+": "+f"{nsd:.2f}"+' dB',
            xy=(0.5, -120), 
            xytext=(0.5, -120), 
            color = 'red',
            ha="center")
pl.savefig('foo.png')