## cobss-matlab
# Streaming Consistent Overhead Byte Stuffing for MATLAB

[Consistent Overhead Byte Stuffing (COBS)](http://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) is an encoding that removes all zero bytes from arbitrary binary data. The encoded data consists only of bytes with values from 0x01 to 0xFF. This is useful for preparing data for transmission over a serial link (RS-232 or RS-485 for example), as the 0 byte can be used to unambiguously indicate packet boundaries. COBS also has the advantage of adding very little overhead (at least 1 byte, plus up to an additional byte per 254 bytes of data) and being computationally efficient.

This is a streaming (byte-wise) implementation for MATLAB for use in practical or simulated applications where encoded messages are not immediately available in their entirety and the user wishes to preview a decoded partial message or gradually build a message.

In memory-restricted hardware applications, a streaming approach can require much less memory than the block coded approach which requires memory for nearly twice the encoded message size.

I developed this specifically for use in my half-duplex [CSMA/CA for Serial Peripheral Interface (SPI)](https://github.com/phyrwork/csma-ca-spi) project, which I am modelling in MATLAB before a hardware implementation. My MATLAB implementation of the faster, block coded approach is also available: [Consistent Overhead Byte Stuffing (COBS) for MATLAB](https://github.com/phyrwork/cobs-matlab)

# Usage

Due to the coding having a small amount of memory (counting values between '0') in order to support multiple simultaneous encode/decode streams I provide classes as opposed to functions with some persistence.

**cobss** --- encoder

	encoder = cobss; % instantiate an encoder object

	encoder.push(byte); % push the next byte in the message onto the encoder

	coded_msg = encoder.get; % return the coded message

The get() message returns the valid coded equivalent of the data processed up until that point. If desired you may continue to push data and get() again at any time.

There is no reset - just create a new instance.


**cobssi** --- decoder

	decoder = cobssi; % instantiate a decoder object

	decoder.push(byte); % push the next byte in a coded message onto the decoder

	decoded_preview = decoder.peek; % return a preview of a decoded message partial

	decoded_msg = decoder.get; % return the decoded message

The peek() method will return whatever has been decoded so far regardless of whether it is a valid message.

The get() method will throw an error if it believes the decoded message to be incomplete - this could occur if the byte stuffing 'code' symbols are corrupted.