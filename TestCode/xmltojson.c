#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "xmlparser2.h"

#define TYPE_UNKNOWN 0
#define TYPE_NUMBER 1
#define TYPE_STRING 2

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
Backspace to be replaced with \b

Form feed to be replaced with \f

Newline to be replaced with \n

Carriage return to be replaced with \r

Tab to be replaced with \t

Double quote to be replaced with \"

Backslash to be replaced with \\
*/
char *escapejsonstring(const char *plain)
{
    int i, j;
    int N = 0;
    char *answer = 0;
    
    for (i =0; plain[i]; i++)
    {
        switch (plain[i])
        {
            case '\b': N += 2; break;
            case '\f': N += 2; break;
            case '\n': N += 2; break;
            case '\r': N += 2; break;
            case '\t': N += 2; break;
            case '\"': N += 2; break;
            case '\\': N += 2; break;
            default: N++; break;
        }
    }
    
    answer = malloc(N +1);
    if (!answer)
        goto out_of_memory;
    j =  0;
    for (i =0; plain[i]; i++)
    {
        switch (plain[i])
        {
            case '\b': answer[j++] = '\\'; answer[j++] = 'b'; break;
            case '\f': answer[j++] = '\\'; answer[j++] = 'f'; break;
            case '\n': answer[j++] = '\\'; answer[j++] = 'n'; break;
            case '\r': answer[j++] = '\\'; answer[j++] = 'r'; break;
            case '\t': answer[j++] = '\\'; answer[j++] = 't'; break;
            case '\"': answer[j++] = '\\'; answer[j++] =  '\"'; break;
            case '\\': answer[j++] = '\\'; answer[j++] = '\\'; break;
            default: answer[j++] = plain[i]; break;
        }
    }
    answer[j++] = 0;
    
    return answer;
out_of_memory:
    return 0;
}

char *trim(const char *str)
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
    answer[j] = 0;
    while (j--)
    {
        if (!isspace((unsigned char)answer[j]))
            break;
        answer[j] = 0;
    }
    
    return answer;
}

char *mystrdup(const char *str)
{
    char *answer = malloc(strlen(str) +1);
    if (answer)
        strcpy(answer, str);
    return answer;
}


int is_array(XMLNODE *node)
{
    XMLNODE *child;
    
    if (xml_Nchildren(node) < 2)
        return 0;
    
    child = node->child->next;
    while (child)
    {
        if (!nodeshavesamestructure(node->child, child, 0, 1))
            return 0;
        child = child->next;
    }
    
    return 1;
}

int is_object(XMLNODE *node)
{
    if (node->child)
        return 1;
    return 0;
}

int is_field(XMLNODE *node)
{
   if (node->child)
       return 0;
    return 1;
    
}

int childrenallfields(XMLNODE *node)
{
    XMLNODE *child;
    
    child = node->child;
    while (child)
    {
        if (!is_field(child))
            return 0;
            
        child = child->next;
    }
    return 1;
}

void writefield(FILE *fp, XMLNODE *node)
{
    int type;
    char *data = 0;
    char *jsondata = 0;
    
    data = trim(xml_getdata(node));
    
    type = getdatatype(data);
    if (type == TYPE_NUMBER)
        fprintf(fp, "%s", data);
    else if (type == TYPE_STRING)
    {
        jsondata = escapejsonstring(data);
        fprintf(fp, "\"%s\"", jsondata);
        free(jsondata);
        jsondata = 0;
    }
    else
        fprintf(fp, "null");
}

void xmltojson_r(FILE *fp, XMLNODE *node, int depth, int writetag, int newline)
{
    int i;
    int sameline;
    
    while (node)
    {
        if (is_array(node))
        {
            if (newline)
            {
                for (i = 0; i < depth; i++)
                    fprintf(fp, "  ");
            }
            if (writetag)
                fprintf(fp, "%s:", xml_gettag(node));
            sameline = childrenallfields(node);
            fprintf(fp, "[");
            if (!sameline)
                fprintf(fp,"\n");
            xmltojson_r(fp, node->child, depth + 1, 0, !sameline);
            if (newline && !sameline)
            {
                for (i = 0; i < depth; i++)
                    fprintf(fp, "  ");
            }
            fprintf(fp, "]");
        }
        else if (is_object(node))
        {
            if (newline)
            {
                for (i = 0; i < depth; i++)
                    fprintf(fp, "  ");
            }
            if (writetag)
                fprintf(fp, "%s:", xml_gettag(node));
            fprintf(fp, "{\n");
            xmltojson_r(fp, node->child, depth + 1, 1, 1);
            if (newline)
            {
                for (i = 0; i < depth; i++)
                    fprintf(fp, "  ");
            }
            fprintf(fp, "}");
        }
        else if (is_field(node))
        {
            if (newline)
            {
                for (i = 0; i < depth; i++)
                    fprintf(fp, "  ");
            }
            if (writetag)
                fprintf(fp, "%s:", xml_gettag(node));
            writefield(fp, node);
        }
        if (node->next)
        {
            if (is_array(node->next) || is_object(node->next) || is_field(node->next))
            {
                fprintf(fp, ",");
                if (newline)
                    fprintf(fp, "\n");
                else
                    fprintf(fp, " ");
            }
        }
        
        node = node->next;
    }
    if (newline)
        fprintf(fp, "\n");
}
                      
int xmltojson(FILE *fp, XMLNODE *node)
{
    if (is_array(node))
    {
        fprintf(fp, "[\n");
        xmltojson_r(fp, node->child, 1, 0, 1);
        fprintf(fp, "]\n");
    }
    else if (is_object(node))
    {
        fprintf(fp, "{\n");
        xmltojson_r(fp, node->child, 1, 1, 1);
        fprintf(fp, "}\n");
    }
    else if (is_field(node))
    {
        
    }
    
    return 0;
}
                      

void usage(void)
{
    fprintf(stderr, "xmltojson - converts xml files to json diles\n");
    fprintf(stderr, "Usage: xmltojson <infile.xml>\n");
    fprintf(stderr, "\n");
    
    exit(EXIT_FAILURE);
}


int main(int argc, char **argv)
{
    XMLDOC *doc;
    char error[1024];
    
    if (argc != 2)
        usage();
        
    doc = loadxmldoc(argv[1],error, 1024);

    if (!doc)
    {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }
    xmltojson(stdout, xml_getroot(doc));
    
    return 0;
}
