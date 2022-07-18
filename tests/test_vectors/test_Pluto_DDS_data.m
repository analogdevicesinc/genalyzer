function test_Pluto_DDS_data()
    %% Tx set up
    tx = adi.Pluto.Tx;
    tx.uri = 'ip:pluto'; 
    tx.DataSource = 'DDS';
    tx.DDSFrequencies = [1e5 1e5; 0 0];
    tx.DDSPhases = [90e3 0; 0 0]; % expressed in millidegrees
    tx.DDSScales = [1 1; 0 0];
    tx.CenterFrequency = 1e9;
    tx();
    pause(1);
    
    %% Rx set up
    rx = adi.Pluto.Rx('uri','ip:pluto');
    rx.CenterFrequency = tx.CenterFrequency;
    y = rx();

    % config settings
    c.wf_type = 2;
    c.npts=rx.SamplesPerFrame;
    c.num_tones=1;
    c.fs=tx.SamplingRate;
    c.freq=tx.DDSFrequencies(1);
    c.scale=tx.DDSScales(1);
    c.phase=tx.DDSPhases(1);
    
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
        c.test_vec_i = real(y);
        c.test_vec_q = imag(y);
    end
        
    test_fname = sprintf('test_pluto_DDS_data_%s.json', num2str(posixtime(datetime())*1000));
    
    fID = fopen(test_fname, 'w');    
    cJSON = jsonencode(c,PrettyPrint=true);
    fprintf(fID, cJSON); 
    fclose(fID);
end