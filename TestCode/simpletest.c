#include <stdio.h>
#include <stdlib.h>

#include "xmlparser2.h"

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


