//
//  upperlower.c
//  simpletest
//
//  Created by Malcolm McLean on 27/05/2024.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "xmlparser2.h"

void dolower(XMLNODE *node);
void doupper(XMLNODE *node);

/*
 This test program is designed to show you how to use the "position" element to
 place children within the parent's dara string when qw aew usung XML as a text mark
 up language. It's a toy language, with two tags, "<U>" and "<L>" which set text
 to upper case and lower case respectively, and can nest.
 
 The tags are only valid when descendants of an "<upperlower>" tag.
 */
const char *testcase =
"<root>"
"  <upperlower>"
   "Mary had a little <U>lamb</U> its fleece\n"
    "was <U>white as snow. <L>And </L>everywhere<U>\n"
    "that <L>Mary</L> went</U> the <L>lamb</L> was</U> sure to go.\n"
"  </upperlower>"
"</root>";

void dolower(XMLNODE *node)
{
    int i = 0;
    const char *data;
    XMLNODE *child;
    
    data = xml_getdata(node);
    child = node->child;
    
    while (child)
    {
        while (i < child->position)
        {
            if  (data[i])
                printf("%c", tolower(data[i++]));
            else
                break;
        }
        if (!strcmp(xml_gettag(child), "U"))
        {
            doupper(child);
        }
        else if (!strcmp(xml_gettag(child), "L"))
        {
            dolower(child);
        }
        
        child = child->next;
    }
    while (data[i])
    {
        printf("%c", tolower(data[i++]));
    }
}

void doupper(XMLNODE *node)
{
    int i = 0;
    const char *data;
    XMLNODE *child;
    
    data = xml_getdata(node);
    child = node->child;
    
    while (child)
    {
        while (i < child->position)
        {
            if  (data[i])
                printf("%c", toupper(data[i++]));
            else
                break;
        }
        if (!strcmp(xml_gettag(child), "U"))
        {
            doupper(child);
        }
        else if (!strcmp(xml_gettag(child), "L"))
        {
            dolower(child);
        }
        
        child = child->next;
    }
    while (data[i])
    {
        printf("%c", toupper(data[i++]));
    }
}

void doupperlower(XMLNODE *node)
{
    int i = 0;
    const char *data;
    XMLNODE *child;
    
    data = xml_getdata(node);
    child = node->child;
    
    while (child)
    {
        while (i < child->position)
        {
            if  (data[i])
                printf("%c", data[i++]);
            else
                break;
        }
        if (!strcmp(xml_gettag(child), "U"))
        {
            doupper(child);
        }
        else if (!strcmp(xml_gettag(child), "L"))
        {
            dolower(child);
        }
        
        child = child->next;
    }
    while (data[i])
    {
        printf("%c", data[i++]);
    }
    if (i > 0 && data[i-1] != '\n')
        printf("\n");
    
}

void upperlower_r(XMLNODE *node)
{
    while (node)
    {
        if (!strcmp(xml_gettag(node), "upperlower"))
            doupperlower(node);
        else if (node->child)
            upperlower_r(node->child);
            node = node->next;
    }
}


int main(int argc, char **argv)
{
    XMLDOC *doc;
    char error[1024];
    
    if (argc == 2)
        doc = loadxmldoc(argv[1], error, 1024);
    else
        doc = xmldocfromstring(testcase, error, 1024);
    if (!doc)
    {
        fprintf(stderr, "%s\n", error);
        return 0;
    }
    upperlower_r(xml_getroot(doc));
    killxmldoc(doc);
    
    return 0;
}
