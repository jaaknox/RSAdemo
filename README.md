# Assignment 6

'keygen' is a c program that can generate public and private keys for
encryption and decryption. 'encrypt' and 'decrypt' are c program that
use these keys, along with an input, to encrypt and decrypt it, respectively.

## Build

Build the program by running `make` or `make all`. The makefile is included.
Individual executables can be built by running `make` followed by the
executable's name.

## Running

After compiling keygen, run it using `./keygen` followed by the inputs
corresponding to the tests and parameters you would like to run.
These inputs are as follows:

-b: Set the minimum bits needed for the public modulus to the argument passed.

-i: Set the number of Miller-Rabin iterations to the argument passed.

-n: Set the public key file pointer to the argument passed.
    Otherwise, it will default to rsa.pub.

-d: Set the private key file pointer to the argument passed.
    Otherwise, it will default to rsa.priv.

-s: Set the seed for the random state to the argument passed.
    Otherwise, it will default to time(NULL).

-v: Makes the program verbose, which prints the generated variables to the
    console after it runs.

-h: Displays the help message.

After compiling encrypt, run it using `./encrypt` followed by the inputs
corresponding to the tests and parameters you would like to run.
These inputs are as follows:

-i: Set the input file (to be encrypted) to the argument passed.
    Otherwise, it will default to stdin.

-o: Set the output file (to write the encrypted data to) to the argument
    passed. Otherwise, it will default to stdout.

-n: Set the public key file pointer (output by keygen) to the argument
    passed. Otherwise, it will default to rsa.pub.

-v: Makes the program verbose, which prints out the user and the variables
    used in encryption.

-h: Displays the help message.

After compiling decrypt, run it using `./decrypt` followed by the inputs
corresponding to the tests and parameters you would like to run.
These inputs are as follows:

-i: Set the input file (to be decrypted) to the argument passed.
    Otherwise, it will default to stdin.

-o: Set the output file (to write the decrypted message to) to the argument
    passed. Otherwise, it will default to stdout.

-n: Set the private key file pointer to the argument passed.
    Otherwise, set it to rsa.prv.

-v: Makes the program verbose, which prints out the variables used.

-h: Displays the help message.

## Step-by-Step

The simplest way to use this program is to:
1. Create a text file with an input you want encrypted
2. Generate keys with `./keygen`
3. Encrypt the text file by running `./encrypt -i (file's name) -o (what you
   want to name the output file)`
4. Decrypt the text file by running `./decrypt -i (what you named the output
   file) -o (what you want to name the decrypted file)`
5. Open the decrypted text file to read it
