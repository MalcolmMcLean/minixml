# xmltocsv
Convert an XML file to a CSV (comma-separated value) file.

## Usage
```
xmltocsv [options] <inputfile.xml>

options: -ignoreattributes - don't consider attribute data
         -ignorechildren - dont consider child elements
         -stdin - pipe input from stdin instead of from a file
```

## Building
If you have CMake, create a directory called "build" under the project
root directory. Navigate to it, then type
```
cmake ..
make
```
or
```
cmake -G <your favourite generator> ..
```
If you don't have CMake, simply navigate to the src directory and type
```
gcc *.c -lm
```
Or use your favourite C compiler.

The code should be completely portable and build anywhere with a C compiler.

## Converting XML to CSV
XML files have a tree structure whilst CSV files are a dataframe, or a 2-dimensional 
data structure with rows representing records and colmns fields, with mixed numbers 
and strings allowed in the fields. So CSV files cannot represent XML data perfectly.
However many XML files are basically dataframes with only a little bit of extra
structure. So what the program does is look for the largest dataframe-like structure
in the XML file, pulls it out, and converts that. Children are folded into the CSV
outout recursively.

So
```
xmltocsv inputfile.xml > outputfile.csv
```
should do what you want, most of the time.

Some XML files structure data with child elements, and some use attributes. If the
attributes are just noise, pass -ignoreattributes to ignore them. If all the data
is in attributes and you want to ignore the child elements, pass -ignorechildren.
(You can't ignore both attributes and children or you will have no data).

Strings containing quotation marks or commas need to be escaped before writing
to CSV. However we don't escape most strings. Whitespace is trimmed, but otherwise they
will appear as in the XML file.

Sometimes you want to set up a pipeline. So pass the -stdin option to pipe the XML
data from standard input, an domit the filename. It's not a very good pipeline 
because, in the nature of XML, the entire document must be read in before the structure 
can be analysed, but it should work on modern systems with plenty of memory.

See how you get on.

