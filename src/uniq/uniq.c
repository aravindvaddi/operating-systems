#include "types.h"
#include "user.h"

#define BUFFER_SIZE 1536

/* function prototypes */
void clear_arr(char *a, int size);
int getstr(int fd, char *buf, int max);
char toUpper(char lc);
int stricmp(const char *p, const char *q);
void error(char* str);
void uniq_c(int fd, int i_switch);
void uniq_d(int fd, int i_switch);
void uniq_n(int fd, int i_switch);
void uniq(int fd, int i_switch, int c_switch, int d_switch);

void uniq(int fd, int i_switch, int c_switch, int d_switch)
{
    if(c_switch)
        uniq_c(fd, i_switch);
    if(d_switch)
        uniq_d(fd, i_switch);
    if(!c_switch && !d_switch)
        uniq_n(fd, i_switch);
}

void uniq_c(int fd, int i_switch)
{
    char mainstring[BUFFER_SIZE], backup[BUFFER_SIZE];
    int count = 0, flag = 0;
    int (*strcmp_f[])(const char *,const char*) = {&strcmp, &stricmp};

    clear_arr(backup, BUFFER_SIZE);
    clear_arr(mainstring, BUFFER_SIZE);
    if(getstr(fd, mainstring, BUFFER_SIZE) <= 0)
        return;
    strcpy(backup, mainstring);
    while(1)
    {
        while((*strcmp_f[i_switch])(mainstring, backup) == 0)
        {
            clear_arr(mainstring, BUFFER_SIZE);
            count++;
            if(getstr(fd, mainstring, BUFFER_SIZE) <= 0)
            {
                flag = 1;
                break;
            }
        }
        if(flag)
        {
            printf(1, "%d %s", count, backup);
            return;
        }
        printf(1, "%d %s", count, backup);
        count = 0;
        strcpy(backup, mainstring);
    }
}

void uniq_d(int fd, int i_switch)
{
    char mainstring[BUFFER_SIZE], backup[BUFFER_SIZE];
    int flag = 0;
    int (*strcmp_f[])(const char *,const char*) = {&strcmp, &stricmp};

    clear_arr(backup, BUFFER_SIZE);
    while(1)
    {
        clear_arr(mainstring, BUFFER_SIZE);
        if(getstr(fd, mainstring, BUFFER_SIZE) <= 0)
            return;
        if((*strcmp_f[i_switch])(mainstring, backup) != 0)
            strcpy(backup, mainstring);
        else
        {
            while((*strcmp_f[i_switch])(mainstring, backup) == 0)
            {
                clear_arr(mainstring, BUFFER_SIZE);
                if(getstr(fd, mainstring, BUFFER_SIZE) <= 0)
                {
                    flag = 1;
                    break;
                }
            }
            if(flag)
            {
                printf(1, "%s", backup);
                return;
            }
            printf(1, "%s", backup);
            strcpy(backup, mainstring);
        }
    }
}

void uniq_n(int fd, int i_switch)
{
    char mainstring[BUFFER_SIZE], backup[BUFFER_SIZE];
    int (*strcmp_f[])(const char *,const char*) = {&strcmp, &stricmp};

    clear_arr(backup, BUFFER_SIZE);
    while(1)
    {
        clear_arr(mainstring, BUFFER_SIZE);
        if(getstr(fd, mainstring, BUFFER_SIZE) <= 0)
            return;
        while((*strcmp_f[i_switch])(mainstring, backup) == 0)
        {
            clear_arr(mainstring, BUFFER_SIZE);
            if(getstr(fd, mainstring, BUFFER_SIZE) <= 0)
                return;
        }
        printf(1, "%s", mainstring);
        strcpy(backup, mainstring);
    }
}

int getstr(int fd, char *buf, int max)
{
    int i, cc;
    char c;

    for(i = 0; i + 1 < max; )
    {
        cc = read(fd, &c, 1);
        if(cc < 1)
        return -1;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
         break;
    }

    buf[i] = '\0';
    return strlen(buf);
}

int stricmp(const char *p, const char *q)
{
    while(*p && toUpper(*p) == toUpper(*q))
        p++, q++;
    return (uchar)*p - (uchar)*q;
}

char toUpper(char lc)
{
    if(lc >= 'a' && lc <= 'z')
        return lc - ('a' - 'A');
    else
        return lc;
}

void clear_arr(char *a, int size)
{
    int i;

    for(i = 0; i < size; i++)
        a[i] = '\0';
}

void error(char* str)
{
    printf(1, "Error: %s. Terminating\n", str);
    exit();
}

int main (int argc, char *argv[])
{
    int fd, j, ckfl;
    int i = 0, c = 0, d = 0;

    for(j = 1; j < argc; j++)
    {
        if(argv[j][0] == '-')
        {
            switch(argv[j][1])
            {
                case 'i':
                    i = 1;
                    break;
                case 'd':
                    d = 1;
                    break;
                case 'c':
                    c = 1;
                    break;
                default:
                    error("Unexpected option, this ain't legal, please try again");
            }
        }
    }

    for(j = 1, ckfl = 0; j < argc; j++)
    {
        if(argv[j][0] != '-')
        {
            ckfl = 1;
            break;
        }
    }

    if(!ckfl)
    {
        uniq(0, i, c, d);
        exit();
    }

    for(j = 1; j < argc; j++)
    {
        if(argv[j][0] == '-')
            continue;
        else if((fd = open(argv[j], 0)) < 0)
        {
            printf(1, "uniq: cannot open file %s\n", argv[j]);
            exit();
        }
        if(c && d)
            error("both c and d cannot be used simultaneously. sorry, this ain't legal.");
      printf(1,"Filename: %s\n", argv[j]);
        uniq(fd, i, c, d);
        close(fd);
    }
    exit();
}
