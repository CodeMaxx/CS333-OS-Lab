#include "types.h"
#include "stat.h"
#include "user.h"

int main()
{
    init_counters();  // init data and locks

    int child_pids[10];
    int i;

    for (i = 0; i < 10; i++) {
      if ((child_pids[i] = fork()) < 0) {
        printf(1, "Error in fork\n");
        exit();
      } else if (child_pids[i] == 0) {
        int j;
        for(j=0; j<1000; j++)
        {
            acquire_lock(i);
            set_var(i, get_var(i)+1);
            release_lock(i);
        }
        exit();
      }
    }

    int j;
    for(j=0; j<1000; j++)
    {
        for(i = 0; i < 10; i++){
            acquire_lock(i);
            set_var(i, get_var(i)+1);
            release_lock(i);
        }
    }

    int val;
    for(i = 0; i < 10; i++){
        wait();
        acquire_lock(i);
        val = get_var(i);
        release_lock(i);
        printf(1, "%d - %d\n", i, val);
    }
    exit();
}
