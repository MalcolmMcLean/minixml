#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "options.h"
#include "xmlparser2.h"

#define TYPE_UNKNOWN 0
#define TYPE_NUMBER 1
#define TYPE_STRING 2

static char *escapecsvstring(const char *str);
static char *trim(const char *str);
static char *mystrdup(const char *str);
static int mystrcount(const char *str, int ch);

/*
  test if two nodes have the same structure. Do the attributes and the hierarchy
    under them match, and are the element tag names the same?
  Params:
     nodea- the first node
     nodeb - the second node
     useattributes - if set, check that the attribute lists match
     usechildren - if set, check that the children match
Returns 1 if the nodes have the same struture, else 0
 */
int nodeshavesamestructure(XMLNODE *nodea, XMLNODE *nodeb, int useattributes, int usechildren)
{
   XMLATTRIBUTE *attra, *attrb;
   XMLNODE *childa, *childb;

   if (strcmp(nodea->tag, nodeb->tag))
      return 0;

   if (useattributes)
   {
      attra = nodea->attributes;
      attrb = nodeb->attributes;
      while (attra && attrb)
      {
         if (strcmp(attra->name, attrb->name))
           return 0;
         attra = attra->next;
         attrb = attrb->next;
      }
      if (attra != NULL || attrb != NULL)
         return 0;
   }
   if (usechildren)
   {
      childa = nodea->child;
      childb = nodeb->child;
      while (childa && childb)
      {
        if (!nodeshavesamestructure(childa, childb, useattributes, usechildren))
          return 0;
        childa = childa->next;
        childb = childb->next;
     }
     if (childa != NULL || childb != NULL)
        return 0;
   }
   
   return 1;
}

/*
   Get the number of children with the same structure
   Params:
      node - the node to test
      useattributes - if set, consider attributes of children
      usechildren - if set, consider descendants of children
   Returns: 0 if the node has mixed childrem otherwise the number of children
 */
int Nchildrenwithsamestructure(XMLNODE *node, int useattributes, int usechildren)
{
   int answer = 0;
   XMLNODE *child;

    for (child = node->child; child; child = child->next)
   {
      if (nodeshavesamestructure(node->child, child, useattributes, usechildren))
        answer++;
      else
        return 0;
   }
    
    return answer;
}

/*
   choose  a parent node (recursive)
     Params:
        node - the node to test
        useattributes - consider attributes
        usechildren - consider children
        best  the best node found so far
     Returns: the best descendant to use as the parent node.
     Notes: it goes throuhg the tree looking for the node with the largest number of identical structure children.
 */
XMLNODE *chooseparentnode_r(XMLNODE *node, int useattributes, int usechildren, int best)
{
    XMLNODE *bestnode = 0;
    XMLNODE *testnode;
    XMLNODE *child;
    int Ngoodchildren;
    

    Ngoodchildren = Nchildrenwithsamestructure(node, useattributes, usechildren);
    
    if (Ngoodchildren > best)
    {
        bestnode = node;
        best = Ngoodchildren;
    }
    for (child = node->child; child; child = child->next)
    {
        testnode = chooseparentnode_r(child, useattributes, usechildren, best);
        if (testnode)
        {
            Ngoodchildren = Nchildrenwithsamestructure(testnode, useattributes, usechildren);
            if (Ngoodchildren > best)
            {
                bestnode = testnode;
                best = Ngoodchildren;
            }
        }
    }
    
    return bestnode;
}

/*
    Fish for a parent node for the CSV data
    Params: node - the node
            useattributes - consder attributes
            usehildren - consider children
    Returns: the node to use for th parent, NULL i dthere isnt a suitable one.
    Note: the parent node is th enode with most children of identical structure.
 */
XMLNODE *chooseparentnode(XMLNODE *node, int useattributes, int usechildren)
{
    return chooseparentnode_r(node, useattributes, usechildren, 0);
}

int getdatatype(const char *str)
{
    char *end;
    int i;
    
    if (str == NULL)
        return TYPE_UNKNOWN;
    
    for (i =0; str[i]; i++)
        if (!isspace((unsigned char)str[i]))
            break;
    if (!str[i])
        return TYPE_UNKNOWN;
    
    strtod(str, &end);
    if (*end == 0)
        return TYPE_NUMBER;
    return TYPE_STRING;
}

/*
   convert a node to a CSV file record (recursive)
   Params:
     node - the node
     useattrinutes - write attribute values
     usechildren - write chld data recursively
     index - idex of the column (for comma)
     fp - output stream
   Returns: number of colums written.
 */
int nodetorecord_r(XMLNODE *node, int useattributes, int usechildren, int index, FILE
*fp)
{
   XMLATTRIBUTE *attr;
   XMLNODE *child;
    char *escaped;
    
   int i = 0;

   if (useattributes)
   {
      for (attr = node->attributes; attr; attr = attr->next)
      {
          if (i + index > 0)
              fprintf(fp, ", ");
          escaped = escapecsvstring(attr->value);
         fprintf(fp, "%s", escaped);
          free(escaped);
         i++;
      }
   }
   if (usechildren)
   {
      for (child = node->child; child; child = child->next)
      {
         if (i + index > 0)
             fprintf(fp, ", ");
         escaped = escapecsvstring(child->data);
         fprintf(fp, "%s", escaped);
         free(escaped);
         i++;
         i += nodetorecord_r(child, useattributes, usechildren, i + index, fp);
      }
   }
  
   return i;
}

/*
   Convert a node to a CSV file record
   Params:
      node - the node
      useattributes - consider the attributes
      usechildren - consider children recursively
      fp - output file.
 */
void nodetorecord(XMLNODE *node, int useattributes, int usechildren, FILE 
*fp)
{
   nodetorecord_r(node, useattributes, usechildren, 0, fp);
   fprintf(fp, "\n");
}  

typedef struct field
{
   char *name;  /* name in CSV header */
   char *tag; /* tag of owning XML element */
   int attribute; /* set if the filed is an attribute */
   int datatype; /* type of data it is */
} FIELD;

typedef struct 
{
  FIELD *field;
  int Nfields;
} FIELDS;




int getfields_r(XMLNODE *node, FIELDS *fields, int useattributes, int
usechildren)
{
    FIELD *temp;
    XMLATTRIBUTE *attr;
    XMLNODE *child;
    int type;
    
   if (useattributes)
   {
      for (attr = node->attributes; attr; attr = attr->next)
      {
         temp = realloc(fields->field, (fields->Nfields +1) * 
sizeof(FIELD));
         fields->field = temp;
         fields->field[fields->Nfields].name = mystrdup(attr->name);
         fields->field[fields->Nfields].tag = mystrdup(node->tag);
         fields->field[fields->Nfields].attribute = 1;
         fields->field[fields->Nfields].datatype = TYPE_UNKNOWN;
         fields->Nfields++;
      }
   }
   if (usechildren)
   {
      for (child = node->child; child; child = child->next)
      {
          temp = realloc(fields->field, (fields->Nfields +1) *
sizeof(FIELD));
         fields->field = temp;
         fields->field[fields->Nfields].name = mystrdup(child->tag);
         fields->field[fields->Nfields].tag = mystrdup(node->tag);
         fields->field[fields->Nfields].attribute = 0;
         fields->field[fields->Nfields].datatype = TYPE_UNKNOWN;
         fields->Nfields++;
         getfields_r(child, fields, useattributes, usechildren);

      }
   }
    
    return 0;
}

int determinefieldtype_r(XMLNODE *node, FIELDS *fields, int
useattributes, int usechildren, int index)
{
   int i = 0;
    XMLATTRIBUTE *attr;
    XMLNODE *child;
    int type;

   if (useattributes)
   {
      for (attr = node->attributes; attr; attr = attr->next)
      {
         type = getdatatype(attr->value);
         if (fields->field[index+i].datatype == TYPE_UNKNOWN)
           fields->field[index+1].datatype = type;
         else if (fields->field[index+i].datatype == TYPE_NUMBER
            && type == TYPE_STRING)
            fields->field[index+i].datatype = type;
        
        i++; 
      }
   }
   
   if (usechildren)
   {
      for (child = node->child; child; child = child->next)
      {
           type = getdatatype(child->data);
         if (fields->field[index+i].datatype == TYPE_UNKNOWN)
           fields->field[index+1].datatype = type;
         else if (fields->field[index+i].datatype == TYPE_NUMBER
            && type == TYPE_STRING)
            fields->field[index+i].datatype = type;
        i++;

        i += determinefieldtype_r(child, fields, useattributes,
usechildren, index + i);
      }
   }

   return i;
}

FIELDS *getfields(XMLNODE *parent, int useattributes, int usechildren)
{
   FIELDS *fields;
    XMLNODE *child;
   
   fields = malloc(sizeof(FIELDS));
   fields->field = 0;
   fields->Nfields = 0;

   getfields_r(parent->child, fields, useattributes, usechildren);
    for (child = parent->child; child; child = child->next)
   {
      determinefieldtype_r(child, fields, useattributes, usechildren, 0);
   }
    
    return fields;

}

void printheader(FIELDS *fields, FILE *fp)
{
   int i;
   
   for (i = 0; i < fields->Nfields; i++)
   {
      if (i > 0)
          fprintf(fp, ", ");
      fprintf(fp, "%s", fields->field[i].name);
   }

   fprintf(fp, "\n");
}

/*
  escape a csv string so that it can go in a field.
   Params: str - the plain text
   Returns: the escaped string.
   Note: most strings don't need to be escaped. Only ones with embedded commas or quotes.
 */
static char *escapecsvstring(const char *str)
{
    int len;
    char *answer = 0;
    char *trimmed = 0;
    int i;
    int j = 0;
    
    if(!str)
        return mystrdup("");
    
    trimmed = trim(str);
    if (!trimmed)
        goto out_of_memory;
    if (strchr(trimmed, '\"') || strchr(trimmed, ','))
    {
        len = strlen(trimmed) + mystrcount(trimmed, '\"') + 2 + 1;
        answer = malloc(len);
        if (!answer)
            goto out_of_memory;
        answer[j++] = '\"';
        for (i = 0; trimmed[i]; i++)
        {
            if (trimmed[i] == '\"')
            {
                answer[j++] = '\"';
            }
            answer[j++] = trimmed[i];
        }
        answer[j++] = '\"';
        answer[j++] = 0;
    }
    else
    {
        answer = mystrdup(trimmed);
        if (!answer)
            goto out_of_memory;
    }
    
    free(trimmed);
    return answer;
out_of_memory:
    free(trimmed);
    free(answer);
    return 0;
}

static char *trim(const char *str)
{
    int i;
    int j = 0;
    char *answer = malloc(strlen(str) + 1);
    if (!answer)
        return 0;
    for (i = 0; isspace((unsigned char) str[i]); i++)
        ;
    while (str[i])
        answer[j++] = str[i++];
    answer[j++] = 0;
    while (j--)
    {
        if (!isspace((unsigned char)answer[j]))
            break;
        answer[j] = 0;
    }
    
    return answer;
}

static char *mystrdup(const char *str)
{
    char *answer = malloc(strlen(str) +1);
    if (answer)
        strcpy(answer, str);
    return answer;
}

static int mystrcount(const char *str, int ch)
{
    int answer = 0;
    int i;
    
    for (i = 0; str[i]; i++)
        if (str[i] == ch)
            answer++;
    
    return answer;
}

void usage(void)
{
    fprintf(stderr, "xmltocsv - converts xml files to csv files\n");
    fprintf(stderr, "Usage: xmltocsv [options] <infile.csv>\n");
    fprintf(stderr, "       Options: -ignorechildren - ignore child nodes\n");
    fprintf(stderr, "                -ignoreattributes - ignore attribute data\n");
    fprintf(stderr, "                -stdin - pipe input instead of reading a file\n");
    fprintf(stderr, "\n");
    
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
   XMLDOC *doc;
    char error[1024];
    XMLNODE *parentnode;
    XMLNODE *child;
    FIELDS *fields;
    OPTIONS *opt;
    char *filename = 0;
    int help  = 0;
    int ignoreattributes = 0;
    int ignorechildren = 0;
    int pipe = 0;
    
    opt = options(argc, argv, 0);
    help = opt_get(opt, "-help -h -H --help /H -?", 0);
    ignorechildren = opt_get(opt, "-ignorechildren", 0);
    ignoreattributes = opt_get(opt, "-ignoreattributes", 0);
    pipe = opt_get(opt, "-stdin", 0);
    if (!pipe)
    {
        if (opt_Nargs(opt) != 1)
            usage();
        filename = opt_arg(opt, 0);
    }
    if (opt_error(opt, stderr))
    {
        killoptions(opt);
        free(filename);
        exit(EXIT_FAILURE);
    }
    killoptions(opt);
    
    opt = 0;
    
    if (help)
        usage();
    
    if (ignoreattributes && ignorechildren)
    {
        fprintf(stderr, "Warning if you ignore both attributes and children there will be no output\n");
    }
    
    if (!strcmp(filename, "-"))
        pipe = 1;
    
   if (pipe)
       doc = floadxmldoc2(stdin, error, 1024);
   else
        doc = loadxmldoc2(filename, error, 1024);
   if (!doc)
   {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
   }
    
    parentnode = chooseparentnode(doc->root, !ignoreattributes, !ignorechildren);
    if (parentnode)
    {
        fields = getfields(parentnode, !ignoreattributes, !ignorechildren);
        printheader(fields, stdout);
        for (child = parentnode->child; child; child = child->next)
            nodetorecord(child, !ignoreattributes, !ignorechildren, stdout);
    }
    else {
        fprintf(stderr, "Couldn't find a suitable array\n");
    }
    
    free(filename);
    
   return 0;
}
