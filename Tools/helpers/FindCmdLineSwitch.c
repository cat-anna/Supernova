
int FindCmdSwitch(int argc, char** argv, char *cmd, int IgnoreCase)
{
    int i = 1;
    if(IgnoreCase)
        for(; i < argc; i++)
            if(strcasecmp(argv[i], cmd) == 0)
                return i;
    else
        for(; i < argc; i++)
            if(strcmp(argv[i], cmd) == 0)
                return i;
    return 0;
}
