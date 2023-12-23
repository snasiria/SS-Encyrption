# Assignment 5: Schmidt-Samoa Public Key Cryptography

## Short Description:

This program will perform Schmidt-Samoa encryption and decryption. The user will be prompted to enter an input file (or type in the terminal) of a message first. Before that, the public and private keys must be generated using the SS algorithm. Then the user will be able to encrypt that message which will produce an output file (or terminal output) that will be the ciphertext. Finally, the user will be able to decrypt that back to plaintext.


## Build:

In order to build, run '$make', '$make all' to create the executable files 'keygen', 'encrypt', and 'decrypt' in a command prompt terminal. In order to individually make each of the executable files, type 'make keygen', 'make encrypt', or 'make decrypt' in the command prompt terminal. This will create all the necessary object files for each executable file, which the user can run.

## Cleaning:

To clean the directory after building all the object files and executable file, type '$make clean' to remove all the executable files and all the object files from the directory.

## User Inputs:

In order to get a list of valid inputs for each exectuable, simply type the exectuable followed with a '-h' which will produce a usage message that will guide the user to the potential options and what they each do. For example, to get the list of user inputs for the exectuable 'keygen', type './keygen -h'. Invalid user inputs will bring the user back to the usage message.

## Potential Bugs/Known Errors:

There are no known bugs in the program and there is no memory leakage from any of the executables. There were also no bugs found when I ran scan-build for each of the 3 executable files.

## Running:

In order to run, type '$./keygen', '$./encrypt', or '$./decrypt', followed by a valid argument(s) that is listed in their respective usage messages. The 'keygen' executable must be ran first, followed by the 'encrypt' executable, and 'decrypt' executable last in order to produce a ciphertext and decrypt it. The private key file that 'keygen' produces will only be accessible to the user who owns that file.
