#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

char current_working_directory_array[PATH_MAX];
char *current_working_directory = current_working_directory_array;

char *get_working_directory(char *for_whom)
{
    if (current_working_directory[0] == '\0')
        if (getcwd(current_working_directory, PATH_MAX) == NULL)
            printf("%s: couldn't get the current working directory");

    return current_working_directory;
}

void set_working_directory(char *working_directory)
{
    strcpy(current_working_directory, working_directory);
}

char *make_absolute(char *main, char *sub)
{
    if (sub[0] == '/')
        return sub;

    char *buff = malloc(strlen(main) + strlen(sub));
    sprintf(buff, "%s/%s", main, sub);
    return buff;
}

void STDOUT()
{
    asm volatile("int $0x80" ::"a"(254));
}

void getCMD(char *cmdHolder)
{
    int i = 0;
    char c;
    while ((c = getchar()) != '\n')
    {
        if (c != EOF && i >= 0)
        {
            // backspace
            if (c == 0x8)
                i--;
            else
                cmdHolder[i++] = c;

            putchar(c);
            STDOUT();
        }
    }
    putchar(c);
    STDOUT();
    cmdHolder[i] = '\0';
}

static int
changeDirectory(char *newDir)
{
    if (current_working_directory == NULL)
        current_working_directory = get_working_directory("chdir");
    char *absolute = make_absolute(current_working_directory, newDir);

    // Try to change directory
    int r = chdir(absolute);
    if (r >= 0)
    {
        // change was successful
        set_working_directory(absolute);
        free(absolute);
        printf("Changed directory to: %s\n", current_working_directory);
    }
    else
    {
        printf("Failed to change directory\n");
    }
}

int main(int argc, char **argv)
{

    current_working_directory = get_working_directory("shell");
    while (1)
    {
        char cwd[PATH_MAX];
        printf("> ");
        STDOUT();

        // Get the command and print each character entered
        char cmd[256];
        getCMD(cmd);

        char *token = strtok(cmd, " ");
        if (token != NULL)
        {
            if (!strcmp(token, "cd"))
            {
                token = strtok(NULL, " ");
                if (token == NULL)
                {
                    printf("Error: Missing argument\n");
                }
                else
                {
                    // cd command
                    changeDirectory(token);
                }
            }
            else if (!strcmp(token, "pwd"))
            {
                printf("Current directory is: %s\n", current_working_directory);
            }
            else if (!strcmp(token, "ls"))
            {
                DIR *dh = opendir(current_working_directory);
                if (!dh)
                {
                    printf("Couldn't open directory %s", current_working_directory);
                }

                struct dirent *d;
                while ((d = readdir(dh)) != NULL)
                {
                    // If hidden files are found we continue
                    printf("%s \n", d->d_name);
                }
            }
            else
            {
                printf("Unknown command: %s\n", token);
            }
            STDOUT();
        }
    }
}