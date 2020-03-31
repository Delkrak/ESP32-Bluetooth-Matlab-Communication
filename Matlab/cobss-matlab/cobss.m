classdef cobss < handle
    %COBSS Consistent Overhead Byte Stuffing Streaming encoder
    % Byte-wise (streaming) COBS encode - see
    % http://www.stuartcheshire.org/papers/cobsforton.pdf for algorithm
    % details
    
    properties (Access = private)
        data = uint8(1);
        index = 1; % index where next code value (distance to next '0') will go
        count = 1; % next code value (incremented at every non-'0' push)
    end
    
    methods        
        function obj = push(obj, data)
            switch data
                case 0 % illegal character - this could be changed to be any other character as long as changes are reflected in cobssi
                    obj.data(obj.index) = obj.count; % write stuffing code to start of block
                    obj.count = 1; % reset stuffing code
                    obj.index = length(obj.data) + 1; % mark location of next stuffing code
                    obj.data(obj.index) = 0; % placeholder
                otherwise
                    obj.data(end+1) = data; % push data to output
                    obj.count = obj.count + 1; % increment distance to next 0
                    if obj.count == 255 % max stuffing code value - indicate 254 characters followed by the next code
                        obj.data(obj.index) = 255;
                        obj.count = 1;
                        obj.index = length(obj.data) + 1; % mark location of next stuffing code
                        obj.data(obj.index) = 0; % placeholder
                    end
            end
        end
        
        function data = get(obj)
            data = obj.data; % don't modify the write-set
            if obj.data(end) == 1; % if last data was a 0
                data(end+1) = 1; % add a phantom '0' so that the decoder doesn't remove a real one
            else
                data(obj.index) = obj.count; % complete last block
            end
        end
    end  
end

