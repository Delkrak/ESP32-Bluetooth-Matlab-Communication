classdef Message
    %MESSAGE Summary of this class goes here
    %   Detailed explanation goes here
    
    properties
        time_received double = -1
        time_created double = -1
        command char = ''
        numbers double = []
        strings cell = cell(0)
    end
    properties (Constant, Hidden)
        STRINGS_SEPARATOR_CHARACTER = 29
        NUMBERS_START_CHARACTER = 30
    end
    
    methods
        function obj = Message(command, numbers, strings, time_received)
            if 1 <= nargin 
                obj.command = command;
            end
            if 2 <= nargin
               obj.numbers = numbers; 
            end
            if 3 <= nargin
               obj.strings = strings; 
            end
            if 4 <= nargin
               obj.time_received = time_received;
            end
            obj.time_created = posixtime(datetime);
        end
        
        function output = eq(a, b)
            output = ...
                strcmp(a.command, b.command) && ...
                all(strcmp(a.strings(:), b.strings(:))) && ...
                isequal(a.numbers(:), b.numbers(:));
        end
        
        function disp(obj)
           disp([obj.command, ' -> [', num2str(obj.numbers), ']']);
        end
        
        function data = to_binary(obj)
            data = uint8(obj.command);
            for string = obj.strings
                data = [data, Message.STRINGS_SEPARATOR_CHARACTER, uint8(string{1})];
            end
            data = [data, uint8(Message.NUMBERS_START_CHARACTER)];
            if ~isempty(obj.numbers)
                numbers_binary = Message.double2bin(obj.numbers); %typecast(single(obj.numbers), 'uint8');
                data = [data, numbers_binary];
            end
        end
    end
    methods (Static)
        function obj = from_binary(data)
            numbers_start_index = find(data==Message.NUMBERS_START_CHARACTER, 1, 'first');
            if isempty(numbers_start_index)
                error('At least one data seperator character is required!');
            end
            
            % Extract command and strings
            string_data = char(data(1:numbers_start_index-1));
            tokenized_string_data = strsplit(string_data, char(Message.STRINGS_SEPARATOR_CHARACTER));
            command = tokenized_string_data{1};
            strings = tokenized_string_data(2:end);

            % Extract numbers
            if mod(length(data(numbers_start_index+1:end)), 4) ~= 0
               warning(['Got invalid data: [', num2str(data), ']']);
               return;
            end
            numbers = Message.bin2double(data(numbers_start_index+1:end)); %double(typecast(data(numbers_start_index+1:end), 'single'));
            obj = Message(command, numbers, strings);
        end
        
        function numbers = bin2double(bin)
            numbers = double(typecast(uint8(bin), 'single'));
        end
        
        function bin = double2bin(numbers)
            bin = uint8(typecast(single(numbers), 'uint8'));
        end
        
        function test()
            command = 'myCommand';
            numbers = [1,2,3,1000];
            strings = {'a', 'b', 'c'};
            
            test_decode_encode(Message(command));
            test_decode_encode(Message(command, numbers));
            test_decode_encode(Message(command, [], strings));
            test_decode_encode(Message(command, numbers, strings));
            
            function test_decode_encode(message)
                binary_message = message.to_binary;
                decoded_message = Message.from_binary(binary_message);
                assert(message == decoded_message);
            end
        end
    end
end

