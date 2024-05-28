#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "text_encoding_detect.h"

static char *mystrdup(const char *str)
{
  char *answer;

  answer = malloc(strlen(str) + 1);
  if(answer)
    strcpy(answer, str);

  return answer;
}


char *getfilename(const char *path)
{
    const char *answer;
    
    answer = strrchr(answer, '/');
    if (answer)
        answer = answer + 1;
    else
        answer = path;
    
    return mystrdup(answer);
}

/*
  load a text file into memory

*/
char *fslurp(FILE *fp)
{
  char *answer;
  char *temp;
  int buffsize = 1024;
  int i = 0;
  int ch;

  answer = malloc(1024);
  if(!answer)
    return 0;
  while( (ch = fgetc(fp)) != EOF )
  {
    if(i == buffsize-2)
    {
      if(buffsize > INT_MAX - 100 - buffsize/10)
      {
    free(answer);
        return 0;
      }
      buffsize = buffsize + 100 * buffsize/10;
      temp = realloc(answer, buffsize);
      if(temp == 0)
      {
        free(answer);
        return 0;
      }
      answer = temp;
    }
    answer[i++] = (char) ch;
  }
  answer[i++] = 0;

  temp = realloc(answer, i);
  if(temp)
    return temp;
  else
    return answer;
}


static char *mystrconcat(const char *prefix, const char *suffix)
{
    int lena, lenb;
    char *answer;
    
    lena = (int) strlen(prefix);
    lenb = (int) strlen(suffix);
    answer = malloc(lena + lenb + 1);
    if (answer)
    {
        strcpy(answer, prefix);
        strcpy(answer + lena, suffix);
    }
    return  answer;
}

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

int isDirectory(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

static char *xml_escape(const char *data)
{
    int i;
    int size = 0;
    char * answer;
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

int writetextfile(FILE *fpout, const char *fname)
{
    FILE *fp = 0;
    char *text = 0;
    char *xmltext = 0;
    int i;
    
    
    fp = fopen(fname, "r");
    text = fslurp(fp);
    fclose(fp);
    fp = 0;
    
    xmltext = xml_escape(text);
    for (i=0; xmltext[i];i++)
        fputc(xmltext[i], fpout);
    
    free(text);
    free(xmltext);
    
    return 0;
    
out_of_memory:
    free(text);
    free(xmltext);
    
    return 0;
}

void processregularfile(const char *path, int depth)
{
    int error;
    TextEncoding encoding;
    char *filename = 0;
    char *xmlfilename = 0;
    int i;
    
    encoding = DetectTextFileEncoding(path, &error);
    if (error)
        return;
    
    filename = getfilename(path);
    xmlfilename = xml_escape(filename);
    
    if (encoding == TEXTENC_ASCII || encoding || TEXTENC_UTF8_NOBOM)
    {
        for (i = 0; i <depth; i++)
            printf("\t");
        printf("<file name=\"%s\" type=\"text\">\n", xmlfilename);
        writetextfile(stdout, path);
        printf("\n");
        for (i= 0; i <depth;i++)
            printf("\t");
        printf("</file>\n");
    }

}

void processdirectory_r(const char *path, int depth)
{
    DIR *dirp;
    struct dirent *dp;
    char *pathslash;
    char *filepath;
    char *xmlfilename;
    int i;

    pathslash = mystrconcat(path, "/");


    if ((dirp = opendir(path)) == NULL) {
        perror("couldn't open '.'");
        return;
    }


    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            
          filepath = mystrconcat(pathslash, dp->d_name);
          if (isDirectory(filepath) && dp->d_name[0] != '.')
          {
              xmlfilename = xml_escape(dp->d_name);
              for (i=0;i<depth;i++)
                      printf("\t");
              printf("<directory name=\"%s\">\n", xmlfilename);
              processdirectory_r(filepath, depth +1);
              for (i=0;i<depth;i++)
                      printf("\t");
              printf("</directory>\n");
              free(xmlfilename);
              xmlfilename = 0;
          }
            else if (is_regular_file(filepath))
                processregularfile(filepath, depth);
          free(filepath);

        }
    } while (dp != NULL);


    if (errno != 0)
        perror("error reading directory");

    closedir(dirp);
    return;
}

int directorytoxml(const char *directory)
{
    char *filename= 0 ;
    char *xmlfilename = 0;
    
    if (!isDirectory(directory))
    {
        fprintf(stderr, "Can't open directory %s\n", directory);
        return -1;
    }
    
    filename = getfilename(directory);
    xmlfilename = xml_escape(filename);
    
    printf("<FileSystem>\n");
    printf("\t<directory name=\"%s\">\n", xmlfilename);
    processdirectory_r(directory, 2);
    printf("\t</directory>\n");
    printf("</FileSystem>\n");
    
    return 0;
}


int main(int argc, char **argv)
{
    if (argc == 1)
        directorytoxml(".");
    else if (argc == 2)
        directorytoxml(argv[1]);
    
    return 0;
}
