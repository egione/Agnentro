/*
Agnentro
Copyright 2017 Russell Leidich
http://agnentropy.blogspot.com

This collection of files constitutes the Agnentro Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Agnentro Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Agnentro Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Agnentro Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
VERSION

Version info for various components is provided in the files starting with "flag".

THEORY

Whitepapers explaining the theory upon which this library is built are available at the webpage linked at the top of this file.

SYSTEM REQUIREMENTS

Access to a command line in Linus, MacOS, or Windows. For Linux, you will need GCC, which is usually already present. For MacOS, you'll need clang. For Windows, you'll need Mingw-w64, which is awkward but can be made to work reliably.

APPS

All apps are simple command line tools. Don't worry! They're really easy to build once you've met the system requirements listed above.

Agnentro File
-------------
This utility exists for one purpose, namely to demonstrate that agnentropy is a valid entropy metric because it predicts the size of agnentropically encoded files within 2 bits. Yes, I spent a year writing an agnentropic codec and a custom large integer library (Biguint) just because I thought no one would believe it worked. For that matter, the Agnentrocodec library would make a lovely proof-of-work function.

Type "make agnentrofile" to build, then run "tmp/agnentrofile" to display the help text. (The slash goes the other way in Windows.)

Agnentro Find
-------------
Agnentro Find is binary search on steroids! You can search files and folders for text, hex values, duplicate files, or approximate matches to all of the above. You can operate on samples (masks) of 1, 2, 3, or 4 bytes. Preprocessing with deltafication and optional bytewise channelization is supported. You can also elect to overlap masks for better entropy contrast. Optionally, dump search results as text, hex values, or files saved to storage. By the way, it uses interval math on the unit interval, through what I call "fractervals"; you can enable an option called "precise" (see the help text) in order to display the results as 128-bit hex fractervals.

Type "make agnentrofind" to build, then run "tmp/agnentrofind" to display the help text. (The slash goes the other way in Windows.)

Agnentro Log
------------
Agnentro Log is a general purpose logarithmic mask quantizer. The input consists of a binary file or set of files, each record of which being from one to 8 bytes in size. The output is then a corresponding list of bytes on [0, 64], corresponding to to the log2 floor of the respective input records, plus one. (Zero maps to zero; one maps to one; (2^63) and ((2^64)-1) map to 64.)

Type "make agnentrolog" to build, then run "tmp/agnentrolog" to display the help text. (The slash goes the other way in Windows.)

Agnentro Quant
--------------
Agnentro Quant is a general purpose mask quantizer. The input consists of a binary file or set of files, each record of which being from one to 8 bytes in size. A record is treated as a fraction on [0, 1). These records can then be converted to masks of up to 4 bytes in size by multiplying that fraction by a specified quantizer. The whole idea is to provide a generic 64-bit data pathway to interface between other numerical types and the unsigned integer types supported by Agnentro.

Type "make agnentroquant" to build, then run "tmp/agnentroquant" to display the help text. (The slash goes the other way in Windows.)

Agnentro Scan
-------------
Agnentro Scan is a general purpose weirdness detector. Look for anomalous signals in files and folders. You can search for low or high values of agnentropy, exoentropy, logfreedom, or Shannon entropy, and optionally normalize the results for meaningful cross comparison. Like Agnentro Find, it also supports deltafication, channelization, and mask overlap.

Type "make agnentroscan" to build, then run "tmp/agnentroscan" to display the help text. (The slash goes the other way in Windows.)

Agnentro Zorb
-------------
Agnentro Zorb (as in "absorb") absorbs masks lists into a progressively accrued frequency list (effectively, probability distribution) stored in a Zorb (ZRB) file, then issues alerts whenever a given mask list is more, or less, "weird" than some threshold, relative to said frequency list.

Type "make agnentrozorb" to build, then run "tmp/agnentrozorb" to display the help text. (The slash goes the other way in Windows.)

SETI Demo
---------
I wrote this little demo to measure Agnentro's performance on real data from the Search for Extraterrestrial Intelligence. If you want to test it yourself, you'll need some SETI data files from the address given in the help text.

Type "make setidemo" to build, then run "tmp/setidemo" to display the help text. (The slash goes the other way in Windows.)

CODE
----
See code_conventions.txt for all the ugly rules.

CONTACT INFO

Everything you need to sumbit a bug report, request a feature, or just ask a question is provided at the webpage linked at the top of this file.
