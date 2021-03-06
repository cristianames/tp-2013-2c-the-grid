#include "grasa.h"
#include <sys/mman.h> //biblioteca para usar mmap y sus derivadas
#include <commons/bitarray.c> //para las funciones que manejan el BitMap
#include <commons/collections/list.c> //para devolver listas en queHayAca
#include <commons/string.h>//para usar split en how




void* dir_bloque(int n);
void left_strcat(char*destino,char*origen);
uint8_t* posicionBloqueSegunByte(GFile* nodo,int byte);
char* lastNameFromPath(char* path);

pthread_mutex_t mutexBitMap=PTHREAD_MUTEX_INITIALIZER;

int nroBlkInd_by_Size(int size)
{
	return size/(GFILEBYTABLE*BLOQUE);
}

bool isEmpty(int nodoPadre, GFile* nodo)
{
int i=1;	

 while((nodoPadre != nodo[i].parent_dir_block)&&(i<GFILEBYTABLE+1)) i++;
 
if (i==GFILEBYTABLE) return VERDAD;
else return FALSO;
}

int queHayAca(int numNodo,GFile* nodo,t_list* lista){
	int i;
	int dirPadre;
	dirPadre=numNodo;
	for (i=1; i <= 1024;i++)
				if ((dirPadre == nodo[i].parent_dir_block)&&(nodo[i].state!=LIBRE)) list_add(lista, nodo[i].fname);
return 0;
}

/*
 * A partir del path consigue el inodo correspondiente
 */
int nodoByPath(const char* path,GFile* nodo){
	char** nombreHijo;
	int numNodo;
	int acierto;
	acierto=0;
	int numHijo;
	numHijo=0;
	int encontrado;
	
	int numPadre;
	
	
	
	nombreHijo = string_split((char*)path,"/");
	
	
	numPadre=0;
	while (nombreHijo[numHijo]!=NULL){
		numNodo=1;	
		encontrado = 0;
		while(numNodo<=(MAXNODO+2)){
			if(strcmp(nombreHijo[numHijo],(char*)nodo[numNodo].fname)==0) acierto++; //Le agregue el casteo
			if((numPadre==nodo[numNodo].parent_dir_block)) acierto++;
			if (acierto==2 && !encontrado) {numPadre=numNodo; encontrado=1;}
			acierto=0;
			numNodo++;
		
		}
			
		numHijo++;
	}
	
	
	if(strcmp(path, "/") == 0) encontrado=1;
	
	
	if(!encontrado) return FAIL;    
	else return numPadre;
}

int readGrid(char *buf, size_t size, off_t offset,GFile* nodo){
	
	int bytesLeidos;
	int byteRestantes;
	int bytePedido;
	int bytesParaCopiar;
	uint8_t* ptr_datos;
	

	bytePedido=offset;
	bytesLeidos=0;
	
while (bytesLeidos < size) {
	
   ptr_datos = posicionBloqueSegunByte(nodo,(int) bytePedido);
   bool esElPrimero = bytesLeidos == 0;  
   int offsetDentroDelBloque = esElPrimero ? offset % BLOQUE : 0;
   int bytesRestantesDentroDelBloque = BLOQUE - offsetDentroDelBloque;

   int bytesParaCopiar = size - bytesLeidos;
   if (bytesParaCopiar > bytesRestantesDentroDelBloque)
      bytesParaCopiar = bytesRestantesDentroDelBloque;
	
	
   memcpy(buf + bytesLeidos, ptr_datos + offsetDentroDelBloque, bytesParaCopiar);
   bytesLeidos += bytesParaCopiar;
   bytePedido += bytesParaCopiar;
  
	}

	return 0;//Le agregue el return con caca.
}


int blkInd_by_cantBlk(int bloquesDatos)
{
	return bloquesDatos%BLOCK_INDIREC_SIZE? bloquesDatos/BLOCK_INDIREC_SIZE+1:bloquesDatos/BLOCK_INDIREC_SIZE;  	 //si el bloque de datos es mayor a 1024 ya pertenece al indirecto de la pocision 0

}


ptrGBloque* arrayIndex(GFile* nodo,int byte){
	
	int numeroBloqueIndirecto;
	int numeroBloqueDato;
	int dirBloqueArray;
	int offsetDelArray;
	ptrGBloque* ptrBloque;
	numeroBloqueDato = byte/BLOQUE; //desde cual bloque de datos voy a leer
	if(byte<4096) numeroBloqueIndirecto = 0;
	else numeroBloqueIndirecto=numeroBloqueDato/1024;  //si el bloque de datos es mayor a 1024 ya pertenece al indirecto de la pocision 1	
	offsetDelArray = numeroBloqueDato%1024;
	dirBloqueArray=nodo->blk_indirect[numeroBloqueIndirecto]; //tengo la direcion del bloque de array con puntero a bloques de datos
	ptrBloque =(ptrGBloque*) dir_bloque(dirBloqueArray); //ahora estoy apuntando a la primera posicion del bloque de array con punteros a bloque de datos
	ptrBloque=ptrBloque+offsetDelArray;         //ahora estoy apundo a la direccion del bloque que quiero leer
	return ptrBloque;
	
}


 uint8_t* posicionBloqueSegunByte(GFile* nodo, int byte)
{
ptrGBloque* ptrBloque=arrayIndex(nodo,byte);
return (uint8_t*)dir_bloque(*ptrBloque);   //ahora estoy AL FIN estoy apuntando al la primer posicion del bloque de datos que tengo que leer
	

}	



int crearDirectorio(const char* path,GFile* inodo){
	int numNodo;
	int nodoPadre;
	int nodoLibre;
	char** subDirectorios;
	char* pathPadre;
	nodoLibre=0;
	time_t fecha;
	
	if ((fecha = time(NULL)) == -1) {
		error_show("Error getting date!");
		return 0;
	}
	
	
	
	if(strlen(lastNameFromPath((char*)path))>(GFILENAMELENGTH+1)) return -ENAMETOOLONG;
	
	
	numNodo=nodoByPath(path,inodo);
	printf("unnodo: %d",numNodo);
	if (numNodo!=FAIL) return -EEXIST;
	
	
	else {
	
	howIsMyFather((char*)path,&pathPadre);
	nodoPadre=nodoByPath(pathPadre,inodo);
	if (nodoPadre==FAIL) return -ENOENT;
	
	
	while((inodo[nodoLibre].state!=0)&&(nodoLibre<=GFILEBYTABLE))nodoLibre++;
	if (nodoLibre>GFILEBYTABLE) return -ENOSPC;
		
	if ((fecha = time(NULL)) == -1) {
		error_show("Error getting date!");
		return 0;
	}

	
	//grabo
	inodo[nodoLibre].state=DIRECTORIO;
	strcpy(inodo[nodoLibre].fname,lastNameFromPath((char*)path));  
	inodo[nodoLibre].parent_dir_block=nodoPadre;
	inodo[nodoLibre].m_date=(uint64_t)fecha;
	inodo[nodoLibre].c_date=(uint64_t)fecha;
	inodo[nodoLibre].file_size=0;
	
	return 0;
	}
	

}

 int howIsMyFather(char* src, char** dest){
	
	char* lastName;
	int tamRuta;
	int i=0;
	tamRuta= strlen(src) - strlen(lastNameFromPath(src));
	*dest=string_duplicate(string_substring_until(src,strlen(src) - (strlen(lastNameFromPath(src))+1)));
	return 0;
	
}

char* lastNameFromPath(char* path){
	char** subDirectorios;
	subDirectorios = string_split(path,"/");
	return subDirectorios[strlenTwo(subDirectorios)-1];
}


int strlenTwo(char**str){
	int i=0;
		while(str[i])i++;
	return i;
}

int crearArchivo(const char* path,GFile* inodo){
	int numNodo;
	int nodoPadre;
	int nodoLibre;
	int i;
	char* pathPadre;
	nodoLibre=1;
	time_t fecha;
	struct tm *temp;
	
	

	
	
	printf("la ruta que llego es: %s\n",path);
	
	if(strlen(lastNameFromPath((char*)path))>(GFILENAMELENGTH+1)) return -ENAMETOOLONG;
	puts("pase la prueba del nombre");
	
	
	numNodo=nodoByPath(path,inodo);
		printf("unnodo: %d",numNodo);
	printf("el resultado de nodobypath es: %d\n",numNodo);
	if (numNodo!=FAIL) 
	{
		if(inodo[numNodo].state==OCUPADO)
		{
				if ((fecha = time(NULL)) == -1) 
				{
				puts("Error getting date!");
				return 0;
				}	
		puts("entonces el archivo existe\n");
		inodo[numNodo].c_date=fecha;
		
	    }
	    else return -ENOTDIR;
	}
	
	else {
	puts("pase a crear\n");
	howIsMyFather((char*)path,&pathPadre);
	printf("la ruta padre es: %s\n",pathPadre);
	nodoPadre=nodoByPath(pathPadre,inodo);
	printf("el resultado de nodobypath es: %d\n",nodoPadre);
	if (nodoPadre==FAIL) return -ENOENT;
	puts("pase la prueba da la ruta valida");
	while((inodo[nodoLibre].state!=LIBRE)&&(nodoLibre<=(GFILEBYTABLE)))nodoLibre++; //es el 1026
	
	
	if(nodoLibre>(GFILEBYTABLE)) return -ENOSPC;   //si es el 1026
	
	printf("el primer nodo libre es: %d\n",nodoLibre);
	printf("El que quiero agragar es el last name: %s\n",lastNameFromPath((char*)path));
	
	
		
	//grabo
	inodo[nodoLibre].state=OCUPADO;
	strcpy(inodo[nodoLibre].fname,lastNameFromPath((char*)path));  
	inodo[nodoLibre].parent_dir_block=nodoPadre;
	inodo[nodoLibre].m_date=(uint64_t)fecha;
	inodo[nodoLibre].c_date=(uint64_t)fecha;
	inodo[nodoLibre].file_size=0;
	for(i=0;i<=999;i++) inodo[nodoLibre].blk_indirect[i]=0;
	
	return 0;
	}
	
	
}

int bloquesBySize(int size)
{
	
	return (size%BLOQUE==0) ?  size/BLOQUE : (size/BLOQUE)+1;
	
}

int asignarBloque(t_bitarray* bMap)
{	
	pthread_mutex_lock( &mutexBitMap);
	int i=0;
	bool ocupado;
	
		while(bitarray_get_max_bit(bMap)>i)
		{
			i++;
			ocupado = bitarray_test_bit(bMap,i);
			//if (ocupado) printf ("%d) ocupado\n",i);
			if (!ocupado) break;
			
		}
	printf("que bloque le di: %d\n",i);  
	bitarray_set_bit(bMap,i);
	pthread_mutex_unlock( &mutexBitMap);
	puts("");puts("");puts("");
	return i;
}

/* 
 * @Nombre: asignarIndirecto
 * @Precondicion: nodo donde va a ser cargado el array de 1024 bloque de datos
 * @posCondicion: puntero de tipo ptrGBloque a la primer posicion de array de datos 
 *                o null si no hay mas bloques
 */

ptrGBloque* asignarIndirecto(t_bitarray* bMap,GFile* archivo,int refIndirecto)
{
	puts("estoy en asignarbloque");
	int res = asignarBloque(bMap);
	printf("El res es: %d\n",res);
	archivo->blk_indirect[refIndirecto]=res;	
	return (ptrGBloque*) dir_bloque(archivo->blk_indirect[refIndirecto]);
		
		
}

/*
 * @Nombre: difEntreIndirectos
 * @Precondicion: cantidad de bloques actuales, cantidad de bloques nuevos
 * @posCondicion: devuelve un numero positivo si hay diferencia entre bloques indirectos y ese numero es la diferencia. sino devuelve 0 
 */


int difEntreIndirectos(int bActual,int bNuevos)
{
	return blkInd_by_cantBlk(bActual)<blkInd_by_cantBlk(bNuevos) ? blkInd_by_cantBlk(bNuevos)-blkInd_by_cantBlk(bActual) : blkInd_by_cantBlk(bActual)-blkInd_by_cantBlk(bNuevos);
}

int liberarBloque(int byte,t_bitarray* bMap,GFile* nodo)
{
	ptrGBloque* pgBloque=arrayIndex(nodo,byte);
	//pgBloque--;
	printf("que bloque voy a liberar: %d\n", *pgBloque);
	bitarray_clean_bit(bMap,*pgBloque);
	printf("la disponibilidad del bloque %d es: ",*pgBloque);
	if(bitarray_test_bit(bMap,*pgBloque)) puts("desocupado");
	else puts("ocupado");
	puts("");puts("");puts("");
return 0;
}

/* @Nombre: liberarIndirectos
 * @Precondicion: el bitmap, los nodos y la diferencia entre bloques indirectos
 * @posCondicion: devuelve 0 si seteo a 0 los bloques correspondientes 
 */

int liberarIndirecos(t_bitarray* bMap,GFile* nodo,int dif)
{
	int nroIndRef;
	puts("entre a liberar indirectos");
	if (nodo->file_size<(BLOCK_INDIREC_SIZE *BLOQUE)) nroIndRef=0;
	 else nroIndRef= nroBlkInd_by_Size(nodo->file_size);
	 
	if (nodo->file_size==(BLOCK_INDIREC_SIZE *BLOQUE)) nroIndRef--;
	printf("indice ref que libero liberar: %d\n", nroIndRef);
	int i;
	while(dif)
	{
		printf("dif: %d\n", dif);
		printf("que bloque voy a liberar: %d\n", nodo->blk_indirect[nroIndRef]);
		bitarray_clean_bit(bMap,nodo->blk_indirect[nroIndRef]);
		printf("la disponibilidad del bloque %d es\n: ",nodo->blk_indirect[nroIndRef]);
		nroIndRef--;
		dif--;
	}  
	
	
return 0;
}

int bloqueLibresRestantes(t_bitarray* bMap)
{
	int i=0;
	int cant=0;
	bool ocupado;
	
		while(bitarray_get_max_bit(bMap)>i)
		{
			i++;
			ocupado = bitarray_test_bit(bMap,i);
			if (!ocupado) cant++;
		}
	return cant;
}

bool noMeAlcansanLosBloques(int actuales,int offset,int bRestantes)
{
	if((offset+difEntreIndirectos(actuales,offset))>bRestantes) return VERDAD;
	else return FALSO;
		
}

int truncale(const char* path,off_t offset,GFile* nodo,t_bitarray* bMap)
{
	int numNodo= nodoByPath(path,nodo);
	int byteRef;
	ptrGBloque* pGBloque;
	
	
	if(numNodo==FAIL)return -ENOENT;
	printf("el offset que llego es: %d\n",(int)offset);
	//Si el archivo anteriormente era más grande que este tamaño, se pierden los datos extra. 
	
	//Si el archivo anteriormente era más corto, SI se extendió dentro del bloque actualmente asignado me quedo piola 
	//											 SI se extendió al proximo bloque me fijo cuantos bloques SI es el proximo bloque estas dentro de los 1024
	//		
	//																								  SI esta en proximo inodo, pido otro bloque de 1024.				 
	if(nodo[numNodo].state!=OCUPADO) return -ENOTDIR;
	printf("el archivo que vamos a truncar es: %s\n",nodo[numNodo].fname);
	int bloquesDatosActuales = bloquesBySize(nodo[numNodo].file_size);  //HECHO
	printf("los  bloquesDatosActuales son: %d\n",bloquesDatosActuales);
	int bloquesDatosOffset = bloquesBySize(offset);
	printf("los bloquesDatosOffset son: %d\n",bloquesDatosOffset);
	printf("El file size es: %d\n",nodo[numNodo].file_size);
	
	puts("me alcansan los bloques"); 
	
	int indirecRef= ((nodo[numNodo].file_size) ? nroBlkInd_by_Size(nodo[numNodo].file_size)+1 : 0);   //aca HAY ALGO
	printf("ultimo indice de 1000 puntero es: %d\n",indirecRef);
	//if (nodo[numNodo].file_size==0 && offset<BLOQUE) {pGBloque=iniciarNodo(bMap,nodo+numNodo),*pGBloque=asignarBloque(bMap);} 
	
	
	if (bloquesDatosActuales < bloquesDatosOffset)   //se agranda informo el nuevo size del archivo.
	{
		    if(noMeAlcansanLosBloques(bloquesDatosActuales,bloquesDatosOffset,bloqueLibresRestantes(bMap))) return -ENOSPC;	
	  	    puts("entoy agrandando\n");
		    if(nodo[numNodo].file_size) 
		    {
				 pGBloque = arrayIndex(nodo+numNodo,nodo[numNodo].file_size);
			//	 printf("el pGBloque apuanta antes del ++ a: %p\n",pGBloque);
				// printf("el ultimo pGBloque de este archivo es: %d\n",*pGBloque);
				 //pGBloque++;
				 //printf("el pGBloque apuanta despues del ++ a: %p\n",pGBloque);
		    }	
	//else pGBloque=iniciarNodo(bMap,nodo+numNodo);  //si es 0 y offset es mayor a un bloque pasa por aca
	    //pongo esto afuera y soluciono? si lo saco entra a while y el if de verdadero
	 
			while(bloquesDatosActuales < bloquesDatosOffset)
			{
				
				 //printf("el bloque actuales son: %d\n",bloquesDatosActuales);
				 //printf("resultado de bloquesDatosActuales resto: %d\n", bloquesDatosActuales%BLOCK_INDIREC_SIZE);
				 
				 
				 //printf("ultimo indirecto antes de chequear: %d\n",nodo[numNodo].blk_indirect[nroBlkInd_by_Size(nodo[numNodo].file_size)]);    //no me dice el actual bñk por wmpieza de 0
				 if(!(bloquesDatosActuales%BLOCK_INDIREC_SIZE)) 
				   {
					 
					 pGBloque = asignarIndirecto(bMap,nodo+numNodo,indirecRef); 
					 puts("entre al if de asignar otro bloque indirecto");
					 indirecRef++;
					 puts("");puts("");
				   }  
			//	 printf("ultimo indirecto despues de chequear: %d\n",nodo[numNodo].blk_indirect[nroBlkInd_by_Size(nodo[numNodo].file_size)]);
				 *pGBloque=asignarBloque(bMap);   
			//	 printf("el resultado de pGBloque es: %d\n",*pGBloque);
				 bloquesDatosActuales++;
				 //printf("anterior indirecto despues de chequear: %d",nodo->blk_indirect[blkInd_by_cantBlk(bloquesDatosActuales));
				 pGBloque++;
		//		 printf("el pGBloque apuanta despues del ++ a: %p\n",pGBloque);
			}
		}
	
	else{
			puts("entre achicar\n");
			if (bloquesDatosActuales > bloquesDatosOffset) // se achica  informo el nuevo size del achivo, con eso deberia ser suficiente para saber como leer
			{
				int difBloquesIndirectos = difEntreIndirectos(bloquesDatosActuales,bloquesDatosOffset); //HECHO
				byteRef =nodo[numNodo].file_size; // (nodo[numNodo].file_size<BLOQUE)? BLOQUE-1 : nodo[numNodo].file_size-BLOQUE;       
				if(byteRef%(BLOQUE*BLOCK_INDIREC_SIZE)==0) byteRef++;
				while(bloquesDatosActuales > bloquesDatosOffset)                
					{
					 liberarBloque(byteRef,bMap,nodo+numNodo);   
					 if(byteRef>4096) byteRef-=BLOQUE;
					 else byteRef=BLOQUE-1;
					 bloquesDatosActuales--;
					}
				if (difBloquesIndirectos) liberarIndirecos(bMap,nodo+numNodo,difBloquesIndirectos);   
			}
				
	}
	nodo[numNodo].file_size=(uint32_t)offset;
return 0;
}


int writeGrid(const char *buf, size_t size, off_t offset,GFile* nodo){
	
	int bytesGrabados;
	int byteRestantes;
	int bytePedido;
	int bytesParaCopiar;
	uint8_t* ptr_datos;
	bytePedido=offset;
	bytesGrabados=0;
	
while (bytesGrabados < size) 
   {
   ptr_datos = posicionBloqueSegunByte(nodo,(int) bytePedido);
   bool esElPrimero = bytesGrabados == 0;  
   int offsetDentroDelBloque = esElPrimero ? offset % BLOQUE : 0;
   int bytesRestantesDentroDelBloque = BLOQUE - offsetDentroDelBloque;

   int bytesParaCopiar = size - bytesGrabados;
   if (bytesParaCopiar > bytesRestantesDentroDelBloque)
      bytesParaCopiar = bytesRestantesDentroDelBloque;
   memcpy(ptr_datos + offsetDentroDelBloque,buf + bytesGrabados,  bytesParaCopiar);
   bytesGrabados += bytesParaCopiar;
   bytePedido += bytesParaCopiar;
   return bytesGrabados;
	}
}



/*
 * @descripcion: borra direcctorios vacios
 * @precondicion: recibe una ruta y el puntero a la primer posicion de la tabla de inodos
 * @poscondicion: directotio borrado o informa porque no se pudo borrar 
 * 
 */

int32_t borrarDirectorio(const char* path,GFile* inodo)
{
	char* pathPadre;
	
	int numNodo=nodoByPath(path,inodo);
	if (numNodo==FAIL) return -EEXIST;
	//if(!isEmpty(numNodo, inodo+numNodo)) return -EBUSY;
	if (inodo[numNodo].state!=DIRECTORIO) return -ENOTDIR;
	howIsMyFather((char*)path,&pathPadre);
	if (nodoByPath(pathPadre,inodo)==FAIL) return -ENOENT; 
	inodo[numNodo].state=LIBRE;
	strcpy(inodo[numNodo].fname,"");  
	inodo[numNodo].parent_dir_block=-1;
	inodo[numNodo].m_date=(uint64_t)0;
	inodo[numNodo].c_date=(uint64_t)0;
	
	return 0;
	
}



int32_t borrarArchivo(const char* path,GFile* inodo,t_bitarray* bitMap)
{
	char* pathPadre;
	int numNodo=nodoByPath(path,inodo);
		
	if (numNodo==FAIL) return -EEXIST;
	if(inodo[numNodo].state!=OCUPADO) return -ENOTDIR;
	howIsMyFather((char*)path,&pathPadre);
	if (nodoByPath(pathPadre,inodo)==FAIL) return -ENOENT; 
	
	if(inodo[numNodo].file_size)truncale(path,0,inodo,bitMap);
	inodo[numNodo].state=LIBRE;
	strcpy(inodo[numNodo].fname,"");  
	inodo[numNodo].parent_dir_block=-1;
	inodo[numNodo].m_date=(uint64_t)0;
	inodo[numNodo].c_date=(uint64_t)0;
	inodo[numNodo].file_size=0;
	int i;
	for(i=0;i<=999;i++) inodo[numNodo].blk_indirect[i]=0;
	
	return 0;
}

