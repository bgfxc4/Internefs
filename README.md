# Internefs

Internefs is a simple filesystem that can be used to do get and post requests.

# Installation

First, clone the github repo with `git clone https://github.com/bgfxc4/internefs` and navigate into it with `cd internefs`.

Then build it using the makefile: type `make`.

# Usage

1. Mount it with ./internefs /path/to/folder/you/want/it/to/mount/in/ and add the flag -f to get the debug informations.
2. Do a http get request: Read a file in the mountpoint/get directory with the encoded URL as name.
	Example: `cat mountpoint/get/https%3A%2F%2Fexample.com`
3. Do a http post request: Write to a file in the mountpoint/post directory with the encoded URL as the name and the body as file content. You can get the answer by reading the file.
	Example: `echo "foo1=bar1&foo2=bar2" > mountpoint/post/https%3A%2F%2Fpostman-echo.com%2Fpost` and `cat mountpoint/post/https%3A%2F%2Fpostman-echo.com%2Fpost`
4. Do a header request: Read a file in the mountpoint/head directory with the encoded URL as name.
	Example: `cat mountpoint/head/https%3A%2F%2Fexample.com`

If you want to stop using it, you have to unmount it using `sudo umount [mountpoint]` (You may have to use the flag -r).
