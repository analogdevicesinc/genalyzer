%% Set up Library
addpath(genpath(['..',filesep,'bindings',filesep,'c',filesep'include']))
libName = 'libgenalyzer';
genalyzerwrapperh = 'genalyzer-wrapper.h';
genalyzerh = 'cgenalyzer.h';
fp = fileparts(which(genalyzerh));
loadlibraryArgs = {genalyzerwrapperh,'includepath',fp,'addheader',genalyzerh};
if ~libisloaded(libName)
    msgID = 'MATLAB:loadlibrary:StructTypeExists';
    warnStruct = warning('off',msgID);
    [~, ~] = loadlibrary(libName, loadlibraryArgs{:});
    warning(warnStruct);
end

%% Create Configuration
c = libpointer('gn_config_sPtr');

domain = 0;
type = 2;
nfft = 32768;
navg = 1;
fs = 3e6;
fsr = 0;
res = 13;

r = calllib(libName,'gn_config_tone_meas',...
    c,domain,type,nfft,navg,fs,fsr,res,false,false,false);

%% Generate data
sw = dsp.SineWave();
sw.ComplexOutput = true;
sw.SamplesPerFrame = nfft;
sw.Amplitude = 2^11;
y = int32(sw());

%% Take Measurement
metric = char('SFDR');
err_code = libpointer('uint32Ptr',0)

y = [real(y).'; imag(y).'];
y = y(:);

sfdrval = calllib('libgenalyzer', 'gn_metric', c, y, metric, err_code);

disp(sfdrval);
disp(err_code.Value);

clear all;
unloadlibrary('libgenalyzer');
