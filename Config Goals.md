Logger Config Goals:
1. Build the logger as both a shared and dynamic library for Windows. (Linux will be re-added later.)
2. Provide extremely simple installation options. Consumers should be able to:
	a. Use `find_package(DVLogger REQUIRED)`.
	b. Be able to install the library by copying a download containing the library and cmake into their project folder, Or...
	c. Be able to install the library by using `git submodule add ...` within their project folder.
3. Test the library thoroughly by:
	a. Testing the logging levels.
	b. Testing timestamping and message assembly.
	c. Testing stream splitting.
	d. Testing color output only goes to appropriate streams.
	e. Testing logger failure states it can handle gracefully.
	f. Testing logger failure states for undefined behavior.
	g. Testing that logging from different threads politely share resources.
	h. Testing that the logger installs easily and correctly.
4. All build and test targets should be plainly visible to an IDE (like CLion).

