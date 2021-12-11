#include "../include/server.h"

/* Utilizar esta función para levantar el servidor */
int iniciar_servidor(char *ip, char *puerto, t_log *logger) {
	int socket_servidor;
    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next) {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
		}

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	int error = listen(socket_servidor, SOMAXCONN);
    freeaddrinfo(servinfo);
	if(error != -1){
    	log_info(logger, "Componente preparado para recibir conexiones con socket %d", socket_servidor);
    	return socket_servidor;
	}else{
		log_error(logger, "Se produjo un error en la creacion del servidor\n");
		sleep(1);
		return iniciar_servidor(ip, puerto, logger);
	}
}

/* Utilizar esta función para conectarse a otro servidor */
int crear_conexion(char *ip, char *puerto) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1) {
		freeaddrinfo(server_info);	
		return -1;
	}
	printf("Conexion creada usando socket %d\n", socket_cliente);
	freeaddrinfo(server_info);
	return socket_cliente;
}

/* Utilizar esta función cuando es necesario esperar la conexión de un cliente para continuar la ejecución */
int esperar_cliente(int socket_servidor, t_log* logger) {
	struct sockaddr_in dir_cliente;
	socklen_t tam_direccion = sizeof(struct sockaddr_in);
	
	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	if(socket_cliente != -1) {
		log_info(logger, "Se conecto un cliente con socket %d", socket_cliente);
	}
	return socket_cliente;
}

/* Utilizar esta función para serializar el paquete antes de enviarlo */
void *serializar_paquete(t_paquete *paquete, int *bytes) {
	void *magic = malloc(*bytes);
	int desplazamiento = 0;
	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(op_code));
	desplazamiento+= sizeof(op_code);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(u_int32_t));
	desplazamiento+= sizeof(u_int32_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

/* Utilizar esta función para enviar un paquete */
void enviar_paquete(t_paquete *paquete, int socket_cliente) {
	int bytes = paquete->buffer->size + 2 * sizeof(int);
	printf("Paquete a enviar: opcode %d | buffer size %d | dest %d\n", paquete->codigo_operacion, paquete->buffer->size, socket_cliente);
	void *a_enviar = serializar_paquete(paquete, &bytes);
	int error = send(socket_cliente, a_enviar, bytes, 0);

	
	free(paquete);
	if(error == -1){
		printf("Hubo un error en el envio\n");
	}
}

/* Utilizar esta función para recibir un paquete */
t_paquete* recibir_paquete(int socket_cliente) {

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	printf("(recibir paquete)Espero recibir de %d \n",socket_cliente);
	recv(socket_cliente, &(paquete->codigo_operacion), sizeof(uint32_t), MSG_WAITALL);
	printf("(recibir paquete)Recibi de %d esto : %d\n",socket_cliente, paquete->codigo_operacion);
	
	if(paquete->codigo_operacion <= 0 || paquete->codigo_operacion > 100){
		printf("El cliente se desconectó forzosamente...\n");
		paquete->codigo_operacion = CLIENTE_DESCONECTADO;
		paquete->buffer->size = 0;
		return paquete;
	}
	recv(socket_cliente, &(paquete->buffer->size), sizeof(uint32_t), 0);
	if(paquete->buffer->size){
		paquete->buffer->stream = malloc(paquete->buffer->size);
		recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, 0);
	}

	return paquete;
}