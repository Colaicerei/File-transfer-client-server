Compile: type make at one of the flip	


Run ftserver: ./ftserver <portnumber>
	e.g. at flip3, type "./ftserver 11888"


Run client: type "python ftclient.py <host> <portnumber> <-command> "filename" <dataport>, filename is only required for get command.
	e.g. at flip2, type:
		"python ftclient.py flip3 11888 -l 18888" to get directory
		"python ftclient.py flip3 11888 -g shortfile.txt 18888" to get file

wrong number of arguments, wrong port number and wrong command were also tested.
	e.g. at flip2, type:
		"python ftclient.py flip3 12888 -g shor]tfile.txt 18888"
		will return error: flip3: 12888 says FILE NOT FOUND!

		" python ftclient.py flip3 12888 -sg shortfile.txt 18888"
		will return error: flip3: 12888 says ERROR, please send a valid command!


		
		


