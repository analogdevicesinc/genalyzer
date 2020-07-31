genalyzerh = 'genalyzer.h';
fp = split(fileparts(which(genalyzerh)),filesep);
fp = join(fp(1:end-1),filesep);
loadlibraryArgs = {genalyzerh,'includepath',fp{1},'addheader','phase.h','addheader','spectrum.h'};

if not(libisloaded('libgenalyzer'))
    loadlibrary('libgenalyzer', loadlibraryArgs{:});
end
% libfunctionsview libgenalyzer

%% Test
% Test phase est
fs = 1e3; fc = 50; N = 2^14;
t = 0:1/fs:(N-1)/fs;
r = cos(2*pi*fc*t); i = sin(2*pi*fc*t);

r2 = cos(2*pi*fc*t+pi/2); i2 = sin(2*pi*fc*t+pi/2);
out = calllib('libgenalyzer','phase_difference_cdouble',r,i,r2,i2,N);
assert(abs(out-90)<sqrt(eps))

% Test SFDR
fc2 = 10;
N = length(t);
sfdrDB = 40;
s =(10^(-sfdrDB/20));
r = s.*cos(2*pi*fc2*t)+r; i = s.*sin(2*pi*fc2*t)+i;
FS = 2^11;
ro = zeros(size(r));
io = zeros(size(r));
rptr = libpointer('doublePtr',r);
iptr = libpointer('doublePtr',i);
roptr = libpointer('doublePtr',ro);
ioptr = libpointer('doublePtr',io);
sfdrDB_est = calllib('libgenalyzer','sfdr_cdouble', rptr, iptr, FS, N);
assert(abs(sfdrDB-sfdrDB_est) < 0.5);


%% Cleanup
unloadlibrary libgenalyzer