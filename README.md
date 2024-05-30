# minixml
A single file but powerful XML parser, and associated XPath engine

## Building
It is a single file C source for the XML parser, and another for the XPath engine. Simply take the source files and drop them into your own project.

The code should be completely portable and build anywhere with a C compiler.

## Basic usage

XML files have a tree structure.

This is the structure of the nodes.

```c
typedef struct xmlattribute
{
  char *name;                /* attribute name */
  char *value;               /* attribute value (without quotes) */
  struct xmlattribute *next; /* next pointer in linked list */
} XMLATTRIBUTE;

typedef struct xmlnode
{
  char *tag;                 /* tag to identify data type */
  XMLATTRIBUTE *attributes;  /* attributes */
  char *data;                /* data as ascii */
  int position;              /* position of the node within parent's data string */
  int lineno;                /* line number of node in document */
  struct xmlnode *next;      /* sibling node */
  struct xmlnode *child;     /* first child node */
} XMLNODE;

typedef struct
{
  XMLNODE *root;             /* the root node */
} XMLDOC;
```
So to walk the tree, use the following template code.

```c
void walktree_r(XMLNODE *node, int depth)
{
    int i;
    
    while (node)
    {
        for (i =0; i < depth; i++)
          printf("\t");
        printf("Tag %s line %d\n", xml_gettag(node), xml_getlineno(node));
        
        if (node->child)
            walktree_r(node->child, depth + 1);
        node = node->next;
    }
}
```

Very simple and easy.

### Loading XML files
The loaders are the only non-trivial functions in the file. They are extremely powerful and will load XML files in the main encodings, UTF-8, UTF-16 big endian, and UTF-16 little endian. They don't quite support all of XML but they will load most documents.

There are three loaders

```c
XMLDOC *loadxmldoc(const char *fname, char *errormessage, int Nerr);
XMLDOC *floadxmldoc(FILE *fp, char *errormessage, int Nerr);
XMLDOC *xmldocfromstring(const char *str,char *errormessage, int Nerr);
```
   
They return an XML document on success, NULL on fail. xmldocfromstring has to be passed a string encoded in UTF-8, which usually means plain ASCII. The error message is a buffer for diagnostics if thing go wrong, which is often very important for the user.

Here's an example program.

```c
#include "xmparser2.h"

int main(int argc, char **argv)
{
    XMLDOC *doc;
    
    char error[1024];
       
    if (argc != 2)
          return EXIT_FAILURE;
    doc = loadxmldoc(argv[1], error, 1024);
    if (!doc)
    {
        fprintf(stderr, "%s\n", error);
        return EXIT_FAILURE;
    }
    walktree_r(xml_getroot(doc), 0);
    killxmldoc(doc);
       
    return 0;
}
```  

### Other functions

#### Access functions
```c
const char *xml_gettag(XMLNODE *node);
const char *xml_getdata(XMLNODE *node);
const char *xml_getattribute(XMLNODE *node, const char *attr);
int xml_Nchildren(XMLNODE *node);
int xml_Nchildrenwithtag(XMLNODE *node, const char *tag);
XMLNODE *xml_getchild(XMLNODE *node, const char *tag, int index);
XMLNODE **xml_getdescendants(XMLNODE *node, const char *tag, int *N);
```

xml_gettag(), xml_getdata(), and xml_getattribute() return const pointers to the data members of the node. 
xml_Nchildren() gives the number of direct childen, and xml_Nchildren() gives the number of direct children with a tag. xml_getchild() returns the child with that tag, and the given index. It is a slow but easy way of iterating over children with a given tag.
xml_getdescendants is a fishing expedition. It is essentially the XPath query ("//tag"), but implemented far more efficiently. It picks out all descendants with the given tag.

#### Error reporting functions
The strength of the minixml parser is its error reporting support. 
```c
int xml_getlineno(XMLNODE *node);
XMLATTRIBUTE *xml_unknownattributes(XMLNODE *node, ...);
```

xml_getlineno() is a vital little function when reporting any error in a large XML file to the user. He must know the line at which the bad element occurred, so minixml keeps track of this. And whilst you will naturally detect unknown tags whilst walking the tree, detecting unknown attributes is a little trickier. So minixml provides a handy little function for you.
```c
XLMATTRIBUTE *badattr;
XMLATTRIBUTE *attr;
char *end = NULL;

badattr = xml_unknownattributes(node, "mytag", "faith", "hope" "charity", end);
if (badattr)
{
   printf("Node <mytag> line %d only attributes allowed are faith, hope and charity\n",
                xml_getlineno(node));
   for (attr = badatttr; attr != NULL; attr = attr->next)
   {
      printf("Bad attribute name: %s value: %s\n", attr->name, attr-value);
   }
}
```
You can free the bad attributes recursively. They are deep copies.
Note a quirk of C. You must not pass a raw 0 or even a NULL to a variadic function which expects a character pointer, as it might be treated as 32 bit integer whilst pointers are 64 bits. 


## Test Code
There is nice suite of test programs which use the parser. Whilst they are mainly written for demonstration purposes, some of them are also hoped to be useful. 

### Demonstration programs

- simpletest - a simple test program to walk the tree
- striptags - strip all tags from XML
- upperlower - simple markup language with two tags

  These are intended as simple programs to test the parser, and show how to use it.
  
### Format converters

- xmltojson - XML to JSON converter
- xmltocsv - XML to CSV converter

  These are two file format converters. They are simple, but intended to be usable for real.
  
### The XML FileSystem project

- directorytoxml - package a directry as an XML file
- directory - extract files from a FileSystem XML file packaged by previous program.
- listdirectory - list files in a FileSystem XML file
  
  This is a small but very real project. The idea is to package up a directory or folder in a single XML file, so that it can then be embedded in a program as a string, and used as an internal filesystem. You could also use it as a cheap and cheerful alternative to the Unix program tar.

  Check out progress at the [Baby X resource compiler](http://malcolmmclean.github.io/babyxrc/importingdirectories.html).
    
## Copyright
All the code is authored by Malcolm McLean

It is available as a public service for any use.

[XML Parser docs](http://malcolmmclean.github.io/babyxrc/usingxmlparser.html).  

