//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2003-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//




import ned.DatarateChannel;


simple ticIrati
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
        output out;
}

simple tocIrati
{
    parameters:
        @display("i=block/process");
    gates:
        input in;
        output out;
}

channel C extends ned.DatarateChannel {
    // parameters of this channel, for example:
    datarate = 1Mbps;
    delay = 100ms;
    ber = 1e-10;
}

network tictocIrati
{
    submodules:
        tic: ticIrati {
            parameters:
                @display("i=,cyan");
        }
        toc: tocIrati {
            parameters:
                @display("i=,red");
        }
    connections:
        tic.out --> C --> toc.in;
        tic.in <-- C <-- toc.out;
}

