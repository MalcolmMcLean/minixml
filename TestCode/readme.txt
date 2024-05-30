TestCode for xmlparser2.c

by Malcolm McLean


This is a suite of test programs to show off the XML parser. 

They are all short single file programs, with the exception of the dependency on the parser itself, with the exception of xmltocsv, which uses an options parser to handle commandline options. Some of them are purely for demonstration purposes, which it is hoped that others might actually be useful.

The CMake script works for XCode on an Apple Mac. It might break horribly on the platforms. If you can compile for the commandline you don't really need it - you only have two source files to specify to the compiler. Which is the beauty of single file solution like XMLParser2  - you don't mess about for ages setting things up, just drop the file and its header into your project.

simpletest.c

This just tests the parser's load function, and then walks the document tree, printing out the tags. But it shows off the error reporting capabilities.

printxml.c

Run the XML through the parser, and print it back out. Essential test program.

striptags.c

Shows how utterly simple it is to strip the tags and obtain the data string using the facilities of the parser. It also shows up one of the problems - whitespace handling in XML is difficult and currently the parser does not attempt it.

uppertolower.c

This little program implements a scheme whereby the <U> tag sets to upper case and the <L> rage sets to lower case. And of course the tags can nest. And it shows how to use the position member to get the child's position within the parent's direct string. You can of course use this as a template for any text markup scheme.

Converters

xmltojson.c

Converts XML to json. Very basic and doesn't consider attributes.

xmltocsv.c

Converts XML to CSV. This is altogether a bit more sophisticated, as CSV data is tabular and so the tree structure of XML cannot be represented. So it tries to pick out the actual data and ignore any supplementary nodes by looking for the largest array-like sequence. There are also options to use children as field, or attributes as fields, or both.

XML directory project

This is a use of the XML parser for "real". The problem is to take a directory or folder on a machine, and package it up into an XML file. Then to query the XML to get the files out. So you could package the directory portably and then recreate it on a target machine. Or the actual use I have is to embed it into programs so that they have an internal filesystem they can use for data. So the program directorytoxml.c is in fact nothing to do with the parser and generates the XML files. Whist the program directory.c extracts files from the XML, and the program lisdirectory.c lists the files in the XML.

directorytoxml.c

Unfortunately this one can't be written portably. There's just no way in ANSI C to get at a directory. So we have to use the readdir() function from Posix. Be prepared for some very large files. 

The XML has the following structure

<FileSystem>
      <directory name="poems">
            <directory name="Shakespeare">
               <file name="Sonnet 12" type="text">
                  When do I count the clock that tells the time?
                </file>
            </directory>
		<file name="Tyger" type="text">
                  Tyger, tyger, burning bright,
                  Through the forests of the night
                </file>
		<file name="The Lamb" type="text">
                  Little lamb, who made thee?
                </file>
	        <file name="artwork.png" type="binary">
                  <![CDATA[..uunencoded_gibberish...?>l&..&gt;.^^^..]]>
                </file>
            <directory name="Blake">
            </directory>
            <file name="readme.txt" type="text">
                Curiosity killed the cat.
            </file>
    
      </directory>
</FileSystem>    

directory.c

This program extracts files from the FileSystem XML.

Call it like this

directory poems.xml /poems/Blake/Tyger

And you should get the output

Tyger, tyger, burning bright,
Through the forests of the night

And if you call with a binary and redirect to a file, you should get the binary recreated. Though the OS may take action against executables.

listdirectory.c

This list the files in the directory. Pass it a glob. So  

listdirectory poems.xml "/poems/*"

should produce the output

Shakespeare
Blake
readme.txt


Cooyright

With the exception of the routines to do uuencoding / decoding, which were stolen and reworked to make them work on strings, all the code is by me and is free for any use. If you  earn your living by selling copyright software then I'm anxious to help, though the project is mainly aimed at hobby programmers. 

Malcolm McLean


