# Internefs

Internefs is a simple filesystem that can be used to do get and post requests.

# Usage:

1. Mount it with ./main /path/to/folder/you/want/it/to/mount/in/ and add the flag -f to get the debug informations.
2. Do a http get request: read a file in the mountpoint/get folder with the encoded URL as name.
	Example: `cat mountpoint/get/https%3A%2F%2Fexample.com`
3. Do a http post request: write to a file with the encoded URL as the name and the body as file content. you can get the answer by reading the file.
	Example: `echo "foo1=bar1&foo2=bar2" > mountpoint/post/https%3A%2F%2Fpostman-echo.com%2Fpost` and `cat mountpoint/post/https%3A%2F%2Fpostman-echo.com%2Fpost` 
