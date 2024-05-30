#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "xmlparser2.h"

/*
   Program to print an XML file.
 */


/*
   Does a string cnsist entirely of white space? (also treat nulls as white)
 */
int strwhitespace(const char *str)
{
    if (!str)
        return 1;
    while (*str)
    {
        if (!isspace((unsigned char) *str))
            return  0;
        str++;
    }
    
    return 1;
}

/*
   Escape a single character, putting it in a buffer
    Returns: pointer to the buffer (for convenience)
 */
static char *xml_escapech(char *buff, int ch)
{
    switch(ch) {
        case '&':  strcpy(buff, "&amp;");  break;
        case '\"': strcpy(buff, "&quot;"); break;
        case '\'': strcpy(buff, "&apos;"); break;
        case '<':  strcpy(buff, "&lt;");  break;
        case '>':  strcpy(buff, "&gt;"); break;
        default:   buff[0] = ch; buff[1] = 0; break;
    }
    
    return buff;
}

/*
   Escape a string to write as XML
 */
static char *xml_escape(const char *data)
{
    int i;
    int size = 0;
    char *answer;
    char *ptr;
    
    for (i = 0; data[i]; i++)
    {
        switch(data[i]) {
            case '&':  size += 5; break;
            case '\"': size += 6; break;
            case '\'': size += 6; break;
            case '<':  size += 4; break;
            case '>':  size += 4; break;
            default:   size += 1; break;
        }
    }
    answer = malloc(size+1);
    if (!answer)
        goto out_of_memory;
    
    ptr = answer;
    for (i = 0; data[i]; i++) {
        switch(data[i]) {
            case '&':  strcpy(ptr, "&amp;"); ptr += 5;     break;
            case '\"': strcpy(ptr, "&quot;"); ptr += 6;    break;
            case '\'': strcpy(ptr, "&apos;"); ptr += 6;     break;
            case '<':  strcpy(ptr, "&lt;"); ptr += 4;      break;
            case '>':  strcpy(ptr, "&gt;"); ptr += 4;      break;
            default:   *ptr++ = data[i]; break;
        }
    }
    *ptr++ = 0;
    
    return answer;
out_of_memory:
    return 0;
}

/*
    Does a node have children embedded within its data string ?
 */
int xml_isembedded(XMLNODE *node)
{
    if (!node->child)
        return 0;
    if (strwhitespace(xml_getdata(node)))
        return 0;
    return  1;
}

/*
   Print the attribute list
 */
int xml_printattributelist(FILE *fp, XMLATTRIBUTE *attr)
{
    char *xmlvalue = 0;
    while (attr)
    {
        xmlvalue = xml_escape(attr->value);
        if (!xmlvalue)
            goto out_of_memory;
        fprintf(fp, " %s=\"%s\"", attr->name, xmlvalue);
        free(xmlvalue);
        xmlvalue = 0;
        attr = attr->next;
    }
    return 0;
out_of_memory:
    return -1;
}

/*
    Process a node in embedded mode, printing children in their correct place in the parent data string
 */
void doembedded(FILE *fp, XMLNODE *node)
{
    int i = 0;
    const char *data;
    XMLNODE *child;
    char buff[16];
    
    data = xml_getdata(node);
    if (!data)
        data = "";
    child = node->child;
    
    while (child)
    {
        while (i < child->position)
        {
            if  (data[i])
                fprintf(fp, "%s", xml_escapech(buff, data[i++]));
            else
                break;
        }
        fprintf(fp, "<%s>", xml_gettag(child));
        doembedded(fp, child);
        fprintf(fp, "</%s>",xml_gettag(child));
    
        child = child->next;
    }
    while (data[i])
    {
        fprintf(fp, "%s", xml_escapech(buff, data[i++]));
    }
}

/*
    Print the tree
 */
void printtree_r(FILE *fp, XMLNODE *node, int depth)
{
    int i;
    char *xmldata = 0;
    
    while (node)
    {
        if (0 && xml_isembedded(node))
        {
            for (i = 0; i < depth; i++)
                fprintf(fp, "\t");
            fprintf(fp, "<%s", xml_gettag(node));
            xml_printattributelist(fp, node->attributes);
            fprintf(fp, ">");
            doembedded(fp, node);
            fprintf(fp, "</%s>\n",xml_gettag(node));
        }
        else
        {
            for (i = 0; i < depth; i++)
              fprintf(fp, "\t");
            fprintf(fp, "<%s", xml_gettag(node));
            xml_printattributelist(fp, node->attributes);
            if (node->child)
            {
                fprintf(fp, ">\n");
                printtree_r(fp, node->child, depth +1);
                for (i = 0; i < depth; i++)
                  fprintf(fp, "\t");
                fprintf(fp, "</%s>\n", xml_gettag(node));
            }
            else
            {
                if (xml_getdata(node))
                {
                    xmldata = xml_escape(xml_getdata(node));
                    fprintf(fp, ">");
                    /* this is the point at which you might change whitespace handling */
                    fprintf(fp, "%s", xmldata);
                    fprintf(fp, "</%s>\n", xml_gettag(node));
                    free(xmldata);
                    xmldata = 0;
                }
                else
                {
                    fprintf(fp, " />\n");
                }
            }
        }
        node = node->next;
    }
}

void usage()
{
    fprintf(stderr, "printxml - prints an XML document\n");
    fprintf(stderr, "usage: printxml <file.xml>");
    fprintf(stderr, "\n");
    fprintf(stderr, "XML passed though the parser, so it will be simplified and reformatted\n");
    exit(EXIT_FAILURE);
}


int main(int argc, char **argv)
{
    XMLDOC *doc;
    char error[1024];
       
    if (argc != 2)
        usage();
    doc = loadxmldoc(argv[1], error, 1024);
    if (!doc)
    {
        fprintf(stderr, "%s\n", error);
        return EXIT_FAILURE;
    }
    printtree_r(stdout, xml_getroot(doc), 0);
    killxmldoc(doc);
       
    return 0;
}


