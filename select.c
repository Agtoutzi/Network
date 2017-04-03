

/*  YLOPOIISI TOU XEIRISTI TIS KLISIS SYSTIMATOS do_select  */



#include "pm.h"
#include <sys/wait.h>
#include <minix/callnr.h>
#include <minix/com.h>
#include <sys/resource.h>
#include <signal.h>
#include "mproc.h"
#include "param.h"




PUBLIC int do_select(){




	if(mp->mp_effuid != 0){


		if(m_in.m1_i1 == 1){
		
			selection = 1;
		}else if(m_in.m1_i1 == 2){
		
			selection = 2;
		}else if(m_in.m1_i1 == 3){
		
			selection = 3;
		}else if(m_in.m1_i1 == 4){
		
			selection = 4;
		
		}else{
			return(EINVAL);
		}
	}else{
		return(EACCES);
	}


/* epistrefetai i katallili timi apo tin do_select */

	return(selection);
}
