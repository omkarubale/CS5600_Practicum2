# CS5600_Practicum2

Make the Project:
>> make clean
>> make 

Run the server:
>> cd server
>> ./server

To run the test file:

// Navigate to Client
>> cd ../client
>> ./testing

To run individual commands on the client:
// No. of arguments as per the command.
>> ./fget COMMAND ARG1 ARG2 

eg1: ./fget GET h5.txt f1/h2.txt
eg2: ./fget GET h5.txt 

eg3: ./fget INFO h3.txt
eg4: ./fget INFO folder

eg5: ./fget MD newFolder
eg6: ./fget PUT lorem/loremContent.txt bigFile.txt

eg7: ./fget PUT lorem/loremContent.txt
eg8: ./fget RM newFolder
eg9: ./fget RM filr.txt


