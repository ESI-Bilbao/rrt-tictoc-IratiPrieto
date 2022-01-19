packet Packet {
	int fromSource = false; // Para saber si el origen es la fuente
	int sequenceNumber;
	int origin; // Cual es la fuente
	int hopCount=0;  //Para los estadisticos - cuantos saltos da el paquete
}