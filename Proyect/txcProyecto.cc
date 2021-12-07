//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2003-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

#include "packet_m.h"


using namespace omnetpp;

/**
 * In the previous model we just created another packet if we needed to
 * retransmit. This is OK because the packet didn't contain much, but
 * in real life it's usually more practical to keep a copy of the original
 * packet so that we can re-send it without the need to build it again.
 */
class Tic : public cSimpleModule
{
  private:
    simtime_t timeout;  // timeout
    cMessage *timeoutEvent;  // holds pointer to the timeout self-message
    int seq;  // message sequence number
    cMessage *message;  // message that has to be re-sent on timeout
    cPacket *packet;
    int sequenceNumber = 0;

  public:
    Tic();
    virtual ~Tic();

  protected:
    virtual cMessage *generateNewMessage();
    virtual cPacket *generateNewPacket();
    virtual void sendCopyOf(cPacket *pkt);
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Tic);

Tic::Tic()
{
    timeoutEvent = message = nullptr;
}

Tic::~Tic()
{
    cancelAndDelete(timeoutEvent);
    delete message;
}

void Tic::initialize()
{
    // Initialize variables.
    seq = 0;
    timeout = 1.0;
    timeoutEvent = new cMessage("timeoutEvent");

    // Generate and send initial message.
    EV << "Sending initial message\n";

    packet = generateNewPacket();
    sendCopyOf(packet);
    scheduleAt(simTime()+timeout, timeoutEvent);
}


void Tic::handleMessage(cMessage *msg)
{
    if (msg == timeoutEvent) {
        // If we receive the timeout event, that means the packet hasn't
        // arrived in time and we have to re-send it.
        EV << "Timeout expired, resending message and restarting timer\n";
        sendCopyOf(packet);
        scheduleAt(simTime()+timeout, timeoutEvent);
    }
    else {  // message arrived
            // Acknowledgement received!
        EV << "Received: " << msg->getName() << "\n";
        delete msg;

        // Also delete the stored message and cancel the timeout event.
        EV << "Timer cancelled.\n";
        cancelEvent(timeoutEvent);
        delete packet;

        // Ready to send another one.
        packet = generateNewPacket();
        sendCopyOf(packet);
        scheduleAt(simTime()+timeout, timeoutEvent);
    }
}

cPacket *Tic::generateNewPacket()
{
    // Generate a message with a different name every time.
    int length=9600;
    cPacket *pkt = new cPacket("prueba", 0, length);
    return pkt;
}

cMessage *Tic::generateNewMessage()
{
    // Generate a message with a different name every time.
    char msgname[20];
    sprintf(msgname, "tic-%d", ++seq);
    cMessage *msg = new cMessage(msgname);
    return msg;
}

/*void Tic::sendCopyOf(cMessage *msg)
{
    // Duplicate message and send the copy.
    cMessage *copy = (cMessage *)msg->dup();
    send(copy, "out");
}*/

void Tic::sendCopyOf(cPacket *pkt)
{
    // Duplicate message and send the copy.
    cPacket *copy = (cPacket *)pkt->dup();
    send(copy, "out");
}

/**
 * Sends back an acknowledgement -- or not.
 */

class Toc: public cSimpleModule
{
  protected:
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Toc);

void Toc::handleMessage(cMessage *msg)
{
    cPacket *pkt=check_and_cast<cPacket *>(msg);

    if (uniform(0, 1) < 0.5) { //Se ha subido la probabilidad para poder ver fallos
        EV << "\"Losing\" message " << msg << endl;
        bubble("message lost");
        delete msg;
    }
    else {
        EV << msg << " received, sending back an acknowledgement.\n";
        delete msg;
        bubble("message received");
        send(new cMessage("ack"), "out");
    }
}

