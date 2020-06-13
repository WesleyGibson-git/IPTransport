IPTransport

	Send my own dynamic address to the firewall exception of the static address server.

 
	First, we need to use RSA to generate the key (If you have OpenSSL installed)：
		openssl genrsa -out rsa_private_key.pem 1024
		openssl rsa -in rsa_private_key.pem -pubout -out rsa_public_key.pem
	Copy them into the program document,client use public key,server use private key.

	Second, config the /etc/rsyslog.conf, add 2 lines at the end:
		#TransferLog
		local2.* /var/log/transferAddress.log
	The logs will write in /var/log/transferAddress.log.

	Third, install ccl(use it to read config file)：
		wget https://files.sbooth.org/ccl-0.1.1.tar.gz
		tar zxvf ccl-0.1.1.tar.gz
		cd ccl-0.1.1
		./configure
		./make
		sudo make install


	My Visual code tasks.json: 
	{
	    // See https://go.microsoft.com/fwlink/?LinkId=733558 
	    // for the documentation about the tasks.json format
	    "version": "2.0.0",
	    "tasks": [
	        {
	            "type": "shell",
	            "label": "gcc build active file",
	            "command": "/usr/bin/gcc",
	            "args": [
	                "-g",
	                "${fileDirname}/*.c",
	                "${fileDirname}/*.h",
	                "-o",
	                "${fileDirname}/${fileBasenameNoExtension}",
	                "-lcrypto",
	                "-lssl"
	            ],
	            "options": {
	                "cwd": "/usr/bin"
	            },
	            "problemMatcher": [
	                "$gcc"
	            ],
	            "group": "build"
	        }
	    ]
	}




If request belongs to the following four situations，and three times, the IP address will add the black list:

1. Receive the length of data is wrong.

2. RSA decrypt failed.

3. The sandwich string is wrong.

4. The ip address of transporting is not a valid ip address.
