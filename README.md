# Reliable File Transfer Over UDP
## High Level Approach
In this project, I first started out by defining the data
I wanted to put in different types of packets by using structs.
This allowed me to easily visualize the data I am sending and
getting and let's me easily check if the data is valid.

Then I implemented the Go-Back-N protocol by sending a character
buffer. Afterwards I implemented reading and writing files.

Then I did command line arguments and tested by adding code to
duplicate and drop packets randomly into the sender and receiver program.

## How to use
`./sender -m <mode> -p <port> -h <hostname> -f <filename>`
Mode is an integer greater than 1 (defaults to 1)
Port is a valid port number (defaults to 15180)
This must match the receiver port
Hostname is the host to send to (defaults to 127.0.0.1)
Filename is the file to send (defaults to README)

`./receiver -m <mode> -p <port> -h <hostname>`
Mode is an integer greater than 1 (defaults to 1)
This must match the sender mode
Port is a valid port number to listen on
Hostname is the host to listen on (defaults to 127.0.0.1)
This should match the hostname of the system you are on

## Examples

`./sender -m 3 -p 15180 -h trilby.ccs.neu.edu -f README`
Sender is on gatsby sending README to trilby.ccs.neu.edu:15180
with a window size of 3

`./receiver -m 3 -p 15180 -h trilby.ccs.neu.edu`
Receiver is on trilby waiting to receive a file
with a window size of 3

## Testing

1. Test mismatched window sizes
Receiver exits
Sender exits after 3 retries
2. Test network errors
Tested dropping packets randomly from the sender and receiver
and also randomly duplicating acks and data packets
3. Test sending image
Sent a 200K image and ran a checksum
to make sure the files matched
4. Test sending large file
Tested sending a 100MB file.
5. Test different hosts
Transferred file from trilby to gatsby
Transferred file from turban to nightcap
6. Test disconnecting
Sender disconnected after retrying 10 times
7. Test long file name
Filename truncates to 254 characters before sending
