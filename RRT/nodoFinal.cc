#include <stdio.h>
#include <string.h>
#include <random>

#include "packet_m.h"


using namespace omnetpp;



class nodoFinal : public cSimpleModule
{
    private:
        long numReceived;
        long totRec;
        cLongHistogram hopCountStats;
        cOutVector hopCountVector;
        int hopCount;
        simsignal_t arrivalSignal;
    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;
        virtual void refreshDisplay() const override;
        virtual void finish() override;
};

Define_Module(nodoFinal);

void nodoFinal::initialize() {
    numReceived=0;
    totRec=0;
}
void nodoFinal::handleMessage(cMessage *msg)
{
    Packet *pkt = (Packet*) msg;
    cGate *arrivalGate = pkt -> getArrivalGate();
    int arrivalGateIndex = arrivalGate -> getIndex();
    EV << "Packet arrived from gate " + std::to_string(arrivalGateIndex) + "\n";

    if (pkt -> getKind() == 1) { // Paquete de tipo 1: Ha llegado un paquete de informacion
        //Actualizacion de estadisticos
        pkt -> setHopCount((pkt -> getHopCount())+1);
        hopCountVector.record(pkt -> getHopCount());
        hopCountStats.collect(pkt -> getHopCount());
        if (pkt -> hasBitError()) { // Ha llegado un paquete con errores, se envia un NACK de vuelta
            // Se crea un nuevo paquete de tipo 3 (NACK) y se envia al puerto del nodo desde el que se ha recibido el paquete
            EV << "Packet arrived with error, send NAK\n";
            Packet *nak = new Packet("NAK");
            nak -> setKind(3);
            send(nak, "outPort", arrivalGateIndex);
            // Se aumenta el numero total de paquetes recibidos (totRec) para los estadisticos
            totRec++;
            refreshDisplay();
        }
        else { // Ha llegado un paquete sin errores, se envia un ACK para que no lo reenvien
            EV << "Packet arrived without error, send ACK\n";
            Packet *ack = new Packet("ACK");
            ack -> setKind(2);
            send(ack, "outPort", arrivalGateIndex);
            EV << "Packet it's okay!"; // En este caso, como es un nodo final, no hay que enviar el siguiente paquete

            // Se aumenta el numero total de paquetes recibidos (totRec) y el numero total de paquetes recibidos correctamente (numReceived) para los estadisticos
            numReceived++;
            totRec++;
            refreshDisplay();
        }
    }
}
void nodoFinal::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "Tot rcvd: %ld, rcvd OK: %ld", totRec, numReceived);
    getDisplayString().setTagArg("t", 0, buf);
}
void nodoFinal::finish() // Funcion para generacion de estadisticos
{
    // This function is called by OMNeT++ at the end of the simulation.
    EV << "Received: " << numReceived << endl;
    EV << "Hop count, min:    " << hopCountStats.getMin() << endl;
    EV << "Hop count, max:    " << hopCountStats.getMax() << endl;
    EV << "Hop count, mean:   " << hopCountStats.getMean() << endl;
    EV << "Hop count, stddev: " << hopCountStats.getStddev() << endl;

    recordScalar("#receivedOK", numReceived);
    recordScalar("#receivedTotal", totRec);

    hopCountStats.recordAs("hop count");
}
