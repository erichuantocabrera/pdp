#include "buddy_system.h"
#include "memoria.h"


//=========================================================
int cantidad_separador(char* palabra, char separador)
{
	int cantidad = 0;

	for(int i=0; i<string_length(palabra); i++)
	{
		if(palabra[i] == separador)
			cantidad++;
	}
	return cantidad;
}

void agregar_caracter(char** palabra, char caracter)
{
	int tamanio = string_length(*palabra);
	char* string = malloc(tamanio +1 +1);

	strcpy(string,*palabra);
	free(*palabra);

	string[tamanio] = caracter;
	string[tamanio+1] = '\0';
	*palabra = string;
}

void quita_ultima_palabra(char** palabra, char separador)
{
	char* string = string_new();
	char* cadena = *palabra;
	int cantidad = cantidad_separador(cadena, separador);

	for(int i=0; i<string_length(cadena); i++)
	{
		if(cadena[i] == separador)
			cantidad--;

		if(cantidad==0)
			break;

		agregar_caracter(&string, cadena[i]);
	}

	free(*palabra);
	*palabra = string;
}
//=========================================================

void iniciar_memoria_buddy()
{
	particiones = list_create();

	flag_memoria = 0;
	fifo = 0;

	t_particion* inicio = malloc(sizeof(t_particion));
	inicio->inicio_particion = inicio_memoria;
	inicio->fin_particion = inicio_memoria;
	inicio->libre = false;
	inicio->flag = string_new();
	string_append(&(inicio->flag), ",-1");
	inicio->fifo = -1;

	list_add(particiones, inicio);

	t_particion* memoria = malloc(sizeof(t_particion));
	memoria->inicio_particion = inicio_memoria;
	memoria->fin_particion = inicio_memoria + TAMANO_MEMORIA;
	memoria->libre = true;
	memoria->flag = string_new();
	string_append(&(memoria->flag), "0");
	memoria->fifo = -1;
	flag_memoria++;

	list_add(particiones, memoria);

	t_particion* fin = malloc(sizeof(t_particion));
	fin->inicio_particion = inicio_memoria + TAMANO_MEMORIA;
	fin->fin_particion = inicio_memoria + TAMANO_MEMORIA;
	fin->libre = false;
	fin->flag = string_new();
	string_append(&(fin->flag), ",-1");
	fin->fifo = -1;

	list_add(particiones, fin);

}
//==========================================================================
void* creo_particiones_internas(t_particion* particion, int posicion, int size)
{
	if((particion->fin_particion - particion->inicio_particion)/2 < size) {
		particion->libre = false;
		particion->fifo = fifo;
		fifo++;
		return particion->inicio_particion;
	}

	t_particion* particionA = malloc(sizeof(t_particion));
	particionA->inicio_particion = particion->inicio_particion;
	particionA->fin_particion = particion->inicio_particion + (particion->fin_particion - particion->inicio_particion)/2;
	particionA->libre = true;
	particionA->flag = string_new();
	string_append(&particionA->flag, particion->flag);
	agregar_caracter(&particionA->flag, ',');
	string_append(&particionA->flag, string_itoa(flag_memoria));
	particionA->fifo = -1;
	list_add_in_index(particiones, posicion, particionA);

	t_particion* particionB = malloc(sizeof(t_particion));
	particionB->inicio_particion = particionA->fin_particion;
	particionB->fin_particion = particion->fin_particion;
	particionB->libre = true;
	particionB->flag = string_new();
	string_append(&particionB->flag, particion->flag);
	agregar_caracter(&particionB->flag, ',');
	string_append(&particionB->flag, string_itoa(flag_memoria));
	particionB->fifo = -1;
	list_add_in_index(particiones, posicion +1, particionB);

	flag_memoria++;

	list_remove_and_destroy_element(particiones, posicion +2, free);

	creo_particiones_internas(particionA, posicion, size);

}

void* buscar_particion_libre(int size)
{
	int tamanio_particion;
	for(int i=0; i<(list_size(particiones) - 1); i++)
	{
		t_particion* particion = list_get(particiones, i);

		if(!particion->libre)// si la particion esta ocupada, se debe pasar de largo
			continue;

		tamanio_particion = particion->fin_particion - particion->inicio_particion;
		if(tamanio_particion < size)// si la particion es pequeña buscamos otra.
			continue;

		if(tamanio_particion >= size)// si la particion alcanza hay dos opciones
			return creo_particiones_internas(particion, i, size);

	}

	printf("no hay memoria disponilbe  \n");
	return NULL;
}

void* pedir_memoria_buddy(int size)
{

	void* inicio_particion = buscar_particion_libre(size);

	if(inicio_particion == NULL) {
		printf("no se encontro un espacio libre");
		return NULL;
	}
	else
		return inicio_particion;

}
//==========================================================================
void imprimirparticion(t_particion* particion)
{
	int tamanio = particion->fin_particion - particion->inicio_particion;
	char* palabra = malloc(tamanio +1);

	memcpy((void*)palabra, particion->inicio_particion, tamanio);
	palabra[tamanio] = '\0';

	printf(" %s   \n", palabra);

}
void esta_libre(bool flag)
{
	if(flag)
		printf(" SI \n");
	else
		printf(" NO \n");
}
void dump_memoria_buddy()
{
	void* comienzo;
	printf("////////////////////////////////////////////////////////////////// \n");
	for(int i=0; i<(list_size(particiones) -1); i++)
	{
		t_particion* particion = list_get(particiones, i);
		if(i == 0){
			comienzo = particion->inicio_particion;
			continue;
		}

		printf("inicio_particion: %d ", (int)(particion->inicio_particion - comienzo));
		//imprimirparticion(particion);
		printf("tamanio: %d ", particion->fin_particion - particion->inicio_particion);
		printf("libre: "); esta_libre(particion->libre);
		printf("\n");
	}
	printf("////////////////////////////////////////////////////////////////// \n");
}
//===========================================================================
void compactar_buddy()
{
	//en la ultima version se descarto esta yuppy.
}
//===========================================================================
void unificar(int posicion)
{
	t_particion* particion1 = list_get(particiones, posicion);
	t_particion* particion2 = list_get(particiones, posicion +1);

	particion1->fin_particion = particion2->fin_particion;
	quita_ultima_palabra(&(particion1->flag), ',');

	list_remove_and_destroy_element(particiones, posicion +1, free);

	//bool hay_mas = false;
	//int nueva_pos;

	for(int i=0; i<(list_size(particiones) - 1); i++)
	{
		t_particion* particionA = list_get(particiones, i);
		t_particion* particionB = list_get(particiones, i+1);

		if(particionA->libre && particionB->libre)
		{
			if(string_equals_ignore_case(particionA->flag, particionB->flag))
			{
				//nueva_pos = i;
				//hay_mas = true;
				//break;
				unificar(i);
				break;
			}
		}
	}

	//if(hay_mas)
		//unificar(nueva_pos);
}

void consolidar_buddy()
{
	bool hay_que_consolidar = false;
	int numero_particion;

	for(int i=0; i<(list_size(particiones) - 1); i++)
	{
		t_particion* particion1 = list_get(particiones, i);
		t_particion* particion2 = list_get(particiones, i+1);

		if(particion1->libre && particion2->libre)
		{
			if(string_equals_ignore_case(particion1->flag, particion2->flag))
			{
				numero_particion = i;
				hay_que_consolidar = true;
				break;
			}

		}
	}
	if(hay_que_consolidar)
		unificar(numero_particion);
}

