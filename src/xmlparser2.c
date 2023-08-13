/*
 XML parser 2, by Malcolm McLean
 Vanilla XML parser
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>


typedef struct xmlattribute
{
  char *name;                /* attriibute name */
  char *value;               /* attribute value (without quotes) */
  struct xmlattribute *next; /* next pointer in linked list */
} XMLATTRIBUTE;

typedef struct xmlnode
{
  char *tag;                 /* tag to identify data type */
  XMLATTRIBUTE *attributes;  /* attributes */
  char *data;                /* data as ascii */
  int position;              /* position of the node within parent's data string */
  struct xmlnode *next;      /* sibling node */
  struct xmlnode *child;     /* first child node */
} XMLNODE;

typedef struct
{
  XMLNODE *root;             /* the root node */
} XMLDOC;

struct strbuff
{
    const char *str;
    int pos;
};

typedef struct
{
  char *str;
  int capacity;
  int N;
} STRING;

typedef struct
{
  int set;
  char message[1024];
    struct lexer *lexer;
} ERROR;

typedef struct lexer
{
  int (*getch)(void *ptr);
  void *ptr;
  int token;
  int lineno;
  int columnno;
  int badmatch;
  ERROR *err;
} LEXER;

static int fileaccess(void *ptr);
static int stringaccess(void *ptr);

void killxmlnode(XMLNODE *node);
static void killxmlattribute(XMLATTRIBUTE *attr);

static int isinitidentifier(int ch);
static int iselementnamech(int ch);
static int isattributenamech(int ch);

static int string_init(STRING *s);
static void string_push(STRING *s, int ch, ERROR *err);
static void string_concat(STRING *s, const char *str, ERROR *err);
static char *string_release(STRING *s);

static XMLDOC *xmldocument(LEXER *lex, ERROR *err);
static XMLNODE *xmlnode(LEXER *lex, ERROR *err);
static XMLNODE *comment(LEXER *lex, ERROR *err);
static XMLATTRIBUTE *attributelist(LEXER *lex, ERROR *err);
static XMLATTRIBUTE *xmlattribute(LEXER *lex, ERROR *err);
static char *quotedstring(LEXER *lex, ERROR *err);
static char *textspan(LEXER *lex, ERROR *err);
static char *attributename(LEXER *lex, ERROR *err);
static char *elementname(LEXER *lex, ERROR *err);
static int escapechar(LEXER *lex, ERROR *err);
static void skipbom(LEXER *lex, ERROR *err);
static void skipunknowntag(LEXER *lex, ERROR *err);
static void skipwhitespace(LEXER *lex, ERROR *err);

static void reporterror(ERROR *err, const char *fmt, ...);

static void initlexer(LEXER *lex, ERROR *err, int (*getch)(void *), void *ptr);
static int gettoken(LEXER *lex);
static int match(LEXER *lex, int token);




XMLDOC *loadxmldoc2(const char *filename,char *errormessage, int Nerr)
{
   FILE *fp;
   ERROR error;
   LEXER lexer;
   XMLDOC *answer = 0;

   error.set = 0;
   error.message[0] = 0;
    error.lexer = 0;

   if (errormessage && Nerr > 0)
      errormessage[0] = 0;

   fp = fopen(filename, "r");
   if (!fp)
   {
      snprintf(errormessage, Nerr, "Can't open %s", filename);
      return 0;
   }
   else
   {
      initlexer(&lexer, &error, fileaccess, fp);
      answer = xmldocument(&lexer, &error);
      if (error.set)
      {
         snprintf(errormessage, Nerr, "%s", error.message);
      }
      return answer;
   }   
}

XMLDOC *floadxmldoc2(FILE *fp, char *errormessage, int Nerr)
{
    ERROR error;
    LEXER lexer;
    XMLDOC *answer = 0;

    error.set = 0;
    error.message[0] = 0;
     error.lexer = 0;

    if (errormessage && Nerr > 0)
       errormessage[0] = 0;

    initlexer(&lexer, &error, fileaccess, fp);
    answer = xmldocument(&lexer, &error);
    if (error.set)
    {
       snprintf(errormessage, Nerr, "%s", error.message);
    }
       
    return answer;
}

static int fileaccess(void *ptr)
{
   FILE *fp = ptr;
   return fgetc(fp);
}

XMLDOC *xmldoc2fromstring(const char *str,char *errormessage, int Nout)
{
   FILE *fp;
   ERROR error;
   LEXER lexer;
   XMLDOC *answer = 0;
   struct strbuff strbuf;

   
   error.set = 0;
   error.message[0] = 0;
    error.lexer = 0;

   if (errormessage && Nout > 0)
      errormessage[0] = 0;

    strbuf.str = str;
    strbuf.pos = 0;
    initlexer(&lexer, &error, stringaccess, &strbuf);
    answer = xmldocument(&lexer, &error);
    if (error.set)
    {
         snprintf(errormessage, Nout, "%s", error.message);
    }
    return answer;
}

static int stringaccess(void *ptr)
{
    struct strbuff *s = ptr;
    if (s->str[s->pos])
        return s->str[s->pos++];
    else
        return EOF;
}


/*
  document destructor
*/
void killxmldoc(XMLDOC *doc)
{
  if(doc)
  {
      killxmlnode(doc->root);
      free(doc);
  }
}

/*
  get the root node of the document
*/
XMLNODE *xml_getroot(XMLDOC *doc)
{
  return doc->root;
}

/*
  get a node's tag
*/
const char *xml_gettag(XMLNODE *node)
{
    return node->tag;
}

/*
  get a node's data
*/
const char *xml_getdata(XMLNODE *node)
{
    return node->data;
}

/*
  get a node's attributes
*/
const char *xml_getattribute(XMLNODE *node, const char *attr)
{
  XMLATTRIBUTE *next;

  for(next = node->attributes; next; next = next->next)
    if(!strcmp(next->name, attr))
        return next->value;

  return 0;
}

/*
  get the number of direct children of the node
*/
int xml_Nchildren(XMLNODE *node)
{
  XMLNODE *next;
  int answer = 0;

  if(node->child)
  {
    next = node->child;
    while(next)
    {
      answer++;
      next = next->next;
    }
  }

  return answer;
}

/*
  get the number of direct children with a particular tag
  Params: node - the node
          tag - the tag (NULL for all children)
  Returns: numer of children with that tag
*/
int xml_Nchildrenwithtag(XMLNODE *node, const char *tag)
{
  XMLNODE *next;
  int answer = 0;

  if(node->child)
  {
    next = node->child;
    while(next)
    {
      if(tag == 0 || (next->tag && !strcmp(next->tag, tag)))
        answer++;
      next = next->next;
    }
  }

  return answer;
}

/*
  get child with tag and index
  Params: node - the node
          tag - tag of child (NULL for all children)
          index - index number of child to retrieve
  Returns: child, or null on fail
  Notes: slow, only call for nodes with relatively small
    numbers of children. If the child list is very long,
    step through the linked list manually.
*/
XMLNODE *xml_getchild(XMLNODE *node, const char *tag, int index)
{
  XMLNODE *next;
  int count = 0;

  if(node->child)
  {
    next = node->child;
    while(next)
    {
      if(tag == 0 || (next->tag && !strcmp(next->tag, tag)))
      {
        if(count == index)
          return next;
        count++;
      }
      next = next->next;
    }
  }

  return 0;
}

/*
  recursive get descendants
  Params; node the the node
          tag - tag to retrieve
          list = pointer to return list of pointers to matchign nodes
          N - return for number of nodes found, also index of current place to write
  Returns: 0 on success -1 on out of memory
  Notes:
    we are descending the tree, allocating space for a pointer for every
    matching node.

*/
static int getdescendants_r(XMLNODE *node, const char *tag,  XMLNODE ***list, int *N)
{
  XMLNODE **temp;
  XMLNODE *next;
  int err;

  next = node;
  while(next)
  {
    if(tag == 0 || (next->tag && !strcmp(next->tag, tag)))
    {
      temp = realloc(*list, (*N +1) * sizeof(XMLNODE *));
      if(!temp)
        return -1;
      *list = temp;
      (*list)[*N] = next;
      (*N)++;
    }
    if(next->child)
    {
      err = getdescendants_r(next->child, tag, list, N);
      if(err)
        return err;
    }
    next = next->next;
  }

  return 0;
}

/*
 get all descendants that match a particular tag
   Params: node - the root node
           tag - the tag
           N - return for number found
   Returns: 0 on success, -1 on out of memory
   Notes: useful for fishing. Save you are reading the crossword
     tag, but you don't know exactly whther it is root or
     some child element. You also don't know if several of them are
     in the file. Just call to extract a list, then query for
     children so you know that the tag is an actual match.
     Don't call for huge lists as inefficient.
*/
XMLNODE **xml_getdescendants(XMLNODE *node, const char *tag, int *N)
{
  XMLNODE **answer = 0;
  int err;

  *N = 0;
  err = getdescendants_r(node, tag, &answer, N);
  if(err)
  {
    free(answer);
    return 0;
  }

  return answer;
}

static void getnestedata_r(XMLNODE *node, STRING *str, ERROR *err)
{
    XMLNODE *child;
    int i = 0;
    
    for (child = node->child; child; child = child->next)
    {
        while (node->data && i < child->position && node->data[i])
            string_push(str, node->data[i++], err);
        getnestedata_r(child, str, err);
    }
    while (node->data && node->data[i])
        string_push(str, node->data[i++], err);
    
}

char *xml_getnesteddata(XMLNODE *node)
{
    ERROR error;
    STRING str;
    error.set = 0;
    error.lexer = 0;
    string_init(&str);
    getnestedata_r(node, &str, &error);
    return string_release(&str);
}

/*
  xml node destructor
  Notes: destroy siblings in a list, chilren recursively
    as children are unlikely to be nested very deep

*/
void killxmlnode(XMLNODE *node)
{
  XMLNODE *next;

  if(node)
  {
    while(node)
    {
      next = node->next;
      if(node->child)
        killxmlnode(node->child);
      killxmlattribute(node->attributes);
      free(node->data);
      free(node->tag);
      free(node);
      node = next;
    }
  }
}

/*
  destroy the attributes list
*/
static void killxmlattribute(XMLATTRIBUTE *attr)
{
  XMLATTRIBUTE *next;
  if(attr)
  {
    while(attr)
    {
       next = attr->next;
       free(attr->name);
       free(attr->value);
       free(attr);
       attr = next;
    }
  }
}

static int isinitidentifier(int ch)
{
   if (isalpha(ch) || ch == '_')
     return 1;
   return 0;
}

static int iselementnamech(int ch)
{
   if (isalpha(ch) || isdigit(ch) || ch == '_' || ch == '-' || ch == '.')
     return 1;
   return 0;
}

static int isattributenamech(int ch)
{
   if (isalpha(ch) || isdigit(ch) || ch == '_' || ch == '-' || ch == '.' || ch == ':')
     return 1;
   return 0;
}

static void trim(char *str)
{
    int i;
    
    for (i = 0; str[i]; i++)
        if (!isspace((unsigned char) str[i]))
            break;
    if (i != 0)
        memmove(str, &str[i], strlen(str) -i + 1);
    if (str[0])
    {
        i = strlen(str) -1;
        while (i >= 0 && isspace((unsigned char)str[i]))
           str[i--] = 0;
    }
        
}

static int string_init(STRING *s)
{
  s->str = 0;
  s->capacity = 0;
  s->N = 0;
    
  return 0;
}

static void string_push(STRING *s, int ch, ERROR *err)
{
    char *temp = 0;
    
   if (s->capacity < s->N * 2 + 2)
   {
     temp = realloc(s->str, s->N * 2 + 2);
     if (!temp)
       goto out_of_memory;
     s->str = temp;
     s->capacity = s->N * 2 + 2; 
   }
   s->str[s->N++] = ch;
   s->str[s->N] = 0;
   return;
out_of_memory:
   reporterror(err, "out of memory");

}

static void string_concat(STRING *s, const char *str, ERROR *err)
{
    int i;
    
    for (i =0; str[i]; i++)
        string_push(s, str[i], err);
}

static char *string_release(STRING *s)
{
   char *answer;

   if (s->str == 0)
     return 0;
   answer = realloc(s->str, s->N + 1);
   s->str = 0;
   s->N = 0;
   s->capacity = 0;

   return answer;
}

static XMLDOC *xmldocument(LEXER *lex, ERROR *err)
{
    XMLNODE *node;
    XMLDOC *doc;
    int ch;
    
    doc = malloc(sizeof(XMLDOC));
    if (!doc)
    {
        reporterror(err, "out of memory");
        return 0;
    }
    
    skipbom(lex, err);

    do {
        skipwhitespace(lex, err);
        if (!match(lex, '<'))
            reporterror(err, "can't find opening tag");
        ch = gettoken(lex);
        if (isinitidentifier(ch))
        {
            node = xmlnode(lex, err);
            if (node)
            {
                if (!err->set)
                {
                    doc->root = node;
                    return doc;
                }
                else
                {
                    killxmlnode(node);
                    break;
                }
            }
            else
            {
                reporterror(err, "bad root node");
            }
        }
        else {
            skipunknowntag(lex, err);
        }
    } while (ch != EOF);
    
    free(doc);
    return 0;
}

static XMLNODE *xmlnode(LEXER *lex, ERROR *err)
{
    int ch;
    char *tag = 0;
    XMLATTRIBUTE *attributes = 0;
    XMLNODE *node = 0;
    XMLNODE *lastchild = 0;
    STRING datastr;
    
    if (err->set)
        return 0;
    
    string_init(&datastr);
    
    tag = elementname(lex, err);
    if (!tag)
        goto parse_error;
    attributes = attributelist(lex, err);
    skipwhitespace(lex, err);
    ch = gettoken(lex);
    if (ch == '/')
    {
        match(lex, '/');
        if (!match(lex, '>'))
            goto parse_error;
        node = malloc(sizeof(XMLNODE));
        if (!node)
            goto out_of_memory;
        node->tag = tag;
        node->attributes = attributes;
        node->data = 0;
        node->position = 0;
        node->child = 0;
        node->next = 0;
        return node;
    }
    else if (ch == '>')
    {
        match(lex, '>');
        node = malloc(sizeof(XMLNODE));
        if (!node)
            goto out_of_memory;
        node->tag = tag;
        node->attributes = attributes;
        node->data = 0;
        node->position = 0;
        node->child = 0;
        node->next = 0;
        tag = 0;
        attributes = 0;
        
        do {
            char *text = textspan(lex, err);
            if (text)
            {
                string_concat(&datastr, text, err);
                free(text);
                text = 0;
            }
            ch = gettoken(lex);
            if (ch == '<')
            {
                match(lex, '<');
                ch = gettoken(lex);
                if (isinitidentifier(ch))
                {
                    XMLNODE *child = xmlnode(lex, err);
                    if (!child)
                        goto parse_error;
                    if (lastchild)
                        lastchild->next = child;
                    else
                        node->child = child;
                    lastchild = child;
                    child->position = datastr.N;
                }
                else if(ch == '/')
                {
                    match(lex, '/');
                    tag = elementname(lex,err);
                    if (tag && !strcmp(tag, node->tag))
                    {
                        node->data = string_release(&datastr);
                        free(tag);
                        match(lex, '>');
                        return node;
                    }
                    else
                    {
                        reporterror(err, "bad closing tag %s", tag);
                        goto parse_error;
                    }
                }
                else if (ch == '!'){
                    comment(lex, err);
                }
                    
            }else{
                goto parse_error;
            }
        } while (ch != EOF);
    }
    else
    {
        goto parse_error;
    }
parse_error:
    reporterror(err, "error parsing element");
    return 0;
out_of_memory:
    reporterror(err, "out of memory");
    return 0;
}

static XMLNODE *comment(LEXER *lex, ERROR *err)
{
    STRING str;
    char *text;
    
    string_init(&str);
    char buff[4] = {0};
    int ch;
    if (!match(lex, '!'))
        goto parse_error;
    if (!match(lex, '-'))
        goto parse_error;
    if (!match(lex, '-'))
        goto parse_error;
    
    while ((ch = gettoken(lex)) != EOF)
    {
        string_push(&str, ch, err);
        match(lex, ch);
        memmove(buff, buff+1, 3);
        buff[2] = ch;
        if (!strcmp(buff, "-->"))
            return 0;
    }
parse_error:
    
    reporterror(err, "bad comment");
    return 0;
          
}

static XMLATTRIBUTE *attributelist(LEXER *lex, ERROR *err)
{
    int ch;
    XMLATTRIBUTE *answer = 0;
    XMLATTRIBUTE *last = 0;
    XMLATTRIBUTE *attr = 0;
    
    while(1)
    {
        skipwhitespace(lex, err);
        ch = gettoken(lex);
        if (isinitidentifier(ch))
        {
            attr = xmlattribute(lex, err);
            if (!attr)
                goto parse_error;
            if (last)
            {
                last->next = attr;
                last = attr;
            }
            else
            {
                answer = attr;
                last = answer;
            }
        }
        else
        {
            break;
        }
    }
    
    return answer;
parse_error:
    killxmlattribute(answer);
    
    return 0;
}

static XMLATTRIBUTE *xmlattribute(LEXER *lex, ERROR *err)
{
    char *name = 0;
    char *value = 0;
    XMLATTRIBUTE *answer = 0;
    
    name = attributename(lex, err);
    if (!name)
        goto parse_error;
    skipwhitespace(lex, err);
    if (!match(lex, '='))
        goto parse_error;
    skipwhitespace(lex, err);
    value = quotedstring(lex, err);
    if (!value)
        goto parse_error;
    
    answer = malloc(sizeof(XMLATTRIBUTE));
    if (!answer)
        goto out_of_memory;
    answer->name = name;
    answer->value = value;
    answer->next = 0;
    return answer;
    
parse_error:
    reporterror(err, "error in attribute");
    free(name);
    free(value);
    free(answer);
    return 0;
    
out_of_memory:
    reporterror(err, "out of memory");
    free(name);
    free(value);
    free(answer);
    return 0;
}



static char *quotedstring(LEXER *lex, ERROR *err)
{
    int quotech;
    STRING str;
    int ch;
    
    string_init(&str);
    
    quotech = gettoken(lex);
    if (quotech != '\"' && quotech != '\'')
    {
        goto parse_error;
    }
    match(lex, quotech);
    while ((ch = gettoken(lex)) != EOF)
    {
        if (ch == quotech)
            break;
        else if (ch == '&')
        {
            ch = escapechar(lex, err);
            string_push(&str, ch, err);
        }
        else if (ch == '\n')
            goto parse_error;
        else
        {
            string_push(&str,ch, err);
            match(lex, ch);
        }
    }
    if (!match(lex, quotech))
        goto parse_error;
    return string_release(&str);
parse_error:
    free(string_release(&str));
    reporterror(err, "bad quoted string");
    return 0;
    
}

static char *textspan(LEXER *lex, ERROR *err)
{
   int ch;
   STRING str;

   string_init(&str);

   while ( (ch = gettoken(lex)) != EOF)
   {
      if (ch == '<')
          break;
      if (ch == '&')
        ch = escapechar(lex, err);
      else
         match(lex, ch);
      string_push(&str, ch, err);
   } 
   
    return string_release(&str);
}

static char *attributename(LEXER *lex, ERROR *err)
{
    int ch;
    STRING str;

    string_init(&str);
    
    ch = gettoken(lex);
    if (!isinitidentifier(ch))
      goto parse_error;
    while (isattributenamech(ch))
    {
       match(lex, ch);
       string_push(&str, ch, err);
       ch = gettoken(lex);
    }
    return string_release(&str);
 parse_error:
    free(string_release(&str));
    return 0;
}

static char *elementname(LEXER *lex, ERROR *err)
{
   int ch;
   STRING str;
    char *temp = 0;

   string_init(&str);
   
   ch = gettoken(lex);
   if (!isinitidentifier(ch))
     goto parse_error;
   while (iselementnamech(ch))
   {
      match(lex, ch);
      string_push(&str, ch, err);
      ch = gettoken(lex);
   }
   return string_release(&str);
parse_error:
    temp = string_release(&str);
    free(temp);
   return 0;
}

static int escapechar(LEXER *lex, ERROR *err)
{
    int ch;
    STRING str;
    char *escaped = 0;
    int answer = 0;
    
    string_init(&str);
    
    ch = gettoken(lex);
    if (!match(lex, '&'))
        goto parse_error;
    string_push(&str, ch, err);
    while ( (ch = gettoken(lex)) != EOF)
    {
        string_push(&str, ch, err);
        match(lex, ch);
        if (ch == ';')
            break;
        if (ch == '\n')
            goto parse_error;
    }
    escaped = string_release(&str);
    if (!strcmp(escaped, "&amp;"))
        answer = '&';
    else if (!strcmp(escaped, "&gt;"))
        answer = '>';
    else if (!strcmp(escaped, "&lt;"))
        answer = '<';
    else if (!strcmp(escaped, "&quot;"))
        answer = '\"';
    else if (!strcmp(escaped, "&apos;"))
        answer = '\'';
    if (answer == 0)
        reporterror(err, "Unrecognised escape sequence %s", escaped);
    
    free(escaped);
    return answer;
parse_error:
    free(escaped);
    return 0;
}

static void skipbom(LEXER *lex, ERROR *err)
{
    int ch;
    ch = gettoken(lex);
    if (ch == 0xEF)
    {
        match(lex, ch);
        if (!match(lex, 0xBB))
            reporterror(err, "input has bad byte order marker");
        if (!match(lex, 0xBF))
            reporterror(err, "input has bad byte order marker");
    }
}
               
static void skipunknowntag(LEXER *lex, ERROR *err)
{
    int ch;
    
    while ((ch = gettoken(lex)) != EOF)
    {
        match(lex, ch);
        if (ch == '>')
            break;
    }
}

static void skipwhitespace(LEXER *lex, ERROR *err)
{
   int ch = gettoken(lex);
   while (isspace(ch))
   {
     match(lex, ch);
     ch = gettoken(lex);
   }
}
           
static void reporterror(ERROR *err, const char *fmt, ...)
{
    char buff[1024];
    va_list args;
    
    if (!err->set)
    {
        va_start(args, fmt);
        vsnprintf(buff, 1024, fmt, args);
        va_end(args);
        if (err->lexer)
            snprintf(err->message, 1024, "Error line %d: %s.", err->lexer->lineno, buff);
        else
            strcpy(err->message, buff);
        err->set = 1;
    }
}

static void initlexer(LEXER *lex, ERROR *err, int (*getch)(void *), void *ptr)
{
  lex->getch = getch;
  lex->ptr = ptr;
  lex->err = err;
  lex->lineno = 0;
  lex->columnno = 0;
  lex->badmatch = 0;
  err->lexer = lex;
        
  lex->token = (*lex->getch)(lex->ptr);
  if (lex->token != EOF)
  {
    lex->lineno = 1;
    lex->columnno = 1; 
  }
  else
  {
     reporterror(lex->err, "Can't read data\n");
  }

}

static int gettoken(LEXER *lex)
{
    if (lex->badmatch)
        return EOF;
   return lex->token;
}

static int match(LEXER *lex, int token)
{
   if (lex->token == token)
   {
       if (lex->token == '\n')
       {
          lex->lineno++;
          lex->columnno = 1;
       }
      else
      {
        lex->columnno++;
      }
       lex->token = (*lex->getch)(lex->ptr);
      return 1;
   }
   else
   {
       lex->badmatch = 1;
     return 0;
   }
}

static void printnode_r(XMLNODE *node, int depth)
{
    int i;
    XMLNODE *child;
    
    for (i =0; i < depth; i++)
        printf("\t");
    printf("<%s>\n", node->tag);
    for (child = node->child; child; child = child->next)
        printnode_r(child, depth +1);
    for (i =0; i < depth; i++)
        printf("\t");
    printf("</%s>\n", node->tag);
}

static void printdocument(XMLDOC *doc)
{
    if (!doc)
        printf("Document was null\n");
    else if(!doc->root)
        printf("root null\n");
    else
        printnode_r(doc->root, 0);
}

int xmlparser2main(int argc, char **argv)
{
    XMLDOC *doc;
    
    char error[1024];
    if (argc == 1)
    {
        doc = xmldoc2fromstring("<!-- --><FRED attr=\"Fred\">Fred<JIM/>Bert<JIM/>Harry</FRED>", error, 1204);
    }else
    {
        doc = loadxmldoc2(argv[1], error, 1024);
        printf("%s\n", xml_getnesteddata(doc->root));
    }
    if (error[0])
        printf("%s\n", error);
    else
        printdocument(doc);
    
    return 0;
}

