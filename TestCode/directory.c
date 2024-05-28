//
//  directory.c
//  directorytoxml
//
//  Created by Malcolm McLean on 28/05/2024.
//
#include "xmlparser2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

char *directoryname(const char *path, int pos)
{
    int i;
    int j = 0;
    int len = 0;
    char *answer = 0;
    
    for(i = pos + 1; path[i]; i++)
        if(path[i] == '/')
            break;
    len = i - pos - 1;
    answer = malloc(len + 1);
    if (!answer)
        goto out_of_memory;
    for(i = pos + 1; path[i]; i++)
    {
        if(path[i] == '/')
            break;
        answer[j++] = path[i];
    }
    answer[j++] = 0;
    return answer;
out_of_memory:
    return 0;
}

FILE *file_fopen(XMLNODE *node)
{
    FILE *fp;
    int len;
    const char *data;
    char *last;
    int trailing = 0;
    
    fp = tmpfile();
    if (!fp)
        goto error_exit;
    data = xml_getdata(node);
    len = (int) strlen(data);
    last = strrchr(data, '\n');
    if (last && strwhitespace(last))
        trailing = len - (int)(last - data);
    if (len - trailing < 1)
        goto error_exit;
    if (fwrite(data +1, 1, len - trailing - 1, fp) != len - trailing - 1)
        goto error_exit;
    fseek(fp, 0, SEEK_SET);
    return fp;
    
error_exit:
    fclose(fp);
    return 0;
}

FILE *directory_fopen_r(XMLNODE *node, const char *path, int pos)
{
    FILE *answer = 0;
    const char *nodename;
    char *name = 0;
    XMLNODE *child;
    int lastdir;
    
    name = directoryname(path, pos);
    while (node)
    {
        if (!strcmp(xml_gettag(node), "directory"))
        {
            nodename = xml_getattribute(node, "name");
            if (nodename && !strcmp(name, nodename))
            {
                answer = directory_fopen_r(node->child, path, pos + strlen(name) + 1);
            }
        }
        else if(!strcmp(xml_gettag(node), "file"))
        {
            nodename = xml_getattribute(node, "name");
            if (nodename && !strcmp(name, nodename))
            {
                answer = file_fopen(node);
                break;
            }
        }
        
        node = node->next;
    }
    
    free(name);
    return  answer;
}

FILE *xml_fopen_r(XMLNODE *node, const char *path)
{
    FILE *answer = 0;
    while (node)
    {
        if (!strcmp(xml_gettag(node), "FileSystem"))
        {
            answer = directory_fopen_r(node->child, path, 0);
            if (answer)
                break;
        }
        if (node->child)
        {
            answer = xml_fopen_r(node->child, path);
            if (answer)
                break;
        }
        node = node->next;
    }
    
    return answer;
}

FILE *xml_fopen(XMLDOC *doc, const char *path, const char *mode)
{
    return xml_fopen_r(xml_getroot(doc), path);
}

int main(int argc, char **argv)
{
    XMLDOC *doc = 0;
    char error[1024];
    FILE *fp;
    int ch;
    
    doc = loadxmldoc(argv[1], error, 1024);
    if (!doc)
    {
        fprintf(stderr, "%s\n", error);
        return -1;
    }
    
    fp = xml_fopen(doc, argv[2], "r");
    if (fp)
    {
        while ( (ch = fgetc(fp)) != EOF)
            fputc(ch, stdout);
    }
    else
    {
        fprintf(stderr, "Can't open %s\n", argv[2]);
    }

    fclose(fp);
    killxmldoc(doc);
    
    return 0;
}

