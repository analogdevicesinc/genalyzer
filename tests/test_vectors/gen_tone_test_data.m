function gen_tone_test_data()
    % config settings
    rng shuffle;
    c.wf_type = 0;%randi([0, 2]);
    c.npts=2^randi([11, 16]);
    num_tones=1;%randi(3);
    c.fs=randi([4, 20])*0.5*1e8;
    c.freq=randi(80, 1, num_tones)*0.25*1e6;
    c.scale=randi(4, 1, num_tones)*0.25;
    c.phase=0.1*randi([0, 5], 1, num_tones);
    
    % waveform
    delta_f = 1/c.fs;
    t = 0:delta_f:delta_f*(c.npts-1);
    c.test_vec = zeros(1, c.npts);
    for n = 1:num_tones        
        if (c.wf_type == 0)
            c.test_vec = c.test_vec + c.scale(n)*cos(2*pi*c.freq(n)*t+c.phase(n));
        elseif (c.wf_type == 1)
            c.test_vec = c.test_vec + c.scale(n)*sin(2*pi*c.freq(n)*t+c.phase(n));
        elseif (c.wf_type == 2)
            c.test_vec = c.test_vec + ...
                c.scale(n)*cos(2*pi*c.freq(n)*t+c.phase(n)) + ...
                1i*c.scale(n)*sin(2*pi*c.freq(n)*t+c.phase(n));
        end
    end
    
    if (c.wf_type == 2)
        tmp = c.test_vec;
        c.test_vec = zeros(1, 2*c.npts);
        c.test_vec(1:2:end) = real(tmp);
        c.test_vec(2:2:end) = imag(tmp);
    end
    cJSON = jsonencode(c,PrettyPrint=true);
    
    test_fname = sprintf('test_gen_tone_%s.json', num2str(posixtime(datetime())*1000));
    fID = fopen(test_fname, 'w');    
    fprintf(fID, cJSON); 
    fclose(fID);
end