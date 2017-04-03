

/*  YLOPOIISI TOU XEIRISTI TIS KLISIS SYSTIMATOS do_sema  */



#include "pm.h"
#include <sys/wait.h>
#include <minix/callnr.h>
#include <minix/com.h>
#include <sys/resource.h>
#include <signal.h>
#include "mproc.h"
#include "param.h"


#define N 32		/* default timi gia to megethos tou pinaka simaforwn */



/* struct - simaforos (periexei ola ta stoixeia enos simaforou) */

struct semaphore{

	int counter;	/* metritis simaforou */
	int usrid;	/* id xristi-idioktiti simaforou */
	int head;	/* arxi listas simaforou */
	int tail;	/* telos listas simaforou */
};


struct semaphore sema[N];	/* pinakas simaforwn */




/* elegxei tin orthi leitourgeia se kapoio simaforo (epistrefei error an yparxei sfalma, alliws epistrefei miden) */ 


int check(int checkpos){

	if( (checkpos >= N)||(checkpos < 0) ){		/* elegxos an prospathoume na prospelasoume epitrepti thesi, dld thesi tou pinaka simaforwn */
		return(EINVAL);

	}else if( ( mp->mp_effuid != 0 ) && ( mp->mp_effuid != sema[checkpos].usrid) && ( sema[checkpos].usrid != -1 ) ){	/* elegxos an exoume prosvasi ston simaforo */
		return(EACCES);

	}else if( sema[checkpos].usrid == -1 ){		/* elegxos an exei arxikopoiithei o simaforos */
		return(EPERM);
	}

	return(0);
}



/* arxikopoiei tin fifo enos simaforou (epistrefei OK) */


int initfifo(int pos){
	sema[pos].head = -1;
	sema[pos].tail = -1;
	return(OK);
}



/* prosthetei ena stoixeio sto telos tis fifo enos simaforou (epistrefei OK) */


int addfifo(int pos){
	
	if(sema[pos].head == -1){		/* elegxos an i fifo einai adeia i gemati */

		sema[pos].head = who_p;
		sema[pos].tail = who_p;
		return(OK);
	}else{

		mproc[sema[pos].tail].nextproc = who_p;
		sema[pos].tail = who_p;
		return(OK);
	}
}



/* afairei to prwto stoixeio apo mia fifo enos simaforou. (epistrefei OK) */


int rmvfifo(int pos){

	if(sema[pos].head == sema[pos].tail){		/* elegxos an yparxei kai allo stoixeio sti fifo */
		
		sema[pos].head = sema[pos].tail = -1;
		return(OK);
	}else{

		sema[pos].head = mproc[sema[pos].head].nextproc;
		return(OK);
	}
}




/* arxikopoiei ena simaforo (epistrefei ti thesi tou simaforou pou arxikopoiithike, i error an yparxei sfalma) */


PUBLIC int init(int count){

	int i=0;

	if(count <= 0){		/* elegxos an o metritis pou dinetai einai epitreptos */
		return(EINVAL);
	}
	

	while((i<N) && (sema[i].usrid != -1)){		/* euresi thesis stin opoia tha arxikopoiithei o simaforos */
		i++;
	}

	if(i==32){				/* elegxos an dn yparxei thesi gia tin arxikopoiisi */
		return(ENOMEM);
	}
	
	sema[i].counter = count;
	sema[i].usrid = mproc[who_p].mp_effuid;
	initfifo(i);
	return(i);
}



/* katastrefei ena simaforo (epistrefei 0 gia epityxia, kai error gia sfalma) */


PUBLIC int destroy(int sem){

	int error;

	error = check(sem);

	if(error == 0){		/* an den yparxei sfalma, proxwrame stin katastrofi */

		sema[sem].usrid = -1;

		return(0);
	}
	return(error);
}



/* kanei up mia diergasia se ena simaforo (epistrefei 0 an egine to up kanonika, kai error an yparxei sfalma) */


PUBLIC int up(int sem){

	int error;

	error = check(sem);

	if(error == 0){		/* an den yparxei sfalma, kanoume up */

		sema[sem].counter++;

		if(sema[sem].counter <= 0){

			setreply( sema[sem].head , OK );
			rmvfifo(sem);
		}
		return(0);
	}
	return(error);
}



/* kanei down mia diergasia se ena simaforo (epistrefei 1 an prepei na blokarei, 0 an mporei na treksei, kai error an yparxei sfalma) */


PUBLIC int down(int sem){

	int error;


	error = check(sem);
	if(error == 0){		/* an den yparxei sfalma, kanoume down */

		--sema[sem].counter;

		if(sema[sem].counter < 0){		/* elegxos an prepei na blokarei i diergasia i oxi */

			addfifo(sem);
			return(1);
		}

		return(0);
	}
	return(error);
}




PUBLIC int do_sema(){


	int returned;  		/* Metavliti stin opoia epistrefetai to apotelesma apo tin klisi twn destroy, up, down*/


/* epilegetai poia praksi tha ginei */

	if(m_in.m1_i1 == 1){
		
		returned = init(m_in.m1_i2);
	}else if(m_in.m1_i1 == 2){
		
		returned = destroy(m_in.m1_i3);
	}else if(m_in.m1_i1 == 3){
		
		returned = up(m_in.m1_i3);
	}else if(m_in.m1_i1 == 4){
		
		returned = down(m_in.m1_i3);
		if(returned == 1){		/* elegxos an prepei na blokaristei i diergasia */
			return(SUSPEND);
		}
	}else{
		return(EINVAL);
	}


/* epistrefetai i katallili timi apo tin do_sema */

	return(returned);
}



/* arxikopoiei olous tous simaforous ws eleutherous (mi xrisimopoioumenous) */


PUBLIC void initialise_sema(){

	int i;
	for(i=0;i<N;i++){
		sema[i].usrid = -1;		/* topothetei sto id xristi olwn twn simaforwn tin timi -1 */
	}
}
