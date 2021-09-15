#include "../include/server.h"

int iniciar_servidor(char* IP, char* PUERTO, t_log* logger) {
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(IP, PUERTO, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next) {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    log_trace(logger, "Listo para escuchar a mi cliente");

    return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* logger) {
	struct sockaddr_in dir_cliente;
	socklen_t tam_direccion = sizeof(struct sockaddr_in);
	
	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	if(socket_cliente!=-1) {
		log_info(logger, "Se conecto un cliente! %d",socket_cliente);
	}
	return socket_cliente;
}

int recibir_operacion(int socket_cliente, t_log* logger) {
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente, t_log* logger) {
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

t_paquete* recibir_mensaje(int socket_cliente) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	//int a = recv(socket_cliente, &(paquete->codigo_operacion), sizeof(uint32_t), MSG_WAITALL);
	recv(socket_cliente, &(paquete->buffer->size), sizeof(uint32_t), 0);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, 0);

	// Primero recibimos el codigo de operacion
	/*if(recv(socket_cliente, &(paquete->codigo_operacion), sizeof(int), MSG_WAITALL) == -1)
		paquete->codigo_operacion = -1;*/

	/*if(paquete->codigo_operacion > 0){
		// Despues ya podemos recibir el buffer. Primero su tamano seguido del contenido
		recv(socket_cliente, &(paquete->buffer->size), sizeof(uint32_t), 0);
		paquete->buffer->stream = malloc(paquete->buffer->size);
		recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, 0);
	}*/
	return paquete;
}

//podemos usar la lista de valores para poder hablar del for y de como recorrer la lista
t_list* recibir_paquete(int socket_cliente, t_log* logger) {
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente, logger);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
	return NULL;
}

void enviar_mensaje(t_paquete* paquete, int socket_cliente) {
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, &bytes);
	send(socket_cliente, a_enviar, bytes, 0);
	free(a_enviar);
}

void* serializar_paquete(t_paquete* paquete, int *bytes) {
	void * magic = malloc(*bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(op_code));
	desplazamiento+= sizeof(op_code);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(u_int32_t));
	desplazamiento+= sizeof(u_int32_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}