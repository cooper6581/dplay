#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"

int note_from_period(int p)
{
	int imin, imid, imax;
	imin = 0;
	imax = sizeof(freq_table) / sizeof(freq_table[0]);
	
	while (imax >= imid) {
		imid = imin + ((imax - imin) >> 1);
		if (freq_table[imid] < p)
			imin = imid + 1;
		else
			imax = imid;
	}
	
	if (imax == imin && A[imin] == key)
    		return imin;
    	else
    		return -1;
}

long bpm_to_rate(int bpm)
{
	return 1000000 / ((bpm * 2) / 5);
}
