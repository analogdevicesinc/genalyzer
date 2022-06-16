function gen_tone_test_data()
    % config settings
    rng shuffle;
    c.wf_type = randi(3)-1;
    c.npts=2^randi([11, 16]);
    c.num_tones=randi(5);
    c.fs=randi([4, 20])*0.5*1e8;
    c.freq=randi(80, 1, c.num_tones)*0.25*1e6;
    c.scale=randi(4, 1, c.num_tones)*0.25;
    c.phase=0.1*randi([0, 5], 1, c.num_tones);
    
    % waveform
    delta_f = 1/c.fs;
    t = 0:delta_f:delta_f*(c.npts-1);
    if (c.wf_type == 2)
        c.test_vec_i = zeros(1, c.npts);
        c.test_vec_q = zeros(1, c.npts);
    else
        c.test_vec = zeros(1, c.npts);
    end
    for n = 1:c.num_tones        
        if (c.wf_type == 0)
            c.test_vec = c.test_vec + c.scale(n)*cos(2*pi*c.freq(n)*t+c.phase(n));
        elseif (c.wf_type == 1)
            c.test_vec = c.test_vec + c.scale(n)*sin(2*pi*c.freq(n)*t+c.phase(n));
        elseif (c.wf_type == 2)
            c.test_vec_i = c.test_vec_i + ...
                c.scale(n)*cos(2*pi*c.freq(n)*t+c.phase(n));
            c.test_vec_q = c.test_vec_q + ...
                c.scale(n)*sin(2*pi*c.freq(n)*t+c.phase(n));
        end
    end
        
    if (c.wf_type == 2)
        test_fname = sprintf('test_gen_complex_tone_%s.json', num2str(posixtime(datetime())*1000));
    else
        test_fname = sprintf('test_gen_real_tone_%s.json', num2str(posixtime(datetime())*1000));
    end
    fID = fopen(test_fname, 'w');    
    cJSON = jsonencode(c,PrettyPrint=true);
    fprintf(fID, cJSON); 
    fclose(fID);
end