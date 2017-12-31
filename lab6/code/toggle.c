#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
    init_counters();

    int ret;
    acquire_lock(0);
    ret = fork();

    if(ret == 0){
        for(int i = 0; i < 30; i++){
            acquire_lock(0);
            set_var(0, get_var(0)-1);
            printf(1, "In child,  %d\n", get_var(0));
            release_lock(1);
        }
    }
    else {
        for(int i = 0; i < 30; i++){
            acquire_lock(1);
            set_var(0, get_var(0)+1);
            printf(1, "In parent, %d\n", get_var(0));
            release_lock(0);
        }
        wait();
    }
    exit();
}