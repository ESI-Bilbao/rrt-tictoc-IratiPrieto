#include <stdio.h>
#include <string.h>
#include <random>

#include "packet_m.h"


using namespace omnetpp;



class nodoFuente : public cSimpleModule
{
    private:
        double lambda = 2;
        double meanPacketLength;
        int samples = 100;
        int sequenceNumber = 0;
    protected:
        virtual void initialize() override;
        virtual std::vector<double> getDepartures(double lambda, int samples);
        virtual std::vector<double> getLengths(double mu, int samples);
        virtual Packet* getPacket();
        virtual void handleMessage(cMessage *msg) override;
};

Define_Module(nodoFuente);

void nodoFuente::initialize() {
    meanPacketLength = (double) par("meanPacketLength");
    // Get departure times, generate packets and schedule them
    std::vector<double> departures = getDepartures(lambda, samples);
    std::vector<double> lengths = getLengths(meanPacketLength, samples);
    for(int i = 0; i < departures.size(); i++) {
        // Scheduled packets will arrived to the same module at
        // departures[i]
        Packet *pkt = getPacket();
        pkt -> setBitLength(lengths[i]);
        scheduleAt(departures[i], pkt);
    }
}

std::vector<double> nodoFuente::getDepartures(double lambda, int samples) {
    // Generates an exponential distribution with SAMPLES length
    // and using LAMDBA.
    std::uniform_real_distribution<double> randomReal(0.0, 1.0);
    std::default_random_engine generator(time(NULL));
    std::vector<double> departures(samples);
    for(int i = 0; i < departures.size(); i++) {
        double randomNumber = randomReal(generator);
        double number = (-1/lambda) * log(randomNumber);
        if (i != 0)
            departures[i] = departures[i - 1] + number;
        else
            departures[i] = number;
    }
    return departures;
}

std::vector<double> nodoFuente::getLengths(double meanPacketLength, int samples) {
    std::uniform_real_distribution<double> randomReal(0.0, 1.0);
    std::default_random_engine generator(time(NULL));
    std::vector<double> lengths(samples);
    for(int i = 0; i < lengths.size(); i++) {
        double randomNumber = randomReal(generator);
        double number = (-meanPacketLength) * log(randomNumber);
        lengths[i] = number;
    }
    return lengths;
}

Packet* nodoFuente::getPacket() {
    std::string packetName = "packet::" + std::to_string(getId()) + "::" + std::to_string(sequenceNumber);
    char *charPacketName = new char[packetName.length() + 1];
    strcpy(charPacketName, packetName.c_str());
    Packet *pkt = new Packet(charPacketName);
    pkt -> setFromSource(true);
    pkt -> setKind(1);
    pkt -> setSequenceNumber(sequenceNumber);
    pkt -> setOrigin(getId());
    sequenceNumber++;
    return pkt;
}

void nodoFuente::handleMessage(cMessage *msg) {
    // Send scheduled packet
    Packet *pkt = check_and_cast<Packet *> (msg);
    send(pkt, "outPort");
}
