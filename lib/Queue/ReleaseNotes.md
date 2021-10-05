Queue handling library (designed on Arduino)
2017-2020 SMFSW

Feel free to share your thoughts @ xgarmanboziax@gmail.com about:
	- issues encountered
	- optimizations
	- improvements & new functionalities

------------

** Actual:

v1.9:	4 Nov 2020:
- Queue class renamed to cppQueue

v1.8:	4 Nov 2019:
- const qualifiers added where missing
- Added peekIdx and peekPrevious methods
- Added related examples

v1.7:	2 Jun 2019:
- Fixed README.md thanks to @reydelleon
- INC_IDX & DEC_IDX macros changed to inlines
- Added nonnull function attribute where needed
- Updated Doxyfile

v1.6	26 May 2018:
- Constructor does not check anymore if class instance is already allocated (as it supposedly isn't)
- Added getRemainingCount inline returning how much records are left in the queue
- Added sizeOf inline to check full queue size in byte (may also be used to check if queue has been allocated properly)
- Adding support for unit tests and doxygen documentation generation with Travis CI (using travis-ci-arduino from adafruit before custom bash files needed)
- Travis bash scripts found in SMFSW travis-ci-arduino forked repository
- Removed Doxygen anchor with version in source headers
- Updated README.md
- Added more example sketches & updated LibTst example using latest inlines additions

v1.5	14 March 2018:
- Added isInitialized inline to be able to check after init if queue has been properly allocated
- Added flush inline (to have the same functions as in cQueue library)
- LIFO peek temporary variable is uint16_t (same type as in variable)
- Comments fixes

v1.4	21 November 2017:
- Added const qualifier for function parameters

v1.3	12 July 2017:
- #2 fix for esp8266: renamed cpp/h files : header name already used in compiler sys includes
- examples updated with new header file name (cppQueue.h)
- comply with Arduino v1.5+ IDE source located in src subfolder

v1.2	07 July 2017:
- #1 added pull inline for compatibility with older versions (v1.0)
- #2 surrounded c libs with extern C

v1.1	06 July 2017:
- pop keyword used (instead of pull)
- peek & drop functions added
- examples updated to reflect latest changes

v1.0	22 March 2017:
- First release
