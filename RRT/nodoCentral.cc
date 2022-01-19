#include <stdio.h>
#include <string.h>
#include "packet_m.h"

using namespace omnetpp;

class nodoCentral : public cSimpleModule
{
    private:
        cChannel *channel[2];
        cQueue *queue[2];
        double probability;
        long numSent;
        long numReceived;
        long totRec;
        cLongHistogram hopCountStats;
        cOutVector hopCountVector;
        int hopCount;
    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;
        virtual void sendNew(Packet *pkt);
        virtual void sendNext(int gateIndex);
        virtual void sendPacket(Packet *pkt, int gateIndex);
        virtual void refreshDisplay() const override;
        virtual void finish() override;
};

Define_Module(nodoCentral);

void nodoCentral::initialize() {
    // Punteros de cada uno de los canales de transmision
    channel[0] = gate("outPort", 0) -> getTransmissionChannel();
    channel[1] = gate("outPort", 1) -> getTransmissionChannel();

    // Colas de paquetes para cada uno de los canales
    queue[0] = new cQueue("queueZero");
    queue[1] = new cQueue("queueOne");

    // Inicializacion del generador de numeros aleatorios
    srand(time(NULL));

    // Probabilidad
    probability = (double) par("probability");

    // Parametros para calculo de estadisticas
    numReceived = 0;
    numSent = 0;
    totRec = 0;


}
void nodoCentral::refreshDisplay() const
{
    // Informacion que se muestra en cada uno de los nodos en la simulacion
    char buf[40];
    sprintf(buf, "Tot rcvd: %ld, rcvd OK: %ld, sent: %ld", totRec, numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void nodoCentral::handleMessage(cMessage *msg)
{
    // Se lee un paquete recogido y desde donde se recoge
    Packet *pkt = check_and_cast<Packet *> (msg);
    cGate *arrivalGate = pkt -> getArrivalGate();
    int arrivalGateIndex = arrivalGate -> getIndex();
    EV << "Packet arrived from gate " + std::to_string(arrivalGateIndex) + "\n";

    if (pkt -> getFromSource()) {
        // Se recoge un paquete que llega desde la fuente
        EV << "Forward packet from source\n";
        pkt -> setFromSource(false); // El paquete que se reenvia ya no se manda desde la fuente sino desde un nodo central
        // Se envia un paquete al siguiente nodo y se actualizan las estadisticas
        sendNew(pkt);
        numSent++;
        refreshDisplay();
        return;
    }
    if (pkt -> getKind() == 1) { // Paquete de tipo 1: Ha llegado un paquete de informacion
        //Actualizacion de estadisticos
        pkt -> setHopCount((pkt -> getHopCount())+1);
        hopCountVector.record(pkt -> getHopCount());
        hopCountStats.collect(pkt -> getHopCount());
        if (pkt -> hasBitError()) { // Ha llegado un paquete con errores, se envia un NACK de vuelta
            EV << "Packet arrived with error, send NAK\n";
            // Se crea un nuevo paquete de tipo 3 (NACK) y se envia al puerto del nodo desde el que se ha recibido el paquete
            Packet *nak = new Packet("NAK");
            nak -> setKind(3);
            send(nak, "outPort", arrivalGateIndex);
            totRec++; // Se aumenta el numero total de paquetes recibidos (totRec) para los estadisticos
            refreshDisplay();
        }
        else { // Ha llegado un paquete sin errores, se envia un ACK para que no lo reenvien y se reenvia el paquete al siguiente nodo
            EV << "Packet arrived without error, send ACK\n";
            // Se crea un paquete nuevo de tipo ACK (2) y se envia al puerto del nodo desde el que se ha recibido el paquete
            Packet *ack = new Packet("ACK");
            ack -> setKind(2);
            send(ack, "outPort", arrivalGateIndex);
            sendNew(pkt); // Se envia el paquete al siguiente nodo

            // Se aumenta el numero total de paquetes recibidos (totRec) y el numero total de paquetes recibidos correctamente (numReceived) para los estadisticos
            numReceived++;
            totRec++;
            refreshDisplay();
        }
    }
    // Para la recepcion de (N)ACK se mantiene el paquete recien enviado en la cola por si acaso hay que reenviarlo
    else if (pkt -> getKind() == 2) { // Paquete de tipo 2: Ha llegado un ACK
        EV << "ACK from next node\n";
        if (queue[arrivalGateIndex] -> isEmpty()) // El paquete que deberia estar en cola no esta
            EV << "WARNING: there are not packets in queue, but ACK arrived\n";
        else { // Se quita el paquete que se estaba manteniendo en cola porque no hay que reenviarlo
            queue[arrivalGateIndex] -> pop();
            sendNext(arrivalGateIndex);
        }
    }
    else { // Paquete de tipo 3: Ha llegado un NACK
        EV << "NAK from next node\n";
        sendNext(arrivalGateIndex); // Se reenvia el primer paquete que sigue en la cola porque no habia llegado bien
    }


}

void nodoCentral::sendNew(Packet *pkt) {
    int gateIndex;

    // Se elige desde que puerto de salida se va a enviar el paquete dependiendo de la probabilidad
    double randomNumber = ((double) rand() / (RAND_MAX));
    if (randomNumber < probability)
        gateIndex = 0;
    else
        gateIndex = 1;


    if (queue[gateIndex] -> isEmpty()) { // La cola de envio de paquetes esta vacia, se inserta el paquete y se manda directamente
        EV << "Queue is empty, send packet and wait\n";
        queue[gateIndex] -> insert(pkt); // Insercion del paquete en cola
        sendPacket(pkt, gateIndex); // Envio del paquete por el puerto seleccionado

    } else { // La cola de envio de paquetes no esta vacia, se inserta el paquete al final y se espera a que llegue al comienzo de la cola para mandarlo
        EV << "Queue is not empty, add to back and wait\n";
        queue[gateIndex] -> insert(pkt);
    }
}

void nodoCentral::sendNext(int gateIndex) {
    if (queue[gateIndex] -> isEmpty()) // No hay mas paquetes en cola para mandar
        EV << "No more packets in queue\n";
    else { // Se manda el primer paquete que hay en la cola sin eliminarlo por si hay que reenviarlo
        Packet *pkt = check_and_cast<Packet *> (queue[gateIndex] -> front());
        sendPacket(pkt, gateIndex);
    }
}

void nodoCentral::sendPacket(Packet *pkt, int gateIndex) {
    if (channel[gateIndex] -> isBusy()) {
        EV << "WARNING: channel is busy, check that everything is working fine\n";
    } else {
        // OMNeT++ can't send a packet while it is queued, must send a copy
        Packet *newPkt = check_and_cast<Packet *> (pkt -> dup());
        send(newPkt, "outPort", gateIndex);

        // Actualizacion de paquetes enviados para estadisticos
        numSent++;
        refreshDisplay();
    }
}
void nodoCentral::finish() // Funcion para generacion de estadisticos
{
    // This function is called by OMNeT++ at the end of the simulation.
    EV << "Sent:     " << numSent << endl;
    EV << "Received: " << numReceived << endl;
    EV << "Hop count, min:    " << hopCountStats.getMin() << endl;
    EV << "Hop count, max:    " << hopCountStats.getMax() << endl;
    EV << "Hop count, mean:   " << hopCountStats.getMean() << endl;
    EV << "Hop count, stddev: " << hopCountStats.getStddev() << endl;

    recordScalar("#sent", numSent);
    recordScalar("#receivedOK", numReceived);
    recordScalar("#receivedTotal", totRec);

    hopCountStats.recordAs("hop count");
}
