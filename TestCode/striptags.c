//
//  striptags.c
//  upperlower
//
//  Created by Malcolm McLean on 29/05/2024.
//

#include <stdio.h>
#include <stdlib.h>

#include "xmlparser2.h"

int printdata(FILE *fp, XMLNODE *node)
{
    char *nested;
    
    nested = xml_getnesteddata(node);
    if (!nested)
        return - 1;
    
    fputs(nested, fp);
    free(nested);
    
    return 0;
}

void usage(void)
{
    fprintf(stderr, "striptags: strips all tags from a xml document\n");
    fprintf(stderr, "Usage: striptags <xmlfile.xml>\n");
}

int main(int argc, char **argv)
{
    XMLDOC *doc;
    char error[1024];
    
    if (argc != 2)
    {
        usage();
        return -1;
    }
    doc = loadxmldoc(argv[1], error, 1024);
    if (!doc)
    {
        fprintf(stderr, "%s\n", error);
        return -1;
    }
    printdata(stdout, xml_getroot(doc));
    killxmldoc(doc);
    
    return 0;
}
