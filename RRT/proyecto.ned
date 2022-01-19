simple nodoCentral
{
    parameters:
        @display("i=block/departure");
        double probability;
    gates:
        input inPort[];
        output outPort[];
}

simple nodoFuente
{
    parameters:
        @display("i=block/source");
        double meanPacketLength;
    gates:
        output outPort;
}

simple nodoFinal
{
    parameters:
        @display("i=block/arrival");
        
    gates:
        input inPort[];
        output outPort[];
}

network Proyecto
{
        
    @display("bgb=541,344");
    types:
        channel Channel extends ned.DatarateChannel
        {
            datarate = 9600bps;
            delay = 100us;
            per = 0.25;
        }
    submodules:
        nodo1: nodoCentral {
            @display("p=148,149");
            probability = 1/4;
            gates:
                inPort[3];
                outPort[2];
        }
        nodo2: nodoCentral {
            @display("p=226,104");
            probability = 1/3;
            gates:
                inPort[5];
                outPort[4];
        }
        nodo3: nodoFinal {
            @display("p=403,104");
            gates:
                inPort[1];
                outPort[1];
        }
        nodo4: nodoFinal {
            @display("p=390,228");
            gates:
                inPort[2];
                outPort[2];
        }
        nodo5: nodoCentral {
            @display("p=226,228");
            probability = 1/2;
            gates:
                inPort[4];
                outPort[3];
        }
        nodoFuente1: nodoFuente {
            @display("p=62,149");
            meanPacketLength = 1000/3;
        }
        nodoFuente2: nodoFuente {
            @display("p=226,23");
            meanPacketLength = 1000/3;
        }
        nodoFuente3: nodoFuente {
            @display("p=226,301");
            meanPacketLength = 1000/3;
        }
    connections:
        nodo1.outPort[0] --> Channel --> nodo2.inPort[2];
        nodo1.inPort[0] <-- Channel <-- nodo2.outPort[2];

        nodo1.outPort[1] --> Channel --> nodo5.inPort[2];
        nodo1.inPort[1] <-- Channel <-- nodo5.outPort[2];

        nodo2.outPort[0] --> Channel --> nodo3.inPort[0];
        nodo2.inPort[0] <-- Channel <-- nodo3.outPort[0];

        nodo2.outPort[1] --> Channel --> nodo4.inPort[0];
        nodo2.inPort[1] <-- Channel <-- nodo4.outPort[0];

        nodo5.outPort[0] --> Channel --> nodo2.inPort[3];
        nodo5.inPort[0] <-- Channel <-- nodo2.outPort[3];

        nodo5.outPort[1] --> Channel --> nodo4.inPort[1];
        nodo5.inPort[1] <-- Channel <-- nodo4.outPort[1];

        nodoFuente1.outPort --> nodo1.inPort[2];
        nodoFuente2.outPort --> nodo2.inPort[4];
        nodoFuente3.outPort --> nodo5.inPort[3];
}
