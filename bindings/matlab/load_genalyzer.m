function load_genalyzer()
    libName = 'libgenalyzer';
    genalyzer_wrapperh = 'genalyzer-wrapper.h';
    genalyzer_h = 'cgenalyzer.h'; 
    fp = fileparts(which(genalyzer_wrapperh));
    loadlibraryArgs = {genalyzer_wrapperh,'includepath',fp,'addheader',genalyzer_h};
    if ~libisloaded(libName)
        msgID = 'MATLAB:loadlibrary:StructTypeExists';
        warnStruct = warning('off',msgID);
        [~, ~] = loadlibrary(libName, loadlibraryArgs{:});
        warning(warnStruct);
    end
end