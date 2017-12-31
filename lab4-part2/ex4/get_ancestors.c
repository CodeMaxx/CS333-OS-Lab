void
itoa(int n, char** dest) {
    static char digits[] = "0123456789";
    char buf[16];
    buf[15] = ' ';

    int i = 14;
    do{
        buf[i--] = digits[n % 10];
    }while((n /= 10) != 0);

    int len = 16 - i;
    memmove(*dest, &buf[i+1], len + 1);
    *dest += len + 1;
}

int
get_ancestors(int n, char* buf)
{
    int ppid = 0;
    struct proc* parent = proc->parent;
    while(n-- > 0 && ppid != 1) {
        ppid = parent->pid;
        itoa(ppid, &buf);
        parent = parent->parent;
    }

    if(n == 0)
        return 1;
    else
        return 0;
}