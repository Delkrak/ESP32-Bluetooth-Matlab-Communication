classdef SppBluetooth < handle
    %MYBLUETOOTH Summary of this class goes here
    %   Detailed explanation goes here
    
    properties
        name char
        description char
        channel double
        
        b
        message_buffer Message
        message_buffer_read_idx double
        message_buffer_write_idx double
        
        signal_buffer double = []
        signal_n double = 0
        signal_buffer_read_idx double = 0
        signal_buffer_write_idx double = 0
        signal_cmd char = 'sig'
        signal_low_frequency_names = {}
        signal_low_frequency_length double = -1;
        signal_high_frequency_names = {}
        signal_high_frequency_length double = -1;
        signal_names = {}
        signal_send_signals_ratio double = -1
        
        signal_file_handle double = -1
        
        signal_playout_n double = -1
        signal_playout_size double = 0
    end
    
    methods
    
        %
        % Contructor and destructor
        %
        function obj = SppBluetooth(name, board_task_name, signal_buffer_size, message_buffer_size)
            if nargin < 4
                message_buffer_size = 1000;
            end
            obj.name = name;
            obj.channel = 1;
            obj.connect();
            obj.init(board_task_name, signal_buffer_size, message_buffer_size);
        end
        
        function delete(obj)
           obj.stop_log_signals_to_file(); 
        end
        
        function disconnect(obj)
            if ~isempty(obj.b)
                disp(['Disconnecting from "', obj.name, '"...']);
                fclose(obj.b);
                delete(obj.b);
                obj.b = [];
            end
        end
                
        function connect(obj)
            obj.initialize_message_buffer();
            
            for old_device = instrfind()
                if strcmpi(old_device.RemoteName, obj.name) && ...
                        old_device.Channel == obj.channel && ...
                        strcmp(old_device.Status, 'open')
                    obj.b = old_device;
                    obj.b.BytesAvailableFcn = @obj.read_callback;
                    if obj.is_connected
                        disp(['Reusing old "', obj.name, '" connection.']);
                       return;
                    else
                        obj.disconnect;
                    end
                end
            end
            
            % Make a new bluetooth connection
            disp(['Connecting to "', obj.name, '"...']);
            obj.b = Bluetooth(obj.name, obj.channel, 'Timeout',3);           
            obj.b.InputBufferSize = 4096*2;
            obj.b.OutputBufferSize = 4096*2;
            
            % Open connection and configur callbacks
            obj.b.Terminator = 0;
            obj.b.BytesAvailableFcnMode = 'terminator';
            obj.b.BytesAvailableFcn = @obj.read_callback;
            fopen(obj.b);
            assert(obj.is_connected());
            disp(['Connected to "', obj.name, '".']);
        end
        
        function init(obj, board_task_name, signal_buffer_size, message_buffer_size)
            if ~strcmp(obj.board_task_name, board_task_name)
                warning(['Changing board task to "', board_task_name, '". The board is rebooted for the changes to take effect.']);
                obj.board_task_name = board_task_name;
                obj.restart();
                pause(1);
                obj.connect();
                assert(strcmp(obj.board_task_name, board_task_name));
            end
            obj.description = obj.get_description;
            obj.send_signals = false;
            if isempty(obj.send_signals_ratio_buffered)
                obj.send_signals; % Updates the buffered state.
            end
            obj.initialize_message_buffer(message_buffer_size);
            obj.initialize_signal_buffer(signal_buffer_size);
        end
        
        function y = is_connected(obj)
            try
                y = ~isnan(obj.ping);
            catch
                y = false;
            end
        end
                
        %
        % send, query and flush
        %
        function send(obj, command, numbers, strings)            
            if nargin < 3
               numbers = []; 
            end
            if nargin < 4
                strings = cell(0);
            end
            data = to_binary(Message(command, numbers, strings));
            
            encoder = cobss;
            for d = data
                encoder.push(d);
            end
            encoded_message = uint8([encoder.get, 0]);
            fwrite(obj.b, encoded_message);
        end
        
        function q = query(obj, command, numbers, strings)
            q = [];
            if nargin == 4
                obj.send(command, numbers, strings);
            elseif nargin == 3
                obj.send(command, numbers);
            else
                obj.send(command);
            end

            t = tic;
            while toc(t) < obj.b.Timeout
                if 0 < obj.messages_availiable
                    msg = obj.get_next_message;
                    if startsWith(msg.command, command)
                        q = msg;
                        return
                    end
                end
                %pause(0.0001);
            end
            warning([obj.name, ' - Did not get response on command: "', command, '"']);
        end

        function read_callback(obj, ~, ~)
            try
                encoded_data = uint8(fgets(obj.b));
                if encoded_data(end) ~= 0
                   error('Incorrect termination character!'); 
                end

                % Decode data
                decoder = cobssi;
                for c = encoded_data(1:end-1)
                    decoder.push(c);
                end
                data = decoder.get;
                
                % Convert to message
                message = Message.from_binary(data);
                message.time_received = posixtime(datetime);

                % Add to signal or command buffer
                if strcmp(message.command, obj.signal_cmd)
                    obj.add_signal_to_buffer(message.numbers);
                else
                    obj.add_message_to_buffer(message);
                end
            catch em
                warning(em.getReport);
            end
        end
        
        function flush(obj)
           obj.message_buffer_read_idx = 0;
           obj.message_buffer_write_idx = 0;
           obj.signal_buffer = obj.signal_buffer * nan;
           obj.signal_buffer_write_idx = 0;
           obj.signal_n = 0;
        end
        
        %
        % message fifo buffer initialization, add and get
        %
        function initialize_message_buffer(obj, buffer_size)
           if nargin == 1
              buffer_size = 1000; 
           end
           obj.message_buffer = repmat(Message(), 1, buffer_size);
           obj.flush;
        end

        function x = messages_availiable(obj)
            x = obj.message_buffer_write_idx - obj.message_buffer_read_idx;
        end
        
        function add_message_to_buffer(obj, message)
            n = length(obj.message_buffer);
            idx = 1 + mod(obj.message_buffer_write_idx, n);
            
            obj.message_buffer(idx) = message;
            
            obj.message_buffer_write_idx = obj.message_buffer_write_idx + 1;
            if obj.message_buffer_read_idx + n < obj.message_buffer_write_idx
                %warning('Buffer overflow!');
                obj.message_buffer_read_idx = obj.message_buffer_write_idx - n;
            end
        end
        
        function message = get_next_message(obj)
            n = length(obj.message_buffer);
            idx = 1 + mod(obj.message_buffer_read_idx, n);

            if obj.message_buffer_read_idx >= obj.message_buffer_write_idx
               error('No availiable messages!');
            end
            message = obj.message_buffer(idx);
            
            obj.message_buffer_read_idx = obj.message_buffer_read_idx + 1;
        end
        
        %
        % signal fifo buffer initialization, add and get
        %
        function initialize_signal_buffer(obj, buffer_size)
            msg = obj.query(obj.lf_signal_names_get_cmd);
            obj.signal_low_frequency_names = msg.strings;
            obj.signal_low_frequency_length = length(obj.signal_low_frequency_names);

            msg = obj.query(obj.hf_signal_names_get_cmd);
            obj.signal_high_frequency_names = msg.strings;
            obj.signal_high_frequency_length = length(obj.signal_high_frequency_names);
            
            obj.signal_names = [obj.signal_low_frequency_names, obj.signal_high_frequency_names]; 
           
           obj.signal_buffer = nan * zeros(buffer_size, ...
               obj.signal_low_frequency_length + obj.signal_high_frequency_length);
           obj.flush;
        end
        
        function add_signal_to_buffer(obj, numbers)
            n = size(obj.signal_buffer, 1);
            
            high_frequency_samples = obj.send_signals_ratio_buffered;
            expected_numbers_length = obj.signal_low_frequency_length + obj.signal_high_frequency_length * high_frequency_samples;
            if size(numbers,1) ~= 1 || size(numbers, 2) ~= expected_numbers_length
               warning('Incorrect number of signals!'); 
               return
            end
            samples = max([high_frequency_samples, 1]);
            high_frequency_numbers = numbers(1:end-obj.signal_low_frequency_length);
            low_frequency_numbers = numbers(end-obj.signal_low_frequency_length+1:end);
            
            if 0 < high_frequency_samples
                augmented_numbers = ...
                    [repmat(low_frequency_numbers, high_frequency_samples, 1), ...
                     reshape(high_frequency_numbers, [], high_frequency_samples)'];
            else
                augmented_numbers = repmat(low_frequency_numbers, high_frequency_samples, 1);
            end
            
            if 0 <= obj.signal_file_handle
               obj.append_signals_to_file(augmented_numbers); 
            end
            
            idx = 1 + mod(obj.signal_buffer_write_idx + (0:samples-1), n);
            obj.signal_buffer(idx, :) = augmented_numbers;
            
            obj.signal_buffer_write_idx = obj.signal_buffer_write_idx + samples;
            obj.signal_n = obj.signal_n + 1;
        end
        
        function signal = get_signals(obj, signal_idx, sample_idx)
            n = size(obj.signal_buffer, 1);
            min_sample_idx = max([obj.signal_buffer_write_idx - n + 1, 1]);
            max_sample_idx = obj.signal_buffer_write_idx;            
            min_signal_idx = 1;
            max_signal_idx = obj.signal_low_frequency_length+obj.signal_high_frequency_length;
            
            % Parse sample_idx
            if nargin < 2
                signal_idx = min_signal_idx:max_signal_idx;
            elseif ischar(signal_idx)
                signal_idx = find(strcmp(obj.signal_names, signal_idx));
            elseif iscell(signal_idx)
                cell_signal_idx = signal_idx;
                signal_idx = []; 
                for signal = cell_signal_idx
                    signal_idx(end+1) = find(strcmp(obj.signal_names, signal{1}), 1);
                end
            elseif isnumeric(signal_idx) || isempty(signal_idx)
            else
                error('Unknown sample_idx format.');
            end
            
            if nargin < 3
               sample_idx = min_sample_idx:max_sample_idx;
            elseif all(sample_idx<1)
                sample_idx = max_sample_idx + sample_idx;
            end
            
            if any(sample_idx<min_sample_idx) || any(max_sample_idx < sample_idx)
                error('Sample index out of bounds!');
            end
            if any(signal_idx<min_signal_idx) || any(max_signal_idx < signal_idx)
                error('Signal index out of bounds!');
            end
            
            idx = 1 + mod(sample_idx-1, n);
            signal = obj.signal_buffer(idx, signal_idx);
        end
        
        %
        % Signals write to file buffer handle
        %
        
        function start_log_signals_to_file(obj, filename, comment)
            if nargin < 3
               comment = filename; 
            end
            if 0 <= obj.signal_file_handle
                warning('signal filename already exists!');
                return;
            end
            obj.signal_file_handle = fopen(filename, 'w');
            
            binary_signals_header = Message( ...
                comment, [], obj.signal_names);
            binary_signals_header_data = binary_signals_header.to_binary();
            fwrite(obj.signal_file_handle, binary_signals_header_data, 'uint8');
        end
        
        function stop_log_signals_to_file(obj)
            if 0 <= obj.signal_file_handle
                fclose(obj.signal_file_handle);
            end
            obj.signal_file_handle = -1;
        end
        
        function append_signals_to_file(obj, signals)
            expected_columns = obj.signal_low_frequency_length + obj.signal_high_frequency_length;
            if expected_columns ~= size(signals, 2)
               error('incorrect number of signals'); 
            end
            
            signals_binary_buffer = uint8([]);
            for i = 1:size(signals, 1)
                signals_binary_buffer = [signals_binary_buffer, Message.double2bin(signals(i, :))];
            end
            fwrite(obj.signal_file_handle, uint8(signals_binary_buffer), 'uint8');
        end
    end
    
    methods (Static)
        function t = load_signals_from_file(filename)
            file_id = fopen(filename, 'r');
            raw_binary_data = fread(file_id);
            fclose(file_id);
            
            msg = Message.from_binary(raw_binary_data(:)');
            header = msg.strings;
            if isempty(msg.numbers)
                data = zeros(0, length(msg.strings));
            else
                data = reshape(msg.numbers, [], length(msg.numbers) / length(msg.strings))';
            end
            t = array2table(data, 'VariableNames', header); 
        end
    end
    
    %
    % Remote getters, setters and commands
    %
    properties
        board_task_name
        sample_frequency       
        send_signals_ratio
        send_signals_ratio_buffered
        send_signals
        u
        low_frequency_signal_names
        high_frequency_signal_names
    end
    properties (Constant, Hidden)
        board_task_name_get_cmd = 'get_board_task_name'
        board_task_name_set_cmd = 'set_board_task_name'
        sample_frequency_get_cmd = 'get_sample_frequency'
        sample_frequency_set_cmd = 'set_sample_frequency'
        send_signals_ratio_get_cmd = 'get_send_signals_ratio'
        send_signals_ratio_set_cmd = 'set_send_signals_ratio'
        send_signals_get_cmd = 'get_send_signals'
        send_signals_set_cmd = 'set_send_signals'
        
        ping_cmd = 'ping'
        restart_cmd = 'restart';
        
        lf_signal_names_get_cmd = 'get_lf_signal_names'
        hf_signal_names_get_cmd = 'get_hf_signal_names'
        task_description_get_cmd = 'get_task_description';
    end
    methods
        % Remote getters and setters
        function set.board_task_name(obj, val)
           obj.send(obj.board_task_name_set_cmd, [], {val});
           assert(strcmp(obj.board_task_name, val));
        end
        function val = get.board_task_name(obj)
           msg = obj.query(obj.board_task_name_get_cmd);
           val = msg.strings{1};
        end
        
        function set.sample_frequency(obj, val)
           obj.send(obj.sample_frequency_set_cmd, val);
           assert(obj.sample_frequency == val);
        end
        function val = get.sample_frequency(obj)
           msg = obj.query(obj.sample_frequency_get_cmd);
           val = msg.numbers(1);
        end
        
        function set.send_signals_ratio(obj, val)
           obj.send(obj.send_signals_ratio_set_cmd, val);
           assert(obj.send_signals_ratio == val);
           
           send_frequency = obj.send_frequency;
           if 125 < send_frequency
               warning(['Send frequency is ' num2str(send_frequency) ' Hz. Near-realtime requirements can require it to be less than 125 Hz.']);
           end
        end
        function val = get.send_signals_ratio(obj)
           msg = obj.query(obj.send_signals_ratio_get_cmd);
           val = msg.numbers(1);
           obj.send_signals_ratio_buffered = val;
        end
        
        function set.send_signals(obj, val)
           obj.send(obj.send_signals_set_cmd, val);
           assert(obj.send_signals == val);
           
           send_frequency = obj.send_frequency;
           if 125 < send_frequency
               warning(['Send frequency is ' num2str(send_frequency) ' Hz. Near-realtime requirements can require it to be less than 125 Hz.']);
           end
        end
        function val = get.send_signals(obj)
           msg = obj.query(obj.send_signals_get_cmd);
           val =  msg.numbers(1);
        end
        
        % Remote commands
        function p = ping(obj)
           time_start = posixtime(datetime);
           q = obj.query(obj.ping_cmd);
           if isempty(q)
              p = nan;
              return
           end
           p = (q.time_received - time_start)/2;
        end
        
        function success = restart(obj)
            success = ~isempty(obj.query(obj.restart_cmd));
        end
        
        function description = get_description(obj)
            msg = obj.query(obj.task_description_get_cmd);
            description = msg.strings{1};
        end
        
        function val = send_frequency(obj)
           val = obj.sample_frequency / obj.send_signals_ratio;
        end
    end
    
    %
    % Helper commands
    %
    methods
        function [mean_latancy, std_latancy, latancy_arr] = estimate_latancy(obj, num_times)
            if nargin < 2
                num_times = 20;
            end
            latancy_arr = zeros(1, num_times);
            
            for i = 1:num_times
                latancy_arr(i) = obj.ping()/2;
            end
            mean_latancy = mean(latancy_arr);
            std_latancy = std(latancy_arr);
            
            if nargout == 0
               disp(['Latancy analysis of ' num2str(num_times) ' experiments.']); 
               disp([' * mean: ' num2str(mean_latancy*1e3), ' ms']);
               disp([' * std:  ' num2str(std_latancy*1e3), ' ms']);
            end
        end    
    end
    methods (Static)
        function list_availiable_bluetooth_devices()
            disp('Determining availiable bluetooth devices. Please wait..');
            devices =  instrhwinfo('Bluetooth');
            disp('Availiable bluetooth devices:');
            for i = 1:length(devices.RemoteNames)
               disp([' * ',  devices.RemoteNames{i}]);
            end
        end
    end 
end

