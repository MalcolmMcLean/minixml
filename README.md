# minixml
A single file but powerful XML parser, and associated XPath engine

## Building
It is a single file C source for the XML parser, and another for the XPath engine. Simply take the source files and drop them into your own project.

The code should be completely portable and build anywhere with a C compiler.

## Basic usage

XML files have a tree structure.

This is the structure of the nodes.

```
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

```
void walkttree_r(XMLNODE *node, int depth)
{
    int i;
    
    while (node)
    {
        for (i =0mi < depth; i++)
          printf("\t");
        printf("Tag %s line %d\n", xml_gettag(node), xml_getlineno(node));
        
        if (node->child)
            walktree_r(node->child, depth + 1);
        node->node->next;
    }
}

```

Very simple and easy.

## Copyright
All the code is authored by Malcolm McLean

It is available as a public service for any use.

[XML Parser docs](http://malcolmmclean.github.io/babyxrc/usingxmlparser.html).  

